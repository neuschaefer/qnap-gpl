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
#include "flashcache_ioctl.h"

extern u_int64_t size_hist[];

static char *flashcache_cons_procfs_cachename(struct cache_c *dmc, char *path_component);
static char *flashcache_cons_sysctl_devname(struct cache_c *dmc);

#define FLASHCACHE_PROC_ROOTDIR_NAME	"flashcache"

static int
flashcache_io_latency_init(ctl_table *table, int write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
			   struct file *file,
#endif
			   void __user *buffer,
			   size_t *length, loff_t *ppos)
{
	struct cache_c *dmc = (struct cache_c *)table->extra1;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
	proc_dointvec(table, write, file, buffer, length, ppos);
#else
	proc_dointvec(table, write, buffer, length, ppos);
#endif
	if (write) {
		if (dmc->sysctl_io_latency_hist) {
			int i;
				
			for (i = 0 ; i < IO_LATENCY_BUCKETS ; i++)
				dmc->latency_hist[i] = 0;
			dmc->latency_hist_10ms = 0;
		}
	}
	return 0;
}

static int 
flashcache_zerostats_sysctl(ctl_table *table, int write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
			    struct file *file, 
#endif
			    void __user *buffer, 
			    size_t *length, loff_t *ppos)
{
	struct cache_c *dmc = (struct cache_c *)table->extra1;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
	proc_dointvec(table, write, file, buffer, length, ppos);
#else
	proc_dointvec(table, write, buffer, length, ppos);
#endif
	if (write) {
		if (dmc->sysctl_zerostats) {
			int i;

			memset(&dmc->flashcache_stats, 0, sizeof(struct flashcache_stats));
			for (i = 0 ; i < IO_LATENCY_BUCKETS ; i++)
				dmc->latency_hist[i] = 0;
			dmc->latency_hist_10ms = 0;
		}
	}
	return 0;
}

static int 
flashcache_zeroerrors_sysctl(ctl_table *table, int write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
			    struct file *file, 
#endif
			    void __user *buffer, 
			    size_t *length, loff_t *ppos)
{
	struct cache_c *dmc = (struct cache_c *)table->extra1;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
	proc_dointvec(table, write, file, buffer, length, ppos);
#else
	proc_dointvec(table, write, buffer, length, ppos);
#endif
	if (write) {
		if (dmc->sysctl_zeroerrors)
			memset(&dmc->flashcache_errors, 0, sizeof(struct flashcache_errors));
	}
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
#define CTL_UNNUMBERED			-2
#endif

/*
 * Each ctl_table array needs to be 1 more than the actual number of
 * entries - zero padded at the end ! Therefore the NUM_*_SYSCTLS
 * is 1 more than then number of sysctls.
 */
#define FLASHCACHE_NUM_WRITETHROUGH_SYSCTLS	12

static struct flashcache_writethrough_sysctl_table {
	struct ctl_table_header *sysctl_header;
	ctl_table		vars[FLASHCACHE_NUM_WRITETHROUGH_SYSCTLS];
	ctl_table		dev[2];
	ctl_table		dir[2];
	ctl_table		root[2];
} flashcache_writethrough_sysctl = {
	.vars = {
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "io_latency_hist",
			.maxlen		= sizeof(int),
			.mode		= 0644,
			.proc_handler	= &flashcache_io_latency_init,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.strategy	= &sysctl_intvec,
#endif
		},
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "do_pid_expiry",
			.maxlen		= sizeof(int),
			.mode		= 0644,
			.proc_handler	= &proc_dointvec,
		},
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "max_pids",
			.maxlen		= sizeof(int),
			.mode		= 0644,
			.proc_handler	= &proc_dointvec,
		},
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "pid_expiry_secs",
			.maxlen		= sizeof(int),
			.mode		= 0644,
			.proc_handler	= &proc_dointvec,
		},
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "reclaim_policy",
			.maxlen		= sizeof(int),
			.mode		= 0644,
			.proc_handler	= &proc_dointvec,
		},
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "zero_stats",
			.maxlen		= sizeof(int),
			.mode		= 0644,
			.proc_handler	= &flashcache_zerostats_sysctl,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.strategy	= &sysctl_intvec,
