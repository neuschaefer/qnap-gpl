/****************************************************************************
 *  flashcache_conf.c
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
#include <linux/reboot.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/vmalloc.h>  /* Benjamin 20131106 for error: implicit declaration of function 'vfree' */
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#include <linux/dm-kcopyd.h>
DECLARE_DM_KCOPYD_THROTTLE_WITH_MODULE_PARM(flashcache_copy_throttle,
	"A percentage of time allocated for copying to and/or from flashcache");
#endif

#endif

#include "flashcache.h"
#include "flashcache_ioctl.h"

struct cache_c *cache_list_head = NULL;
struct work_struct _kcached_wq;
u_int64_t size_hist[11];

struct kmem_cache *_job_cache;
mempool_t *_job_pool;
struct kmem_cache *_pending_job_cache;
mempool_t *_pending_job_pool;

atomic_t nr_cache_jobs;
atomic_t nr_pending_jobs;

extern struct list_head *_pending_jobs;
extern struct list_head *_io_jobs;
extern struct list_head *_md_io_jobs;
extern struct list_head *_md_complete_jobs;

struct flashcache_control_s {
	unsigned long synch_flags;
};

struct flashcache_control_s *flashcache_control;

/* Bit offsets for wait_on_bit_lock() */
#define FLASHCACHE_UPDATE_LIST		0

static int flashcache_notify_reboot(struct notifier_block *this,
				    unsigned long code, void *x);

extern char *flashcache_sw_version;

static int
flashcache_wait_schedule(void *unused)
{
	schedule();
	return 0;
}

static int 
flashcache_jobs_init(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
	_job_cache = kmem_cache_create("kcached-jobs",
	                               sizeof(struct kcached_job),
	                               __alignof__(struct kcached_job),
	                               0, NULL, NULL);
#else
	_job_cache = kmem_cache_create("kcached-jobs",
	                               sizeof(struct kcached_job),
	                               __alignof__(struct kcached_job),
	                               0, NULL);
#endif
	if (!_job_cache)
		return -ENOMEM;

	_job_pool = mempool_create(MIN_JOBS, mempool_alloc_slab,
	                           mempool_free_slab, _job_cache);
	if (!_job_pool) {
		kmem_cache_destroy(_job_cache);
		return -ENOMEM;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
	_pending_job_cache = kmem_cache_create("pending-jobs",
					       sizeof(struct pending_job),
					       __alignof__(struct pending_job),
					       0, NULL, NULL);
#else
	_pending_job_cache = kmem_cache_create("pending-jobs",
					       sizeof(struct pending_job),
					       __alignof__(struct pending_job),
					       0, NULL);
#endif
	if (!_pending_job_cache) {
		mempool_destroy(_job_pool);
		kmem_cache_destroy(_job_cache);
		return -ENOMEM;
	}

	_pending_job_pool = mempool_create(MIN_JOBS, mempool_alloc_slab,
					   mempool_free_slab, _pending_job_cache);
	if (!_pending_job_pool) {
		kmem_cache_destroy(_pending_job_cache);
		mempool_destroy(_job_pool);
		kmem_cache_destroy(_job_cache);
		return -ENOMEM;
	}

	return 0;
}

static void 
flashcache_jobs_exit(void)
{
	VERIFY(flashcache_pending_empty());
	VERIFY(flashcache_io_empty());

	mempool_destroy(_job_pool);
	kmem_cache_destroy(_job_cache);
	_job_pool = NULL;
	_job_cache = NULL;
	mempool_destroy(_pending_job_pool);
	kmem_cache_destroy(_pending_job_cache);
	_pending_job_pool = NULL;
	_pending_job_cache = NULL;
}

static int 
flashcache_kcached_init(struct cache_c *dmc)
{
	init_waitqueue_head(&dmc->destroyq);
	init_waitqueue_head(&dmc->plugoutq);
	atomic_set(&dmc->nr_jobs, 0);
	return 0;
}

static int 
flashcache_writethrough_create(struct cache_c *dmc)
{
	sector_t cache_size, dev_size;
	sector_t order;
	int i;
	
	/* 
	 * Convert size (in sectors) to blocks.
	 * Then round size (in blocks now) down to a multiple of associativity 
	 */
	dmc->size /= dmc->block_size;
	dmc->size = (dmc->size / dmc->assoc) * dmc->assoc;

	/* Check cache size against device size */
	// use SECTOR_SHIFT to replace to_sector, in 32bit kernel, to_sector only deals with 32bit integer well
	dev_size = dmc->cache_dev->bdev->bd_inode->i_size >> SECTOR_SHIFT;
	cache_size = dmc->size * dmc->block_size;
	if (cache_size > dev_size) {
		DMERR("Requested cache size exeeds the cache device's capacity" \
		      "(%llu>%llu)",
  		      cache_size, dev_size);
		return 1;
	}
	order = dmc->size * sizeof(struct cacheblock);
	DMINFO("Allocate %lluKB (%luB per) mem for %llu-entry cache" \
	       "(capacity:%lluMB, associativity:%u, block size:%u " \
	       "sectors(%uKB))",
	       order >> 10, sizeof(struct cacheblock), dmc->size,
	       cache_size >> (20-SECTOR_SHIFT), dmc->assoc, dmc->block_size,
	       dmc->block_size >> (10-SECTOR_SHIFT));
	dmc->cache = (struct cacheblock *)vmalloc(order);
	if (!dmc->cache) {
		DMERR("flashcache_writethrough_create: Unable to allocate cache md");
		return 1;
	}
	/* Initialize the cache structs */
	for (i = 0; i < dmc->size ; i++) {
		dmc->cache[i].dbn = 0;
#ifdef FLASHCACHE_DO_CHECKSUMS
		dmc->cache[i].checksum = 0;
#endif
		dmc->cache[i].cache_state = INVALID;
		dmc->cache[i].nr_queued = 0;
	}
	dmc->md_blocks = 0;
	return 0;
}

static int inline
flashcache_get_dev(struct dm_target *ti, char *pth, struct dm_dev **dmd,
		   char *dmc_dname, sector_t tilen)
{
	int rc;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
	rc = dm_get_device(ti, pth,
			   dm_table_get_mode(ti->table), dmd);
#else
#if defined(RHEL_MAJOR) && RHEL_MAJOR == 6
	rc = dm_get_device(ti, pth,
			   dm_table_get_mode(ti->table), dmd);
#else 
	rc = dm_get_device(ti, pth, 0, tilen,
			   dm_table_get_mode(ti->table), dmd);
#endif
#endif
	if (!rc)
		strncpy(dmc_dname, pth, DEV_PATHLEN);
	return rc;
}

#define HT_RESET_INTERVAL (jiffies+HZ*60)
#define HT_RESET_SHIFT	1
static void ht_reset_timer(unsigned long data)
{
	struct cache_c *dmc = (struct cache_c*)data;
	struct flashcache_stats *stats = &dmc->flashcache_stats;
	unsigned long flags;

	spin_lock_irqsave(&dmc->cache_spin_lock, flags);
	stats->reads >>= HT_RESET_SHIFT;
	stats->read_hits >>= HT_RESET_SHIFT;
	stats->writes >>= HT_RESET_SHIFT;
	stats->write_hits >>= HT_RESET_SHIFT;
	spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);

	mod_timer(&dmc->ht_timer, HT_RESET_INTERVAL);
}

struct cache_c* get_dmc(struct dm_target *ti, char *groupname)
{
	struct cache_c *dmc;
	struct cache_c **nodepp;
	int dmc_first_init = 0;
	int r = -EINVAL;

