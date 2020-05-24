/****************************************************************************
 *  flashcache.h
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

#ifndef FLASHCACHE_H
#define FLASHCACHE_H

#define CONFIG_ARM

#define FLASHCACHE_VERSION		2

#define PARTIAL_BITMAP
#define BITMAP_ARRAY
#ifdef QTS_SNAPSHOT
#define BITMAP_COUNT    8	/* one bitmap is for 64k only, so it needs 8 * 64k = 512K */
#else
#define BITMAP_COUNT    16	/* one bitmap is for 64k only, so it needs 16 * 64k = 1M */
#endif
/* set to 512kB always for fast block cloning */
#define BITMAP_COUNT    8	/* one bitmap is for 64k only, so it needs 8 * 64k = 512K */
#define DEV_PATHLEN	128

#ifdef __KERNEL__

/* Like ASSERT() but always compiled in */

#define VERIFY(x) do { \
	if (unlikely(!(x))) { \
		dump_stack(); \
		panic("VERIFY: assertion (%s) failed at %s (%d)\n", \
		      #x,  __FILE__ , __LINE__);		    \
	} \
} while(0)

#define DMC_DEBUG 0
#define DMC_DEBUG_LITE 0

#define DM_MSG_PREFIX "flashcache"
#define DMC_PREFIX "flashcache: "

#if DMC_DEBUG
#define JPRINTK( s, arg... ) if (!spin_is_locked(&dmc->cache_spin_lock)) printk(DMC_PREFIX "pid %d slot %d " s "\n", pid_nr(task_pid(current)), job->slot->id, ##arg)
#define DPRINTK( s, arg... ) if (!spin_is_locked(&dmc->cache_spin_lock)) printk(DMC_PREFIX "pid %d slot %d " s "\n", pid_nr(task_pid(current)), slot->id, ##arg)
#define NPRINTK( s, arg... ) if (!spin_is_locked(&dmc->cache_spin_lock)) printk(DMC_PREFIX "pid %d " s "\n", pid_nr(task_pid(current)), ##arg)
#define SPRINTK( s, arg... ) if (!spin_is_locked(&dmc->cache_spin_lock)) printk(DMC_PREFIX "pid %d slot %d " s "\n", pid_nr(task_pid(current)), slot->id, ##arg)
#define ERRPRINTK( s, arg... ) if (!spin_is_locked(&dmc->cache_spin_lock)) printk(DMC_PREFIX "pid %d slot %d " s "\n", pid_nr(task_pid(current)), slot->id, ##arg)
//#define JPRINTK( s, arg... ) printk(DMC_PREFIX "pid %d slot %d " s "\n", pid_nr(task_pid(current)), job->slot->id, ##arg)
//#define DPRINTK( s, arg... ) printk(DMC_PREFIX "pid %d spin_is_locked %d " s "\n", pid_nr(task_pid(current)), spin_is_locked(&dmc->cache_spin_lock), ##arg)
//#define NPRINTK( s, arg... ) printk(DMC_PREFIX "pid %d " s "\n", pid_nr(task_pid(current)), ##arg)
//#define SPRINTK( s, arg... ) printk(DMC_PREFIX "pid %d spin_is_locked %d " s "\n", pid_nr(task_pid(current)), spin_is_locked(&dmc->cache_spin_lock), ##arg)
//#define ERRPRINTK( s, arg... ) printk(DMC_PREFIX "pid %d spin_is_locked %d " s "\n", pid_nr(task_pid(current)), spin_is_locked(&dmc->cache_spin_lock), ##arg)
#else
#define JPRINTK( s, arg... )
#define DPRINTK( s, arg... )
#define NPRINTK( s, arg... )
#define SPRINTK( s, arg... )
#define ERRPRINTK( s, arg... )
#endif