#endif
		},
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "zero_errors",
			.maxlen		= sizeof(int),
			.mode		= 0644,
			.proc_handler	= &flashcache_zeroerrors_sysctl,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.strategy	= &sysctl_intvec,
#endif
		},
//#ifdef notdef
		/* 
		 * Disable this for all except devel builds 
		 * If you enable this, you must bump FLASHCACHE_NUM_WRITEBACK_SYSCTLS
		 * by 1 !
		 */
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "error_inject",
			.maxlen		= sizeof(int),
			.mode		= 0644,
			.proc_handler	= &proc_dointvec,
		},
//#endif
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "cache_all",
			.maxlen		= sizeof(int),
			.mode		= 0644,
			.proc_handler	= &proc_dointvec,
		},
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "skip_seq_thresh_kb",
			.maxlen		= sizeof(int),
			.mode		= 0644,
			.proc_handler	= &proc_dointvec,
		},
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "auto_plugout_error_thresh",
			.maxlen		= sizeof(int),
			.mode		= 0644,
			.proc_handler	= &proc_dointvec,
		},
	},
	.dev = {
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= "flashcache-dev",
			.maxlen		= 0,
			.mode		= S_IRUGO|S_IXUGO,
			.child		= flashcache_writethrough_sysctl.vars,
		},
	},
	.dir = {
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_UNNUMBERED,
#endif
			.procname	= FLASHCACHE_PROC_ROOTDIR_NAME,
			.maxlen		= 0,
			.mode		= S_IRUGO|S_IXUGO,
			.child		= flashcache_writethrough_sysctl.dev,
		},
	},
	.root = {
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
			.ctl_name	= CTL_DEV,
#endif
			.procname	= "dev",
			.maxlen		= 0,
			.mode		= 0555,
			.child		= flashcache_writethrough_sysctl.dir,
		},
	},
};

int *
flashcache_find_sysctl_data(struct cache_c *dmc, ctl_table *vars)
{
	if (strcmp(vars->procname, "io_latency_hist") == 0)
		return &dmc->sysctl_io_latency_hist;
	else if (strcmp(vars->procname, "do_pid_expiry") == 0) 
		return &dmc->sysctl_pid_do_expiry;
	else if (strcmp(vars->procname, "max_pids") == 0) 
		return &dmc->sysctl_max_pids;
	else if (strcmp(vars->procname, "pid_expiry_secs") == 0) 
		return &dmc->sysctl_pid_expiry_secs;
	else if (strcmp(vars->procname, "reclaim_policy") == 0) 
		return &dmc->sysctl_reclaim_policy;
	else if (strcmp(vars->procname, "zero_stats") == 0) 
		return &dmc->sysctl_zerostats;
	else if (strcmp(vars->procname, "zero_errors") == 0) 
		return &dmc->sysctl_zeroerrors;
	else if (strcmp(vars->procname, "error_inject") == 0) 
		return &dmc->sysctl_error_inject;
	else if (strcmp(vars->procname, "cache_all") == 0) 
		return &dmc->sysctl_cache_all;
	else if (strcmp(vars->procname, "skip_seq_thresh_kb") == 0) 
		return &dmc->sysctl_skip_seq_thresh_kb;
	else if (strcmp(vars->procname, "auto_plugout_error_thresh") == 0) 
		return &dmc->sysctl_auto_plugout_error_thresh;
	VERIFY(0);
	return NULL;
}