	dmc = NULL;
	(void)wait_on_bit_lock(&flashcache_control->synch_flags, 
			       FLASHCACHE_UPDATE_LIST,
			       flashcache_wait_schedule, 
			       TASK_UNINTERRUPTIBLE);
	nodepp = &cache_list_head;
	while (*nodepp != NULL) {
		if (strncmp((*nodepp)->groupname, groupname, DEV_PATHLEN) == 0) {
			dmc = *nodepp;
			break;
		}
		nodepp = &((*nodepp)->next_cache);
	}
	clear_bit(FLASHCACHE_UPDATE_LIST, &flashcache_control->synch_flags);
	smp_mb__after_clear_bit();
	wake_up_bit(&flashcache_control->synch_flags, FLASHCACHE_UPDATE_LIST);

	if (dmc == NULL) {
		dmc = kzalloc(sizeof(*dmc), GFP_KERNEL);
		dmc_first_init = 1;
	}

	if (dmc == NULL) {
		ti->error = "flashcache: Failed to allocate cache context";
		r = ENOMEM;
		goto bad;
	}

	if (dmc_first_init) {
		spin_lock_init(&dmc->cache_spin_lock);
		mutex_init(&dmc->hp_mutex);
		
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
		INIT_WORK(&dmc->kerror_wq, do_error, NULL);
#else
		INIT_WORK(&dmc->kerror_wq, do_error);
#endif
		dmc->cache_dev = NULL;
		clear_bit(SSD_READY, &dmc->SSD_state);
		atomic_set(&dmc->nr_map_jobs, 0);
		strncpy(dmc->groupname, groupname, DEV_PATHLEN);

		r = flashcache_kcached_init(dmc);

		if (r) {
			ti->error = "Failed to initialize kcached";
			goto bad;
		}

		(void)wait_on_bit_lock(&flashcache_control->synch_flags, FLASHCACHE_UPDATE_LIST,
			       flashcache_wait_schedule, TASK_UNINTERRUPTIBLE);
		dmc->next_cache = cache_list_head;
		cache_list_head = dmc;
		clear_bit(FLASHCACHE_UPDATE_LIST, &flashcache_control->synch_flags);
		smp_mb__after_clear_bit();
		wake_up_bit(&flashcache_control->synch_flags, FLASHCACHE_UPDATE_LIST);
	}

	return dmc;

bad:
	return NULL;
}

/*
 * Construct a cache mapping.
 *  arg[0]: path to source device
 *  arg[1]: path to cache device
 *  arg[2]: md virtual device name
 *  arg[3]: cache mode (from flashcache.h)
 *  arg[4]: cache persistence (if set, cache conf is loaded from disk)
 * Cache configuration parameters (if not set, default values are used.
 *  arg[5]: cache block size (in sectors)
 *  arg[6]: cache size (in blocks)
 *  arg[7]: cache associativity
 *  arg[8]: md block size (in sectors)
 */
int 
flashcache_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
	struct cache_c *dmc;
	struct cache_c **nodepp;
	struct disk_slot *slot;
	int j;
	int r = -EINVAL;
	char groupname[DEV_PATHLEN], disk_devname[DEV_PATHLEN];

	if (argc < 3) {
		ti->error = "flashcache: Need at least 3 arguments";
		goto bad;
	}

	if (sscanf(argv[0], "%s", (char *)&groupname) != 1) {
		ti->error = "flashcache: group name lookup failed";
		goto bad;
	}

	if (sscanf(argv[1], "%s", (char *)&disk_devname) != 1) {
		ti->error = "flashcache: device name lookup failed";
		goto bad;
	}

	dmc = get_dmc(ti, groupname);

	if (!dmc)
		goto bad;

	slot = NULL;
	for (j=0; j<MAX_SLOT; j++) {
		if (dmc->slot[j].slot_state == SLOT_NOTUSED) {
			slot = &dmc->slot[j];
			slot->id = j;
			break;
		}
	}
	if (slot == NULL) {
		ti->error = "flashcache: No available disk slot";
		r = ENOMEM;
		goto bad;
	}

	if (sscanf(argv[2], "%s", (char *)&slot->dm_vdevname) != 1) {
		ti->error = "flashcache: Virtual device name lookup failed";
		goto bad1;
	}

	if ((r = flashcache_get_dev(ti, disk_devname, &slot->disk_dev, 
				    slot->disk_devname, ti->len))) {
		if (r == -EBUSY)
			ti->error = "flashcache: Disk device is busy, cannot create cache";
		else
			ti->error = "flashcache: Disk device lookup failed";
		goto bad1;
	}

	slot->ti = ti;
	slot->dmc = dmc;
	slot->slot_state = SLOT_USED;
	slot->cached_blocks = 0;
	memset(&slot->flashcache_errors, 0, sizeof(struct flashcache_errors));
	memset(&slot->flashcache_stats, 0, sizeof(struct flashcache_stats));
	clear_bit(CACHE_ENABLED, &slot->cache_enabled);
	ti->private = slot;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	ti->num_flush_bios = 1;
	ti->num_discard_bios = 1;
	ti->split_discard_bios = true;
#else
	ti->num_flush_requests = 1;
	ti->num_discard_requests = 1;
	ti->split_discard_requests = true;
#endif

	dmc->refcount++;

	return 0;
bad1:
	slot->slot_state = SLOT_NOTUSED;
	clear_bit(CACHE_ENABLED, &slot->cache_enabled);
	if (!dmc->refcount && !dmc->cache_dev) {
		(void)wait_on_bit_lock(&flashcache_control->synch_flags, 
			FLASHCACHE_UPDATE_LIST,
			flashcache_wait_schedule, 
			TASK_UNINTERRUPTIBLE);
		nodepp = &cache_list_head;
		while (*nodepp != NULL) {
			if (*nodepp == dmc) {
				*nodepp = dmc->next_cache;
				break;
			}
			nodepp = &((*nodepp)->next_cache);
		}
		clear_bit(FLASHCACHE_UPDATE_LIST, &flashcache_control->synch_flags);
		smp_mb__after_clear_bit();
		wake_up_bit(&flashcache_control->synch_flags, FLASHCACHE_UPDATE_LIST);
		kfree(dmc);
	}
bad:
	return r;
}	

static int flashcache_merge(struct dm_target *ti, struct bvec_merge_data *bvm,
		      struct bio_vec *biovec, int max_size)
{
	struct disk_slot *slot = ti->private;
	struct request_queue *q = bdev_get_queue(slot->disk_dev->bdev);
	//char b1[BDEVNAME_SIZE], b2[BDEVNAME_SIZE];

	//printk("flashcache_merge0: %s, %s, %s, %d, %u, %u\n", ti->type->name, bdevname(bvm->bi_bdev, b1), bdevname(slot->disk_dev->bdev, b2), max_size, bvm->bi_size, biovec->bv_offset);
	if (!q->merge_bvec_fn)
		return max_size;

	bvm->bi_bdev = slot->disk_dev->bdev;

	return min(max_size, q->merge_bvec_fn(q, bvm, biovec));
}

static int flashcache_locate_thin(struct dm_target *ti, locate_thin_callout_fn fn, sector_t start, sector_t len, void *remap_desc, void **pool)
{
	struct disk_slot *slot = ti->private;

	return fn(slot->disk_dev->bdev, start, len, remap_desc, pool);
}

#define CACHE_BLOCK_MASK (u_int64_t)(dmc->block_size-1)