#if 0
#define LOCK_START	printk("Lock start pid %d %s %d \n", pid_nr(task_pid(current)),  __FILE__ , __LINE__);
#define LOCK_END	printk("Lock end pid %d %s %d \n", pid_nr(task_pid(current)),  __FILE__ , __LINE__);
#else
#define LOCK_START
#define LOCK_END
#endif

/*
 * Block checksums :
 * Block checksums seem a good idea (especially for debugging, I found a couple 
 * of bugs with this), but in practice there are a number of issues with this
 * in production.
 * 1) If a flash write fails, there is no guarantee that the failure was atomic.
 * Some sectors may have been written to flash. If so, the checksum we have
 * is wrong. We could re-read the flash block and recompute the checksum, but
 * the read could fail too. 
 * 2) On a node crash, we could have crashed between the flash data write and the
 * flash metadata update (which updates the new checksum to flash metadata). When
 * we reboot, the checksum we read from metadata is wrong. This is worked around
 * by having the cache load recompute checksums after an unclean shutdown.
 * 3) Checksums require 4 or 8 more bytes per block in terms of metadata overhead.
 * Especially because the metadata is wired into memory.
 * 4) Checksums force us to do a flash metadata IO on a block re-dirty. If we 
 * didn't maintain checksums, we could avoid the metadata IO on a re-dirty.
 * Therefore in production we disable block checksums.
 */
#if 0
#define FLASHCACHE_DO_CHECKSUMS
#endif

#if DMC_DEBUG_LITE
#define DPRINTK_LITE( s, arg... ) printk(DMC_PREFIX s "\n", ##arg)
#else
#define DPRINTK_LITE( s, arg... )
#endif

/* Number of pages for I/O */
#define FLASHCACHE_COPY_PAGES (1024)

/* Default cache parameters */
#define DEFAULT_CACHE_SIZE	65536
#define DEFAULT_CACHE_ASSOC	512
#define DEFAULT_BLOCK_SIZE	(BITMAP_COUNT*128)	/* 1024 KB */
//#define DEFAULT_BLOCK_SIZE	2048	/* 1024 KB */
//#define DEFAULT_BLOCK_SIZE	8	/* 4 KB */
#define DEFAULT_MD_BLOCK_SIZE	8	/* 4 KB */
#define FLASHCACHE_MAX_MD_BLOCK_SIZE	128	/* 64 KB */

#define FLASHCACHE_FIFO		0
#define FLASHCACHE_LRU		1

/*
 * The LRU pointers are maintained as set-relative offsets, instead of 
 * pointers. This enables us to store the LRU pointers per cacheblock
 * using 4 bytes instead of 16 bytes. The upshot of this is that we 
 * are required to clamp the associativity at an 8K max.
 */
#define FLASHCACHE_MIN_ASSOC	 256
#define FLASHCACHE_MAX_ASSOC	8192
#define FLASHCACHE_LRU_NULL	0xFFFF

struct cacheblock;

struct cache_set {
	u_int32_t		set_fifo_next;
	u_int32_t		set_clean_next;
	u_int16_t		lru_head, lru_tail;
};

struct flashcache_errors {
	int	ssd_errors;
	int	discard_errors;
	int	disk_read_errors;
	int	disk_write_errors;
	int	ssd_read_errors;
	int	ssd_write_errors;
	int	memory_alloc_errors;
	int	verify_errors;
	int	verify_errors1;
	int	verify_errors2;
	int	verify_errors3;
	int	verify_errors4;
	int	verify_errors5;
};

