/****************************************************************************
 *  flashcache_main.c
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
#include <linux/pid.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
#include <linux/device-mapper.h>
#include <linux/bio.h>
#endif
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
#include "flashcache_ioctl.h"

#ifndef DM_MAPIO_SUBMITTED
#define DM_MAPIO_SUBMITTED	0
#endif

/*
 * TODO List :
 * 1) Management of non cache pids : Needs improvement. Remove registration
 * on process exits (with  a pseudo filesstem'ish approach perhaps) ?
 * 2) Breaking up the cache spinlock : Right now contention on the spinlock
 * is not a problem. Might need change in future.
 * 3) Use the standard linked list manipulation macros instead rolling our own.
 * 4) Fix a security hole : A malicious process with 'ro' access to a file can 
 * potentially corrupt file data. This can be fixed by copying the data on a
 * cache read miss.
 */

#define FLASHCACHE_SW_VERSION "flashcache-1.1"
char *flashcache_sw_version = FLASHCACHE_SW_VERSION;

static void flashcache_read_miss(int partial, struct disk_slot *slot, struct bio* bio,
				 int index);
static void flashcache_write(struct disk_slot *slot, struct bio* bio);
static int flashcache_inval_blocks(struct disk_slot *slot, struct bio *bio);
static int flashcache_discard_blocks(struct disk_slot *slot, struct bio *bio);
static void flashcache_start_uncached_io(struct disk_slot *slot, struct bio *bio);
static void flashcache_start_discard_io(struct disk_slot *slot, struct bio *bio);
extern mempool_t *_job_pool;
extern struct work_struct _kcached_wq;
extern u_int64_t size_hist[];

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
extern struct dm_kcopyd_client *flashcache_kcp_client; /* Kcopyd client for writing back data */
#else
extern struct kcopyd_client *flashcache_kcp_client; /* Kcopyd client for writing back data */
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
extern struct dm_io_client *flashcache_io_client; /* Client memory pool*/
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
int dm_io_async_bvec(unsigned int num_regions, 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
			    struct dm_io_region *where, 
#else
			    struct io_region *where, 
#endif
			    int rw, 
			    struct bio_vec *bvec, io_notify_fn fn, 
			    void *context)
{
	struct dm_io_request iorq;

	iorq.bi_rw = rw;
	iorq.mem.type = DM_IO_BVEC;
	iorq.mem.ptr.bvec = bvec;
	iorq.notify.fn = fn;
	iorq.notify.context = context;
	iorq.client = flashcache_io_client;
	return dm_io(&iorq, num_regions, where, NULL);
}
#endif

#ifdef PARTIAL_BITMAP
#if 0
char* printk_binary(u_int64_t val, char* buf)
{
        int i, j=0;
        u_int64_t tmp;
        int byte;
	int bit_shift;
#ifdef PARTIAL_4K
        byte = 2;
#else
        byte = 8;
#endif
        bit_shift = byte*8-1;

        for (i=bit_shift; i>=0; i--) {
                tmp = (u_int64_t)1<<i;
                buf[j++] = (val&tmp)?'1':'0';
        }
        buf[j] = '\0';
        return buf;
}

void printk_binary_array(u_int64_t *bitmap)
{
        char tmp[72];
        int i;
        for (i=BITMAP_COUNT-1; i>=0; i--) {
                printk("bitmap[%2d] %s\n", i, printk_binary(bitmap[i], tmp));
        }
}
#endif

/*
 * convert io range to 16 bits bitmap, one bit represent 2^BLOCK_SHIFT4K sector
 */

u_int16_t to_bitmap16(int offset, int size)
{
        int start, end;
        int bit_start, bit_end;
        u_int16_t bitmap;

        start = offset;
        end = start + size;

        bit_start = (start>>BLOCK_SHIFT);
        bit_end = (end>>BLOCK_SHIFT);

        if (bit_end == BITMAP_SIZE)
                bitmap = (u_int16_t)(~0)<<(bit_start) & BLOCK_MASK;
        else
                bitmap = (u_int16_t)(~0)<<(bit_start) & (~((u_int16_t)(~0)<<bit_end));

        bitmap = (u_int16_t)BLOCK_MASK<<(bit_start) & (u_int16_t)BLOCK_MASK>>(BITMAP_SIZE-bit_end);
        return bitmap;
}

/* offset and size are even */
u_int64_t to_bitmap64(int start, int size)
{
        int bit_start, bit_end;
        u_int64_t bitmap;
        int end = start + size;

        bit_start = (start>>BLOCK_SHIFT);
        bit_end = (end>>BLOCK_SHIFT);

        bitmap = (u_int64_t)BLOCK_MASK<<(bit_start) & (u_int64_t)BLOCK_MASK>>(BITMAP_SIZE-bit_end);
        return bitmap;
}

#ifdef BITMAP_ARRAY
void to_bitmap64_array(u_int64_t *bitmap, int offset, int size)
{
        int i;
        int tmpoffset, tmpsize;

	memset(bitmap, 0, sizeof(u_int64_t)*BITMAP_COUNT);

        for (i=0; i<BITMAP_COUNT; i++) {
                if (offset < i*SECTOR_COUNT) {
                        break;
                }
        }
        i--;
        offset -= i*SECTOR_COUNT;

        for (; i<BITMAP_COUNT; i++) {
                tmpoffset = offset;
                if (tmpoffset+size > SECTOR_COUNT) {
                        tmpsize = SECTOR_COUNT-tmpoffset;
                } else {
                        tmpsize = size;
                }
                bitmap[i] = TO_BITMAP(tmpoffset, tmpsize);

                offset = SECTOR_COUNT-(tmpoffset+tmpsize);
                size -= tmpsize;
                if (size <= 0)
                        break;
        }
}

int bitmap64_array_contain(u_int64_t *source, u_int64_t *filter)
{
        int i=0;
        for (; i<BITMAP_COUNT; i++) {
                if (!BITMAP_CONTAIN(source[i], filter[i])) {
			return 0;
                }
        }

        return 1;
}

int bitmap64_array_notcontain(u_int64_t *source, u_int64_t *filter)
{
        int i=0;
        for (; i<BITMAP_COUNT; i++) {
                if (!BITMAP_NOTCONTAIN(source[i], filter[i])) {
                        return 0;
                }
        }

        return 1;
}

void bitmap64_array_add(u_int64_t *dest, u_int64_t *source, u_int64_t *result)
{
        int i=0;
        for (; i<BITMAP_COUNT; i++) {
                result[i] = BITMAP_ADD(source[i], dest[i]);
        }
}

void bitmap64_array_copy(u_int64_t *dest, u_int64_t *source)
{
	memcpy(dest, source, sizeof(u_int64_t)*BITMAP_COUNT);
}

void bitmap64_array_set_mask(u_int64_t *source)
{
	memset(source, 0xFF, sizeof(u_int64_t)*BITMAP_COUNT);
}

int bitmap64_array_check_mask(u_int64_t *source)
{
        int i=0;
        for (; i<BITMAP_COUNT; i++) {
                if (source[i] != BLOCK_MASK)
			return 0;
        }

	return 1;
}
#endif /* end of BITMAP_ARRAY define */
#endif  /* end of PARTIAL_BITMAP define */

void 
flashcache_io_callback(unsigned long error, void *context)
{
	struct kcached_job *job = (struct kcached_job *) context;
	struct cache_c *dmc = job->slot->dmc;
	struct bio *bio;
	unsigned long flags;
	unsigned long error_tmp;
	int index = job->index;
	struct cacheblock *cacheblk = &dmc->cache[index];

	VERIFY(index != -1);		
	bio = job->bio;
	VERIFY(bio != NULL);
	if (unlikely(error)) {
		error_tmp = error;
		error = -EIO;
//		DMERR("flashcache_io_callback: io error %ld block %lu action %d", 
//		      error, job->job_io_regions.disk.sector, job->action);
	}
	job->error = error;

	switch (job->action) {
	case READDISK_UNCACHED:
		JPRINTK("flashcache_io_callback: READDISK_UNCACHED %d",
			index);
		if (unlikely(dmc->sysctl_error_inject & READDISK_ERROR)) {
			job->error = error = -EIO;
			dmc->sysctl_error_inject &= ~READDISK_ERROR;
		}
		/* no need to set IN_ERROR even if error
		 * because IN_ERROR is already set in READCACHE error condition
		 */ 
		if (unlikely(error))
			dmc->flashcache_errors.disk_read_errors++;			
		break;
	case READDISK:
		JPRINTK("flashcache_io_callback: READDISK %d",
			index);
		if (unlikely(dmc->sysctl_error_inject & READDISK_ERROR)) {
			job->error = error = -EIO;
			dmc->sysctl_error_inject &= ~READDISK_ERROR;
		}
		if (unlikely(error)) {
			LOCK_START
			spin_lock_irqsave(&dmc->cache_spin_lock, flags);
			cacheblk->cache_state |= IN_ERROR;
			spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
			LOCK_END
		}
		
		if (likely(error == 0)) {
			/* Kick off the write to the cache */
			job->action = READFILL;
			push_io(job);
			schedule_work(&_kcached_wq);
			return;
		} else
			dmc->flashcache_errors.disk_read_errors++;			
		break;
	case READCACHE:
		JPRINTK("flashcache_io_callback: READCACHE %d",
			index);
		if (unlikely(dmc->sysctl_error_inject & READCACHE_ERROR)) {
			job->error = error = -EIO;
			dmc->sysctl_error_inject &= ~READCACHE_ERROR;
		}
		if (unlikely(error)) {
			LOCK_START
			spin_lock_irqsave(&dmc->cache_spin_lock, flags);
			cacheblk->cache_state |= IN_ERROR;
			spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
			LOCK_END

			dmc->flashcache_errors.ssd_errors++;
			dmc->flashcache_errors.ssd_read_errors++;
			if (!dmc->ssd_forced_plugout && (dmc->sysctl_auto_plugout_error_thresh > 0) && (dmc->flashcache_errors.ssd_errors >= dmc->sysctl_auto_plugout_error_thresh)) {
				dmc->ssd_forced_plugout = 1;
				schedule_work(&dmc->kerror_wq);
			}

			/* ssd read is failed, retry to get data from source device */
			/* Kick off the write to the cache */
			job->action = READDISK_UNCACHED;
			push_io(job);
			schedule_work(&_kcached_wq);
			return;
		}
#ifdef FLASHCACHE_DO_CHECKSUMS
		if (likely(error == 0)) {
			if (flashcache_validate_checksum(job)) {
				DMERR("flashcache_io_callback: Checksum mismatch at disk offset %lu", 
				      job->job_io_regions.disk.sector);
				error = -EIO;
			}
		}
#endif
		break;		       
	case READFILL:
		JPRINTK("flashcache_io_callback: READFILL %d",
			index);
		if (unlikely(dmc->sysctl_error_inject & READFILL_ERROR)) {
			job->error = error = -EIO;
			dmc->sysctl_error_inject &= ~READFILL_ERROR;
		}
		if (unlikely(error)) {
			LOCK_START
			spin_lock_irqsave(&dmc->cache_spin_lock, flags);
			cacheblk->cache_state |= IN_ERROR;
			spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
			LOCK_END

			error = 0;	/* disk read is successfully, end io with success and invalidate current cache block */
			dmc->flashcache_errors.ssd_errors++;
			dmc->flashcache_errors.ssd_write_errors++;
			if (!dmc->ssd_forced_plugout && (dmc->sysctl_auto_plugout_error_thresh > 0) && (dmc->flashcache_errors.ssd_errors >= dmc->sysctl_auto_plugout_error_thresh)) {
				dmc->ssd_forced_plugout = 1;
				schedule_work(&dmc->kerror_wq);
			}
		}
		break;
	case WRITECACHE:
		JPRINTK("flashcache_io_callback: WRITECACHE %d",
			index);
		if (unlikely(dmc->sysctl_error_inject & WRITECACHE_ERROR)) {
			job->error = error = -EIO;
			dmc->sysctl_error_inject &= ~WRITECACHE_ERROR;
		}
		if (unlikely(error)) {
			LOCK_START
			spin_lock_irqsave(&dmc->cache_spin_lock, flags);
			cacheblk->cache_state |= IN_ERROR;
			spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
			LOCK_END
		}
		if (likely(error == 0)) {
			{ /* cache_mode == WRITE_THROUGH */
				/* Writs to both disk and cache completed */
				VERIFY(dmc->cache_mode == FLASHCACHE_WRITE_THROUGH);
#ifdef FLASHCACHE_DO_CHECKSUMS
				flashcache_store_checksum(job);
				job->dmc->flashcache_stats.checksum_store++;
#endif
			}
		} else {
			/* disk write no error */
			if (!test_bit(0, &error_tmp)) {
				error = 0;	/* disk write is successfully, end io with success and invalidate current cache block */
				dmc->flashcache_errors.ssd_errors++;
				dmc->flashcache_errors.ssd_write_errors++;
				if (!dmc->ssd_forced_plugout && (dmc->sysctl_auto_plugout_error_thresh > 0) && (dmc->flashcache_errors.ssd_errors >= dmc->sysctl_auto_plugout_error_thresh)) {
					dmc->ssd_forced_plugout = 1;
					schedule_work(&dmc->kerror_wq);
				}
			}
			if (dmc->cache_mode == FLASHCACHE_WRITE_THROUGH)
				/* 
				 * We don't know if the IO failed because of a ssd write
				 * error or a disk write error. Bump up both.
				 * XXX - TO DO. We could check the error bits and allow
				 * the IO to succeed as long as the disk write suceeded.
				 * and invalidate the cache block.
				 */
				dmc->flashcache_errors.disk_write_errors++;
		}
		break;
	}
	flashcache_bio_endio(bio, error, dmc, &job->io_start_time);
	/* 
	 * The INPROG flag is still set. We cannot turn that off until all the pending requests
	 * processed. We need to loop the pending requests back to a workqueue. We have the job,
	 * add it to the pending req queue.
	 */
	LOCK_START
	spin_lock_irqsave(&dmc->cache_spin_lock, flags);
//	VERIFY(cacheblk->nr_concurrent >= 0);
	if (cacheblk->nr_concurrent < 0)
		dmc->flashcache_errors.verify_errors1++;
	cacheblk->nr_concurrent--;  /* dec nr_concurrent and check nr_concurrent in the same spin_lock scope */

	if (unlikely(cacheblk->nr_concurrent==0) && ((cacheblk->cache_state & IN_ERROR) || (cacheblk->nr_queued > 0))) {
		if (cacheblk->cache_state & IN_ERROR) {
			job->error = -EIO;
		}
		spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
		LOCK_END
		push_pending(job);
		schedule_work(&_kcached_wq);
	} else {
		if (!cacheblk->nr_concurrent)
			cacheblk->cache_state &= ~BLOCK_IO_INPROG;
		spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
		LOCK_END
		flashcache_free_cache_job(job);
		if (atomic_dec_and_test(&dmc->nr_jobs))
			wake_up(&dmc->destroyq);
	}
}

