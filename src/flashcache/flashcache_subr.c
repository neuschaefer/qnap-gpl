/****************************************************************************
 *  flashcache_subr.c
 *  FlashCache: Device mapper target for block-level disk caching
 *
 *  Copyright 2010 Facebook, Inc.
 *  Author: Mohan Srinivasan (mohan@fb.com)
 *
 *  Based on DM-Cache:
 *   Copyright (C) International Business Machines Corp., 2006
 *   Author: Ming Zhao (mingzhao@ufl.edu)
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; under version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include <asm/atomic.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/slab.h>
#include <linux/hash.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/pagemap.h>
#include <linux/random.h>
#include <linux/hardirq.h>
#include <linux/sysctl.h>
#include <linux/version.h>
#include <linux/sort.h>
#include <linux/time.h>
#include <asm/kmap_types.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#include "dm.h"
#include "dm-io.h"
#include "dm-bio-list.h"
#include "kcopyd.h"
#else
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,27)
#include "dm.h"
#endif
#include <linux/device-mapper.h>
#include <linux/bio.h>
#include <linux/dm-kcopyd.h>
#endif
#include "flashcache.h"

static DEFINE_SPINLOCK(_job_lock);

extern mempool_t *_job_pool;
extern mempool_t *_pending_job_pool;

extern atomic_t nr_cache_jobs;
extern atomic_t nr_pending_jobs;

LIST_HEAD(_pending_jobs);
LIST_HEAD(_io_jobs);
LIST_HEAD(_uncached_io_complete_jobs);

int
flashcache_pending_empty(void)
{
	return list_empty(&_pending_jobs);
}

int
flashcache_io_empty(void)
{
	return list_empty(&_io_jobs);
}

int
flashcache_uncached_io_complete_empty(void)
{
	return list_empty(&_uncached_io_complete_jobs);
}

struct kcached_job *
flashcache_alloc_cache_job(void)
{
	struct kcached_job *job;

	job = mempool_alloc(_job_pool, GFP_NOIO);
	if (likely(job))
		atomic_inc(&nr_cache_jobs);
	return job;
}

void
flashcache_free_cache_job(struct kcached_job *job)
{
	mempool_free(job, _job_pool);
	atomic_dec(&nr_cache_jobs);
}

struct pending_job *
flashcache_alloc_pending_job(struct cache_c *dmc)
{
	struct pending_job *job;

	job = mempool_alloc(_pending_job_pool, GFP_ATOMIC);
	if (likely(job))
		atomic_inc(&nr_pending_jobs);
	else
		dmc->flashcache_errors.memory_alloc_errors++;
	return job;
}

void
flashcache_free_pending_job(struct pending_job *job)
{
	mempool_free(job, _pending_job_pool);
	atomic_dec(&nr_pending_jobs);
}

#define FLASHCACHE_PENDING_JOB_HASH(INDEX)		((INDEX) % PENDING_JOB_HASH_SIZE)

void 
flashcache_enq_pending(struct cache_c *dmc, struct bio* bio,
		       int index, int action, struct pending_job *job)
{
	struct pending_job **head;
	
	head = &dmc->pending_job_hashbuckets[FLASHCACHE_PENDING_JOB_HASH(index)];
	VERIFY(job != NULL);
	job->action = action;
	job->index = index;
	job->bio = bio;
	job->prev = NULL;
	job->next = *head;
	if (*head)
		(*head)->prev = job;
	*head = job;
	dmc->cache[index].nr_queued++;
	dmc->flashcache_stats.enqueues++;
	dmc->pending_jobs_count++;
}

/*
 * Deq and move all pending jobs that match the index for this slot to list returned
 */