static void
flashcache_writethrough_sysctl_register(struct cache_c *dmc)
{
	int i;
	struct flashcache_writethrough_sysctl_table *t;
	
	t = kmemdup(&flashcache_writethrough_sysctl, sizeof(*t), GFP_KERNEL);
	if (t == NULL)
		return;
	for (i = 0 ; i < ARRAY_SIZE(t->vars) - 1 ; i++) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
		t->vars[i].de = NULL;
#endif
		t->vars[i].data = flashcache_find_sysctl_data(dmc, &t->vars[i]);
		t->vars[i].extra1 = dmc;
	}
	
	t->dev[0].procname = flashcache_cons_sysctl_devname(dmc);
	t->dev[0].child = t->vars;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
	t->dev[0].de = NULL;
#endif
	t->dir[0].child = t->dev;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
	t->dir[0].de = NULL;
#endif
	t->root[0].child = t->dir;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
	t->root[0].de = NULL;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
	t->sysctl_header = register_sysctl_table(t->root, 0);
#else
	t->sysctl_header = register_sysctl_table(t->root);
#endif
	if (t->sysctl_header == NULL)
		goto out;
	
	dmc->sysctl_handle = t;
	return;

out:
	kfree(t->dev[0].procname);
	kfree(t);
}

static void
flashcache_writethrough_sysctl_unregister(struct cache_c *dmc)
{
	struct flashcache_writethrough_sysctl_table *t;

	t = dmc->sysctl_handle;
	if (t != NULL) {
		dmc->sysctl_handle = NULL;
		unregister_sysctl_table(t->sysctl_header);
		kfree(t->dev[0].procname);
		kfree(t);		
	}
}


static int 
flashcache_stats_show(struct seq_file *seq, void *v)
{
	struct cache_c *dmc = seq->private;
	struct flashcache_stats *stats;
	int read_hit_pct, write_hit_pct;

	stats = &dmc->flashcache_stats;
	if (stats->reads > 0) {
		if (stats->reads < 100)
			read_hit_pct = stats->read_hits * 100 / 100;
		else
			read_hit_pct = stats->read_hits * 100 / stats->reads;
	} else
		read_hit_pct = 0;
	if (stats->writes > 0) {
		write_hit_pct = stats->write_hits * 100 / stats->writes;
	} else {
		write_hit_pct = 0;
	}
	seq_printf(seq, "raw_reads:\t%lu\n", stats->raw_reads);
	seq_printf(seq, "raw_writes:\t%lu\n", stats->raw_writes);
	seq_printf(seq, "raw_read_hits:\t%lu\n", stats->raw_read_hits);
	if (dmc->cache_mode == FLASHCACHE_WRITE_BACK || dmc->cache_mode == FLASHCACHE_WRITE_THROUGH)
		seq_printf(seq, "raw_write_hits:\t%lu\n", stats->raw_write_hits);
	seq_printf(seq, "reads:\t%lu\n", stats->reads);
	seq_printf(seq, "writes:\t%lu\n", stats->writes);
	seq_printf(seq, "read_hits:\t%lu\n", stats->read_hits);
	seq_printf(seq, "read_hit_percent:\t%d\n", read_hit_pct);
	if (dmc->cache_mode == FLASHCACHE_WRITE_BACK || dmc->cache_mode == FLASHCACHE_WRITE_THROUGH) {
		seq_printf(seq, "write_hits:\t%lu\n", stats->write_hits);
		seq_printf(seq, "write_hit_percent:\t%d\n", write_hit_pct);
	}
	if (dmc->cache_mode == FLASHCACHE_WRITE_BACK || dmc->cache_mode == FLASHCACHE_WRITE_THROUGH) {
		seq_printf(seq, "replacement:\t%lu\n", stats->replace);
		seq_printf(seq, "write_replacement:\t%lu\n", stats->wr_replace);
		seq_printf(seq, "write_invalidates:\t%lu\n", stats->wr_invalidates);
		seq_printf(seq, "read_invalidates:\t%lu\n", stats->rd_invalidates);
	} else {	/* WRITE_AROUND */
		seq_printf(seq, "replacement:\t%lu\n",
			   stats->replace);
		seq_printf(seq, "read_invalidates:\t%lu\n",
			   stats->rd_invalidates);
	}
#ifdef FLASHCACHE_DO_CHECKSUMS
	seq_printf(seq,  "checksum_store=%ld checksum_valid=%ld checksum_invalid=%ld ",
		stats->checksum_store, stats->checksum_valid, stats->checksum_invalid);
#endif
	seq_printf(seq,  "direct:\t%lu\n", stats->direct);
	seq_printf(seq,  "pending_enqueues:\t%lu\n", stats->enqueues);
	seq_printf(seq,  "pending_inval:\t%lu\n", stats->pending_inval);

	seq_printf(seq, "no_room:\t%lu\n",
		   stats->noroom);

	seq_printf(seq,  "disk_reads:\t%lu\n", stats->disk_reads);
	seq_printf(seq,  "disk_writes:\t%lu\n", stats->disk_writes);
	seq_printf(seq,  "ssd_reads:\t%lu\n", stats->ssd_reads);
	seq_printf(seq,  "ssd_writes:\t%lu\n", stats->ssd_writes);

	seq_printf(seq,  "uncached_reads:\t%lu\n", stats->uncached_reads);
	seq_printf(seq,  "uncached_writes:\t%lu\n", stats->uncached_writes);
	seq_printf(seq,  "uncached_IO_requeue:\t%lu\n", stats->uncached_io_requeue);

	seq_printf(seq,  "uncached_sequential_reads:\t%lu\n", stats->uncached_sequential_reads);
	seq_printf(seq,  "uncached_sequential_writes:\t%lu\n", stats->uncached_sequential_writes);

	seq_printf(seq,  "total_blocks:\t%lu\n", dmc->size);
	seq_printf(seq,  "cached_blocks:\t%lu\n", dmc->cached_blocks);

	return 0;
}