static void
flashcache_free_pending_jobs(struct cache_c *dmc, struct cacheblock *cacheblk, 
			     int error)
{
	struct pending_job *pending_job, *freelist = NULL;

	VERIFY(spin_is_locked(&dmc->cache_spin_lock));
	freelist = flashcache_deq_pending(dmc, cacheblk - &dmc->cache[0]);
	while (freelist != NULL) {
		pending_job = freelist;
		freelist = pending_job->next;
		VERIFY(cacheblk->nr_queued > 0);
		cacheblk->nr_queued--;
		flashcache_bio_endio(pending_job->bio, error, dmc, NULL);
		flashcache_free_pending_job(pending_job);
	}
	VERIFY(cacheblk->nr_queued == 0);
}

/* 
 * Common error handling for everything.
 * 1) If the block isn't dirty, invalidate it.
 * 2) Error all pending IOs that totally or partly overlap this block.
 * 3) Free the job.
 */
static void
flashcache_do_pending_error(struct kcached_job *job)
{
	struct disk_slot *slot = job->slot;
	struct cache_c *dmc = slot->dmc;
	int index = job->index;
	unsigned long flags;
	struct pending_job *pending_job, *freelist;
	int queued;
	struct cacheblock *cacheblk = &dmc->cache[job->index];

//	DMERR("flashcache_do_pending_error: error %d block %lu action %d", 
//	      job->error, job->job_io_regions.disk.sector, job->action);
	LOCK_START
	spin_lock_irqsave(&dmc->cache_spin_lock, flags);
//	VERIFY(cacheblk->cache_state & VALID);
	if ((cacheblk->cache_state & VALID) == 0)
		dmc->flashcache_errors.verify_errors2++;
	/* Invalidate block if possible */
	/* it will not be DIRTY */
	{
		dmc->cached_blocks--;
		dmc->flashcache_stats.pending_inval++;
		slot->cached_blocks--;
		slot->flashcache_stats.pending_inval++;
		cacheblk->cache_state &= ~(VALID | PVALID);
		cacheblk->cache_state |= INVALID;
	}
	/* do uncached io if have queued jobs, not just bio_end with error */
	if (cacheblk->nr_queued) {
		while ((freelist = flashcache_deq_pending(dmc, index)) != NULL) {
			while (freelist != NULL) {
				pending_job = freelist;
				freelist = pending_job->next;
				VERIFY(cacheblk->nr_queued > 0);
				cacheblk->nr_queued--;
				if (pending_job->action == INVALIDATE) {
					DPRINTK("flashcache_do_pending: INVALIDATE index %d %llu",
						pending_job->index, pending_job->bio->bi_sector);
					VERIFY(pending_job->bio != NULL);
					queued = flashcache_inval_blocks(slot, pending_job->bio);
					if (queued) {
						if (unlikely(queued < 0)) {
							/*
						 	* Memory allocation failure inside inval_blocks.
						 	* Fail this io.
						 	*/
							flashcache_bio_endio(pending_job->bio, -EIO, dmc, NULL);
						}
						flashcache_free_pending_job(pending_job);
						continue;
					}
				}
				spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
				LOCK_END
				DPRINTK("flashcache_do_pending: Sending down IO index %d %llu",
					pending_job->index, pending_job->bio->bi_sector);
				/* Start uncached IO */
				flashcache_start_uncached_io(slot, pending_job->bio);
				flashcache_free_pending_job(pending_job);
				LOCK_START
				spin_lock_irqsave(&dmc->cache_spin_lock, flags);
			}
		}
		if (cacheblk->nr_queued != 0)
			dmc->flashcache_errors.verify_errors4++;
	}

//	flashcache_free_pending_jobs(dmc, cacheblk, job->error);
	cacheblk->cache_state &= ~(BLOCK_IO_INPROG);
	cacheblk->cache_state &= ~IN_ERROR;
	spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
	LOCK_END
	flashcache_free_cache_job(job);
	if (atomic_dec_and_test(&dmc->nr_jobs))
		wake_up(&dmc->destroyq);
}

static void
flashcache_do_pending_noerror(struct kcached_job *job)
{
	struct disk_slot *slot = job->slot;
	struct cache_c *dmc = slot->dmc;
	int index = job->index;
	unsigned long flags;
	struct pending_job *pending_job, *freelist;
	int queued;
	struct cacheblock *cacheblk = &dmc->cache[index];

	LOCK_START
	spin_lock_irqsave(&dmc->cache_spin_lock, flags);
//	VERIFY(cacheblk->cache_state & VALID);
	if ((cacheblk->cache_state & VALID) == 0)
		dmc->flashcache_errors.verify_errors3++;

	dmc->cached_blocks--;
	dmc->flashcache_stats.pending_inval++;
	slot->cached_blocks--;
	slot->flashcache_stats.pending_inval++;
	cacheblk->cache_state &= ~(VALID | PVALID);
	cacheblk->cache_state |= INVALID;
	DPRINTK("flashcache_do_pending: Index %d 0x%lx",
		index, cacheblk->cache_state);
	while ((freelist = flashcache_deq_pending(dmc, index)) != NULL) {
		while (freelist != NULL) {
			pending_job = freelist;
			freelist = pending_job->next;
			VERIFY(cacheblk->nr_queued > 0);
			cacheblk->nr_queued--;
			if (pending_job->action == INVALIDATE) {
				DPRINTK("flashcache_do_pending: INVALIDATE index %d %llu",
					pending_job->index, pending_job->bio->bi_sector);
				VERIFY(pending_job->bio != NULL);
				queued = flashcache_inval_blocks(slot, pending_job->bio);
				if (queued) {
					if (unlikely(queued < 0)) {
						/*
						 * Memory allocation failure inside inval_blocks.
						 * Fail this io.
						 */
						flashcache_bio_endio(pending_job->bio, -EIO, dmc, NULL);
					}
					flashcache_free_pending_job(pending_job);
					continue;
				}
			} else if (pending_job->action == DISCARD) {
				DPRINTK("flashcache_do_pending: DISCARD index %d %llu",
					pending_job->index, pending_job->bio->bi_sector);
				VERIFY(pending_job->bio != NULL);
				queued = flashcache_discard_blocks(slot, pending_job->bio);
				if (queued) {
					if (unlikely(queued < 0)) {
						/*
						 * Memory allocation failure inside inval_blocks.
						 * Fail this io.
						 */
						flashcache_bio_endio(pending_job->bio, -EIO, dmc, NULL);
					}
					flashcache_free_pending_job(pending_job);
					continue;
				}
			}
			spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
			LOCK_END
			DPRINTK("flashcache_do_pending: Sending down IO index %d %llu",
				pending_job->index, pending_job->bio->bi_sector);
			/* Start uncached IO */
			if (pending_job->action == DISCARD)
				flashcache_start_discard_io(slot, pending_job->bio);
			else
				flashcache_start_uncached_io(slot, pending_job->bio);
			flashcache_free_pending_job(pending_job);
			LOCK_START
			spin_lock_irqsave(&dmc->cache_spin_lock, flags);
		}
	}
//	VERIFY(cacheblk->nr_queued == 0);
	if (cacheblk->nr_queued != 0)
		dmc->flashcache_errors.verify_errors4++;
	cacheblk->cache_state &= ~(BLOCK_IO_INPROG);
	spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
	LOCK_END
	DPRINTK("flashcache_do_pending: Index %d 0x%lx",
		index, cacheblk->cache_state);

	flashcache_free_cache_job(job);
	if (atomic_dec_and_test(&dmc->nr_jobs))
		wake_up(&dmc->destroyq);
}

void
flashcache_do_pending(struct kcached_job *job)
{
	if (job->error)
		flashcache_do_pending_error(job);
	else
		flashcache_do_pending_noerror(job);
}

void
flashcache_do_io(struct kcached_job *job)
{
	struct bio *bio = job->bio;
	int r = 0;

	if (job->action == READFILL) {	
	VERIFY(job->action == READFILL);
#ifdef FLASHCACHE_DO_CHECKSUMS
	flashcache_store_checksum(job);
	job->dmc->flashcache_stats.checksum_store++;
#endif
	/* Write to cache device */
	job->slot->dmc->flashcache_stats.ssd_writes++;
	r = dm_io_async_bvec(1, &job->job_io_regions.cache, WRITE, bio->bi_io_vec + bio->bi_idx,
			     flashcache_io_callback, job);
	VERIFY(r == 0);
	/* In our case, dm_io_async_bvec() must always return 0 */
	} else if (job->action == READDISK_UNCACHED) {
		dm_io_async_bvec(1, &job->job_io_regions.disk, READ,
			bio->bi_io_vec + bio->bi_idx,
			flashcache_io_callback, job);
	}
}

/*
 * Map a block from the source device to a block in the cache device.
 */
static unsigned long 
hash_block(struct cache_c *dmc, sector_t dbn)
{
	unsigned long set_number, value;

	value = (unsigned long)
		(dbn >> (dmc->block_shift + dmc->assoc_shift));
	set_number = value % dmc->num_sets;
//	SPRINTK("Hash: %llu(%lu)->%lu", dbn, value, set_number);
	return set_number;
}

static void
find_valid_dbn(struct disk_slot *slot, struct bio *bio, sector_t dbn, 
	       int start_index, int *valid, int *pvalid, int *invalid)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	int i;
	int end_index = start_index + dmc->assoc;

	*valid = *pvalid = *invalid = -1;
	for (i = start_index ; i < end_index ; i++) {
		if ((dmc->cache[i].dbn <= dbn && dbn <(dmc->cache[i].dbn+dmc->block_size)) &&
		    (slot->id == dmc->cache[i].slot_id) &&
		    (dmc->cache[i].cache_state & VALID)) {
			if (to_sector(bio->bi_size) == dmc->block_size) 
				*valid = i;
			else 
				*pvalid = i;
			if (dmc->sysctl_reclaim_policy == FLASHCACHE_LRU &&
			 	((dmc->cache[i].cache_state & BLOCK_IO_INPROG) == 0))
				flashcache_reclaim_lru_movetail(dmc, i);
			return;
		}
		if (*invalid == -1 && dmc->cache[i].cache_state & INVALID && (dmc->cache[i].cache_state & BLOCK_IO_INPROG)==0) {
			*invalid = i;
		}
	}

	if (*valid == -1 && *pvalid == -1 && *invalid != -1)
		if (dmc->sysctl_reclaim_policy == FLASHCACHE_LRU)
			flashcache_reclaim_lru_movetail(dmc, *invalid);
}

static void
find_valid_dbn_noreclaim(struct disk_slot *slot, sector_t dbn, 
	       int start_index, int *valid)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	int i;
	int end_index = start_index + dmc->assoc;

	*valid = -1;
	for (i = start_index ; i < end_index ; i++) {
		if ((dmc->cache[i].dbn <= dbn && dbn <(dmc->cache[i].dbn+dmc->block_size)) &&
		    (slot->id == dmc->cache[i].slot_id) &&
		    (dmc->cache[i].cache_state & VALID)) {
			*valid = i;
			return;
		}
	}
}

/* Search for a slot that we can reclaim */
static void
find_reclaim_dbn(struct cache_c *dmc, int start_index, int *index)
{
	int set = start_index / dmc->assoc;
	struct cache_set *cache_set = &dmc->cache_sets[set];
	struct cacheblock *cacheblk;
	
	if (dmc->sysctl_reclaim_policy == FLASHCACHE_FIFO) {
		int end_index = start_index + dmc->assoc;
		int slots_searched = 0;
		int i;

		i = cache_set->set_fifo_next;
		while (slots_searched < dmc->assoc) {
			VERIFY(i >= start_index);
			VERIFY(i < end_index);
			if ((dmc->cache[i].cache_state & VALID) &&
				!(dmc->cache[i].cache_state & BLOCK_IO_INPROG)) {
				*index = i;
				break;
			}
			slots_searched++;
			i++;
			if (i == end_index)
				i = start_index;
		}
		i++;
		if (i == end_index)
			i = start_index;
		cache_set->set_fifo_next = i;
	} else { /* reclaim_policy == FLASHCACHE_LRU */
		int lru_rel_index;

		lru_rel_index = cache_set->lru_head;
		while (lru_rel_index != FLASHCACHE_LRU_NULL) {
			cacheblk = &dmc->cache[lru_rel_index + start_index];
			if ((cacheblk->cache_state & VALID) &&
				!(cacheblk->cache_state & BLOCK_IO_INPROG)) {
				VERIFY((cacheblk - &dmc->cache[0]) == 
				       (lru_rel_index + start_index));
				*index = cacheblk - &dmc->cache[0];
				flashcache_reclaim_lru_movetail(dmc, *index);
				break;
			}
			lru_rel_index = cacheblk->lru_next;
		}
	}
}

/* 
 * dbn is the starting sector, io_size is the number of sectors.
 */