struct pending_job *
flashcache_deq_pending(struct cache_c *dmc, int index)
{
	struct pending_job *node, *next, *movelist = NULL;
	int moved = 0;
	struct pending_job **head;
	
	VERIFY(spin_is_locked(&dmc->cache_spin_lock));
	head = &dmc->pending_job_hashbuckets[FLASHCACHE_PENDING_JOB_HASH(index)];
	NPRINTK("flashcache_deq_pending");
	for (node = *head ; node != NULL ; node = next) {
		next = node->next;
		if (node->index == index) {
			/* 
			 * Remove pending job from the global list of 
			 * jobs and move it to the private list for freeing 
			 */
			if (node->prev == NULL) {
				*head = node->next;
				if (node->next)
					node->next->prev = NULL;
			} else
				node->prev->next = node->next;
			if (node->next == NULL) {
				if (node->prev)
					node->prev->next = NULL;
			} else
				node->next->prev = node->prev;
			node->prev = NULL;
			node->next = movelist;
			movelist = node;
			moved++;
		}
	}
	VERIFY(dmc->pending_jobs_count >= moved);
	dmc->pending_jobs_count -= moved;
	return movelist;
}

#ifdef FLASHCACHE_DO_CHECKSUMS
int
flashcache_read_compute_checksum(struct cache_c *dmc, int index, void *block)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	struct io_region where;
#else
	struct dm_io_region where;
#endif
	int error;
	u_int64_t sum = 0, *idx;
	int cnt;

	where.bdev = dmc->cache_dev->bdev;
	where.sector = INDEX_TO_CACHE_ADDR(dmc, index);
	where.count = dmc->block_size;
	error = flashcache_dm_io_sync_vm(dmc, &where, READ, block);
	if (error)
		return error;
	cnt = dmc->block_size * 512;
	idx = (u_int64_t *)block;
	while (cnt > 0) {
		sum += *idx++;
		cnt -= sizeof(u_int64_t);		
	}
	dmc->cache[index].checksum = sum;
	return 0;
}

u_int64_t
flashcache_compute_checksum(struct bio *bio)
{
	int i;	
	u_int64_t sum = 0, *idx;
	int cnt;
	int kmap_type;
	void *kvaddr;

	if (in_interrupt())
		kmap_type = KM_SOFTIRQ0;
	else
		kmap_type = KM_USER0;
	for (i = bio->bi_idx ; i < bio->bi_vcnt ; i++) {
		kvaddr = kmap_atomic(bio->bi_io_vec[i].bv_page, kmap_type);
		idx = (u_int64_t *)
			((char *)kvaddr + bio->bi_io_vec[i].bv_offset);
		cnt = bio->bi_io_vec[i].bv_len;
		while (cnt > 0) {
			sum += *idx++;
			cnt -= sizeof(u_int64_t);
		}
		kunmap_atomic(kvaddr, kmap_type);
	}
	return sum;
}

void
flashcache_store_checksum(struct kcached_job *job)
{
	u_int64_t sum;
	unsigned long flags;
	
	sum = flashcache_compute_checksum(job->bio);
	spin_lock_irqsave(&job->dmc->cache_spin_lock, flags);
	job->dmc->cache[job->index].checksum = sum;
	spin_unlock_irqrestore(&job->dmc->cache_spin_lock, flags);
}

int
flashcache_validate_checksum(struct kcached_job *job)
{
	u_int64_t sum;
	int retval;
	unsigned long flags;
	
	sum = flashcache_compute_checksum(job->bio);
	spin_lock_irqsave(&job->dmc->cache_spin_lock, flags);
	if (likely(job->dmc->cache[job->index].checksum == sum)) {
		job->dmc->flashcache_stats.checksum_valid++;		
		retval = 0;
	} else {
		job->dmc->flashcache_stats.checksum_invalid++;
		retval = 1;
	}
	spin_unlock_irqrestore(&job->dmc->cache_spin_lock, flags);
	return retval;
}
#endif

/*
 * Functions to push and pop a job onto the head of a given job list.
 */
struct kcached_job *
pop(struct list_head *jobs)
{
	struct kcached_job *job = NULL;
	unsigned long flags;

	LOCK_START
	spin_lock_irqsave(&_job_lock, flags);
	if (!list_empty(jobs)) {
		job = list_entry(jobs->next, struct kcached_job, list);
		list_del(&job->list);
	}
	spin_unlock_irqrestore(&_job_lock, flags);
	LOCK_END
	return job;
}