static int 
flashcache_stats_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	//	[2014-01-16 by LIN_WC]:Kernel 3.12.X parameter change
	return single_open(file, &flashcache_stats_show, PDE_DATA(inode));
#else
	return single_open(file, &flashcache_stats_show, PDE(inode)->data);
#endif
}

static struct file_operations flashcache_stats_operations = {
	.open		= flashcache_stats_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int 
flashcache_errors_show(struct seq_file *seq, void *v)
{
	struct cache_c *dmc = seq->private;

	seq_printf(seq, "disk_read_errors=%d disk_write_errors=%d ",
		   dmc->flashcache_errors.disk_read_errors, 
		   dmc->flashcache_errors.disk_write_errors);
	seq_printf(seq, "ssd_read_errors=%d ssd_write_errors=%d ",
		   dmc->flashcache_errors.ssd_read_errors, 
		   dmc->flashcache_errors.ssd_write_errors);
	seq_printf(seq, "memory_alloc_errors=%d\n", 
		   dmc->flashcache_errors.memory_alloc_errors);
	seq_printf(seq, "ssd_errors=%d\n", 
		   dmc->flashcache_errors.ssd_errors);
	seq_printf(seq, "verify_errors1=%d\n",
		   dmc->flashcache_errors.verify_errors1);
	seq_printf(seq, "verify_errors2=%d\n",
		   dmc->flashcache_errors.verify_errors2);
	seq_printf(seq, "verify_errors3=%d\n",
		   dmc->flashcache_errors.verify_errors3);
	seq_printf(seq, "verify_errors4=%d\n",
		   dmc->flashcache_errors.verify_errors4);
	seq_printf(seq, "verify_errors5=%d\n",
		   dmc->flashcache_errors.verify_errors5);
	return 0;
}

static int 
flashcache_errors_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	//      [2014-01-16 by LIN_WC]:Kernel 3.12.X parameter change
	return single_open(file, &flashcache_errors_show, PDE_DATA(inode));
#else
	return single_open(file, &flashcache_errors_show, PDE(inode)->data);
#endif
}