static int 
flashcache_lookup(struct disk_slot *slot, struct bio *bio, int *index)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	sector_t dbn = bio->bi_sector;
#if DMC_DEBUG
	int io_size = to_sector(bio->bi_size);
#endif
	unsigned long set_number = hash_block(dmc, dbn);
	int invalid, oldest_clean = -1;
	int valid = -1, pvalid = -1;
	int start_index;

	start_index = dmc->assoc * set_number;
//	SPRINTK("Cache lookup : dbn %llu(%lu), set = %d",
//		dbn, io_size, set_number);
	find_valid_dbn(slot, bio, dbn, start_index, &valid, &pvalid, &invalid);

	if (pvalid >= 0) {
		*index = pvalid;
		SPRINTK("Cache lookup HIT: Disk %d Block %llu(%lu): Partial VALID index %d",
			     slot->id, dbn, io_size, *index);
		/* We found the exact range of blocks we are looking for */
		return PVALID;
	} else if (valid >= 0) {
		*index = valid;
		SPRINTK("Cache lookup HIT: Disk %d Block %llu(%lu): VALID index %d",
			     slot->id, dbn, io_size, *index);
		/* We found the exact range of blocks we are looking for */
		return VALID;
	}
	if (invalid == -1) {
		/* We didn't find an invalid entry, search for oldest valid entry */
		find_reclaim_dbn(dmc, start_index, &oldest_clean);
	}
	/* 
	 * Cache miss :
	 * We can't choose an entry marked INPROG, but choose the oldest
	 * INVALID or the oldest VALID entry.
	 */
	*index = start_index + dmc->assoc;
	if (invalid != -1) {
		SPRINTK("Cache lookup MISS (INVALID): Disk %d dbn %llu(%lu), set = %d, index = %d, start_index = %d",
			     slot->id, dbn, io_size, set_number, invalid, start_index);
		*index = invalid;
	} else if (oldest_clean != -1) {
		SPRINTK("Cache lookup MISS (VALID): Disk %d dbn %llu(%lu), set = %d, index = %d, start_index = %d",
			     slot->id, dbn, io_size, set_number, oldest_clean, start_index);
		*index = oldest_clean;
	} else {
		SPRINTK("Cache read lookup MISS (NOROOM): Disk %d dbn %llu(%lu), set = %d",
			slot->id, dbn, io_size, set_number);
	}
	if (*index < (start_index + dmc->assoc))
		return INVALID;
	else {
		dmc->flashcache_stats.noroom++;
		slot->flashcache_stats.noroom++;
		return -1;
	}
}

/* 
 * dbn is the starting sector, io_size is the number of sectors.
 */
int 
flashcache_lookup_noreclaim(struct disk_slot *slot, sector_t start, int *index)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	sector_t dbn = start;
	unsigned long set_number = hash_block(dmc, dbn);
	int valid = -1;
	int start_index;

	start_index = dmc->assoc * set_number;
//	SPRINTK("Cache lookup : dbn %llu(%lu), set = %d",
//		dbn, io_size, set_number);
	find_valid_dbn_noreclaim(slot, dbn, start_index, &valid);

	if (valid >= 0) {
		*index = valid;
		return VALID;
	}
	return INVALID;
}

/*
 * This function encodes the background disk cleaning logic.
 * Background disk cleaning is triggered for 2 reasons.
 A) Dirty blocks are lying fallow in the set, making them good 
    candidates for being cleaned.
 B) This set has dirty blocks over the configured threshold 
    for a set.
 * (A) takes precedence over (B). Fallow dirty blocks are cleaned
 * first.
 * The cleaning of disk blocks is subject to the write limits per
 * set and across the cache, which this function enforces.
 *
 * 1) Select the n blocks that we want to clean (choosing whatever policy), 
 *    sort them.
 * 2) Then sweep the entire set looking for other DIRTY blocks that can be 
 *    tacked onto any of these blocks to form larger contigous writes. 
 *    The idea here is that if you are going to do a write anyway, then we 
 *    might as well opportunistically write out any contigous blocks for 
 *    free.
 */

#ifndef PARTIAL_BITMAP
static void
flashcache_read_hit(int partial, struct disk_slot *slot, struct bio* bio, int index)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	struct cacheblock *cacheblk;
	struct pending_job *pjob;

	int    action=-1;
	int16_t offset=-1;
	int16_t size=-1;
	int16_t io_offset=-1;
	int16_t io_size=-1;
	u_int16_t       cache_state_a=0, cache_state_r=0;
	int can_concurrent = 0;

	cacheblk = &dmc->cache[index];

	if (cacheblk->cache_state & PVALID) {
		if (partial) {
			SPRINTK("Cache read P->P: Disk %d Block %llu(%lu), index = %d",
					slot->id, bio->bi_sector, bio->bi_size, index);
			/* partial cache hit, but all hit 1
				bio size < cache size
			  		|<---cache--->|
 			                |<---bio--->|

			   partial cache hit, but all hit 2
				bio size < cache size
			  		|<---cache--->|
    			                 |<---bio--->|

			   partial cache hit, but all hit 3
				bio size < cache size
			  		|<---cache--->|
    		        	          |<---bio--->|
			*/
			if ((cache_start_sector(cacheblk)<=flashcache_bio_start_sector(bio, cacheblk)) && (flashcache_bio_end_sector(bio, cacheblk)<=cache_end_sector(cacheblk))) {
				offset = cacheblk->offset;
				size = cacheblk->size;
				action = READCACHE;
				cache_state_a = CACHEREADINPROG;
				SPRINTK("hit 123 %d %d", offset, size);
			}

			/* cache replace 1
			  		|<---cache--->|
			  |<---bio--->|
			*/
			else if (flashcache_bio_end_sector(bio, cacheblk) < cache_start_sector(cacheblk)) {
				offset = flashcache_bio_start_sector(bio, cacheblk);
				size = to_sector(bio->bi_size);
				action = READDISK;
				cache_state_a = DISKREADINPROG;
				SPRINTK("replace 1 %d %d", offset, size);
			}

			/* cache merge 1
			  		|<---cache--->|
   			    |<---bio--->|
			*/
			else if (flashcache_bio_end_sector(bio, cacheblk) == cache_start_sector(cacheblk)) {
				offset = flashcache_bio_start_sector(bio, cacheblk);
				size = cacheblk->size + to_sector(bio->bi_size);
				action = READDISK;
				cache_state_a = DISKREADINPROG;
				if (size == dmc->block_size) {
					cache_state_r = PVALID;
					SPRINTK("merge to cache block");
				}

				if (flashcache_bio_end_sector(bio, cacheblk) == cacheio_start_sector(cacheblk)) {
					io_offset = flashcache_bio_start_sector(bio, cacheblk);
					io_size = cacheblk->io_size + to_sector(bio->bi_size);
					can_concurrent = 1;
				}
				SPRINTK("merge 1 %d %d can_concurrent %d", offset, size, can_concurrent);
			}

			/* cache merge 2
			  		|<---cache--->|
    			       |<---bio--->|
			*/
			else if (flashcache_bio_start_sector(bio, cacheblk) < cache_start_sector(cacheblk) &&
				flashcache_bio_end_sector(bio, cacheblk) > cache_start_sector(cacheblk) &&
				flashcache_bio_end_sector(bio, cacheblk) < cache_end_sector(cacheblk) ) {
				offset = flashcache_bio_start_sector(bio, cacheblk);
				size = cache_end_sector(cacheblk) - flashcache_bio_start_sector(bio, cacheblk);
				action = READDISK;
				cache_state_a = DISKREADINPROG;
				if (size == dmc->block_size) {
					cache_state_r = PVALID;
					SPRINTK("merge to cache block");
				}
				SPRINTK("merge 2 %d %d", offset, size);
			}

			/* cache replace 2
				bio size > cache size
			  		|<---cache--->|
	  		           |<---bio---------->|

			   cache replace 3
				bio size > cache size
			  		|<---cache--->|
    			             |<---bio---------->|

			   cache replace 4
				bio size > cache size
			  		|<---cache--->|
	    		                |<---bio---------->|
			*/
			else if ((flashcache_bio_start_sector(bio, cacheblk)<=cache_start_sector(cacheblk)) && (cache_end_sector(cacheblk)<=flashcache_bio_end_sector(bio, cacheblk))) {
				offset = flashcache_bio_start_sector(bio, cacheblk);
				size = to_sector(bio->bi_size);
				action = READDISK;
				cache_state_a = DISKREADINPROG;
				SPRINTK("replace 234 %d %d", offset, size);
			}

			/* cache merge 3
			  		|<---cache--->|
      		        	              |<---bio--->|
			*/
			else if (cache_start_sector(cacheblk) < flashcache_bio_start_sector(bio, cacheblk) &&
				cache_end_sector(cacheblk) > flashcache_bio_start_sector(bio, cacheblk) &&
				cache_end_sector(cacheblk) < flashcache_bio_end_sector(bio, cacheblk) ) {
				offset = cache_start_sector(cacheblk);
				size = flashcache_bio_end_sector(bio, cacheblk) - cache_start_sector(cacheblk);
				action = READDISK;
				cache_state_a = DISKREADINPROG;
				if (size == dmc->block_size) {
					cache_state_r = PVALID;
					SPRINTK("merge to cache block");
				}
				SPRINTK("merge 3 %d %d", offset, size);
			}

			/* cache merge 4
			  		|<---cache--->|
      			                              |<---bio--->|
			*/
			else if (cache_end_sector(cacheblk) == flashcache_bio_start_sector(bio, cacheblk)) {
				offset = cache_start_sector(cacheblk);
				size = cacheblk->size + to_sector(bio->bi_size);
				action = READDISK;
				cache_state_a = DISKREADINPROG;
				if (size == dmc->block_size) {
					cache_state_r = PVALID;
					SPRINTK("merge to cache block");
				}

				if (flashcache_bio_start_sector(bio, cacheblk) == cacheio_end_sector(cacheblk)) {
					io_offset = cacheio_start_sector(cacheblk);
					io_size = cacheblk->io_size + to_sector(bio->bi_size);
					can_concurrent = 1;
				}
				SPRINTK("merge 4 %d %d can_concurrent %d", offset, size, can_concurrent);
			}

			/* cache replace 5
			  		|<---cache--->|
      			                                |<---bio--->|
			*/
			else if (cache_end_sector(cacheblk) < flashcache_bio_start_sector(bio, cacheblk)) {
				offset = flashcache_bio_start_sector(bio, cacheblk);
				size = to_sector(bio->bi_size);
				action = READDISK;
				cache_state_a = DISKREADINPROG;
				SPRINTK("replace 5 %d %d", offset, size);
			} else {
				printk("ouch! impossible!!!!!!!\n");
			}
		} else {
			SPRINTK("Cache read P->V: Disk %d Block %llu(%lu), index = %d",
					slot->id, bio->bi_sector, bio->bi_size, index);
			offset = 0;
			size = dmc->block_size;
			action = READDISK;
			cache_state_r = PVALID;
			cache_state_a = DISKREADINPROG;

			dmc->flashcache_stats.read_phits++;
			slot->flashcache_stats.read_phits++;
			/* cacheblk->dbn no change */
		}
	} else { /* cacheblk->cache_state & VALID */
		if (partial) {
			SPRINTK("Cache read V->P: Disk %d Block %llu(%lu), index = %d",
					slot->id, bio->bi_sector, bio->bi_size, index);
			offset = 0;
			size = dmc->block_size;
			action = READCACHE;
			cache_state_a = CACHEREADINPROG;
			dmc->flashcache_stats.read_phits++;
			slot->flashcache_stats.read_phits++;

			/* io merge 1
			  		|<---cacheio--->|
	   		    |<---bio--->|
			*/
			if (flashcache_bio_end_sector(bio, cacheblk) == cacheio_start_sector(cacheblk)) {
				io_offset = flashcache_bio_start_sector(bio, cacheblk);
				io_size = cacheblk->io_size + to_sector(bio->bi_size);
				can_concurrent = 1;
				SPRINTK("io merge 1 %d %d can_concurrent %d", offset, size, can_concurrent);
			}
			/* io merge 4
			  		|<---cacheio--->|
      			                                |<---bio--->|
			*/
			else if (flashcache_bio_start_sector(bio, cacheblk) == cacheio_end_sector(cacheblk)) {
				io_offset = cacheio_start_sector(cacheblk);
				io_size = cacheblk->io_size + to_sector(bio->bi_size);
				can_concurrent = 1;
				SPRINTK("io merge 4 %d %d can_concurrent %d", offset, size, can_concurrent);
			}
		} else {
			SPRINTK("Cache read V->V: Disk %d Block %llu(%lu), index = %d",
					slot->id, bio->bi_sector, bio->bi_size, index);
			offset = 0;
			size = dmc->block_size;
			action = READCACHE;
			cache_state_a = CACHEREADINPROG;
//			dmc->flashcache_stats.read_hits++;
		}
	}

	/* If block is busy, queue IO pending completion of in-progress IO */
	if ((cacheblk->nr_queued == 0) && (can_concurrent || !(cacheblk->cache_state & BLOCK_IO_INPROG))) {
		struct kcached_job *job;
			
		cacheblk->offset = offset;
		cacheblk->size = size;
		cacheblk->cache_state &= ~cache_state_r;
		cacheblk->cache_state |= cache_state_a;

		if (!cacheblk->nr_concurrent) {
			cacheblk->io_offset = flashcache_bio_start_sector(bio, cacheblk);
			cacheblk->io_size = to_sector(bio->bi_size);
		} else if (can_concurrent) {
			cacheblk->io_offset = io_offset;
			cacheblk->io_size = io_size;
		}
		cacheblk->nr_concurrent++;

		spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
		DPRINTK("Cache read: Disk %d Block %llu(%lu), index = %d:%s%s%s %d %d nr_concurrent %d",
			slot->id, bio->bi_sector, bio->bi_size, index, "CACHE", partial?" PARTIAL ":" ", "HIT", cacheblk->io_offset, cacheblk->io_size, cacheblk->nr_concurrent);

		job = new_kcached_job(slot, bio, index);
		if (unlikely(dmc->sysctl_error_inject & READ_HIT_JOB_ALLOC_FAIL)) {
			if (job)
				flashcache_free_cache_job(job);
			job = NULL;
			dmc->sysctl_error_inject &= ~READ_HIT_JOB_ALLOC_FAIL;
		}
		if (unlikely(job == NULL)) {
			/* 
			 * We have a read hit, and can't allocate a job.
			 * Since we dropped the spinlock, we have to drain any 
			 * pending jobs.
			 */
			DMERR("flashcache: Read (hit) failed ! Can't allocate memory for cache IO, block %lu", 
			      cacheblk->dbn);
			flashcache_bio_endio(bio, -EIO, dmc, NULL);
			LOCK_START
			spin_lock_irq(&dmc->cache_spin_lock);
			flashcache_free_pending_jobs(dmc, cacheblk, -EIO);
			cacheblk->nr_concurrent--;
			if (cacheblk->nr_concurrent)
                                cacheblk->cache_state |= IN_ERROR;
			if (!cacheblk->nr_concurrent)
				cacheblk->cache_state &= ~(BLOCK_IO_INPROG);
			spin_unlock_irq(&dmc->cache_spin_lock);
			LOCK_END
		} else {
			if (action == READCACHE) {
				job->action = READCACHE; /* Fetch data from cache */
				atomic_inc(&dmc->nr_jobs);
				dmc->flashcache_stats.raw_read_hits++;
				dmc->flashcache_stats.read_hits++;
				slot->flashcache_stats.read_hits++;
				dmc->flashcache_stats.ssd_reads++;
				slot->flashcache_stats.ssd_reads++;
				dm_io_async_bvec(1, &job->job_io_regions.cache, READ,
					 bio->bi_io_vec + bio->bi_idx,
					 flashcache_io_callback, job);
			} else {
				job->action = READDISK; /* Fetch data from the source device */
				atomic_inc(&dmc->nr_jobs);
				dmc->flashcache_stats.disk_reads++;
				slot->flashcache_stats.disk_reads++;
				dm_io_async_bvec(1, &job->job_io_regions.disk, READ,
					 bio->bi_io_vec + bio->bi_idx,
					 flashcache_io_callback, job);
			}
		}
	} else {
		pjob = flashcache_alloc_pending_job(dmc);
		pjob->partial = partial;
		if (unlikely(dmc->sysctl_error_inject & READ_HIT_PENDING_JOB_ALLOC_FAIL)) {
			if (pjob) {
				flashcache_free_pending_job(pjob);
				pjob = NULL;
			}
			dmc->sysctl_error_inject &= ~READ_HIT_PENDING_JOB_ALLOC_FAIL;
		}
		if (pjob == NULL)
			flashcache_bio_endio(bio, -EIO, dmc, NULL);
		else
			flashcache_enq_pending(dmc, bio, index, READCACHE, pjob);
		spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
	}
}