void 
push(struct list_head *jobs, struct kcached_job *job)
{
	unsigned long flags;

	LOCK_START
	spin_lock_irqsave(&_job_lock, flags);
	list_add_tail(&job->list, jobs);
	spin_unlock_irqrestore(&_job_lock, flags);
	LOCK_END
}

void
push_pending(struct kcached_job *job)
{
	push(&_pending_jobs, job);	
}

void
push_io(struct kcached_job *job)
{
	push(&_io_jobs, job);	
}

void
push_uncached_io_complete(struct kcached_job *job)
{
	push(&_uncached_io_complete_jobs, job);	
}

static void
process_jobs(struct list_head *jobs,
	     void (*fn) (struct kcached_job *))
{
	struct kcached_job *job;

	while ((job = pop(jobs)))
		(void)fn(job);
}

void 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
do_work(void *unused)
#else
do_work(struct work_struct *unused)
#endif
{
	process_jobs(&_pending_jobs, flashcache_do_pending);
	process_jobs(&_io_jobs, flashcache_do_io);
	process_jobs(&_uncached_io_complete_jobs, flashcache_uncached_io_complete);
}

void 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
do_error(void *unused)
#else
do_error(struct work_struct *work)
#endif
{
	struct cache_c *dmc = container_of(work, struct cache_c, kerror_wq);
	DMERR("too much SSD IO error; force to plugout SSD!!!");
	flashcache_plugout(dmc);
}
#if 0

struct kcached_job *
new_kcached_job(struct disk_slot *slot, struct bio* bio, int index)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	struct kcached_job *job;

	job = flashcache_alloc_cache_job();
	if (unlikely(job == NULL)) {
		dmc->flashcache_errors.memory_alloc_errors++;
		return NULL;
	}
	job->slot = slot;
	job->index = index;
	if (dmc->cache_dev)
		job->job_io_regions.cache.bdev = dmc->cache_dev->bdev;
	if (index != -1) {
		job->job_io_regions.cache.sector = INDEX_TO_CACHE_ADDR(dmc, index);
		job->job_io_regions.cache.count = dmc->block_size;	
	}
	job->error = 0;	
	job->bio = bio;
	job->job_io_regions.disk.bdev = slot->disk_dev->bdev;
	if (index != -1) {
		job->job_io_regions.disk.sector = dmc->cache[index].dbn;
		job->job_io_regions.disk.count = dmc->block_size;
	} else {
		job->job_io_regions.disk.sector = bio->bi_sector;
		job->job_io_regions.disk.count = to_sector(bio->bi_size);
	}
	job->next = NULL;
//	DPRINTK("new_kcached_job : index = %d: Block %llu(%u)", index, job->job_io_regions.cache.sector, job->job_io_regions.cache.count);
	if (dmc->sysctl_io_latency_hist)
		do_gettimeofday(&job->io_start_time);
	else {
		job->io_start_time.tv_sec = 0;
		job->io_start_time.tv_usec = 0;
	}
	return job;
}
#endif

struct kcached_job *
new_kcached_job(struct disk_slot *slot, struct bio* bio, int index)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	struct kcached_job *job;

	job = flashcache_alloc_cache_job();
	if (unlikely(job == NULL)) {
		dmc->flashcache_errors.memory_alloc_errors++;
		return NULL;
	}
	job->slot = slot;
	job->index = index;
	if (dmc->cache_dev)
		job->job_io_regions.cache.bdev = dmc->cache_dev->bdev;
	if (index != -1) {
		job->job_io_regions.cache.sector = INDEX_TO_CACHE_ADDR(dmc, index) + flashcache_bio_start_sector(bio, (&dmc->cache[index]));
		job->job_io_regions.cache.count = to_sector(bio->bi_size);
	}
	job->error = 0;	
	job->bio = bio;
	job->job_io_regions.disk.bdev = slot->disk_dev->bdev;
	job->job_io_regions.disk.sector = bio->bi_sector;
	job->job_io_regions.disk.count = to_sector(bio->bi_size);
	job->next = NULL;
	if (dmc->sysctl_io_latency_hist)
		do_gettimeofday(&job->io_start_time);
	else {
		job->io_start_time.tv_sec = 0;
		job->io_start_time.tv_usec = 0;
	}
	return job;
}