static int flashcache_invalidate(struct dm_target *ti, sector_t start, sector_t len, invalidate_callback_fn fn, void* data)
{
	struct disk_slot *slot = (struct disk_slot *) ti->private;
	struct cache_c *dmc = (struct cache_c *) slot->dmc;
	struct cacheblock *cacheblk = NULL;
	sector_t dbn;
	int err = 0;
	int index = -1;
	int llen = len; //change to signed variable

	if (!test_bit(CACHE_ENABLED, &slot->cache_enabled) || !test_bit(SSD_READY, &dmc->SSD_state))
                err = 0;
	else {
                if (!(start & CACHE_BLOCK_MASK)) //aligned
                    dbn = start;
                else {
                    dbn = (start & (~CACHE_BLOCK_MASK));
                    llen += (start - dbn); //let len start aligned boundary
                }
                while(llen > 0) {
			LOCK_START
			spin_lock_irq(&dmc->cache_spin_lock);
			if (VALID == flashcache_lookup_noreclaim(slot, dbn, &index)) {
        			cacheblk = &dmc->cache[index];
	                	cacheblk->cache_state &= ~(VALID | PVALID);
        	        	cacheblk->cache_state |= INVALID;
			}
			spin_unlock_irq(&dmc->cache_spin_lock);
			LOCK_END
			dbn += dmc->block_size;
			llen -= dmc->block_size;
		}
		err = 0;
	}

	fn(err, data, slot->disk_dev->bdev, start);
}

static int flashcache_ssd_merge(struct dm_target *ti, struct bvec_merge_data *bvm,
		      struct bio_vec *biovec, int max_size)
{
	struct cache_c *dmc = ti->private;
	struct request_queue *q = bdev_get_queue(dmc->cache_dev->bdev);
	//char b1[BDEVNAME_SIZE], b2[BDEVNAME_SIZE];

	//printk("flashcache_merge0: %s, %s, %s, %d, %u, %u\n", ti->type->name, bdevname(bvm->bi_bdev, b1), bdevname(slot->disk_dev->bdev, b2), max_size, bvm->bi_size, biovec->bv_offset);
	if (!q->merge_bvec_fn)
		return max_size;

	bvm->bi_bdev = dmc->cache_dev->bdev;

	return min(max_size, q->merge_bvec_fn(q, bvm, biovec));
}

int 
flashcache_plugin(struct cache_c *dmc, struct ssd_hotplug_cmd *cmd)
{
	sector_t i, order;
	int r = -EINVAL;
	int persistence = 0;
	int j;

	mutex_lock(&dmc->hp_mutex);

	if (!dmc->cache_dev) {
		printk("flashcache: ssd device is not created\n");
		r = -EINVAL;
		goto bad;
	}

	if (test_bit(SSD_READY, &dmc->SSD_state)) {
		mutex_unlock(&dmc->hp_mutex);
		return 0;
	}

	dmc->cache_mode = cmd->cache_mode;
	persistence = cmd->flashcache_cmd;
	dmc->block_size = cmd->block_size;
	dmc->size = cmd->cache_size;
	dmc->assoc = cmd->assoc;
	dmc->md_block_size = 8;
//	dmc->md_block_size = cmd->md_block_size;

	if (cmd->cache_mode < FLASHCACHE_WRITE_BACK || 
	    cmd->cache_mode > FLASHCACHE_WRITE_AROUND) {
		DMERR("cache_mode = %d", cmd->cache_mode);
		printk("flashcache: Invalid cache mode\n");
		r = -EINVAL;
		goto bad;
	}
	
	/* 
	 * XXX - Persistence is totally ignored for write through and write around.
	 * Maybe this should really be moved to the end of the param list ?
	 */
	persistence = CACHE_CREATE;

	if (!dmc->block_size || (dmc->block_size & (dmc->block_size - 1))) {
		printk("flashcache: Invalid block size\n");
		r = -EINVAL;
		goto bad;
	}
	
	if (!dmc->block_size)
		dmc->block_size = DEFAULT_BLOCK_SIZE;
	dmc->block_size = DEFAULT_BLOCK_SIZE;	/* make it fixed */
	dmc->block_shift = ffs(dmc->block_size) - 1;
	dmc->block_mask = dmc->block_size - 1;

	/* dmc->size is specified in sectors here, and converted to blocks later */
	// use SECTOR_SHIFT to replace to_sector, in 32bit kernel, to_sector only deals with 32bit integer well
	if (!dmc->size)
		dmc->size = dmc->cache_dev->bdev->bd_inode->i_size >> SECTOR_SHIFT;

	if (!dmc->assoc || (dmc->assoc & (dmc->assoc - 1)) ||
		dmc->assoc > FLASHCACHE_MAX_ASSOC ||
		dmc->assoc < FLASHCACHE_MIN_ASSOC ||
		dmc->size < dmc->assoc) {
		printk("flashcache: Invalid cache associativity\n");
		r = -EINVAL;
		goto bad;
	}

	if (!dmc->assoc)
		dmc->assoc = DEFAULT_CACHE_ASSOC;
	dmc->assoc_shift = ffs(dmc->assoc) - 1;

	if (flashcache_writethrough_create(dmc)) {
		r = -EINVAL;
		goto bad;
	}

	dmc->num_sets = dmc->size >> dmc->assoc_shift;
	order = dmc->num_sets * sizeof(struct cache_set);
	dmc->cache_sets = (struct cache_set *)vmalloc(order);
	if (!dmc->cache_sets) {
		printk("Unable to allocate memory\n");
		r = -ENOMEM;
		vfree((void *)dmc->cache);
		goto bad;
	}				

	for (i = 0 ; i < dmc->num_sets ; i++) {
		dmc->cache_sets[i].set_fifo_next = i * dmc->assoc;
		dmc->cache_sets[i].set_clean_next = i * dmc->assoc;
		dmc->cache_sets[i].lru_tail = FLASHCACHE_LRU_NULL;
		dmc->cache_sets[i].lru_head = FLASHCACHE_LRU_NULL;
	}

	/* Push all blocks into the set specific LRUs */
	for (i = 0 ; i < dmc->size ; i++) {
		dmc->cache[i].lru_prev = FLASHCACHE_LRU_NULL;
		dmc->cache[i].lru_next = FLASHCACHE_LRU_NULL;
		flashcache_reclaim_lru_movetail(dmc, i);
	}

	dmc->cached_blocks = 0;
	dmc->ssd_forced_plugout = 0;

	for (j=0; j<MAX_SLOT; j++) {
		if (dmc->slot[j].slot_state == SLOT_USED) {
			if (test_bit(CACHE_ENABLED, &dmc->slot[j].cache_enabled))
				dmc->slot[j].ti->max_io_len = dmc->block_size;
			dmc->slot[j].cached_blocks = 0;
			memset(&dmc->slot[j].flashcache_errors, 0, sizeof(struct flashcache_errors));
			memset(&dmc->slot[j].flashcache_stats, 0, sizeof(struct flashcache_stats));
		}
	}

	/* Other sysctl defaults */
	dmc->sysctl_io_latency_hist = 0;
	dmc->sysctl_pid_do_expiry = 0;
	dmc->sysctl_max_pids = MAX_PIDS;
	dmc->sysctl_pid_expiry_secs = PID_EXPIRY_SECS;
	dmc->sysctl_reclaim_policy = FLASHCACHE_FIFO;
	dmc->sysctl_zerostats = 0;
	dmc->sysctl_zeroerrors = 0;
	dmc->sysctl_error_inject = 0;
	dmc->sysctl_fast_remove = 0;
	dmc->sysctl_cache_all = 1;
	dmc->sysctl_skip_seq_thresh_kb = SKIP_SEQUENTIAL_THRESHOLD;
	dmc->sysctl_auto_plugout_error_thresh = AUTO_PLUGOUT_THRESHOLD;

	memset(&dmc->flashcache_errors, 0, sizeof(struct flashcache_errors));
	memset(&dmc->flashcache_stats, 0, sizeof(struct flashcache_stats));
	for (i = 0 ; i < IO_LATENCY_BUCKETS ; i++)
		dmc->latency_hist[i] = 0;
	dmc->latency_hist_10ms = 0;

	/* set seq_io_head to NULL to prevent infinite loop */
	dmc->seq_io_head = NULL;
	/* Sequential i/o spotting */	
	for (i = 0; i < SEQUENTIAL_TRACKER_QUEUE_DEPTH; i++) {
		dmc->seq_recent_ios[i].slot_id = -1;
		dmc->seq_recent_ios[i].most_recent_sector = 0;
		dmc->seq_recent_ios[i].sequential_count = 0;
		dmc->seq_recent_ios[i].prev = (struct sequential_io *)NULL;
		dmc->seq_recent_ios[i].next = (struct sequential_io *)NULL;
		seq_io_move_to_lruhead(dmc, &dmc->seq_recent_ios[i]);
	}
	dmc->seq_io_tail = &dmc->seq_recent_ios[0];

#ifdef FLASHCACHE_PID_CONTROL
	dmc->whitelist_head = NULL;
	dmc->whitelist_tail = NULL;
	dmc->blacklist_head = NULL;
	dmc->blacklist_tail = NULL;
	dmc->num_whitelist_pids = 0;
	dmc->num_blacklist_pids = 0;
#endif

	flashcache_ctr_procfs(dmc);

	init_timer(&dmc->ht_timer);
	dmc->ht_timer.function = ht_reset_timer;
	dmc->ht_timer.data = (unsigned long) dmc;
	mod_timer(&dmc->ht_timer, HT_RESET_INTERVAL);

	set_bit(SSD_READY, &dmc->SSD_state);

	mutex_unlock(&dmc->hp_mutex);
	return 0;

bad:
	mutex_unlock(&dmc->hp_mutex);
	return r;
}