#else

static void
flashcache_read_hit(int partial, struct disk_slot *slot, struct bio* bio, int index)
{
        struct cache_c *dmc = (struct cache_c *) slot->dmc;
        struct cacheblock *cacheblk;
        struct pending_job *pjob;
        struct kcached_job *job;

        int    action=-1;
        int16_t offset=-1;
        int16_t size=-1;
        u_int16_t       cache_state_a=0, cache_state_r=0;
        int can_concurrent = 0;
#ifdef BITMAP_ARRAY
        u_int64_t       bitmap[BITMAP_COUNT], io_bitmap[BITMAP_COUNT], tmp_bitmap[BITMAP_COUNT];
	memset(io_bitmap, 0, sizeof(io_bitmap));
#else
        u_int64_t       bitmap, io_bitmap=0, tmp_bitmap;
#endif

        cacheblk = &dmc->cache[index];

        /* If block is error or busy, queue IO pending completion of in-progress IO */
        if (!(cacheblk->cache_state&IN_ERROR) && (cacheblk->nr_queued == 0)) {
	        if (cacheblk->cache_state & PVALID) {
        	        if (partial) {
	                        SPRINTK("Cache read P->P: Disk %d Block %llu(%lu), index = %d",
	        	                	slot->id, bio->bi_sector, bio->bi_size, index);

	                        offset = flashcache_bio_start_sector(bio, cacheblk);
        	                size = to_sector(bio->bi_size);
#ifdef BITMAP_ARRAY
				to_bitmap64_array((void*)&tmp_bitmap, offset, size);

	                        /* cache all hit */
        	                if (bitmap64_array_contain((void*)&cacheblk->bitmap, (void*)&tmp_bitmap)) {
					bitmap64_array_copy((void*)&bitmap, (void*)&cacheblk->bitmap);
	                                action = READCACHE;
        	                        cache_state_a = CACHEREADINPROG;
	                        } else {
					bitmap64_array_add((void*)&cacheblk->bitmap, (void*)&tmp_bitmap, (void*)&bitmap);
                	                action = READDISK;
	                                cache_state_a = DISKREADINPROG;
        	                        if (bitmap64_array_check_mask((void*)&bitmap)) {
                	                        cache_state_r = PVALID;
	                                        SPRINTK("merge to cache block");
        	                        }
                	        }

	                        if (bitmap64_array_notcontain((void*)&cacheblk->io_bitmap, (void*)&tmp_bitmap)) {
					bitmap64_array_add((void*)&cacheblk->io_bitmap, (void*)&tmp_bitmap, (void*)&io_bitmap);
                	                can_concurrent = 1;
	                        } /* else goto pending io */
#else
        	                tmp_bitmap = TO_BITMAP(offset, size);

	                        /* cache all hit */
        	                if (BITMAP_CONTAIN(cacheblk->bitmap, tmp_bitmap)) {
                	                bitmap = cacheblk->bitmap;
                        	        action = READCACHE;
                                	cache_state_a = CACHEREADINPROG;
	                        } else {
        	                        bitmap = BITMAP_ADD(cacheblk->bitmap, tmp_bitmap);
                	                action = READDISK;
                        	        cache_state_a = DISKREADINPROG;
	                                if (bitmap == BLOCK_MASK) {
        	                                cache_state_r = PVALID;
                	                        SPRINTK("merge to cache block");
                        	        }
	                        }

        	                if (BITMAP_NOTCONTAIN(cacheblk->io_bitmap, tmp_bitmap)) {
                	                io_bitmap = BITMAP_ADD(cacheblk->io_bitmap, tmp_bitmap);
                        	        can_concurrent = 1;
	                        } /* else goto pending io */
#endif
	                } else {
        	                SPRINTK("Cache read P->V: Disk %d Block %llu(%lu), index = %d",
        	                                slot->id, bio->bi_sector, bio->bi_size, index);

#ifdef BITMAP_ARRAY
				bitmap64_array_set_mask((void*)&bitmap);
				bitmap64_array_set_mask((void*)&io_bitmap);
#else
	                        bitmap = BLOCK_MASK;
        	                io_bitmap = BLOCK_MASK;
#endif
	                        action = READDISK;
        	                cache_state_r = PVALID;
                	        cache_state_a = DISKREADINPROG;

	                        dmc->flashcache_stats.read_phits++;
        	                slot->flashcache_stats.read_phits++;
               		        /* cacheblk->dbn no change */
	                }
	        } else { /* cacheblk->cache_state & VALID */
#ifdef BITMAP_ARRAY
			bitmap64_array_set_mask((void*)&bitmap);
#else
			bitmap = BLOCK_MASK;
#endif
	                if (partial) {
        	                SPRINTK("Cache read V->P: Disk %d Block %llu(%lu), index = %d",
                	                        slot->id, bio->bi_sector, bio->bi_size, index);

	                        action = READCACHE;
        	                cache_state_a = CACHEREADINPROG;
                	        dmc->flashcache_stats.read_phits++;
	                        slot->flashcache_stats.read_phits++;

        	                offset = flashcache_bio_start_sector(bio, cacheblk);
                	        size = to_sector(bio->bi_size);
#ifdef BITMAP_ARRAY
				to_bitmap64_array((void*)&tmp_bitmap, offset, size);

        	                if (bitmap64_array_notcontain((void*)&cacheblk->io_bitmap, (void*)&tmp_bitmap)) {
					bitmap64_array_add((void*)&cacheblk->io_bitmap, (void*)&tmp_bitmap, (void*)&io_bitmap);
                        	        can_concurrent = 1;
	                        } /* else goto pending io */
#else
        	                tmp_bitmap = TO_BITMAP(offset, size);

                	        if (BITMAP_NOTCONTAIN(cacheblk->io_bitmap, tmp_bitmap)) {
                        	        io_bitmap = BITMAP_ADD(cacheblk->io_bitmap, tmp_bitmap);
                                	can_concurrent = 1;
	                        } /* else goto pending io */
#endif
	                } else {
        	                SPRINTK("Cache read V->V: Disk %d Block %llu(%lu), index = %d",
                	                        slot->id, bio->bi_sector, bio->bi_size, index);

#ifdef BITMAP_ARRAY
				bitmap64_array_set_mask((void*)&io_bitmap);
#else
        	                io_bitmap = BLOCK_MASK;
#endif
	                        action = READCACHE;
        	                cache_state_a = CACHEREADINPROG;
//              	        dmc->flashcache_stats.read_hits++;
	                }
        	}
	} else
		goto queue_job;

	if (can_concurrent || !(cacheblk->cache_state & BLOCK_IO_INPROG)) {
#ifdef BITMAP_ARRAY
		bitmap64_array_copy((void*)&cacheblk->bitmap, (void*)&bitmap);
#else
                cacheblk->bitmap = bitmap;
#endif
                cacheblk->cache_state &= ~cache_state_r;
                cacheblk->cache_state |= cache_state_a;

                if (!cacheblk->nr_concurrent) {
                        offset = flashcache_bio_start_sector(bio, cacheblk);
                        size = to_sector(bio->bi_size);
#ifdef BITMAP_ARRAY
                        to_bitmap64_array((void*)&cacheblk->io_bitmap, offset, size);
                } else if (can_concurrent) {
                        bitmap64_array_copy((void*)&cacheblk->io_bitmap, (void*)&io_bitmap);
#else
                        cacheblk->io_bitmap = TO_BITMAP(offset, size);
                } else if (can_concurrent) {
                        cacheblk->io_bitmap = io_bitmap;
#endif
                }
                cacheblk->nr_concurrent++;

                spin_unlock_irq(&dmc->cache_spin_lock);
                LOCK_END
                DPRINTK("Cache read: Disk %d Block %llu(%lu), index = %d:%s%s%s nr_concurrent %d",
                        slot->id, bio->bi_sector, bio->bi_size, index, "CACHE", partial?" PARTIAL ":" ", "HIT", cacheblk->nr_concurrent);

//              char tmp[72];
//              char tmp1[72];
//              DPRINTK("Cache read: Disk %d Block %llu(%lu), index = %d:%s%s%s %s %s nr_concurrent %d",
//                      slot->id, bio->bi_sector, bio->bi_size, index, "CACHE", partial?" PARTIAL ":" ", "HIT", printk_binary(cacheblk->bitmap, tmp),
//                      printk_binary(cacheblk->io_bitmap, tmp1), cacheblk->nr_concurrent);

                job = new_kcached_job(slot, bio, index);
                if (unlikely(dmc->sysctl_error_inject & READ_HIT_JOB_ALLOC_FAIL)) {
                        if (job)
                                flashcache_free_cache_job(job);
                        job = NULL;
                        dmc->sysctl_error_inject &= ~READ_HIT_JOB_ALLOC_FAIL;
                }
                if (unlikely(job == NULL)) {
                        /*
                         * We have a read hit, and can't allocate a job.
                         * Since we dropped the spinlock, we have to drain any
                         * pending jobs.
                         */
                        DMERR("flashcache: Slot %d Read (hit) failed ! Can't allocate memory for cache IO, index %d block %lu nr_concurrent %d",
                              slot->id, index, cacheblk->dbn, cacheblk->nr_concurrent);
                        flashcache_bio_endio(bio, -EIO, dmc, NULL);
                        LOCK_START
                        spin_lock_irq(&dmc->cache_spin_lock);
                        flashcache_free_pending_jobs(dmc, cacheblk, -EIO);
                        cacheblk->nr_concurrent--;
			if (cacheblk->nr_concurrent)
                                cacheblk->cache_state |= IN_ERROR;
                        if (!cacheblk->nr_concurrent) {
				dmc->cached_blocks--;
				slot->cached_blocks--;
                                cacheblk->cache_state &= ~(BLOCK_IO_INPROG);
				/* since io error and no more concurrent job, invalid it */
				cacheblk->cache_state &= ~(VALID | PVALID);
				cacheblk->cache_state |= INVALID;
			}
                        spin_unlock_irq(&dmc->cache_spin_lock);
                        LOCK_END
                } else {
                        if (action == READCACHE) {
                                job->action = READCACHE; /* Fetch data from cache */
                                atomic_inc(&dmc->nr_jobs);
                		dmc->flashcache_stats.raw_read_hits++;
                		dmc->flashcache_stats.read_hits++;
				slot->flashcache_stats.read_hits++;
                                dmc->flashcache_stats.ssd_reads++;
                                slot->flashcache_stats.ssd_reads++;
                                dm_io_async_bvec(1, &job->job_io_regions.cache, READ,
                                         bio->bi_io_vec + bio->bi_idx,
                                         flashcache_io_callback, job);
                        } else {
                                job->action = READDISK; /* Fetch data from the source device */
                                atomic_inc(&dmc->nr_jobs);
                                dmc->flashcache_stats.disk_reads++;
                                slot->flashcache_stats.disk_reads++;
                                dm_io_async_bvec(1, &job->job_io_regions.disk, READ,
                                         bio->bi_io_vec + bio->bi_idx,
                                         flashcache_io_callback, job);
                        }
                }
        } else {
queue_job:
                pjob = flashcache_alloc_pending_job(dmc);
                pjob->partial = partial;
                if (unlikely(dmc->sysctl_error_inject & READ_HIT_PENDING_JOB_ALLOC_FAIL)) {
                        if (pjob) {
                                flashcache_free_pending_job(pjob);
                                pjob = NULL;
                        }
                        dmc->sysctl_error_inject &= ~READ_HIT_PENDING_JOB_ALLOC_FAIL;
                }
                if (pjob == NULL)
                        flashcache_bio_endio(bio, -EIO, dmc, NULL);
                else
                        flashcache_enq_pending(dmc, bio, index, READCACHE, pjob);
                spin_unlock_irq(&dmc->cache_spin_lock);
                LOCK_END
        }
}
#endif	/* end of PARTIAL_BITMAP define */