static void
flashcache_record_latency(struct cache_c *dmc, struct timeval *start_tv)
{
	struct timeval latency;
	int64_t us;
	
	do_gettimeofday(&latency);
	latency.tv_sec -= start_tv->tv_sec;
	latency.tv_usec -= start_tv->tv_usec;	
	us = latency.tv_sec * USEC_PER_SEC + latency.tv_usec;
	us /= IO_LATENCY_GRAN_USECS;	/* histogram 250us gran, scale 10ms total */
	if (us < IO_LATENCY_BUCKETS)
		/* < 10ms latency, track it */
		dmc->latency_hist[us]++;
	else
		/* else count it in 10ms+ bucket */
		dmc->latency_hist_10ms++;
}

void
flashcache_bio_endio(struct bio *bio, int error, 
		     struct cache_c *dmc, struct timeval *start_time)
{
	if (unlikely(dmc->sysctl_io_latency_hist && 
		     start_time != NULL && 
		     start_time->tv_sec != 0))
		flashcache_record_latency(dmc, start_time);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
	bio_endio(bio, bio->bi_size, error);
#else
	bio_endio(bio, error);
#endif	
}

void
flashcache_reclaim_lru_movetail(struct cache_c *dmc, int index)
{
	int set = index / dmc->assoc;
	int start_index = set * dmc->assoc;
	int my_index = index - start_index;
	struct cacheblock *cacheblk = &dmc->cache[index];

	/* Remove from LRU */
	if (likely((cacheblk->lru_prev != FLASHCACHE_LRU_NULL) ||
		   (cacheblk->lru_next != FLASHCACHE_LRU_NULL))) {
		if (cacheblk->lru_prev != FLASHCACHE_LRU_NULL)
			dmc->cache[cacheblk->lru_prev + start_index].lru_next = 
				cacheblk->lru_next;
		else
			dmc->cache_sets[set].lru_head = cacheblk->lru_next;
		if (cacheblk->lru_next != FLASHCACHE_LRU_NULL)
			dmc->cache[cacheblk->lru_next + start_index].lru_prev = 
				cacheblk->lru_prev;
		else
			dmc->cache_sets[set].lru_tail = cacheblk->lru_prev;
	}
	/* And add it to LRU Tail */
	cacheblk->lru_next = FLASHCACHE_LRU_NULL;
	cacheblk->lru_prev = dmc->cache_sets[set].lru_tail;
	if (dmc->cache_sets[set].lru_tail == FLASHCACHE_LRU_NULL)
		dmc->cache_sets[set].lru_head = my_index;
	else
		dmc->cache[dmc->cache_sets[set].lru_tail + start_index].lru_next = 
			my_index;
	dmc->cache_sets[set].lru_tail = my_index;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
extern struct dm_io_client *flashcache_io_client; /* Client memory pool*/
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
int 
flashcache_dm_io_async_vm(struct cache_c *dmc, unsigned int num_regions, 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
			  struct io_region *where, 
#else
			  struct dm_io_region *where, 
#endif
			  int rw,
			  void *data, io_notify_fn fn, void *context)
{
	unsigned long error_bits = 0;
	int error;
	struct dm_io_request io_req = {
		.bi_rw = rw,
		.mem.type = DM_IO_VMA,
		.mem.ptr.vma = data,
		.mem.offset = 0,
		.notify.fn = fn,
		.notify.context = context,
		.client = flashcache_io_client,
	};

	error = dm_io(&io_req, 1, where, &error_bits);
	if (error)
		return error;
	if (error_bits)
		return error_bits;
	return 0;
}
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
/*
 * Wrappers for doing DM sync IO, using DM async IO.
 * It is a shame we need do this, but DM sync IO is interruptible :(
 * And we want uninterruptible disk IO :)
 * 
 * This is fixed in 2.6.30, where sync DM IO is uninterruptible.
 */
#define FLASHCACHE_DM_IO_SYNC_INPROG	0x01

static DECLARE_WAIT_QUEUE_HEAD(flashcache_dm_io_sync_waitqueue);
static DEFINE_SPINLOCK(flashcache_dm_io_sync_spinlock);

struct flashcache_dm_io_sync_state {
	int			error;
	int			flags;
};

static void
flashcache_dm_io_sync_vm_callback(unsigned long error, void *context)
{
	struct flashcache_dm_io_sync_state *state = 
		(struct flashcache_dm_io_sync_state *)context;
	unsigned long flags;
	
	spin_lock_irqsave(&flashcache_dm_io_sync_spinlock, flags);
	state->flags &= ~FLASHCACHE_DM_IO_SYNC_INPROG;
	state->error = error;
	wake_up(&flashcache_dm_io_sync_waitqueue);
	spin_unlock_irqrestore(&flashcache_dm_io_sync_spinlock, flags);
}

int
flashcache_dm_io_sync_vm(struct cache_c *dmc, 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
			 struct io_region *where, 
#else
			  struct dm_io_region *where, 
#endif
			 int rw, void *data)
{
        DEFINE_WAIT(wait);
	struct flashcache_dm_io_sync_state state;

	state.error = -EINTR;
	state.flags = FLASHCACHE_DM_IO_SYNC_INPROG;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
	dm_io_async_vm(1, where, rw, data, flashcache_dm_io_sync_vm_callback, &state);
#else
	flashcache_dm_io_async_vm(dmc, 1, where, rw, data, flashcache_dm_io_sync_vm_callback, &state);
#endif
	spin_lock_irq(&flashcache_dm_io_sync_spinlock);
	while (state.flags & FLASHCACHE_DM_IO_SYNC_INPROG) {
		prepare_to_wait(&flashcache_dm_io_sync_waitqueue, &wait, 
				TASK_UNINTERRUPTIBLE);
		spin_unlock_irq(&flashcache_dm_io_sync_spinlock);
		schedule();
		spin_lock_irq(&flashcache_dm_io_sync_spinlock);
	}
	finish_wait(&flashcache_dm_io_sync_waitqueue, &wait);
	spin_unlock_irq(&flashcache_dm_io_sync_spinlock);
	return state.error;
}
#else
int
flashcache_dm_io_sync_vm(struct cache_c *dmc, struct dm_io_region *where, int rw, void *data)
{
	unsigned long error_bits = 0;
	int error;
	struct dm_io_request io_req = {
		.bi_rw = rw,
		.mem.type = DM_IO_VMA,
		.mem.ptr.vma = data,
		.mem.offset = 0,
		.notify.fn = NULL,
		.client = flashcache_io_client,
	};

	error = dm_io(&io_req, 1, where, &error_bits);
	if (error)
		return error;
	if (error_bits)
		return error_bits;
	return 0;
}
#endif

EXPORT_SYMBOL(flashcache_alloc_cache_job);
EXPORT_SYMBOL(flashcache_free_cache_job);
EXPORT_SYMBOL(flashcache_alloc_pending_job);
EXPORT_SYMBOL(flashcache_free_pending_job);
EXPORT_SYMBOL(pop);
EXPORT_SYMBOL(push);
EXPORT_SYMBOL(push_pending);
EXPORT_SYMBOL(push_io);
EXPORT_SYMBOL(process_jobs);
EXPORT_SYMBOL(do_work);
EXPORT_SYMBOL(new_kcached_job);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
EXPORT_SYMBOL(flashcache_dm_io_sync_vm_callback);
#endif
EXPORT_SYMBOL(flashcache_dm_io_sync_vm);
EXPORT_SYMBOL(flashcache_reclaim_lru_movetail);
EXPORT_SYMBOL(flashcache_enq_pending);