static void
flashcache_dtr_stats_print(struct cache_c *dmc)
{
	int read_hit_pct, write_hit_pct;
	struct flashcache_stats *stats = &dmc->flashcache_stats;
	u_int64_t  cache_pct;
	char *cache_mode;
	int i;
	
	if (stats->reads > 0)
		read_hit_pct = stats->read_hits * 100 / stats->reads;
	else
		read_hit_pct = 0;
	if (stats->writes > 0) {
		write_hit_pct = stats->write_hits * 100 / stats->writes;
	} else {
		write_hit_pct = 0;
	}
	
	DMINFO("stats: \n\treads(%lu), writes(%lu)", stats->reads, stats->writes);

	if (dmc->cache_mode == FLASHCACHE_WRITE_THROUGH) {
		DMINFO("\tread hits(%lu), read hit percent(%d)\n"	\
		       "\twrite hits(%lu) write hit percent(%d)\n"	\
		       "\treplacement(%lu)\n"				\
		       "\twrite invalidates(%lu), read invalidates(%lu)\n",
		       stats->read_hits, read_hit_pct,
		       stats->write_hits, write_hit_pct,
		       stats->replace,
		       stats->wr_invalidates, stats->rd_invalidates);
#ifdef FLASHCACHE_DO_CHECKSUMS
		DMINFO("\tchecksum store(%ld), checksum valid(%ld), checksum invalid(%ld)\n",
		       stats->checksum_store, stats->checksum_valid, stats->checksum_invalid);
#endif
		DMINFO("\tpending enqueues(%lu), pending inval(%lu)\n"	\
		       "\tno room(%lu)\n",
		       stats->enqueues, stats->pending_inval,
		       stats->noroom);
	} else 	{	/* WRITE_AROUND */
		DMINFO("\tread hits(%lu), read hit percent(%d)\n"	\
		       "\treplacement(%lu)\n"				\
		       "\tinvalidates(%lu)\n",
		       stats->read_hits, read_hit_pct,
		       stats->replace,
		       stats->rd_invalidates);
#ifdef FLASHCACHE_DO_CHECKSUMS
		DMINFO("\tchecksum store(%ld), checksum valid(%ld), checksum invalid(%ld)\n",
		       stats->checksum_store, stats->checksum_valid, stats->checksum_invalid);
#endif
		DMINFO("\tpending enqueues(%lu), pending inval(%lu)\n"	\
		       "\tno room(%lu)\n",
		       stats->enqueues, stats->pending_inval,
		       stats->noroom);
	}
	/* All modes */
        DMINFO("\tdisk reads(%lu), disk writes(%lu) ssd reads(%lu) ssd writes(%lu)\n" \
               "\tuncached reads(%lu), uncached writes(%lu), uncached IO requeue(%lu)\n" \
	       "\tuncached sequential reads(%lu), uncached sequential writes(%lu)\n" \
               "\tpid_adds(%lu), pid_dels(%lu), pid_drops(%lu) pid_expiry(%lu)",
               stats->disk_reads, stats->disk_writes, stats->ssd_reads, stats->ssd_writes,
               stats->uncached_reads, stats->uncached_writes, stats->uncached_io_requeue,
	       stats->uncached_sequential_reads, stats->uncached_sequential_writes,
               stats->pid_adds, stats->pid_dels, stats->pid_drops, stats->expiry);
	if (dmc->size > 0) {
		cache_pct = ((u_int64_t)dmc->cached_blocks * 100) / dmc->size;
	} else {
		cache_pct = 0;
	}
	if (dmc->cache_mode == FLASHCACHE_WRITE_BACK)
		cache_mode = "WRITE_BACK";
	else if (dmc->cache_mode == FLASHCACHE_WRITE_THROUGH)
		cache_mode = "WRITE_THROUGH";
	else
		cache_mode = "WRITE_AROUND";
	DMINFO("conf:\n"						\
	       "\tssd dev (%s), cache mode(%s)\n"		\
	       "\tcapacity(%lluM), associativity(%u), data block size(%uK) metadata block size(%ub)\n" \
	       "\tskip sequential thresh(%uK), auto plugout error thresh(%d)\n" \
	       "\ttotal blocks(%llu), cached blocks(%lu), cache percent(%d)\n",
	       dmc->cache_devname, cache_mode,
	       dmc->size*dmc->block_size>>11, dmc->assoc,
	       dmc->block_size>>(10-SECTOR_SHIFT), 
	       dmc->md_block_size * 512, 
	       dmc->sysctl_skip_seq_thresh_kb,
	       dmc->sysctl_auto_plugout_error_thresh,
	       dmc->size, dmc->cached_blocks, 
	       (int)cache_pct);
	DMINFO("\tnr_queued(%lu)\n", dmc->pending_jobs_count);
	DMINFO("Size Hist: ");
	for (i = 0 ; i <= 10 ; i++) {
		if (size_hist[i] > 0)
			DMINFO("%d:%llu ", 1<<i, size_hist[i]);
	}
}

/*
 * Destroy the cache mapping.
 */