static void
flashcache_read_miss(int partial, struct disk_slot *slot, struct bio* bio,
		     int index)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	struct kcached_job *job;
	struct cacheblock *cacheblk = &dmc->cache[index];

	job = new_kcached_job(slot, bio, index);

	if (unlikely(dmc->sysctl_error_inject & READ_MISS_JOB_ALLOC_FAIL)) {
		if (job)
			flashcache_free_cache_job(job);
		job = NULL;
		dmc->sysctl_error_inject &= ~READ_MISS_JOB_ALLOC_FAIL;
	}
	if (unlikely(job == NULL)) {
		/* 
		 * We have a read miss, and can't allocate a job.
		 * Since we dropped the spinlock, we have to drain any 
		 * pending jobs.
		 */
		DMERR("flashcache: Read (miss) failed ! Can't allocate memory for cache IO, block %lu", 
		      cacheblk->dbn);
		flashcache_bio_endio(bio, -EIO, dmc, NULL);
		LOCK_START
		spin_lock_irq(&dmc->cache_spin_lock);
		dmc->cached_blocks--;
		slot->cached_blocks--;
		cacheblk->cache_state &= ~(VALID | PVALID);
		cacheblk->cache_state |= INVALID;
		flashcache_free_pending_jobs(dmc, cacheblk, -EIO);
		cacheblk->nr_concurrent = 0;
		cacheblk->cache_state &= ~(BLOCK_IO_INPROG);
		spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
	} else {
		job->action = READDISK; /* Fetch data from the source device */
		atomic_inc(&dmc->nr_jobs);
		dmc->flashcache_stats.disk_reads++;
		dm_io_async_bvec(1, &job->job_io_regions.disk, READ,
				 bio->bi_io_vec + bio->bi_idx,
				 flashcache_io_callback, job);
	}
}

static void
flashcache_read(struct disk_slot *slot, struct bio *bio)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;

	int index;
	int res;
	int queued;
	int partial;
        int16_t         offset; /* offset within block in sector */
        int16_t         size;   /* size in sector */
	
	DPRINTK("Got a %s for %llu (%u bytes)",
	        (bio_rw(bio) == READ ? "READ":"READA"), 
		bio->bi_sector, bio->bi_size);

	LOCK_START
	spin_lock_irq(&dmc->cache_spin_lock);
	res = flashcache_lookup(slot, bio, &index);
	/* Cache Read Hit case */
	if (res > 0) {
		if (res == VALID) {
			flashcache_read_hit(0, slot, bio, index);
			return;
		} else if (res == PVALID) {
			flashcache_read_hit(1, slot, bio, index);
			return;
		}
	}
	/*
	 * In all cases except for a cache hit (and VALID), test for potential 
	 * invalidations that we need to do.
	 */
	queued = flashcache_inval_blocks(slot, bio);
	if (queued) {
		if (unlikely(queued < 0))
			flashcache_bio_endio(bio, -EIO, dmc, NULL);
		spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
		return;
	}

	if (res == -1) {
		/* No room , non-cacheable or sequential i/o means not wanted in cache */
		spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
		DPRINTK("Cache read: Disk %d Block %llu(%lu):%s",
			slot->id, bio->bi_sector, bio->bi_size, "CACHE MISS & NO ROOM");
		/* Start uncached IO */
		flashcache_start_uncached_io(slot, bio);
		return;
	}
	/* 
	 * (res == INVALID) Cache Miss 
	 * And we found cache blocks to replace
	 * Claim the cache blocks before giving up the spinlock
	 */
	if (dmc->cache[index].cache_state & VALID) {
		dmc->flashcache_stats.replace++;
		slot->flashcache_stats.replace++;
		dmc->slot[dmc->cache[index].slot_id].cached_blocks--;
	} else {
		dmc->cached_blocks++;
	}
	slot->cached_blocks++;

	dmc->cache[index].slot_id = slot->id;
	dmc->cache[index].dbn = bio->bi_sector & ~((sector_t)dmc->block_size-1);
	if (to_sector(bio->bi_size) != dmc->block_size) {
		dmc->cache[index].cache_state = VALID | PVALID | DISKREADINPROG;
#ifdef PARTIAL_BITMAP
		offset = flashcache_bio_start_sector(bio, (&dmc->cache[index]));
                size = to_sector(bio->bi_size);
#ifdef BITMAP_ARRAY
		to_bitmap64_array((void*)&dmc->cache[index].bitmap, offset, size);
#else
                dmc->cache[index].bitmap = TO_BITMAP(offset, size);
#endif
#endif
		partial = 1;
	} else {
		dmc->cache[index].cache_state = VALID | DISKREADINPROG;
#ifdef PARTIAL_BITMAP
#ifdef BITMAP_ARRAY
		bitmap64_array_set_mask((void*)&dmc->cache[index].bitmap);
#else
		dmc->cache[index].bitmap = BLOCK_MASK;
#endif
#endif
		partial = 0;
	}

#ifdef PARTIAL_BITMAP
#ifdef BITMAP_ARRAY
	bitmap64_array_copy((void*)&dmc->cache[index].io_bitmap, (void*)&dmc->cache[index].bitmap);
#else
	dmc->cache[index].io_bitmap = dmc->cache[index].bitmap;
#endif
#else
	dmc->cache[index].offset = flashcache_bio_start_sector(bio, (&dmc->cache[index]));
	dmc->cache[index].size = to_sector(bio->bi_size);

	dmc->cache[index].io_offset = dmc->cache[index].offset;
	dmc->cache[index].io_size = dmc->cache[index].size;
#endif

	dmc->cache[index].nr_concurrent = 1;
	spin_unlock_irq(&dmc->cache_spin_lock);
	LOCK_END

	DPRINTK("Cache read: Disk %d Block %llu(%lu), index = %d:%s%s%s nr_concurrent %d",
		slot->id, bio->bi_sector, bio->bi_size, index, "CACHE", partial?" Partial ":" ", "MISS & REPLACE", dmc->cache[index].nr_concurrent);
	flashcache_read_miss(partial, slot, bio, index);
}

/*
 * Invalidate any colliding blocks if they are !BUSY and !DIRTY. If the colliding
 * block is DIRTY, we need to kick off a write. In both cases, we need to wait 
 * until the underlying IO is finished, and then proceed with the invalidation.
 */
static int
flashcache_inval_block_set(struct disk_slot *slot, int set, struct bio *bio, int rw,
			   struct pending_job *pjob)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	sector_t io_start = bio->bi_sector;
	sector_t io_end = bio->bi_sector + (to_sector(bio->bi_size) - 1);
	int start_index, end_index, i;
	struct cacheblock *cacheblk;
	sector_t start_dbn, end_dbn;
	
	start_index = dmc->assoc * set;
	end_index = start_index + dmc->assoc;
	for (i = start_index ; i < end_index ; i++) {
		if (dmc->cache[i].slot_id != slot->id)
			continue;

		start_dbn = dmc->cache[i].dbn;
		end_dbn = start_dbn + dmc->block_size;
		
		cacheblk = &dmc->cache[i];
		if (cacheblk->cache_state & INVALID)
			continue;
		if ((io_start >= start_dbn && io_start < end_dbn) ||
		    (io_end >= start_dbn && io_end < end_dbn)) {
			/* We have a match */
			if (rw == WRITE) {
				dmc->flashcache_stats.wr_invalidates++;
				slot->flashcache_stats.wr_invalidates++;
			} else {
				dmc->flashcache_stats.rd_invalidates++;
				slot->flashcache_stats.rd_invalidates++;
			}
			if (!(cacheblk->cache_state & BLOCK_IO_INPROG) &&
			    (cacheblk->nr_queued == 0)) {
				dmc->cached_blocks--;			
				slot->cached_blocks--;			
				DPRINTK("Cache invalidate (!BUSY): Block %llu %lx",
					start_dbn, cacheblk->cache_state);
				cacheblk->cache_state = INVALID;
				continue;
			}
			/*
			 * The conflicting block has either IO in progress or is 
			 * Dirty. In all cases, we need to add ourselves to the 
			 * pending queue. Then if the block is dirty, we kick off
			 * an IO to clean the block. 
			 * Note that if the block is dirty and IO is in progress
			 * on it, the do_pending handler will clean the block
			 * and then process the pending queue.
			 */
			flashcache_enq_pending(dmc, bio, i, INVALIDATE, pjob);
			return 1;
		}
	}
	return 0;
}

/* 
 * Since md will break up IO into blocksize pieces, we only really need to check 
 * the start set and the end set for overlaps.
 */
static int
flashcache_inval_blocks(struct disk_slot *slot, struct bio *bio)
{	
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	sector_t io_start = bio->bi_sector;
	sector_t io_end = bio->bi_sector + (to_sector(bio->bi_size) - 1);
	int start_set, end_set;
	int queued;
	struct pending_job *pjob1, *pjob2;

	pjob1 = flashcache_alloc_pending_job(dmc);
	if (unlikely(dmc->sysctl_error_inject & INVAL_PENDING_JOB_ALLOC_FAIL)) {
		if (pjob1) {
			flashcache_free_pending_job(pjob1);
			pjob1 = NULL;
		}
		dmc->sysctl_error_inject &= ~INVAL_PENDING_JOB_ALLOC_FAIL;
	}
	if (pjob1 == NULL) {
		queued = -ENOMEM;
		goto out;
	}
	pjob2 = flashcache_alloc_pending_job(dmc);
	if (pjob2 == NULL) {
		flashcache_free_pending_job(pjob1);
		queued = -ENOMEM;
		goto out;
	}
	start_set = hash_block(dmc, io_start);
	end_set = hash_block(dmc, io_end);
	queued = flashcache_inval_block_set(slot, start_set, bio, 
					    bio_data_dir(bio), pjob1);
	if (queued) {
		flashcache_free_pending_job(pjob2);
		goto out;
	} else
		flashcache_free_pending_job(pjob1);		
	if (start_set != end_set) {
		queued = flashcache_inval_block_set(slot, end_set, 
						    bio, bio_data_dir(bio), pjob2);
		if (!queued)
			flashcache_free_pending_job(pjob2);
	} else
		flashcache_free_pending_job(pjob2);		
out:
	return queued;
}

/*
 * Invalidate any colliding blocks if they are !BUSY and !DIRTY. If the colliding
 * block is DIRTY, we need to kick off a write. In both cases, we need to wait 
 * until the underlying IO is finished, and then proceed with the invalidation.
 */
static int
flashcache_discard_block_set(struct disk_slot *slot, int set, struct bio *bio, int rw,
			   struct pending_job *pjob)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	sector_t io_start = bio->bi_sector;
	sector_t io_end = bio->bi_sector + (to_sector(bio->bi_size) - 1);
	int start_index, end_index, i;
	struct cacheblock *cacheblk;
	sector_t start_dbn, end_dbn;
	
	start_index = dmc->assoc * set;
	end_index = start_index + dmc->assoc;
	for (i = start_index ; i < end_index ; i++) {
		if (dmc->cache[i].slot_id != slot->id)
			continue;

		start_dbn = dmc->cache[i].dbn;
		end_dbn = start_dbn + dmc->block_size;
		
		cacheblk = &dmc->cache[i];
		if (cacheblk->cache_state & INVALID)
			continue;
		if ((io_start >= start_dbn && io_start < end_dbn) ||
		    (io_end >= start_dbn && io_end < end_dbn)) {
			/* We have a match */
			if (rw == WRITE) {
				dmc->flashcache_stats.wr_invalidates++;
				slot->flashcache_stats.wr_invalidates++;
			} else {
				dmc->flashcache_stats.rd_invalidates++;
				slot->flashcache_stats.rd_invalidates++;
			}
			if (!(cacheblk->cache_state & BLOCK_IO_INPROG) &&
			    (cacheblk->nr_queued == 0)) {
				dmc->cached_blocks--;			
				slot->cached_blocks--;			
				DPRINTK("Cache invalidate (!BUSY): Block %llu %lx",
					start_dbn, cacheblk->cache_state);
				cacheblk->cache_state = INVALID;
				continue;
			}
			/*
			 * The conflicting block has either IO in progress or is 
			 * Dirty. In all cases, we need to add ourselves to the 
			 * pending queue. Then if the block is dirty, we kick off
			 * an IO to clean the block. 
			 * Note that if the block is dirty and IO is in progress
			 * on it, the do_pending handler will clean the block
			 * and then process the pending queue.
			 */
			flashcache_enq_pending(dmc, bio, i, DISCARD, pjob);
			return 1;
		}
	}
	return 0;
}

/* 
 * Since md will break up IO into blocksize pieces, we only really need to check 
 * the start set and the end set for overlaps.
 */