struct flashcache_stats {
	unsigned long raw_reads;	/* Number of reads */
	unsigned long raw_writes;	/* Number of writes */
	unsigned long reads;		/* Number of reads */
	unsigned long writes;		/* Number of writes */
	unsigned long raw_read_hits;	/* Number of cache hits */
	unsigned long read_hits;	/* Number of cache hits */
	unsigned long read_phits;	/* Number of partial cache hits */
	unsigned long raw_write_hits;	/* Number of write hits (includes dirty write hits) */
	unsigned long write_hits;	/* Number of write hits (includes dirty write hits) */
	unsigned long write_phits;	/* Number of partial write hits (includes dirty write hits) */
	unsigned long replace;		/* Number of cache replacements */
	unsigned long wr_replace;
	unsigned long wr_invalidates;	/* Number of write invalidations */
	unsigned long rd_invalidates;	/* Number of read invalidations */
	unsigned long pending_inval;	/* Invalidations due to concurrent ios on same block */
	unsigned long direct;		/* Number of direct io */
#ifdef FLASHCACHE_DO_CHECKSUMS
	unsigned long checksum_store;
	unsigned long checksum_valid;
	unsigned long checksum_invalid;
#endif
	unsigned long enqueues;		/* enqueues on pending queue */
	unsigned long noroom;		/* No room in set */
	unsigned long pid_drops;
	unsigned long pid_adds;
	unsigned long pid_dels;
	unsigned long expiry;
	unsigned long front_merge, back_merge;	/* Write Merging */
	unsigned long uncached_reads, uncached_writes;
	unsigned long uncached_sequential_reads, uncached_sequential_writes;
	unsigned long disk_reads, disk_writes;
	unsigned long ssd_reads, ssd_writes;
	unsigned long uncached_io_requeue;
	unsigned long skipclean;
	unsigned long trim_blocks;
	unsigned long discards;
};

/* 
 * Sequential block history structure - each one
 * records a 'flow' of i/o.
 */