void 
flashcache_dtr(struct dm_target *ti)
{
        struct disk_slot *slot = (struct disk_slot *) ti->private;
        struct cache_c *dmc = (struct cache_c *) slot->dmc;
	struct cache_c **nodepp;
	int i;
	int dmc_ref = -1;
        u_int16_t       lru_prev, lru_next;

	slot->slot_state = SLOT_NOTUSED;
	clear_bit(CACHE_ENABLED, &slot->cache_enabled);

	/* Invalidate current slot cacheblock */
	if (test_bit(SSD_READY, &dmc->SSD_state)) {
		spin_lock_irq(&dmc->cache_spin_lock);
		for (i = 0 ; i < dmc->size ; i++) {
			if (dmc->cache[i].slot_id == slot->id) {
				if (dmc->cache[i].cache_state & VALID) {
					--dmc->cached_blocks;
				}
		                lru_prev = dmc->cache[i].lru_prev;
		                lru_next = dmc->cache[i].lru_next;
				memset(&dmc->cache[i], 0, sizeof(struct cacheblock));
				dmc->cache[i].cache_state = INVALID;
		                dmc->cache[i].lru_prev = lru_prev;
		                dmc->cache[i].lru_next = lru_next;
			}
		}
		spin_unlock_irq(&dmc->cache_spin_lock);
	}

	dmc_ref = --dmc->refcount;

	if (!dmc_ref && !dmc->cache_dev) {
		(void)wait_on_bit_lock(&flashcache_control->synch_flags, 
			FLASHCACHE_UPDATE_LIST,
			flashcache_wait_schedule, 
			TASK_UNINTERRUPTIBLE);
		nodepp = &cache_list_head;
		while (*nodepp != NULL) {
			if (*nodepp == dmc) {
				*nodepp = dmc->next_cache;
				break;
			}
			nodepp = &((*nodepp)->next_cache);
		}
		clear_bit(FLASHCACHE_UPDATE_LIST, &flashcache_control->synch_flags);
		smp_mb__after_clear_bit();
		wake_up_bit(&flashcache_control->synch_flags, FLASHCACHE_UPDATE_LIST);
		kfree(dmc);
	}
	dm_put_device(ti, slot->disk_dev);
}

int
flashcache_plugout(struct cache_c *dmc)
{
	int i;
	int nr_queued = 0;

	mutex_lock(&dmc->hp_mutex);

	if (!test_bit(SSD_READY, &dmc->SSD_state)) {
		mutex_unlock(&dmc->hp_mutex);
		return 0;
	}

	set_bit(SSD_PURGE, &dmc->SSD_state);

	/* waitting for no map function job */
	do {
		schedule_timeout(HZ);
	} while(atomic_read(&dmc->nr_map_jobs));

	if(atomic_read(&dmc->nr_jobs)) {
		do {
			wait_event_timeout(dmc->destroyq, !atomic_read(&dmc->nr_jobs), HZ);
		} while (atomic_read(&dmc->nr_jobs));
	}
/*
	if(atomic_read(&nr_cache_jobs) || atomic_read(&nr_pending_jobs)) {
		do {
			wait_event_timeout(dmc->destroyq, !atomic_read(&dmc->nr_jobs), HZ);
		} while (atomic_read(&nr_cache_jobs) || atomic_read(&nr_pending_jobs));
	}
*/
	flashcache_dtr_procfs(dmc);

	DMINFO("cache jobs %d, pending jobs %d", atomic_read(&nr_cache_jobs), 
	       atomic_read(&nr_pending_jobs));
	for (i = 0 ; i < dmc->size ; i++)
		nr_queued += dmc->cache[i].nr_queued;
	DMINFO("cache queued jobs %d", nr_queued);	
	flashcache_dtr_stats_print(dmc);

	vfree((void *)dmc->cache);
	vfree((void *)dmc->cache_sets);

#ifdef FLASHCACHE_PID_CONTROL
	flashcache_del_all_pids(dmc, FLASHCACHE_WHITELIST, 1);
	flashcache_del_all_pids(dmc, FLASHCACHE_BLACKLIST, 1);
	VERIFY(dmc->num_whitelist_pids == 0);
	VERIFY(dmc->num_blacklist_pids == 0);
#endif

	for (i=0; i<MAX_SLOT; i++) {
		if (dmc->slot[i].slot_state == SLOT_USED) {
			dmc->slot[i].ti->max_io_len = 0;
			dmc->slot[i].cached_blocks = 0;
        		memset(&dmc->slot[i].flashcache_stats, 0, sizeof(struct flashcache_stats));
        		memset(&dmc->slot[i].flashcache_errors, 0, sizeof(struct flashcache_errors));
		}
	}
	dmc->size = 0;

	clear_bit(SSD_READY, &dmc->SSD_state);
	clear_bit(SSD_PURGE, &dmc->SSD_state);
	wake_up_all(&dmc->plugoutq);

	del_timer_sync(&dmc->ht_timer);

	mutex_unlock(&dmc->hp_mutex);
	return 0;
}