static int
flashcache_discard_blocks(struct disk_slot *slot, struct bio *bio)
{	
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	sector_t io_start = bio->bi_sector;
	sector_t io_end = bio->bi_sector + (to_sector(bio->bi_size) - 1);
	int start_set, end_set;
	int queued;
	struct pending_job *pjob1, *pjob2;

	pjob1 = flashcache_alloc_pending_job(dmc);
	if (unlikely(dmc->sysctl_error_inject & INVAL_PENDING_JOB_ALLOC_FAIL)) {
		if (pjob1) {
			flashcache_free_pending_job(pjob1);
			pjob1 = NULL;
		}
		dmc->sysctl_error_inject &= ~INVAL_PENDING_JOB_ALLOC_FAIL;
	}
	if (pjob1 == NULL) {
		queued = -ENOMEM;
		goto out;
	}
	pjob2 = flashcache_alloc_pending_job(dmc);
	if (pjob2 == NULL) {
		flashcache_free_pending_job(pjob1);
		queued = -ENOMEM;
		goto out;
	}
	start_set = hash_block(dmc, io_start);
	end_set = hash_block(dmc, io_end);
	queued = flashcache_discard_block_set(slot, start_set, bio, 
					    bio_data_dir(bio), pjob1);
	if (queued) {
		flashcache_free_pending_job(pjob2);
		goto out;
	} else
		flashcache_free_pending_job(pjob1);		
	if (start_set != end_set) {
		queued = flashcache_discard_block_set(slot, end_set, 
						    bio, bio_data_dir(bio), pjob2);
		if (!queued)
			flashcache_free_pending_job(pjob2);
	} else
		flashcache_free_pending_job(pjob2);		
out:
	return queued;
}

static void
flashcache_write_miss(int partial, struct disk_slot *slot, struct bio *bio, int index)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	struct cacheblock *cacheblk;
	struct kcached_job *job;
	int queued;
	int16_t         offset; /* offset within block in sector */
        int16_t         size;   /* size in sector */

	cacheblk = &dmc->cache[index];
	queued = flashcache_inval_blocks(slot, bio);
	if (queued) {
		if (unlikely(queued < 0))
			flashcache_bio_endio(bio, -EIO, dmc, NULL);
		spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
		return;
	}
	if (cacheblk->cache_state & VALID) {
		dmc->flashcache_stats.wr_replace++;
		slot->flashcache_stats.wr_replace++;
		dmc->slot[dmc->cache[index].slot_id].cached_blocks--;
	} else {
		dmc->cached_blocks++;
	}
	slot->cached_blocks++;

	cacheblk->slot_id = slot->id;
	cacheblk->dbn = bio->bi_sector & ~((sector_t)dmc->block_size-1);
	if (partial) {
		cacheblk->cache_state = VALID | PVALID | CACHEWRITEINPROG;
#ifdef PARTIAL_BITMAP
		offset = flashcache_bio_start_sector(bio, cacheblk);
                size = to_sector(bio->bi_size);
#ifdef BITMAP_ARRAY
		to_bitmap64_array((void*)&cacheblk->bitmap, offset, size);
#else
                cacheblk->bitmap = TO_BITMAP(offset, size);
#endif
#endif
	} else {
		cacheblk->cache_state = VALID | CACHEWRITEINPROG;
#ifdef PARTIAL_BITMAP
#ifdef BITMAP_ARRAY
		bitmap64_array_set_mask((void*)&cacheblk->bitmap);
#else
		cacheblk->bitmap = BLOCK_MASK;
#endif
#endif
	}

#ifdef PARTIAL_BITMAP
#ifdef BITMAP_ARRAY
	bitmap64_array_copy((void*)&cacheblk->io_bitmap, (void*)&cacheblk->bitmap);
#else
	cacheblk->io_bitmap = cacheblk->bitmap;
#endif
#else
	cacheblk->offset = flashcache_bio_start_sector(bio, cacheblk);
	cacheblk->size = to_sector(bio->bi_size);

	cacheblk->io_offset = cacheblk->offset;
	cacheblk->io_size = cacheblk->size;
#endif

	cacheblk->nr_concurrent = 1;
	spin_unlock_irq(&dmc->cache_spin_lock);
	LOCK_END

	DPRINTK("Cache write: Disk %d Block %llu(%lu), index = %d:%s%s%s nr_concurrent %d",
		slot->id, bio->bi_sector, bio->bi_size, index, "CACHE", partial?" Partial ":" ", "MISS & REPLACE", cacheblk->nr_concurrent);

	job = new_kcached_job(slot, bio, index);
	if (unlikely(dmc->sysctl_error_inject & WRITE_MISS_JOB_ALLOC_FAIL)) {
		if (job)
			flashcache_free_cache_job(job);
		job = NULL;
		dmc->sysctl_error_inject &= ~WRITE_MISS_JOB_ALLOC_FAIL;
	}
	if (unlikely(job == NULL)) {
		/* 
		 * We have a write miss, and can't allocate a job.
		 * Since we dropped the spinlock, we have to drain any 
		 * pending jobs.
		 */
		DMERR("flashcache: Write (miss) failed ! Can't allocate memory for cache IO, block %lu", 
		      cacheblk->dbn);
		flashcache_bio_endio(bio, -EIO, dmc, NULL);
		LOCK_START
		spin_lock_irq(&dmc->cache_spin_lock);
		dmc->cached_blocks--;
		slot->cached_blocks--;
		cacheblk->cache_state &= ~(VALID | PVALID);
		cacheblk->cache_state |= INVALID;
		flashcache_free_pending_jobs(dmc, cacheblk, -EIO);
		cacheblk->nr_concurrent = 0;
		cacheblk->cache_state &= ~(BLOCK_IO_INPROG);
		spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
	} else {
		atomic_inc(&dmc->nr_jobs);
		dmc->flashcache_stats.ssd_writes++;
		job->action = WRITECACHE; 
		{
			VERIFY(dmc->cache_mode == FLASHCACHE_WRITE_THROUGH);
			/* Write data to both disk and cache */
			dmc->flashcache_stats.disk_writes++;
			dm_io_async_bvec(2, 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
					 (struct io_region *)&job->job_io_regions, 
#else
					 (struct dm_io_region *)&job->job_io_regions, 
#endif
					 WRITE, 
					 bio->bi_io_vec + bio->bi_idx,
					 flashcache_io_callback, job);
		}
	}
}

#ifndef PARTIAL_BITMAP
static void
flashcache_write_hit(int partial, struct disk_slot *slot, struct bio *bio, int index)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	struct cacheblock *cacheblk;
	struct pending_job *pjob;
	struct kcached_job *job;

	int16_t offset=-1;
	int16_t size=-1;
	int16_t io_offset=-1;
	int16_t io_size=-1;
	u_int16_t       cache_state_r=0;
	int can_concurrent = 0;

	cacheblk = &dmc->cache[index];

	if (cacheblk->cache_state & PVALID) {
		if (partial) {
			SPRINTK("Cache write P->P: Disk %d Block %llu(%lu), index = %d",
					slot->id, bio->bi_sector, bio->bi_size, index);
			/* partial cache hit, but all hit 1
				bio size < cache size
			  		|<---cache--->|
 			                |<---bio--->|

			   partial cache hit, but all hit 2
				bio size < cache size
			  		|<---cache--->|
    		        	         |<---bio--->|

			   partial cache hit, but all hit 3
				bio size < cache size
			  		|<---cache--->|
	    		                  |<---bio--->|
			*/
			if ((cache_start_sector(cacheblk)<=flashcache_bio_start_sector(bio, cacheblk)) && (flashcache_bio_end_sector(bio, cacheblk)<=cache_end_sector(cacheblk))) {
				offset = cacheblk->offset;
				size = cacheblk->size;
				SPRINTK("hit 123 %d %d", offset, size);
			}

			/* cache replace 1
			  		|<---cache--->|
			  |<---bio--->|
			*/
			else if (flashcache_bio_end_sector(bio, cacheblk) < cache_start_sector(cacheblk)) {
				offset = flashcache_bio_start_sector(bio, cacheblk);
				size = to_sector(bio->bi_size);
				SPRINTK("replace 1 %d %d", offset, size);
			}

			/* cache merge 1
			  		|<---cache--->|
	   		    |<---bio--->|
			*/
			else if (flashcache_bio_end_sector(bio, cacheblk) == cache_start_sector(cacheblk)) {
				offset = flashcache_bio_start_sector(bio, cacheblk);
				size = cacheblk->size + to_sector(bio->bi_size);
				if (size == dmc->block_size) {
					cache_state_r = PVALID;
					SPRINTK("merge to cache block");
				}

				if (flashcache_bio_end_sector(bio, cacheblk) == cacheio_start_sector(cacheblk)) {
					io_offset = flashcache_bio_start_sector(bio, cacheblk);
					io_size = cacheblk->io_size + to_sector(bio->bi_size);
					can_concurrent = 1;
				}
				SPRINTK("merge 1 %d %d can_concurrent %d", offset, size, can_concurrent);
			}

			/* cache merge 2
			  		|<---cache--->|
    			       |<---bio--->|
			*/
			else if (flashcache_bio_start_sector(bio, cacheblk) < cache_start_sector(cacheblk) &&
				flashcache_bio_end_sector(bio, cacheblk) > cache_start_sector(cacheblk) &&
				flashcache_bio_end_sector(bio, cacheblk) < cache_end_sector(cacheblk) ) {
				offset = flashcache_bio_start_sector(bio, cacheblk);
				size = cache_end_sector(cacheblk) - flashcache_bio_start_sector(bio, cacheblk);
				if (size == dmc->block_size) {
					cache_state_r = PVALID;
					SPRINTK("merge to cache block");
				}
				SPRINTK("merge 2 %d %d", offset, size);
			}

			/* cache replace 2
				bio size > cache size
		  			|<---cache--->|
	  		           |<---bio---------->|

			   cache replace 3
				bio size > cache size
		  			|<---cache--->|
	    		             |<---bio---------->|

			   cache replace 4
				bio size > cache size
			  		|<---cache--->|
	    		                |<---bio---------->|
			*/
			else if ((flashcache_bio_start_sector(bio, cacheblk)<=cache_start_sector(cacheblk)) && (cache_end_sector(cacheblk)<=flashcache_bio_end_sector(bio, cacheblk))) {
				offset = flashcache_bio_start_sector(bio, cacheblk);
				size = to_sector(bio->bi_size);
				SPRINTK("replace 234 %d %d", offset, size);
			}

			/* cache merge 3
			  		|<---cache--->|
      		        	              |<---bio--->|
			*/
			else if (cache_start_sector(cacheblk) < flashcache_bio_start_sector(bio, cacheblk) &&
				cache_end_sector(cacheblk) > flashcache_bio_start_sector(bio, cacheblk) &&
				cache_end_sector(cacheblk) < flashcache_bio_end_sector(bio, cacheblk) ) {
				offset = cache_start_sector(cacheblk);
				size = flashcache_bio_end_sector(bio, cacheblk) - cache_start_sector(cacheblk);
				if (size == dmc->block_size) {
					cache_state_r = PVALID;
					SPRINTK("merge to cache block");
				}
				SPRINTK("merge 3 %d %d", offset, size);
			}

			/* cache merge 4
			  		|<---cache--->|
      			                              |<---bio--->|
			*/
			else if (cache_end_sector(cacheblk) == flashcache_bio_start_sector(bio, cacheblk)) {
				offset = cache_start_sector(cacheblk);
				size = cacheblk->size + to_sector(bio->bi_size);
				if (size == dmc->block_size) {
					cache_state_r = PVALID;
					SPRINTK("merge to cache block");
				}

				if (flashcache_bio_start_sector(bio, cacheblk) == cacheio_end_sector(cacheblk)) {
					io_offset = cacheio_start_sector(cacheblk);
					io_size = cacheblk->io_size + to_sector(bio->bi_size);
					can_concurrent = 1;
				}
				SPRINTK("merge 4 %d %d can_concurrent %d", offset, size, can_concurrent);
			}

			/* cache replace 5
			  		|<---cache--->|
	      		                                |<---bio--->|
			*/
			else if (cache_end_sector(cacheblk) < flashcache_bio_start_sector(bio, cacheblk)) {
				offset = flashcache_bio_start_sector(bio, cacheblk);
				size = to_sector(bio->bi_size);
				SPRINTK("replace 5 %d %d", offset, size);
			} else {
				printk("ouch! impossible!!!!!!!\n");
			}
		} else {
			SPRINTK("Cache write P->V: Disk %d Block %llu(%lu), index = %d",
					slot->id, bio->bi_sector, bio->bi_size, index);

			offset = 0;
			size = dmc->block_size;
			cache_state_r = PVALID;

			dmc->flashcache_stats.write_phits++;
			/* cacheblk->dbn no change */
		}
	} else { /* cacheblk->cache_state & VALID */
		if (partial) {
			SPRINTK("Cache write V->P: Disk %d Block %llu(%lu), index = %d",
				slot->id, bio->bi_sector, bio->bi_size, index);
			dmc->flashcache_stats.write_phits++;

			/* io merge 1
			  		|<---cacheio--->|
	   		    |<---bio--->|
			*/
			if (flashcache_bio_end_sector(bio, cacheblk) == cacheio_start_sector(cacheblk)) {
				io_offset = flashcache_bio_start_sector(bio, cacheblk);
				io_size = cacheblk->io_size + to_sector(bio->bi_size);
				can_concurrent = 1;
				SPRINTK("io merge 1 %d %d can_concurrent %d", offset, size, can_concurrent);
			}
			/* io merge 4
			  		|<---cacheio--->|
      			                                |<---bio--->|
			*/
			else if (flashcache_bio_start_sector(bio, cacheblk) == cacheio_end_sector(cacheblk)) {
				io_offset = cacheio_start_sector(cacheblk);
				io_size = cacheblk->io_size + to_sector(bio->bi_size);
				can_concurrent = 1;
				SPRINTK("io merge 4 %d %d can_concurrent %d", offset, size, can_concurrent);
			}
		} else {
			SPRINTK("Cache write V->V: Disk %d Block %llu(%lu), index = %d",
				slot->id, bio->bi_sector, bio->bi_size, index);
//			dmc->flashcache_stats.write_hits++;
		}
	}

	
	if ((cacheblk->nr_queued == 0) && (can_concurrent || !(cacheblk->cache_state & BLOCK_IO_INPROG))) {
		cacheblk->offset = offset;
		cacheblk->size = size;
		cacheblk->cache_state &= ~cache_state_r;
		cacheblk->cache_state |= CACHEWRITEINPROG;
		dmc->flashcache_stats.raw_write_hits++;
		dmc->flashcache_stats.write_hits++;
		slot->flashcache_stats.write_hits++;

		if (!cacheblk->nr_concurrent) {
			cacheblk->io_offset = flashcache_bio_start_sector(bio, cacheblk);
			cacheblk->io_size = to_sector(bio->bi_size);
		} else if (can_concurrent) {
			cacheblk->io_offset = io_offset;
			cacheblk->io_size = io_size;
		}
		cacheblk->nr_concurrent++;

		spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
		DPRINTK("Cache write: Disk %d Block %llu(%lu), index = %d:%s%s%s %d %d nr_concurrent %d",
			slot->id, bio->bi_sector, bio->bi_size, index, "CACHE", partial?" PARTIAL ":" ", "HIT", cacheblk->io_offset, cacheblk->io_size, cacheblk->nr_concurrent);

		job = new_kcached_job(slot, bio, index);
		if (unlikely(dmc->sysctl_error_inject & WRITE_HIT_JOB_ALLOC_FAIL)) {
			if (job)
				flashcache_free_cache_job(job);
			job = NULL;
			dmc->sysctl_error_inject &= ~WRITE_HIT_JOB_ALLOC_FAIL;
		}
		if (unlikely(job == NULL)) {
			/* 
			 * We have a write hit, and can't allocate a job.
			 * Since we dropped the spinlock, we have to drain any 
			 * pending jobs.
			 */
			DMERR("flashcache: Write (hit) failed ! Can't allocate memory for cache IO, block %lu", 
			      cacheblk->dbn);
			flashcache_bio_endio(bio, -EIO, dmc, NULL);
			LOCK_START
			spin_lock_irq(&dmc->cache_spin_lock);
			flashcache_free_pending_jobs(dmc, cacheblk, -EIO);
			cacheblk->nr_concurrent--;
			if (cacheblk->nr_concurrent)
                                cacheblk->cache_state |= IN_ERROR;
			if (!cacheblk->nr_concurrent)
				cacheblk->cache_state &= ~(BLOCK_IO_INPROG);
			spin_unlock_irq(&dmc->cache_spin_lock);
			LOCK_END
		} else {
			atomic_inc(&dmc->nr_jobs);
			dmc->flashcache_stats.ssd_writes++;
			job->action = WRITECACHE;
			{
				VERIFY(dmc->cache_mode == FLASHCACHE_WRITE_THROUGH);
				/* Write data to both disk and cache */
				dmc->flashcache_stats.disk_writes++;
				dm_io_async_bvec(2, 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
						 (struct io_region *)&job->job_io_regions, 
#else
						 (struct dm_io_region *)&job->job_io_regions, 
#endif
						 WRITE, 
						 bio->bi_io_vec + bio->bi_idx,
						 flashcache_io_callback, job);				
			}
		}
	} else {
		pjob = flashcache_alloc_pending_job(dmc);
		pjob->partial = partial;
		if (unlikely(dmc->sysctl_error_inject & WRITE_HIT_PENDING_JOB_ALLOC_FAIL)) {
			if (pjob) {
				flashcache_free_pending_job(pjob);
				pjob = NULL;
			}
			dmc->sysctl_error_inject &= ~WRITE_HIT_PENDING_JOB_ALLOC_FAIL;
		}
		if (unlikely(pjob == NULL))
			flashcache_bio_endio(bio, -EIO, dmc, NULL);
		else
			flashcache_enq_pending(dmc, bio, index, WRITECACHE, pjob);
		spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
	}
}