struct sequential_io {
	int8_t 	slot_id;	/* slot id */
 	sector_t 		most_recent_sector;
 	sector_t 		recent_size;
	unsigned long		sequential_count;
	/* We use LRU replacement when we need to record a new i/o 'flow' */
	struct sequential_io 	*prev, *next;
};
#define SKIP_SEQUENTIAL_THRESHOLD 16384			/* 0 = cache all, >0 = dont cache sequential i/o more than this (kb) */
#define SEQUENTIAL_TRACKER_QUEUE_DEPTH	32		/* How many io 'flows' to track (random i/o will hog many).
							 * This should be large enough so that we don't quickly 
							 * evict sequential i/o when we see some random,
							 * but small enough that searching through it isn't slow
							 * (currently we do linear search, we could consider hashed */
#define AUTO_PLUGOUT_THRESHOLD	50
								
#define SSD_READY	0
//#define SSD_REMOVED	1
#define SSD_PURGE	2

#define CACHE_ENABLED	0
//#define CACHE_DISABLED	1
#define CACHE_DISABLING	2

#define MAX_SLOT	256
#define SLOT_NOTUSED	0
#define SLOT_USED	1

struct cache_c;

struct disk_slot {
	int8_t slot_state;
	int8_t id;
	unsigned long cache_enabled;
	unsigned long cached_blocks;	/* Number of cached blocks */
	struct flashcache_stats flashcache_stats;	/* Stats */
	struct flashcache_errors flashcache_errors;	/* Errors */
	struct dm_target *ti;
	char disk_devname[DEV_PATHLEN];
	char dm_vdevname[DEV_PATHLEN];
	struct dm_dev *disk_dev;			/* disk volume for noSSD IO */
	struct cache_c *dmc;
};

/*
 * Cache context
 */
struct cache_c {
	struct dm_target	*tgt;
	
	struct dm_dev 		*disk_dev;   /* Source device */
	struct dm_dev 		*cache_dev; /* Cache device */

	int 			on_ssd_version;
	
	spinlock_t		cache_spin_lock;
	struct mutex 		hp_mutex;

	struct cacheblock	*cache;	/* Hash table for cache blocks */
	struct cache_set	*cache_sets;
	struct cache_md_block_head *md_blocks_buf;

	unsigned int md_block_size;	/* Metadata block size in sectors */
	
	sector_t size;			/* Cache size */
	unsigned int assoc;		/* Cache associativity */
	unsigned int block_size;	/* Cache block size */
	unsigned int block_shift;	/* Cache block size in bits */
	unsigned int block_mask;	/* Cache block mask */
	unsigned int assoc_shift;	/* Consecutive blocks size in bits */
	unsigned int num_sets;		/* Number of cache sets */
	
	int	cache_mode;

	wait_queue_head_t destroyq;	/* Wait queue for I/O completion */
	wait_queue_head_t plugoutq;	/* Wait queue for I/O completion */
	/* XXX - Updates of nr_jobs should happen inside the lock. But doing it outside
	   is OK since the filesystem is unmounted at this point */
	atomic_t nr_jobs;		/* Number of I/O jobs */

	unsigned long cached_blocks;	/* Number of cached blocks */
	unsigned long pending_jobs_count;
	int	md_blocks;		/* Numbers of metadata blocks, including header */

	/* Stats */
	struct flashcache_stats flashcache_stats;

	/* Errors */
	struct flashcache_errors flashcache_errors;

#define IO_LATENCY_GRAN_USECS	250
#define IO_LATENCY_MAX_US_TRACK	10000	/* 10 ms */
#define IO_LATENCY_BUCKETS	(IO_LATENCY_MAX_US_TRACK / IO_LATENCY_GRAN_USECS)
	unsigned long	latency_hist[IO_LATENCY_BUCKETS];
	unsigned long	latency_hist_10ms;
	
#ifdef FLASHCACHE_PID_CONTROL
	unsigned long pid_expire_check;

	struct flashcache_cachectl_pid *blacklist_head, *blacklist_tail;
	struct flashcache_cachectl_pid *whitelist_head, *whitelist_tail;
	int num_blacklist_pids, num_whitelist_pids;
	unsigned long blacklist_expire_check, whitelist_expire_check;
#endif

#define PENDING_JOB_HASH_SIZE		32
	struct pending_job *pending_job_hashbuckets[PENDING_JOB_HASH_SIZE];
	
	struct cache_c	*next_cache;

	void *sysctl_handle;

        char groupname[DEV_PATHLEN];

	// DM virtual device name, stored in superblock and restored on load
	char dm_vdevname[DEV_PATHLEN];
	// real device names are now stored as UUIDs
	char cache_devname[DEV_PATHLEN];
	char disk_devname[DEV_PATHLEN];

	/* Per device sysctls */
	int sysctl_io_latency_hist;
	int sysctl_pid_do_expiry;
	int sysctl_max_pids;
	int sysctl_pid_expiry_secs;
	int sysctl_reclaim_policy;
	int sysctl_zerostats;
	int sysctl_zeroerrors;
	int sysctl_error_inject;
	int sysctl_fast_remove;
	int sysctl_cache_all;
	int sysctl_skip_seq_thresh_kb;
	int sysctl_auto_plugout_error_thresh;

	/* Sequential I/O spotter */
	struct sequential_io	seq_recent_ios[SEQUENTIAL_TRACKER_QUEUE_DEPTH];
	struct sequential_io	*seq_io_head;
	struct sequential_io 	*seq_io_tail;

	unsigned long SSD_state;
	struct disk_slot slot[MAX_SLOT];
	char ssd_vdevname[DEV_PATHLEN];
	int refcount;
	int ssd_forced_plugout;
	atomic_t nr_map_jobs;
	struct work_struct kerror_wq;
	struct timer_list ht_timer;
};

/* kcached/pending job states */
#define READCACHE	1
#define WRITECACHE	2
#define READDISK	3
#define WRITEDISK	4
#define READFILL	5	/* Read Cache Miss Fill */
#define INVALIDATE	6
#define WRITEDISK_SYNC	7
#define READDISK_UNCACHED	8
#define DISCARD		9

struct kcached_job {
	struct list_head list;
	struct bio *bio;	/* Original bio */
	struct disk_slot *slot;
	struct job_io_regions {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
		struct io_region disk;
		struct io_region cache;
#else
		struct dm_io_region disk;
		struct dm_io_region cache;
#endif
	} job_io_regions;
	int    index;
	int    action;
	int 	error;
	struct bio_vec md_io_bvec;
	struct timeval io_start_time;
	struct kcached_job *next;
};

struct pending_job {
	struct bio *bio;
	int	action;	
	int	index;
	int 	partial;
	struct pending_job *prev, *next;
};
#endif /* __KERNEL__ */

/* Cache Modes */
enum {
	FLASHCACHE_WRITE_BACK=1,
	FLASHCACHE_WRITE_THROUGH=2,
	FLASHCACHE_WRITE_AROUND=3,
};

/* States of a cache block */
#define INVALID			0x01
#define VALID			0x02	/* Valid */
#define DISKREADINPROG		0x04	/* Read from disk in progress */
#define DISKWRITEINPROG		0x08	/* Write to disk in progress */
#define CACHEREADINPROG		0x10	/* Read from cache in progress */
#define CACHEWRITEINPROG	0x20	/* Write to cache in progress */
#define PVALID			0x40	/* Partial Valid */
#define IN_ERROR		0x80	/* Error handling */

#define BLOCK_IO_INPROG	(DISKREADINPROG | DISKWRITEINPROG | CACHEREADINPROG | CACHEWRITEINPROG)

/* Cache metadata is read by Flashcache utilities */
#ifndef __KERNEL__
typedef u_int64_t sector_t;
#endif

/* On Flash (cache metadata) Structures */
#define CACHE_MD_STATE_DIRTY		0xdeadbeef
#define CACHE_MD_STATE_CLEAN		0xfacecafe
#define CACHE_MD_STATE_FASTCLEAN	0xcafefeed
#define CACHE_MD_STATE_UNSTABLE		0xc8249756

/* Cache block metadata structure */
struct cacheblock {
	u_int8_t	cache_state;
	int8_t 	slot_id;	/* slot id */
#ifdef BITMAP_ARRAY
	int16_t 	nr_queued;	/* jobs in pending queue */
	int16_t		nr_concurrent;	/* current concurrent processing jobs */
#else
	int8_t 	nr_queued;	/* jobs in pending queue */
	int8_t		nr_concurrent;	/* current concurrent processing jobs */
#endif
	u_int16_t	lru_prev, lru_next;
	sector_t 	dbn;	/* Sector number of the cached block */
#ifdef PARTIAL_BITMAP
#ifdef BITMAP_ARRAY
	u_int64_t	bitmap[BITMAP_COUNT];
	u_int64_t	io_bitmap[BITMAP_COUNT];
#else
	u_int64_t	bitmap;
	u_int64_t	io_bitmap;
#endif
#else
	int16_t		offset;	/* offset within block in sector */
	int16_t		size;	/* size in sector */
	int16_t		io_offset;	/* offset within block in sector during io */
	int16_t		io_size;	/* size in sector during io */
#endif
#ifdef FLASHCACHE_DO_CHECKSUMS
	u_int64_t 	checksum;
#endif
};

struct flash_superblock {
	sector_t size;		/* Cache size */
	u_int32_t block_size;	/* Cache block size */
	u_int32_t assoc;	/* Cache associativity */
	u_int32_t cache_sb_state;	/* Clean shutdown ? */
	char cache_devname[DEV_PATHLEN]; /* Contains dm_vdev name as of v2 modifications */
	sector_t cache_devsize;
	char disk_devname[DEV_PATHLEN]; /* underlying block device name (use UUID paths!) */
	sector_t disk_devsize;
	u_int32_t cache_version;
	u_int32_t md_block_size;
};

/* 
 * We do metadata updates only when a block trasitions from DIRTY -> CLEAN
 * or from CLEAN -> DIRTY. Consequently, on an unclean shutdown, we only
 * pick up blocks that are marked (DIRTY | CLEAN), we clean these and stick
 * them in the cache.
 * On a clean shutdown, we will sync the state for every block, and we will
 * load every block back into cache on a restart.
 * 
 * Note: When using larger flashcache metadata blocks, it is important to make 
 * sure that a flash_cacheblock does not straddle 2 sectors. This avoids
 * partial writes of a metadata slot on a powerfail/node crash. Aligning this
 * a 16b or 32b struct avoids that issue.
 * 
 * Note: If a on-ssd flash_cacheblock does not fit exactly within a 512b sector,
 * (ie. if there are any remainder runt bytes), logic in flashcache_conf.c which
 * reads and writes flashcache metadata on create/load/remove will break.
 * 
 * If changing these, make sure they remain a ^2 size !
 */
#ifdef FLASHCACHE_DO_CHECKSUMS
struct flash_cacheblock {
	sector_t 	dbn;	/* Sector number of the cached block */
	u_int64_t 	checksum;
	u_int32_t	cache_state; /* INVALID | VALID | DIRTY */
} __attribute__ ((aligned(32)));
#else
struct flash_cacheblock {
	sector_t 	dbn;	/* Sector number of the cached block */
	u_int32_t	cache_state; /* INVALID | VALID | DIRTY */
} __attribute__ ((aligned(16)));
#endif

#define MD_BLOCK_BYTES(DMC)		((DMC)->md_block_size * 512)
#define MD_SECTORS_PER_BLOCK(DMC)	((DMC)->md_block_size)
#define MD_SLOTS_PER_BLOCK(DMC)		(MD_BLOCK_BYTES(DMC) / (sizeof(struct flash_cacheblock)))
#define INDEX_TO_MD_BLOCK(DMC, INDEX)	((INDEX) / MD_SLOTS_PER_BLOCK(DMC))
#define INDEX_TO_MD_BLOCK_OFFSET(DMC, INDEX)	((INDEX) % MD_SLOTS_PER_BLOCK(DMC))

#define METADATA_IO_BLOCKSIZE		(256*1024)
#define METADATA_IO_NUM_BLOCKS(dmc)	(METADATA_IO_BLOCKSIZE / MD_BLOCK_BYTES(dmc))

#define INDEX_TO_CACHE_ADDR(DMC, INDEX)	\
	(((sector_t)(INDEX) << (DMC)->block_shift) + (DMC)->md_blocks * MD_SECTORS_PER_BLOCK((DMC)))

//#ifdef __KERNEL__

/* Cache persistence */
#define CACHE_RELOAD		1
#define CACHE_CREATE		2
#define CACHE_FORCECREATE	3

#ifdef __KERNEL__
/* 
 * We have one of these for *every* cache metadata sector, to keep track
 * of metadata ios in progress for blocks covered in this sector. Only
 * one metadata IO per sector can be in progress at any given point in 
 * time
 */
struct cache_md_block_head {
	u_int32_t		nr_in_prog;
	struct kcached_job	*queued_updates, *md_io_inprog;
};

#define MIN_JOBS 1024

/* Default values for sysctls */
#define MAX_PIDS		100
#define PID_EXPIRY_SECS		60

/* DM async IO mempool sizing */
#define FLASHCACHE_ASYNC_SIZE 1024

enum {
	FLASHCACHE_WHITELIST=0,
	FLASHCACHE_BLACKLIST=1,
};

struct flashcache_cachectl_pid {
	pid_t					pid;
	struct flashcache_cachectl_pid		*next, *prev;
	unsigned long				expiry;
};

/* Error injection flags */
#define READDISK_ERROR				0x00000001
#define READCACHE_ERROR				0x00000002
#define READFILL_ERROR				0x00000004
#define WRITECACHE_ERROR			0x00000008
#define WRITECACHE_MD_ERROR			0x00000010
#define WRITEDISK_MD_ERROR			0x00000020
#define KCOPYD_CALLBACK_ERROR			0x00000040
#define DIRTY_WRITEBACK_JOB_ALLOC_FAIL		0x00000080
#define READ_MISS_JOB_ALLOC_FAIL		0x00000100
#define READ_HIT_JOB_ALLOC_FAIL			0x00000200
#define READ_HIT_PENDING_JOB_ALLOC_FAIL		0x00000400
#define INVAL_PENDING_JOB_ALLOC_FAIL		0x00000800
#define WRITE_HIT_JOB_ALLOC_FAIL		0x00001000
#define WRITE_HIT_PENDING_JOB_ALLOC_FAIL	0x00002000
#define WRITE_MISS_JOB_ALLOC_FAIL		0x00004000
#define WRITES_LIST_ALLOC_FAIL			0x00008000
#define MD_ALLOC_SECTOR_ERROR			0x00010000

/* Inject a 5s delay between syncing blocks and metadata */
#define FLASHCACHE_SYNC_REMOVE_DELAY		5000

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
int flashcache_map(struct dm_target *ti, struct bio *bio);
#else
int flashcache_map(struct dm_target *ti, struct bio *bio,
		   union map_info *map_context);
#endif
int flashcache_ctr(struct dm_target *ti, unsigned int argc,
		   char **argv);
void flashcache_dtr(struct dm_target *ti);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
void flashcache_status(struct dm_target *ti, status_type_t type,
                      unsigned status_flags, char *result, unsigned int maxlen);
#else
int flashcache_status(struct dm_target *ti, status_type_t type,
		      char *result, unsigned int maxlen);
#endif

struct kcached_job *flashcache_alloc_cache_job(void);
void flashcache_free_cache_job(struct kcached_job *job);
struct pending_job *flashcache_alloc_pending_job(struct cache_c *dmc);
void flashcache_free_pending_job(struct pending_job *job);
#ifdef FLASHCACHE_DO_CHECKSUMS
u_int64_t flashcache_compute_checksum(struct bio *bio);
void flashcache_store_checksum(struct kcached_job *job);
int flashcache_validate_checksum(struct kcached_job *job);
int flashcache_read_compute_checksum(struct cache_c *dmc, int index, void *block);
#endif
struct kcached_job *pop(struct list_head *jobs);
void push(struct list_head *jobs, struct kcached_job *job);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
void do_work(void *unused);
#else
void do_work(struct work_struct *unused);
#endif
struct kcached_job *new_kcached_job(struct disk_slot *slot, struct bio* bio,
				    int index);
void push_pending(struct kcached_job *job);
void push_io(struct kcached_job *job);
void push_uncached_io_complete(struct kcached_job *job);
int flashcache_pending_empty(void);
int flashcache_io_empty(void);
void flashcache_do_pending(struct kcached_job *job);
void flashcache_do_io(struct kcached_job *job);
void flashcache_uncached_io_complete(struct kcached_job *job);
void flashcache_sync_all(struct cache_c *dmc);
void flashcache_reclaim_lru_movetail(struct cache_c *dmc, int index);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
int flashcache_dm_io_sync_vm(struct cache_c *dmc, struct io_region *where, 
			     int rw, void *data);
#else
int flashcache_dm_io_sync_vm(struct cache_c *dmc, struct dm_io_region *where, 
			     int rw, void *data);
#endif
void flashcache_enq_pending(struct cache_c *dmc, struct bio* bio,
			    int index, int action, struct pending_job *job);
struct pending_job *flashcache_deq_pending(struct cache_c *dmc, int index);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
int dm_io_async_bvec(unsigned int num_regions, 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
			    struct dm_io_region *where, 
#else
			    struct io_region *where, 
#endif
			    int rw, 
			    struct bio_vec *bvec, io_notify_fn fn, 
			    void *context);
#endif

void flashcache_bio_endio(struct bio *bio, int error, 
			  struct cache_c *dmc, struct timeval *io_start_time);

/* procfs */
void flashcache_module_procfs_init(void);
void flashcache_module_procfs_releae(void);
void flashcache_ctr_procfs(struct cache_c *dmc);
void flashcache_dtr_procfs(struct cache_c *dmc);

/* SSD hotplug function for ioctl */
struct ssd_hotplug_cmd;
int flashcache_plugin(struct cache_c *dmc, struct ssd_hotplug_cmd *cmd);
int flashcache_plugout(struct cache_c *dmc);
int flashcache_enable(struct disk_slot *slot, int enable);

/* for fast block clone   */
int flashcache_lookup_noreclaim(struct disk_slot *slot, sector_t start, int *index);

void
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
do_error(void *unused);
#else
do_error(struct work_struct *unused);
#endif

#define flashcache_bio_start_sector(bio, cacheblk) (bio->bi_sector-cacheblk->dbn)
#define flashcache_bio_end_sector(bio, cacheblk)   (bio->bi_sector-cacheblk->dbn + to_sector(bio->bi_size))
#define cache_start_sector(cacheblk)    (cacheblk->offset)
#define cache_end_sector(cacheblk)      (cacheblk->offset + cacheblk->size)
#define cacheio_start_sector(cacheblk) 	(cacheblk->io_offset)
#define cacheio_end_sector(cacheblk)   	(cacheblk->io_offset + cacheblk->io_size)

#endif /* __KERNEL__ */

#ifdef PARTIAL_BITMAP

#define BITMAP_CONTAIN(source, filter)		(((source & filter)==filter) ? 1:0)
#define BITMAP_NOTCONTAIN(source ,filter)	(!(source & filter) ? 1:0)
#define BITMAP_ADD(source, dest)		(source | dest)

#if defined(BITMAP_ARRAY)
	#define BITMAP_SIZE	64
	#define BLOCK_SHIFT	1
	#define SECTOR_MASK	((1<<BLOCK_SHIFT)-1)
	#define SIZE_MASK	((1<<BLOCK_SHIFT)-1)
	#define BLOCK_MASK	(u_int64_t)0xFFFFFFFFFFFFFFFF	/* one bit for 1 k; 64 bits for 64k*/
	#define TO_BITMAP(offset, size)			to_bitmap64(offset, size)
	#define SECTOR_COUNT    2*BITMAP_SIZE
#elif defined(BITMAP_64)
	#define BITMAP_SIZE	64
	#define BLOCK_SHIFT	1
	#define SECTOR_MASK	((1<<BLOCK_SHIFT)-1)
	#define SIZE_MASK	((1<<BLOCK_SHIFT)-1)
	#define BLOCK_MASK	(u_int64_t)0xFFFFFFFFFFFFFFFF	/* one bit for 1 k; 64 bits for 64k*/
	#define TO_BITMAP(offset, size)			to_bitmap64(offset, size)
#else
	#define BITMAP_SIZE	16
	#define BLOCK_SHIFT	3
	#define SECTOR_MASK	((1<<BLOCK_SHIFT)-1)
	#define SIZE_MASK	((1<<BLOCK_SHIFT)-1)
	#define BLOCK_MASK	(u_int16_t)0xFFFF			/* one bit for 4k; 16 bits for 64k*/
	#define TO_BITMAP(offset, size)			to_bitmap16(offset, size)
#endif

#endif	/* end of PARTIAL_BITMAP define */

#ifdef FLASHCACHE_PID_CONTROL
#undef FLASHCACHE_PID_CONTROL
#endif

#define FLASHCACHE_COMPAT_IOCTL
#ifdef FLASHCACHE_COMPAT_IOCTL
//#undef FLASHCACHE_COMPAT_IOCTL
#endif

#ifndef CONFIG_COMPAT
#undef FLASHCACHE_COMPAT_IOCTL
#endif

#endif