int
flashcache_enable(struct disk_slot *slot, int enable)
{
        struct cache_c *dmc = (struct cache_c *) slot->dmc;
	int i;
        u_int16_t       lru_prev, lru_next;

	mutex_lock(&dmc->hp_mutex);
	if (enable) {
		if (test_bit(CACHE_ENABLED, &slot->cache_enabled)) {
			mutex_unlock(&dmc->hp_mutex);
			return 0;
		}

		if (test_bit(SSD_READY, &dmc->SSD_state)) {
			slot->ti->max_io_len = dmc->block_size;
		}
		set_bit(CACHE_ENABLED, &slot->cache_enabled);
		mutex_unlock(&dmc->hp_mutex);
		return 0;
	}

	if (!test_bit(CACHE_ENABLED, &slot->cache_enabled)) {
		mutex_unlock(&dmc->hp_mutex);
		return 0;
	}

	set_bit(SSD_PURGE, &dmc->SSD_state);

	/* waitting for no map function job */
	do {
		schedule_timeout(HZ);
	} while(atomic_read(&dmc->nr_map_jobs));

	if(atomic_read(&dmc->nr_jobs)) {
		do {
			wait_event_timeout(dmc->destroyq, !atomic_read(&dmc->nr_jobs), HZ);
		} while (atomic_read(&dmc->nr_jobs));
	}
/*
	if(atomic_read(&nr_cache_jobs) || atomic_read(&nr_pending_jobs)) {
		do {
			wait_event_timeout(dmc->destroyq, !atomic_read(&dmc->nr_jobs), HZ);
		} while (atomic_read(&nr_cache_jobs) || atomic_read(&nr_pending_jobs));
	}
*/
	/* Invalidate current slot cacheblock */
	if (test_bit(SSD_READY, &dmc->SSD_state)) {
		spin_lock_irq(&dmc->cache_spin_lock);
		for (i = 0 ; i < dmc->size ; i++) {
			if (dmc->cache[i].slot_id == slot->id) {
				if (dmc->cache[i].cache_state & VALID) {
					--dmc->cached_blocks;
				}
		                lru_prev = dmc->cache[i].lru_prev;
		                lru_next = dmc->cache[i].lru_next;
				memset(&dmc->cache[i], 0, sizeof(struct cacheblock));
				dmc->cache[i].cache_state = INVALID;
		                dmc->cache[i].lru_prev = lru_prev;
		                dmc->cache[i].lru_next = lru_next;
			}
		}
		spin_unlock_irq(&dmc->cache_spin_lock);
	}

	slot->cached_blocks = 0;
	memset(&slot->flashcache_errors, 0, sizeof(struct flashcache_errors));
	memset(&slot->flashcache_stats, 0, sizeof(struct flashcache_stats));

	slot->ti->max_io_len = 0;
	clear_bit(CACHE_ENABLED, &slot->cache_enabled);
	clear_bit(SSD_PURGE, &dmc->SSD_state);
	wake_up_all(&dmc->plugoutq);

	mutex_unlock(&dmc->hp_mutex);
	return 0;
}
void
flashcache_status_info(struct disk_slot *slot, status_type_t type,
		       char *result, unsigned int maxlen)
{
        struct cache_c *dmc = (struct cache_c *) slot->dmc;
	int read_hit_pct, write_hit_pct;
	int sz = 0; /* DMEMIT */
	struct flashcache_stats *stats = &dmc->flashcache_stats;

	
	if (stats->reads > 0)
		read_hit_pct = stats->read_hits * 100 / stats->reads;
	else
		read_hit_pct = 0;
	if (stats->writes > 0) {
		write_hit_pct = stats->write_hits * 100 / stats->writes;
	} else {
		write_hit_pct = 0;
	}
	DMEMIT("stats: \n\treads(%lu), writes(%lu)\n", 
	       stats->reads, stats->writes);

	if (dmc->cache_mode == FLASHCACHE_WRITE_THROUGH) {
		DMEMIT("\tread hits(%lu), read hit percent(%d)\n"	\
		       "\twrite hits(%lu) write hit percent(%d)\n"	\
		       "\treplacement(%lu), write replacement(%lu)\n"	\
		       "\twrite invalidates(%lu), read invalidates(%lu)\n",
		       stats->read_hits, read_hit_pct,
		       stats->write_hits, write_hit_pct,
		       stats->replace, stats->wr_replace, 
		       stats->wr_invalidates, stats->rd_invalidates);
#ifdef FLASHCACHE_DO_CHECKSUMS
		DMEMIT("\tchecksum store(%ld), checksum valid(%ld), checksum invalid(%ld)\n",
		       stats->checksum_store, stats->checksum_valid, stats->checksum_invalid);
#endif
		DMEMIT("\tpending enqueues(%lu), pending inval(%lu)\n"	\
		       "\tno room(%lu)\n",
		       stats->enqueues, stats->pending_inval,
		       stats->noroom);
	} else {	/* WRITE_AROUND */
		DMEMIT("\tread hits(%lu), read hit percent(%d)\n"	\
		       "\treplacement(%lu), write replacement(%lu)\n"	\
		       "\tinvalidates(%lu)\n",
		       stats->read_hits, read_hit_pct,
		       stats->replace, stats->wr_replace, 
		       stats->rd_invalidates);
#ifdef FLASHCACHE_DO_CHECKSUMS
		DMEMIT("\tchecksum store(%ld), checksum valid(%ld), checksum invalid(%ld)\n",
		       stats->checksum_store, stats->checksum_valid, stats->checksum_invalid);
#endif
		DMEMIT("\tpending enqueues(%lu), pending inval(%lu)\n"	\
		       "\tno room(%lu)\n",
		       stats->enqueues, stats->pending_inval,
		       stats->noroom);
	}
	/* All modes */
	DMEMIT("\tdisk reads(%lu), disk writes(%lu) ssd reads(%lu) ssd writes(%lu)\n" \
	       "\tuncached reads(%lu), uncached writes(%lu), uncached IO requeue(%lu)\n" \
	       "\tuncached sequential reads(%lu), uncached sequential writes(%lu)\n" \
	       "\tpid_adds(%lu), pid_dels(%lu), pid_drops(%lu) pid_expiry(%lu)",
	       stats->disk_reads, stats->disk_writes, stats->ssd_reads, stats->ssd_writes,
	       stats->uncached_reads, stats->uncached_writes, stats->uncached_io_requeue,
	       stats->uncached_sequential_reads, stats->uncached_sequential_writes,
	       stats->pid_adds, stats->pid_dels, stats->pid_drops, stats->expiry);
	if (dmc->sysctl_io_latency_hist) {
		int i;
		
		DMEMIT("\nIO Latency Histogram: \n");
		for (i = 1 ; i <= IO_LATENCY_BUCKETS ; i++) {
			DMEMIT("< %d\tusecs : %lu\n", i * IO_LATENCY_GRAN_USECS, dmc->latency_hist[i - 1]);
		}
		DMEMIT("> 10\tmsecs : %lu", dmc->latency_hist_10ms);		
	}
}

static void
flashcache_status_table(struct disk_slot *slot, status_type_t type,
			     char *result, unsigned int maxlen)
{
        struct cache_c *dmc = (struct cache_c *) slot->dmc;
	u_int64_t  cache_pct;
	int sz = 0; /* DMEMIT */
	char *cache_mode;
	
	if (dmc->size > 0) {
		cache_pct = ((u_int64_t)slot->cached_blocks * 100) / dmc->size;
	} else {
		cache_pct = 0;
	}
	if (dmc->cache_mode == FLASHCACHE_WRITE_BACK)
		cache_mode = "WRITE_BACK";
	else if (dmc->cache_mode == FLASHCACHE_WRITE_THROUGH)
		cache_mode = "WRITE_THROUGH";
	else
		cache_mode = "WRITE_AROUND";
	DMEMIT("conf:\n");
/*
flashcache_cg2vdev CG1 cachedev1
flashcache_vdev2cg cachedev1 CG1
flashcache_disk2vdev /dev/sdd1 cachedev1
flashcache_vdev2disk cachedev1 /dev/sdd1
flashcache_ssd2vdev /dev/sdb cachedev1
flashcache_vdev2ssd cachedev1 /dev/sdb
*/

	DMEMIT("\tflashcache_cg2vdev %s %s\n", dmc->groupname, slot->dm_vdevname);
	DMEMIT("\tflashcache_vdev2cg %s %s\n", slot->dm_vdevname, dmc->groupname);
	DMEMIT("\tflashcache_disk2vdev %s %s\n", slot->disk_devname, slot->dm_vdevname);
	DMEMIT("\tflashcache_vdev2disk %s %s\n", slot->dm_vdevname, slot->disk_devname);
	DMEMIT("\tflashcache_plugin %s %s\n", dmc->groupname, test_bit(SSD_READY, &dmc->SSD_state)?"YES":"NO");
	DMEMIT("\tflashcache_enabled %s %s\n", slot->disk_devname, test_bit(CACHE_ENABLED, &slot->cache_enabled)?"ENABLED":"DISABLED");

	DMEMIT("\ttotal blocks(%llu), cached blocks(%lu), cache percent(%d)\n",
	       dmc->size, slot->cached_blocks, (int)cache_pct);
}

/*
 * Report cache status:
 *  Output cache stats upon request of device status;
 *  Output cache configuration upon request of table status.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
void
flashcache_status(struct dm_target *ti, status_type_t type,
                unsigned status_flags, char *result, unsigned int maxlen)
#else
int 
flashcache_status(struct dm_target *ti, status_type_t type,
	     char *result, unsigned int maxlen)
#endif
{
	struct disk_slot *slot = (struct disk_slot *) ti->private;
	
	switch (type) {
	case STATUSTYPE_INFO:
		flashcache_status_info(slot, type, result, maxlen);
		break;
	case STATUSTYPE_TABLE:
		flashcache_status_table(slot, type, result, maxlen);
		break;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	return 0;
#endif
}

/*
 * Construct a cache mapping.
 *  arg[0]: path to source device
 *  arg[1]: path to cache device
 *  arg[2]: md virtual device name
 *  arg[3]: cache mode (from flashcache.h)
 *  arg[4]: cache persistence (if set, cache conf is loaded from disk)
 * Cache configuration parameters (if not set, default values are used.
 *  arg[5]: cache block size (in sectors)
 *  arg[6]: cache size (in blocks)
 *  arg[7]: cache associativity
 *  arg[8]: md block size (in sectors)
 */