#else

static void
flashcache_write_hit(int partial, struct disk_slot *slot, struct bio *bio, int index)
{
        struct cache_c *dmc = (struct cache_c *) slot->dmc;
        struct cacheblock *cacheblk;
        struct pending_job *pjob;
        struct kcached_job *job;

        int16_t offset=-1;
        int16_t size=-1;
        u_int16_t       cache_state_r=0;
        int can_concurrent = 0;
#ifdef BITMAP_ARRAY
        u_int64_t       bitmap[BITMAP_COUNT], io_bitmap[BITMAP_COUNT], tmp_bitmap[BITMAP_COUNT];
	memset(io_bitmap, 0, sizeof(io_bitmap));
#else
        u_int64_t       bitmap, io_bitmap=0, tmp_bitmap;
#endif

        cacheblk = &dmc->cache[index];

        /* If block is error or busy, queue IO pending completion of in-progress IO */
        if (!(cacheblk->cache_state&IN_ERROR) && (cacheblk->nr_queued == 0)) {
 	       if (cacheblk->cache_state & PVALID) {
        	        if (partial) {
                	        SPRINTK("Cache write P->P: Disk %d Block %llu(%lu), index = %d",
         	                		slot->id, bio->bi_sector, bio->bi_size, index);

	                        offset = flashcache_bio_start_sector(bio, cacheblk);
        	                size = to_sector(bio->bi_size);
#ifdef BITMAP_ARRAY
				to_bitmap64_array((void*)&tmp_bitmap, offset, size);

				bitmap64_array_add((void*)&cacheblk->bitmap, (void*)&tmp_bitmap, (void*)&bitmap);

	                        if (bitmap64_array_check_mask((void*)&bitmap)) {
        	                        cache_state_r = PVALID;
                	                SPRINTK("merge to cache block");
                        	}

	                        if (bitmap64_array_notcontain((void*)&cacheblk->io_bitmap, (void*)&tmp_bitmap)) {
        	                        bitmap64_array_add((void*)&cacheblk->io_bitmap, (void*)&tmp_bitmap, (void*)&io_bitmap);
                	                can_concurrent = 1;
	                        } /* else goto pending io */
#else
        	                tmp_bitmap = TO_BITMAP(offset, size);

	                        bitmap = BITMAP_ADD(cacheblk->bitmap, tmp_bitmap);

        	                if (bitmap == BLOCK_MASK) {
                	                cache_state_r = PVALID;
	                                SPRINTK("merge to cache block");
        	                }

	                        if (BITMAP_NOTCONTAIN(cacheblk->io_bitmap, tmp_bitmap)) {
        	                        io_bitmap = BITMAP_ADD(cacheblk->io_bitmap, tmp_bitmap);
                	                can_concurrent = 1;
                        	} /* else goto pending io */
#endif
	                } else {
        	                SPRINTK("Cache write P->V: Disk %d Block %llu(%lu), index = %d",
                		         	slot->id, bio->bi_sector, bio->bi_size, index);

#ifdef BITMAP_ARRAY
				bitmap64_array_set_mask((void*)&bitmap);
				bitmap64_array_set_mask((void*)&io_bitmap);
#else
	                        bitmap = BLOCK_MASK;
        	                io_bitmap = BLOCK_MASK;
#endif
                	        cache_state_r = PVALID;

	                        dmc->flashcache_stats.write_phits++;
        	                slot->flashcache_stats.write_phits++;
                	        /* cacheblk->dbn no change */
	                }
        	} else { /* cacheblk->cache_state & VALID */
#ifdef BITMAP_ARRAY
			bitmap64_array_set_mask((void*)&bitmap);
#else
			bitmap = BLOCK_MASK;
#endif
			if (partial) {
        	                SPRINTK("Cache write V->P: Disk %d Block %llu(%lu), index = %d",
	                	                slot->id, bio->bi_sector, bio->bi_size, index);
	                        dmc->flashcache_stats.write_phits++;
        	                slot->flashcache_stats.write_phits++;

	                        offset = flashcache_bio_start_sector(bio, cacheblk);
        	                size = to_sector(bio->bi_size);
#ifdef BITMAP_ARRAY
				to_bitmap64_array((void*)&tmp_bitmap, offset, size);

				if (bitmap64_array_notcontain((void*)&cacheblk->io_bitmap, (void*)&tmp_bitmap)) {
					bitmap64_array_add((void*)&cacheblk->io_bitmap, (void*)&tmp_bitmap, (void*)&io_bitmap);
                        	        can_concurrent = 1;
	                        } /* else goto pending io */
#else
        	                tmp_bitmap = TO_BITMAP(offset, size);

                	        if (BITMAP_NOTCONTAIN(cacheblk->io_bitmap, tmp_bitmap)) {
	                                io_bitmap = BITMAP_ADD(cacheblk->io_bitmap, tmp_bitmap);
        	                        can_concurrent = 1;
	                        } /* else goto pending io */
#endif
        	        } else {
	                        SPRINTK("Cache write V->V: Disk %d Block %llu(%lu), index = %d",
        		                        slot->id, bio->bi_sector, bio->bi_size, index);
//              	        dmc->flashcache_stats.write_hits++;

#ifdef BITMAP_ARRAY
				bitmap64_array_set_mask((void*)&io_bitmap);
#else
        	                io_bitmap = BLOCK_MASK;
#endif
	                }
        	}
	} else
		goto queue_job;

	if (can_concurrent || !(cacheblk->cache_state & BLOCK_IO_INPROG)) {
#ifdef BITMAP_ARRAY
		bitmap64_array_copy((void*)&cacheblk->bitmap, (void*)&bitmap);
#else
                cacheblk->bitmap = bitmap;
#endif
                cacheblk->cache_state &= ~cache_state_r;
                cacheblk->cache_state |= CACHEWRITEINPROG;
                dmc->flashcache_stats.raw_write_hits++;
                dmc->flashcache_stats.write_hits++;
                slot->flashcache_stats.write_hits++;

                if (!cacheblk->nr_concurrent) {
                        offset = flashcache_bio_start_sector(bio, cacheblk);
                        size = to_sector(bio->bi_size);
#ifdef BITMAP_ARRAY
			to_bitmap64_array((void*)&cacheblk->io_bitmap, offset, size);
#else
                        cacheblk->io_bitmap = TO_BITMAP(offset, size);
#endif
                } else if (can_concurrent) {
#ifdef BITMAP_ARRAY
			bitmap64_array_copy((void*)&cacheblk->io_bitmap, (void*)&io_bitmap);
#else
                        cacheblk->io_bitmap = io_bitmap;
#endif
                }
                cacheblk->nr_concurrent++;

                spin_unlock_irq(&dmc->cache_spin_lock);
                LOCK_END
                DPRINTK("Cache write Disk %d Block %llu(%lu), index = %d:%s%s%s nr_concurrent %d",
                        slot->id, bio->bi_sector, bio->bi_size, index, "CACHE", partial?" PARTIAL ":" ", "HIT", cacheblk->nr_concurrent);

//              char tmp[72];
//              char tmp1[72];
//              DPRINTK("Cache write: Disk %d Block %llu(%lu), index = %d:%s%s%s %s %s nr_concurrent %d",
//                      slot->id, bio->bi_sector, bio->bi_size, index, "CACHE", partial?" PARTIAL ":" ", "HIT", printk_binary(cacheblk->bitmap, tmp),
//                      printk_binary(cacheblk->io_bitmap, tmp1), cacheblk->nr_concurrent);

                job = new_kcached_job(slot, bio, index);
                if (unlikely(dmc->sysctl_error_inject & WRITE_HIT_JOB_ALLOC_FAIL)) {
                        if (job)
                                flashcache_free_cache_job(job);
                        job = NULL;
                        dmc->sysctl_error_inject &= ~WRITE_HIT_JOB_ALLOC_FAIL;
                }
                if (unlikely(job == NULL)) {
                        /*
                         * We have a write hit, and can't allocate a job.
                         * Since we dropped the spinlock, we have to drain any
                         * pending jobs.
                         */
                        DMERR("flashcache: Slot %d Write (hit) failed ! Can't allocate memory for cache IO, index %d block %lu nr_concurrent %d",
                              slot->id, index, cacheblk->dbn, cacheblk->nr_concurrent);
                        flashcache_bio_endio(bio, -EIO, dmc, NULL);
                        LOCK_START
                        spin_lock_irq(&dmc->cache_spin_lock);
                        flashcache_free_pending_jobs(dmc, cacheblk, -EIO);
                        cacheblk->nr_concurrent--;
			if (cacheblk->nr_concurrent)
                                cacheblk->cache_state |= IN_ERROR;
                        if (!cacheblk->nr_concurrent) {
				dmc->cached_blocks--;
                                cacheblk->cache_state &= ~(BLOCK_IO_INPROG);
				/* since io error and no more concurrent job, invalid it */
				cacheblk->cache_state &= ~(VALID | PVALID);
				cacheblk->cache_state |= INVALID;
			}
                        spin_unlock_irq(&dmc->cache_spin_lock);
                        LOCK_END
                } else {
                        atomic_inc(&dmc->nr_jobs);
                        dmc->flashcache_stats.ssd_writes++;
                        job->action = WRITECACHE;
                        {
                                VERIFY(dmc->cache_mode == FLASHCACHE_WRITE_THROUGH);
                                /* Write data to both disk and cache */
                                dmc->flashcache_stats.disk_writes++;
                                dm_io_async_bvec(2,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
                                                 (struct io_region *)&job->job_io_regions,
#else
                                                 (struct dm_io_region *)&job->job_io_regions,
#endif
                                                 WRITE,
                                                 bio->bi_io_vec + bio->bi_idx,
                                                 flashcache_io_callback, job);
                        }
                }
        } else {
queue_job:
                pjob = flashcache_alloc_pending_job(dmc);
                pjob->partial = partial;
                if (unlikely(dmc->sysctl_error_inject & WRITE_HIT_PENDING_JOB_ALLOC_FAIL)) {
                        if (pjob) {
                                flashcache_free_pending_job(pjob);
                                pjob = NULL;
                        }
                        dmc->sysctl_error_inject &= ~WRITE_HIT_PENDING_JOB_ALLOC_FAIL;
                }
                if (unlikely(pjob == NULL))
                        flashcache_bio_endio(bio, -EIO, dmc, NULL);
                else
                        flashcache_enq_pending(dmc, bio, index, WRITECACHE, pjob);
                spin_unlock_irq(&dmc->cache_spin_lock);
                LOCK_END
        }
}
#endif	/* end of PARTIAL_BITMAP define */