static struct file_operations flashcache_errors_operations = {
	.open		= flashcache_errors_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int 
flashcache_iosize_hist_show(struct seq_file *seq, void *v)
{
	int i;
	
	for (i = 0 ; i <= 10 ; i++) {
		seq_printf(seq, "%d:%llu ", 1<<i, size_hist[i]);
	}
	seq_printf(seq, "\n");
	return 0;
}

static int 
flashcache_iosize_hist_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
        //      [2014-01-16 by LIN_WC]:Kernel 3.12.X parameter change
	return single_open(file, &flashcache_iosize_hist_show, PDE_DATA(inode));
#else
	return single_open(file, &flashcache_iosize_hist_show, PDE(inode)->data);
#endif
}

static struct file_operations flashcache_iosize_hist_operations = {
	.open		= flashcache_iosize_hist_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#ifdef FLASHCACHE_PID_CONTROL
static int 
flashcache_pidlists_show(struct seq_file *seq, void *v)
{
	struct cache_c *dmc = seq->private;
	struct flashcache_cachectl_pid *pid_list;
 	unsigned long flags;
	
	spin_lock_irqsave(&dmc->cache_spin_lock, flags);
	seq_printf(seq, "Blacklist: ");
	pid_list = dmc->blacklist_head;
	while (pid_list != NULL) {
		seq_printf(seq, "%u ", pid_list->pid);
		pid_list = pid_list->next;
	}
	seq_printf(seq, "\n");
	seq_printf(seq, "Whitelist: ");
	pid_list = dmc->whitelist_head;
	while (pid_list != NULL) {
		seq_printf(seq, "%u ", pid_list->pid);
		pid_list = pid_list->next;
	}
	seq_printf(seq, "\n");
	spin_unlock_irqrestore(&dmc->cache_spin_lock, flags);
	return 0;
}

static int 
flashcache_pidlists_open(struct inode *inode, struct file *file)
{
	return single_open(file, &flashcache_pidlists_show, PDE(inode)->data);
}

static struct file_operations flashcache_pidlists_operations = {
	.open		= flashcache_pidlists_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif /* end of FLASHCACHE_PID_CONTROL define */

extern char *flashcache_sw_version;

static int 
flashcache_version_show(struct seq_file *seq, void *v)
{
	seq_printf(seq, "Flashcache Version : %s\n", flashcache_sw_version);
#ifdef COMMIT_REV
	seq_printf(seq, "git commit: %s\n", COMMIT_REV);
#endif
	return 0;
}

static int 
flashcache_version_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
        //      [2014-01-16 by LIN_WC]:Kernel 3.12.X parameter change
	return single_open(file, &flashcache_version_show, PDE_DATA(inode));
#else
	return single_open(file, &flashcache_version_show, PDE(inode)->data);
#endif
}

static struct file_operations flashcache_version_operations = {
	.open		= flashcache_version_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

void
flashcache_module_procfs_init(void)
{
#ifdef CONFIG_PROC_FS
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
        //      [2014-01-16 by LIN_WC]:Kernel 3.12.X "create_proc_entry" has been change to proc_create
	if (proc_mkdir("flashcache", NULL))
		proc_create("flashcache/flashcache_version", 0, NULL, &flashcache_version_operations);
#else
	struct proc_dir_entry *entry;

	if (proc_mkdir("flashcache", NULL)) {
		entry = create_proc_entry("flashcache/flashcache_version", 0, NULL);
		if (entry)
			entry->proc_fops =  &flashcache_version_operations;
	}
#endif
#endif /* CONFIG_PROC_FS */
}

void
flashcache_module_procfs_releae(void)
{
#ifdef CONFIG_PROC_FS
	(void)remove_proc_entry("flashcache/flashcache_version", NULL);
	(void)remove_proc_entry("flashcache", NULL);
#endif /* CONFIG_PROC_FS */
}

static char *
flashcache_cons_sysctl_devname(struct cache_c *dmc)
{
	char *pathname;
	
	pathname = kzalloc(strlen(dmc->groupname) + 1,
			   GFP_KERNEL);
	strcpy(pathname, dmc->groupname);
	return pathname;
}

static char *
flashcache_cons_procfs_cachename(struct cache_c *dmc, char *path_component)
{
	char *pathname;
	char *s;
	
	pathname = kzalloc(strlen(dmc->groupname) + 4 + 
			   strlen(FLASHCACHE_PROC_ROOTDIR_NAME) + 
			   strlen(path_component), 
			   GFP_KERNEL);
	strcpy(pathname, FLASHCACHE_PROC_ROOTDIR_NAME);
	strcat(pathname, "/");
	s = strrchr(dmc->groupname, '/');
	if (s) 
		s++;
	else
		s = dmc->groupname;
	strcat(pathname, s);
	if (strcmp(path_component, "") != 0) {
		strcat(pathname, "/");
		strcat(pathname, path_component);
	}
	return pathname;
}

void 
flashcache_ctr_procfs(struct cache_c *dmc)
{
	char *s;
	struct proc_dir_entry *entry;

	s =  flashcache_cons_procfs_cachename(dmc, "");
	entry = proc_mkdir(s, NULL);
	kfree(s);
	if (entry == NULL)
		return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
        //      [2014-01-16 by LIN_WC]:Kernel 3.12.X "create_proc_entry" has been change to proc_create[_data]
	s = flashcache_cons_procfs_cachename(dmc, "flashcache_stats");
	proc_create_data(s, 0, NULL, &flashcache_stats_operations, dmc);
	kfree(s);

	s = flashcache_cons_procfs_cachename(dmc, "flashcache_errors");
	proc_create_data(s, 0, NULL, &flashcache_errors_operations, dmc);
	kfree(s);

	s = flashcache_cons_procfs_cachename(dmc, "flashcache_iosize_hist");
	proc_create_data(s, 0, NULL, &flashcache_iosize_hist_operations, dmc);
	kfree(s);
#else
	s = flashcache_cons_procfs_cachename(dmc, "flashcache_stats");
	entry = create_proc_entry(s, 0, NULL);
	if (entry) {
		entry->proc_fops =  &flashcache_stats_operations;
		entry->data = dmc;
	}
	kfree(s);

	s = flashcache_cons_procfs_cachename(dmc, "flashcache_errors");
	entry = create_proc_entry(s, 0, NULL);
	if (entry) {
		entry->proc_fops =  &flashcache_errors_operations;
		entry->data = dmc;
	}
	kfree(s);

	s = flashcache_cons_procfs_cachename(dmc, "flashcache_iosize_hist");
	entry = create_proc_entry(s, 0, NULL);
	if (entry) {
		entry->proc_fops =  &flashcache_iosize_hist_operations;
		entry->data = dmc;
	}
	kfree(s);
#endif

#ifdef FLASHCACHE_PID_CONTROL
	s = flashcache_cons_procfs_cachename(dmc, "flashcache_pidlists");
	entry = create_proc_entry(s, 0, NULL);
	if (entry) {
		entry->proc_fops =  &flashcache_pidlists_operations;
		entry->data = dmc;			
	}
	kfree(s);
#endif

	flashcache_writethrough_sysctl_register(dmc);
}

void 
flashcache_dtr_procfs(struct cache_c *dmc)
{
	char *s;
	
	s = flashcache_cons_procfs_cachename(dmc, "flashcache_stats");
	remove_proc_entry(s, NULL);
	kfree(s);

	s = flashcache_cons_procfs_cachename(dmc, "flashcache_errors");
	remove_proc_entry(s, NULL);
	kfree(s);

	s = flashcache_cons_procfs_cachename(dmc, "flashcache_iosize_hist");
	remove_proc_entry(s, NULL);
	kfree(s);

#ifdef FLASHCACHE_PID_CONTROL
	s = flashcache_cons_procfs_cachename(dmc, "flashcache_pidlists");
	remove_proc_entry(s, NULL);
	kfree(s);
#endif

	s = flashcache_cons_procfs_cachename(dmc, "");
	remove_proc_entry(s, NULL);
	kfree(s);

	flashcache_writethrough_sysctl_unregister(dmc);

}