int 
flashcache_ssd_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
        struct cache_c *dmc;
	struct cache_c **nodepp;
	int r = -EINVAL;

	char groupname[DEV_PATHLEN], ssd_devname[DEV_PATHLEN];
	
	if (argc < 3) {
		ti->error = "flashcache_ssd: Need at least 3 arguments";
		goto bad;
	}

	if (sscanf(argv[0], "%s", (char *)&groupname) != 1) {
		ti->error = "flashcache_ssd: group name lookup failed";
		goto bad;
	}

	if (sscanf(argv[1], "%s", (char *)&ssd_devname) != 1) {
		ti->error = "flashcache_ssd: device name lookup failed";
		goto bad;
	}

	dmc = get_dmc(ti, groupname);

	if (!dmc)
		goto bad;

	if (dmc->cache_dev) {
		ti->error = "flashcache_ssd: Current cache group already has one Cache device";
		goto bad;
	}

	if (sscanf(argv[2], "%s", (char *)&dmc->ssd_vdevname) != 1) {
		ti->error = "flashcache_ssd: Virtual device name lookup failed";
		goto bad1;
	}

        if ((r = flashcache_get_dev(ti, ssd_devname, &dmc->cache_dev,
                                    dmc->cache_devname, 0))) {
                if (r == -EBUSY)
                        ti->error = "flashcache_ssd: Cache device is busy, cannot create cache";
                else
                        ti->error = "flashcache_ssd: Cache device lookup failed";
                goto bad1;
	}

	ti->private = dmc;
	dmc->refcount++;

	return 0;

bad1:
	if (!dmc->refcount) {
		(void)wait_on_bit_lock(&flashcache_control->synch_flags, 
			FLASHCACHE_UPDATE_LIST,
			flashcache_wait_schedule, 
			TASK_UNINTERRUPTIBLE);
		nodepp = &cache_list_head;
		while (*nodepp != NULL) {
			if (*nodepp == dmc) {
				*nodepp = dmc->next_cache;
				break;
			}
			nodepp = &((*nodepp)->next_cache);
		}
		clear_bit(FLASHCACHE_UPDATE_LIST, &flashcache_control->synch_flags);
		smp_mb__after_clear_bit();
		wake_up_bit(&flashcache_control->synch_flags, FLASHCACHE_UPDATE_LIST);
		kfree(dmc);
	}
bad:
	return r;
}

/*
 * Destroy the cache mapping.
 */
void 
flashcache_ssd_dtr(struct dm_target *ti)
{
        struct cache_c *dmc = (struct cache_c *)ti->private;
	struct cache_c **nodepp;

	if (test_bit(SSD_READY, &dmc->SSD_state))
		flashcache_plugout(dmc);

	dm_put_device(ti, dmc->cache_dev);
	dmc->cache_dev = NULL;
        memset(dmc->cache_devname, 0, sizeof(dmc->cache_devname));
	memset(dmc->ssd_vdevname, 0, sizeof(dmc->ssd_vdevname));

	if (!dmc->refcount) {
		(void)wait_on_bit_lock(&flashcache_control->synch_flags, 
			FLASHCACHE_UPDATE_LIST,
			flashcache_wait_schedule, 
			TASK_UNINTERRUPTIBLE);
		nodepp = &cache_list_head;
		while (*nodepp != NULL) {
			if (*nodepp == dmc) {
				*nodepp = dmc->next_cache;
				break;
			}
			nodepp = &((*nodepp)->next_cache);
		}
		clear_bit(FLASHCACHE_UPDATE_LIST, &flashcache_control->synch_flags);
		smp_mb__after_clear_bit();
		wake_up_bit(&flashcache_control->synch_flags, FLASHCACHE_UPDATE_LIST);
		kfree(dmc);
	}
}

/*
 * Decide the mapping and perform necessary cache operations for a bio request.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
int
flashcache_ssd_map(struct dm_target *ti, struct bio *bio)
#else
int
flashcache_ssd_map(struct dm_target *ti, struct bio *bio,
               union map_info *map_context)
#endif
{
        struct cache_c *dmc = (struct cache_c *)ti->private;
	bio->bi_bdev = dmc->cache_dev->bdev;
	return DM_MAPIO_REMAPPED;
}

/*
 * Report cache status:
 *  Output cache stats upon request of device status;
 *  Output cache configuration upon request of table status.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
void
flashcache_ssd_status(struct dm_target *ti, status_type_t type,
             unsigned status_flags, char *result, unsigned int maxlen)
#else
int 
flashcache_ssd_status(struct dm_target *ti, status_type_t type,
	     char *result, unsigned int maxlen)
#endif
{
	struct cache_c *dmc = (struct cache_c *)ti->private;
	int sz = 0; /* DMEMIT */
	u_int64_t  cache_pct;
	int i;
	char *cache_mode;

	if (dmc->size > 0) {
		cache_pct = ((u_int64_t)dmc->cached_blocks * 100) / dmc->size;
	} else {
		cache_pct = 0;
	}
	
	if (dmc->cache_mode == FLASHCACHE_WRITE_BACK)
		cache_mode = "WRITE_BACK";
	else if (dmc->cache_mode == FLASHCACHE_WRITE_THROUGH)
		cache_mode = "WRITE_THROUGH";
	else
		cache_mode = "WRITE_AROUND";

	DMEMIT("conf:\n");

	switch (type) {
	case STATUSTYPE_INFO:
		break;
	case STATUSTYPE_TABLE:
		DMEMIT("\tflashcache_cg2vdev %s %s\n", dmc->groupname, dmc->ssd_vdevname);
		DMEMIT("\tflashcache_vdev2cg %s %s\n", dmc->ssd_vdevname, dmc->groupname);
		DMEMIT("\tflashcache_cg2ssd %s %s\n", dmc->groupname, dmc->cache_devname);
		DMEMIT("\tflashcache_ssd2cg %s %s\n", dmc->cache_devname, dmc->groupname);
		DMEMIT("\tflashcache_vdev2ssd %s %s\n", dmc->ssd_vdevname, dmc->cache_devname);
		DMEMIT("\tflashcache_ssd2vdev %s %s\n", dmc->cache_devname, dmc->ssd_vdevname);
		DMEMIT("\tflashcache_plugin %s %s\n", dmc->groupname, test_bit(SSD_READY, &dmc->SSD_state)?"YES":"NO");
		DMEMIT("\tflashcache_forced_plugout %s %s\n", dmc->groupname, dmc->ssd_forced_plugout?"YES":"NO");

		DMEMIT("\tcache mode(%s)\n", cache_mode);

		if (dmc->cache_mode != FLASHCACHE_WRITE_BACK) {
			DMEMIT("\tcapacity(%lluM), associativity(%u), data block size(%uK)\n",
		       		dmc->size*dmc->block_size>>11, dmc->assoc,
		       		dmc->block_size>>(10-SECTOR_SHIFT));
		}
		DMEMIT("\tskip sequential thresh(%uK)\n",
	       		dmc->sysctl_skip_seq_thresh_kb);
		DMEMIT("\tauto plugout error thresh(%d)\n",
	       		dmc->sysctl_auto_plugout_error_thresh);
		DMEMIT("\ttotal blocks(%llu), cached blocks(%lu), cache percent(%d)\n",
	       		dmc->size, dmc->cached_blocks, (int)cache_pct);
		DMEMIT("\tnr_queued(%lu)\n", dmc->pending_jobs_count);
		DMEMIT("Size Hist: ");
		for (i = 0 ; i <= 10 ; i++) {
			if (size_hist[i] > 0)
				DMEMIT("%d:%llu ", 1<<i, size_hist[i]);
		}
		break;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	return 0;
#endif
}