static void
flashcache_write(struct disk_slot *slot, struct bio *bio)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;

	int index;
	int res;
	int queued;
	
	LOCK_START
	spin_lock_irq(&dmc->cache_spin_lock);
	res = flashcache_lookup(slot, bio, &index);
	if (res != -1) {
		if (res == VALID) {
			/* Cache Hit */
			flashcache_write_hit(0, slot, bio, index);
		} else if (res == PVALID) {
			/* Partial Cache Hit */
			flashcache_write_hit(1, slot, bio, index);
		} else {
			/* Cache Miss, found block to recycle */
			if (to_sector(bio->bi_size) != dmc->block_size)
				flashcache_write_miss(1, slot, bio, index);
			else
				flashcache_write_miss(0, slot, bio, index);
		}
		return;
	}
	/*
	 * No room in the set. We cannot write to the cache and have to 
	 * send the request to disk. Before we do that, we must check 
	 * for potential invalidations !
	 */
	queued = flashcache_inval_blocks(slot, bio);
	spin_unlock_irq(&dmc->cache_spin_lock);
	LOCK_END
	if (queued) {
		if (unlikely(queued < 0))
			flashcache_bio_endio(bio, -EIO, dmc, NULL);
		return;
	}
	/* Start uncached IO */
	flashcache_start_uncached_io(slot, bio);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
#define bio_barrier(bio)        ((bio)->bi_rw & (1 << BIO_RW_BARRIER))
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
#define bio_barrier(bio)        ((bio)->bi_rw & REQ_HARDBARRIER)
#else
#define bio_barrier(bio)        ((bio)->bi_rw & REQ_FLUSH)
#endif
#endif
#endif

/*
 * Decide the mapping and perform necessary cache operations for a bio request.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
int
flashcache_map(struct dm_target *ti, struct bio *bio)
#else
int 
flashcache_map(struct dm_target *ti, struct bio *bio,
	       union map_info *map_context)
#endif
{
	struct disk_slot *slot = (struct disk_slot *) ti->private;
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	int sectors;
	int queued;
	int i;
	
#if 0
	if (unlikely(bio->bi_rw & REQ_DISCARD)) {
		DPRINTK("REQ_DISCARD for %llu (%u bytes)",
			bio->bi_sector, bio->bi_size);
	}
#endif

	if (unlikely(test_bit(SSD_PURGE, &dmc->SSD_state)))
		wait_event(dmc->plugoutq, !test_bit(SSD_PURGE, &dmc->SSD_state));

	atomic_inc(&dmc->nr_map_jobs);

	if (!test_bit(CACHE_ENABLED, &slot->cache_enabled) || !test_bit(SSD_READY, &dmc->SSD_state)) {
		bio->bi_bdev = slot->disk_dev->bdev;
		atomic_dec(&dmc->nr_map_jobs);
		return DM_MAPIO_REMAPPED;
	}

	VERIFY(bio->bi_size % 512 == 0);

	if (unlikely(bio_barrier(bio))) {
                if (bio->bi_size)
                        dmc->flashcache_errors.verify_errors5++;
                else {
			bio->bi_bdev = slot->disk_dev->bdev;
			atomic_dec(&dmc->nr_map_jobs);
			return DM_MAPIO_REMAPPED;
		}
	}	
	/* zero-sized bio, should be FLUSH */
	else if (unlikely(!bio->bi_size || bio->bi_rw & REQ_QNAP_MAP)) {
		bio->bi_bdev = slot->disk_dev->bdev;
		atomic_dec(&dmc->nr_map_jobs);
		return DM_MAPIO_REMAPPED;
	}

	sectors = to_sector(bio->bi_size);
	if (sectors <= 2048) {
		int s = sectors>>1;
		for(i=0; i<=10; i++) {
			s>>=1;
			if (!s) {
				size_hist[i]++;
				break;
			}
		}
	}

	VERIFY(to_sector(bio->bi_size) <= dmc->block_size);

	if (bio_data_dir(bio) == READ) {
		dmc->flashcache_stats.raw_reads++;
		dmc->flashcache_stats.reads++;
	} else {
		dmc->flashcache_stats.raw_writes++;
		dmc->flashcache_stats.writes++;
	}

	LOCK_START
	spin_lock_irq(&dmc->cache_spin_lock);

#ifdef FLASHCACHE_PID_CONTROL
	if (unlikely(dmc->sysctl_pid_do_expiry && 
		     (dmc->whitelist_head || dmc->blacklist_head)))
		flashcache_pid_expiry_all_locked(dmc);
#endif

#if 0
	if (unlikely(bio->bi_rw & REQ_DISCARD)) {
		dmc->flashcache_stats.discards++;
		slot->flashcache_stats.discards++;
                queued = flashcache_discard_blocks(slot, bio);
                spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
                if (queued) {
                        if (unlikely(queued < 0))
                                flashcache_bio_endio(bio, -EIO, dmc, NULL);
                } else {
                        /* Start discard IO */
                        flashcache_start_discard_io(slot, bio);
                }
	}
#endif
        if ((bio->bi_rw & REQ_QNAP_DIRECT) ||
            (bio->bi_rw & REQ_QNAP_MAP_ZERO) ||
            (bio->bi_rw & REQ_DISCARD)) {
                if (bio->bi_rw & REQ_QNAP_DIRECT)
                        dmc->flashcache_stats.direct++;
                if (bio->bi_rw & REQ_DISCARD) {
                        dmc->flashcache_stats.discards++;
                        slot->flashcache_stats.discards++;
                }
                dmc->flashcache_stats.direct++;
                queued = flashcache_inval_blocks(slot, bio);
                spin_unlock_irq(&dmc->cache_spin_lock);
                LOCK_END
                if (queued) {
                        if (unlikely(queued < 0))
                                flashcache_bio_endio(bio, -EIO, dmc, NULL);
                } else {
                        /* Start uncached IO */
                        flashcache_start_uncached_io(slot, bio);
                }
        }
	/* 1. check read/write sequential bypass first
         * 2. not align to kb, can not to partial cache, go to uncached io
         * 3. write around 
	 */
        else if (((dmc->sysctl_skip_seq_thresh_kb != 0) && flashcache_uncacheable(slot, bio)) ||
		 (bio->bi_sector & 1) ||
        	 (sectors & 1) ||
		 (bio_data_dir(bio) == WRITE && (dmc->cache_mode == FLASHCACHE_WRITE_AROUND))) {
		queued = flashcache_inval_blocks(slot, bio);
		spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
		if (queued) {
			if (unlikely(queued < 0))
				flashcache_bio_endio(bio, -EIO, dmc, NULL);
		} else {
			/* Start uncached IO */
			flashcache_start_uncached_io(slot, bio);
		}
	} else {
		spin_unlock_irq(&dmc->cache_spin_lock);
		LOCK_END
		if (bio_data_dir(bio) == READ)
			flashcache_read(slot, bio);
		else
			flashcache_write(slot, bio);
	}

	atomic_dec(&dmc->nr_map_jobs);
	return DM_MAPIO_SUBMITTED;
}

/*
 * We handle uncached IOs ourselves to deal with the problem of out of ordered
 * IOs corrupting the cache. Consider the case where we get 2 concurent IOs
 * for the same block Write-Read (or a Write-Write). Consider the case where
 * the first Write is uncacheable and the second IO is cacheable. If the 
 * 2 IOs are out-of-ordered below flashcache, then we will cache inconsistent
 * data in flashcache (persistently).
 * 
 * We do invalidations before launching uncacheable IOs to disk. But in case
 * of out of ordering the invalidations before launching the IOs does not help.
 * We need to invalidate after the IO completes.
 * 
 * Doing invalidations after the completion of an uncacheable IO will cause 
 * any overlapping dirty blocks in the cache to be written out and the IO 
 * relaunched. If the overlapping blocks are busy, the IO is relaunched to 
 * disk also (post invalidation). In these 2 cases, we will end up sending
 * 2 disk IOs for the block. But this is a rare case.
 * 
 * When 2 IOs for the same block are sent down (by un co-operating processes)
 * the storage stack is allowed to re-order the IOs at will. So the applications
 * cannot expect any ordering at all.
 * 
 * What we try to avoid here is inconsistencies between disk and the ssd cache.
 */
void 
flashcache_uncached_io_complete(struct kcached_job *job)
{
	struct disk_slot *slot = (struct disk_slot *) job->slot;
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
//	struct cache_c *dmc = job->slot->dmc;
	unsigned long flags;
	int queued;
	int error = job->error;

	if (unlikely(job->bio->bi_rw & REQ_DISCARD))
		JPRINTK("flashcache_discard_io_complete: %lu %lu", job->bio->bi_sector, job->bio->bi_size);
	else
		JPRINTK("flashcache_uncached_io_complete: %lu %lu", job->bio->bi_sector, job->bio->bi_size);

	if (unlikely(error)) {
		DMERR("flashcache uncached disk IO error: io error %d block %lu R/w %s", 
		      error, job->job_io_regions.disk.sector, 
		      (bio_data_dir(job->bio) == WRITE) ? "WRITE" : "READ");
		if (unlikely(job->bio->bi_rw & REQ_DISCARD)) {
			dmc->flashcache_errors.discard_errors++;
			slot->flashcache_errors.discard_errors++;
		} else if (bio_data_dir(job->bio) == WRITE) {
			dmc->flashcache_errors.disk_write_errors++;
			slot->flashcache_errors.disk_write_errors++;
		} else {
			dmc->flashcache_errors.disk_read_errors++;
			slot->flashcache_errors.disk_read_errors++;
		}
	}
	LOCK_START
	spin_lock_irqsave(&dmc->cache_spin_lock, flags);
	if (unlikely(job->bio->bi_rw & REQ_DISCARD))
		queued = flashcache_discard_blocks(job->slot, job->bio);
	else
		queued = flashcache_inval_blocks(job->slot, job->bio);
	spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
	LOCK_END
	if (job->index != -1) {
		JPRINTK("index %d state 0x%x ", job->index, dmc->cache[job->index].cache_state);
	}

	if (queued) {
		if (unlikely(queued < 0))
			flashcache_bio_endio(job->bio, -EIO, dmc, NULL);
		/* 
		 * The IO will be re-executed.
		 * The do_pending logic will re-launch the 
		 * disk IO post-invalidation calling start_uncached_io.
		 * This should be a rare occurrence.
		 */
		dmc->flashcache_stats.uncached_io_requeue++;
	} else {
		flashcache_bio_endio(job->bio, error, dmc, &job->io_start_time);
	}
	flashcache_free_cache_job(job);
	if (atomic_dec_and_test(&dmc->nr_jobs))
		wake_up(&dmc->destroyq);
}

static void 
flashcache_uncached_io_callback(unsigned long error, void *context)
{
	struct kcached_job *job = (struct kcached_job *) context;

	VERIFY(job->index == -1);

	if (unlikely(error))
		job->error = -EIO;
	else
		job->error = 0;
	push_uncached_io_complete(job);
	schedule_work(&_kcached_wq);
}

static void
flashcache_start_uncached_io(struct disk_slot *slot, struct bio *bio)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	int is_write = (bio_data_dir(bio) == WRITE);
	struct kcached_job *job;
	
	DPRINTK("flashcache_start_uncached_io for %llu (%u bytes)",
		bio->bi_sector, bio->bi_size);

	if (is_write) {
		dmc->flashcache_stats.uncached_writes++;
		dmc->flashcache_stats.disk_writes++;
	} else {
		dmc->flashcache_stats.uncached_reads++;
		dmc->flashcache_stats.disk_reads++;
	}
	job = new_kcached_job(slot, bio, -1);
	if (unlikely(job == NULL)) {
		flashcache_bio_endio(bio, -EIO, dmc, NULL);
		return;
	}
	atomic_inc(&dmc->nr_jobs);
	dm_io_async_bvec(1, &job->job_io_regions.disk,
			 bio->bi_rw, 
			 bio->bi_io_vec + bio->bi_idx,
			 flashcache_uncached_io_callback, job);
}

static void
flashcache_start_discard_io(struct disk_slot *slot, struct bio *bio)
{
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	int is_write = (bio_data_dir(bio) == WRITE);
	struct kcached_job *job;
	
	DPRINTK("flashcache_start_discard_io for %llu (%u bytes)",
		bio->bi_sector, bio->bi_size);

	if (is_write) {
		dmc->flashcache_stats.uncached_writes++;
		dmc->flashcache_stats.disk_writes++;
	} else {
		dmc->flashcache_stats.uncached_reads++;
		dmc->flashcache_stats.disk_reads++;
	}
	job = new_kcached_job(slot, bio, -1);
	if (unlikely(job == NULL)) {
		flashcache_bio_endio(bio, -EIO, dmc, NULL);
		return;
	}
	atomic_inc(&dmc->nr_jobs);
/*
 * bio->bi_rw should be REQ_WRITE | REQ_DISCARD
 * bi_rw is set in blkdev_issue_discard
*/
	dm_io_async_bvec(1, &job->job_io_regions.disk,
			 bio->bi_rw, 
			 bio->bi_io_vec + bio->bi_idx,
			 flashcache_uncached_io_callback, job);
}

EXPORT_SYMBOL(flashcache_io_callback);
EXPORT_SYMBOL(flashcache_do_pending_error);
EXPORT_SYMBOL(flashcache_do_pending_noerror);
EXPORT_SYMBOL(flashcache_do_pending);
EXPORT_SYMBOL(flashcache_do_io);
EXPORT_SYMBOL(flashcache_map);
EXPORT_SYMBOL(flashcache_write);
EXPORT_SYMBOL(flashcache_inval_blocks);
EXPORT_SYMBOL(flashcache_inval_block_set);
EXPORT_SYMBOL(flashcache_discard_blocks);
EXPORT_SYMBOL(flashcache_discard_block_set);
EXPORT_SYMBOL(flashcache_read);
EXPORT_SYMBOL(flashcache_read_miss);
EXPORT_SYMBOL(flashcache_lookup);