static int flashcache_ssd_iterate_devices(struct dm_target *ti,
                                  iterate_devices_callout_fn fn, void *data)
{
	struct cache_c *dmc = ti->private;

	return fn(ti, dmc->cache_dev, 0, ti->len, data);
}

static int flashcache_iterate_devices(struct dm_target *ti,
                                  iterate_devices_callout_fn fn, void *data)
{
	struct disk_slot *slot = ti->private;

	return fn(ti, slot->disk_dev, 0, ti->len, data);
}

static struct target_type flashcache_target = {
	.name   = "flashcache",
	.version= {1, 0, 3},
	.module = THIS_MODULE,
	.ctr    = flashcache_ctr,
	.dtr    = flashcache_dtr,
	.map    = flashcache_map,
	.status = flashcache_status,
	.ioctl 	= flashcache_ioctl,
	.iterate_devices = flashcache_iterate_devices,
	.merge  = flashcache_merge,
	.locate_thin = flashcache_locate_thin,
	.invalidate = flashcache_invalidate,
#ifdef FLASHCACHE_COMPAT_IOCTL
	.compat_ioctl 	= flashcache_compat_ioctl,
#endif
};

static struct target_type flashcache_ssd_target = {
	.name   = "flashcache_ssd",
	.version= {1, 0, 3},
	.module = THIS_MODULE,
	.ctr    = flashcache_ssd_ctr,
	.dtr    = flashcache_ssd_dtr,
	.map    = flashcache_ssd_map,
	.status = flashcache_ssd_status,
	.ioctl 	= flashcache_ssd_ioctl,
	.iterate_devices = flashcache_ssd_iterate_devices,
	.merge  = flashcache_ssd_merge,
#ifdef FLASHCACHE_COMPAT_IOCTL
	.compat_ioctl 	= flashcache_ssd_compat_ioctl,
#endif
};

static int 
flashcache_notify_reboot(struct notifier_block *this,
			 unsigned long code, void *x)
{
	(void)wait_on_bit_lock(&flashcache_control->synch_flags, 
			       FLASHCACHE_UPDATE_LIST,
			       flashcache_wait_schedule, 
			       TASK_UNINTERRUPTIBLE);
	clear_bit(FLASHCACHE_UPDATE_LIST, &flashcache_control->synch_flags);
	smp_mb__after_clear_bit();
	wake_up_bit(&flashcache_control->synch_flags, FLASHCACHE_UPDATE_LIST);
	return NOTIFY_DONE;
}

/*
 * The notifiers are registered in descending order of priority and
 * executed in descending order or priority. We should be run before
 * any notifiers of ssd's or other block devices. Typically, devices
 * use a priority of 0.
 * XXX - If in the future we happen to use a md device as the cache
 * block device, we have a problem because md uses a priority of 
 * INT_MAX as well. But we want to run before the md's reboot notifier !
 */
static struct notifier_block flashcache_notifier = {
	.notifier_call	= flashcache_notify_reboot,
	.next		= NULL,
	.priority	= INT_MAX, /* should be > ssd pri's and disk dev pri's */
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
struct dm_kcopyd_client *flashcache_kcp_client; /* Kcopyd client for writing back data */
#else
struct kcopyd_client *flashcache_kcp_client; /* Kcopyd client for writing back data */
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
struct dm_io_client *flashcache_io_client; /* Client memory pool*/
#endif

/*
 * Initiate a cache target.
 */
int __init 
flashcache_init(void)
{
	int r;

	r = flashcache_jobs_init();
	if (r)
		return r;
	atomic_set(&nr_cache_jobs, 0);
	atomic_set(&nr_pending_jobs, 0);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
	r = dm_io_get(FLASHCACHE_ASYNC_SIZE);
	if (r) {
		DMERR("flashcache_init: Could not size dm io pool");
		return r;
	}
	r = kcopyd_client_create(FLASHCACHE_COPY_PAGES, &flashcache_kcp_client);
	if (r) {
		DMERR("flashcache_init: Failed to initialize kcopyd client");
		dm_io_put(FLASHCACHE_ASYNC_SIZE);
		return r;
	}
#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22) */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)) || (defined(RHEL_RELEASE_CODE) && (RHEL_RELEASE_CODE >= 1538))
	flashcache_io_client = dm_io_client_create();
#else
	flashcache_io_client = dm_io_client_create(FLASHCACHE_COPY_PAGES);
#endif
	if (IS_ERR(flashcache_io_client)) {
		DMERR("flashcache_init: Failed to initialize DM IO client");
		return r;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
	r = kcopyd_client_create(FLASHCACHE_COPY_PAGES, &flashcache_kcp_client);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)) || (defined(RHEL_RELEASE_CODE) && (RHEL_RELEASE_CODE >= 1538))
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	flashcache_kcp_client = dm_kcopyd_client_create(&dm_kcopyd_throttle);
#else
	flashcache_kcp_client = dm_kcopyd_client_create();
#endif
	if ((r = IS_ERR(flashcache_kcp_client))) {
		r = PTR_ERR(flashcache_kcp_client);
	}
#else /* .26 <= VERSION < 3.0.0 */
	r = dm_kcopyd_client_create(FLASHCACHE_COPY_PAGES, &flashcache_kcp_client);
#endif /* .26 <= VERSION < 3.0.0 */

	if (r) {
		dm_io_client_destroy(flashcache_io_client);
		DMERR("flashcache_init: Failed to initialize kcopyd client");
		return r;
	}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	INIT_WORK(&_kcached_wq, do_work, NULL);
#else
	INIT_WORK(&_kcached_wq, do_work);
#endif
	for (r = 0 ; r <= 10 ; r++)
		size_hist[r] = 0;
	r = dm_register_target(&flashcache_target);
	if (r < 0) {
		DMERR("cache: register failed %d", r);
	}

	r = dm_register_target(&flashcache_ssd_target);
	if (r < 0) {
		DMERR("cache: register failed %d", r);
	}

        printk("flashcache: %dKB block %s initialized\n", BITMAP_COUNT*64, flashcache_sw_version);

	flashcache_module_procfs_init();
	flashcache_control = (struct flashcache_control_s *)
		kmalloc(sizeof(struct flashcache_control_s), GFP_KERNEL);
	flashcache_control->synch_flags = 0;
	register_reboot_notifier(&flashcache_notifier);
	return r;
}

/*
 * Destroy a cache target.
 */
void 
flashcache_exit(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	int r = dm_unregister_target(&flashcache_target);
	r = dm_unregister_target(&flashcache_ssd_target);

	if (r < 0)
		DMERR("cache: unregister failed %d", r);
#else
	dm_unregister_target(&flashcache_target);
	dm_unregister_target(&flashcache_ssd_target);
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
	kcopyd_client_destroy(flashcache_kcp_client);
#else
	dm_kcopyd_client_destroy(flashcache_kcp_client);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
	dm_io_client_destroy(flashcache_io_client);
#else
	dm_io_put(FLASHCACHE_ASYNC_SIZE);
#endif
	unregister_reboot_notifier(&flashcache_notifier);
	flashcache_jobs_exit();
	flashcache_module_procfs_releae();
	kfree(flashcache_control);
}

module_init(flashcache_init);
module_exit(flashcache_exit);

MODULE_DESCRIPTION(DM_NAME " Facebook flash cache target");
MODULE_AUTHOR("Mohan - based on code by Ming");
MODULE_LICENSE("GPL");
