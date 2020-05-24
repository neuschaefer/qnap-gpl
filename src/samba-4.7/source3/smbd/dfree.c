/* 
   Unix SMB/CIFS implementation.
   functions to calculate the free disk space
   Copyright (C) Andrew Tridgell 1998
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "includes.h"
#include "smbd/smbd.h"
#include "smbd/globals.h"
#include "lib/util_file.h"
#include "lib/util/memcache.h"

#ifdef QNAPNAS
/* QNAPNAS_TIMEMACHINE
 * Bug#94540 (KS-Redmine#23682)
 *
 * Step 1 : Get band-size info from Info.plist
 * Step 2 : Count files on entries in the 'bands' directory
 * Step 3 : Calculate used size as: (file_count - 1) * band-size
 */

/*!
 * Read band-size info from Info.plist XML file of an TM sparsebundle
 *
 * @param path   (r) path to Info.plist file
 * @return           band-size in bytes, -1 on error
 */
static long long int get_tm_bandsize(const char *path)
{
	FILE *file = NULL;
	char buf[512];
	char bandsize_string[51] = {0};
	long long int bandsize = -1;

	file = fopen(path, "r");
	if (file == NULL) {
		DEBUG(3,("get_tm_bandsize(\"%s\"): %s\n", path, strerror(errno)));
		goto cleanup;
	}

	while (fgets(buf, sizeof(buf), file) != NULL) {
		if (strstr(buf, "band-size") == NULL)
			continue;

		if (fscanf(file, " <integer>%50s</integer>", bandsize_string) != 1) {
			DEBUG(3,("get_tm_bandsize(\"%s\"): can't parse band-size\n", path));
			goto cleanup;
		} else {
			bandsize = atoll(bandsize_string);
		}
		break;
	}

cleanup:
	if (file) {
		fclose(file);
	}

	DEBUG(3,("get_tm_bandsize(\"%s\"): bandsize: %lld\n", path, bandsize));
	return bandsize;
}

/*!
 * Return number on entries in a directory
 *
 * @param path   (r) path to dir
 * @return           number of entries, -1 on error
 */
static long long int get_tm_bands(const char *path)
{
	long long int count = 0;

	DIR *dir = opendir(path);
	if (dir == NULL) {
		return -1;
	}

	while (readdir(dir) != NULL) {
		count++;
	}

	count -= 2; /* All OSens I'm aware of return "." and "..", so just substract them, avoiding string comparison in loop */

	closedir(dir);

	return count;
}

/*!
 * Calculate used size of a TimeMachine volume
 *
 * This assumes that the volume is used only for TimeMachine.
 *
 * 1) readdir(path of volume)
 * 2) for every element that matches regex "\(.*\)\.sparsebundle$" :
 * 3) parse "\1.sparsebundle/Info.plist" and read the band-size XML key integer value
 * 4) readdir "\1.sparsebundle/bands/" counting files
 * 5) calculate used size as: (file_count - 1) * band-size
 *
 * @param vol     (rw) volume to calculate
 * @return             0 on success, -1 on error
 */
static int get_tm_used(char *tm_dir, uint64_t *tm_used)
{
	long long int bandsize;
	uint64_t used = 0;
	DIR *dir = NULL;
	const struct dirent *entry;
	const char *p;
	long int links;
	char *tm_path = tm_dir;
	char infoplist[4096];
	char bandsdir[4096];
	size_t sparsebundle_strlen = strlen("sparsebundle");
	size_t backupbundle_strlen = strlen("backupbundle");

	memset(infoplist,0,sizeof(infoplist));
	memset(bandsdir,0,sizeof(bandsdir));

	dir = opendir(tm_path);
	if (dir == NULL) {
		return -1;
	}

	while ((entry = readdir(dir)) != NULL) {
		if ((((p = strstr(entry->d_name, "sparsebundle")) != NULL) && (p[sparsebundle_strlen] == '\0'))
			|| (((p = strstr(entry->d_name, "backupbundle")) != NULL) && (p[backupbundle_strlen] == '\0'))) {
			snprintf(infoplist,sizeof(infoplist),"%s/%s/%s", tm_path, entry->d_name, "Info.plist");

			if ((bandsize = get_tm_bandsize(infoplist)) == -1) {
				continue;
			}

			snprintf(bandsdir,sizeof(bandsdir),"%s/%s/%s/", tm_path, entry->d_name, "bands");

			if ((links = get_tm_bands(bandsdir)) == -1) {
				continue;
			}

			used += (links - 1) * bandsize;
			DEBUG(3,("getused(\"%s\"): bands: %lld bytes\n",bandsdir, used));
		}
	}

    if (dir) {
        closedir(dir);
    }

	*tm_used = (used/1024);
	DEBUG(3,("\ngetused(\"%s\"): %lld bytes\n", tm_path, used));

	return 0;
}
#endif  /* QNAPNAS */

/****************************************************************************
 Normalise for DOS usage.
****************************************************************************/

static void disk_norm(uint64_t *bsize, uint64_t *dfree, uint64_t *dsize)
{
	/* check if the disk is beyond the max disk size */
	uint64_t maxdisksize = lp_max_disk_size();
	if (maxdisksize) {
		/* convert to blocks - and don't overflow */
		maxdisksize = ((maxdisksize*1024)/(*bsize))*1024;
		if (*dsize > maxdisksize) {
			*dsize = maxdisksize;
		}
		if (*dfree > maxdisksize) {
			*dfree = maxdisksize - 1;
		}
		/* the -1 should stop applications getting div by 0
		   errors */
	}
}



/****************************************************************************
 Return number of 1K blocks available on a path and total number.
****************************************************************************/

uint64_t sys_disk_free(connection_struct *conn, struct smb_filename *fname,
		       uint64_t *bsize, uint64_t *dfree, uint64_t *dsize)
{
	uint64_t dfree_retval;
	uint64_t dfree_q = 0;
	uint64_t bsize_q = 0;
	uint64_t dsize_q = 0;
	const char *dfree_command;
	static bool dfree_broken = false;
	const char *path = fname->base_name;

	(*dfree) = (*dsize) = 0;
	(*bsize) = 512;

	/*
	 * If external disk calculation specified, use it.
	 */

	dfree_command = lp_dfree_command(talloc_tos(), SNUM(conn));
	if (dfree_command && *dfree_command) {
		const char *p;
		char **lines = NULL;
		char *syscmd = NULL;

		syscmd = talloc_asprintf(talloc_tos(),
				"%s %s",
				dfree_command,
				path);

		if (!syscmd) {
			return (uint64_t)-1;
		}

		DEBUG (3, ("disk_free: Running command '%s'\n", syscmd));

		lines = file_lines_pload(talloc_tos(), syscmd, NULL);
		if (lines != NULL) {
			char *line = lines[0];

			DEBUG (3, ("Read input from dfree, \"%s\"\n", line));

			*dsize = STR_TO_SMB_BIG_UINT(line, &p);
			while (p && *p && isspace(*p))
				p++;
			if (p && *p)
				*dfree = STR_TO_SMB_BIG_UINT(p, &p);
			while (p && *p && isspace(*p))
				p++;
			if (p && *p)
				*bsize = STR_TO_SMB_BIG_UINT(p, NULL);
			else
				*bsize = 1024;
			TALLOC_FREE(lines);
			DEBUG (3, ("Parsed output of dfree, dsize=%u, dfree=%u, bsize=%u\n",
				(unsigned int)*dsize, (unsigned int)*dfree, (unsigned int)*bsize));

			if (!*dsize)
				*dsize = 2048;
			if (!*dfree)
				*dfree = 1024;

			goto dfree_done;
		}
		DEBUG (0, ("disk_free: file_lines_load() failed for "
			   "command '%s'. Error was : %s\n",
			   syscmd, strerror(errno) ));
	}

	if (SMB_VFS_DISK_FREE(conn, fname, bsize, dfree, dsize) ==
	    (uint64_t)-1) {
		DBG_ERR("VFS disk_free failed. Error was : %s\n",
			strerror(errno));
		return (uint64_t)-1;
	}

	if (disk_quotas(conn, fname, &bsize_q, &dfree_q, &dsize_q)) {
		uint64_t min_bsize = MIN(*bsize, bsize_q);

		(*dfree) = (*dfree) * (*bsize) / min_bsize;
		(*dsize) = (*dsize) * (*bsize) / min_bsize;
		dfree_q = dfree_q * bsize_q / min_bsize;
		dsize_q = dsize_q * bsize_q / min_bsize;

		(*bsize) = min_bsize;
		(*dfree) = MIN(*dfree,dfree_q);
		(*dsize) = MIN(*dsize,dsize_q);
	}

	/* FIXME : Any reason for this assumption ? */
	if (*bsize < 256) {
		DEBUG(5,("disk_free:Warning: bsize == %d < 256 . Changing to assumed correct bsize = 512\n",(int)*bsize));
		*bsize = 512;
	}

	if ((*dsize)<1) {
		if (!dfree_broken) {
			DEBUG(0,("WARNING: dfree is broken on this system\n"));
			dfree_broken=true;
		}
		*dsize = 20*1024*1024/(*bsize);
		*dfree = MAX(1,*dfree);
	}

dfree_done:
	disk_norm(bsize, dfree, dsize);

	if ((*bsize) < 1024) {
		dfree_retval = (*dfree)/(1024/(*bsize));
	} else {
		dfree_retval = ((*bsize)/1024)*(*dfree);
	}

	return(dfree_retval);
}

/****************************************************************************
 Potentially returned cached dfree info.

 Depending on the file system layout and file system features, the free space
 information can be different for different sub directories underneath a SMB
 share. Store the cache information in memcache using the query path as the
 key to accomodate this.
****************************************************************************/

uint64_t get_dfree_info(connection_struct *conn, struct smb_filename *fname,
			uint64_t *bsize, uint64_t *dfree, uint64_t *dsize)
{
	int dfree_cache_time = lp_dfree_cache_time(SNUM(conn));
	struct dfree_cached_info *dfc = NULL;
	struct dfree_cached_info dfc_new = { 0 };
	uint64_t dfree_ret;
	char tmpbuf[PATH_MAX];
	char *full_path = NULL;
	char *to_free = NULL;
	char *key_path = NULL;
	size_t len;
	DATA_BLOB key, value;
	bool found;

#ifdef QNAPNAS
	/* QNAPNAS_TIMEMACHINE
	 * Bug#94540 (KS-Redmine#23682)
	 */
	if (get_current_uid(conn) == (uid_t)100) {
		char *path = lp_path(talloc_tos(), SNUM(conn));
		if ((path != NULL) && (strstr_m(path,".timemachine") != NULL)) {
			uint64_t tm_used = 0;
			uint64_t limitsize = lp_vol_size_limit(SNUM(conn));
			if (limitsize != 0 && get_tm_used(path,&tm_used) == 0) {
				*dsize = limitsize*1024;
				*dfree = (*dsize < tm_used) ? 0 : (*dsize - tm_used);
				*bsize = 1024;
				return *dfree;
			}
		}
	}
#endif /* QNAPNAS */

	if (!dfree_cache_time) {
		return sys_disk_free(conn, fname, bsize, dfree, dsize);
	}

	len = full_path_tos(conn->connectpath,
			    fname->base_name,
			    tmpbuf,
			    sizeof(tmpbuf),
			    &full_path,
			    &to_free);
	if (len == -1) {
		errno = ENOMEM;
		return -1;
	}

	if (VALID_STAT(fname->st) && S_ISREG(fname->st.st_ex_mode)) {
		/*
		 * In case of a file use the parent directory to reduce number
		 * of cache entries.
		 */
		bool ok;

		ok = parent_dirname(talloc_tos(),
				    full_path,
				    &key_path,
				    NULL);
		TALLOC_FREE(to_free); /* We're done with full_path */

		if (!ok) {
			errno = ENOMEM;
			return -1;
		}

		/*
		 * key_path is always a talloced object.
		 */
		to_free = key_path;
	} else {
		/*
		 * key_path might not be a talloced object; rely on
		 * to_free set from full_path_tos.
		 */
		key_path = full_path;
	}

	key = data_blob_const(key_path, strlen(key_path));
	found = memcache_lookup(smbd_memcache(),
				DFREE_CACHE,
				key,
				&value);
	dfc = found ? (struct dfree_cached_info *)value.data : NULL;

	if (dfc && (conn->lastused - dfc->last_dfree_time < dfree_cache_time)) {
		DBG_DEBUG("Returning dfree cache entry for %s\n", key_path);
		*bsize = dfc->bsize;
		*dfree = dfc->dfree;
		*dsize = dfc->dsize;
		dfree_ret = dfc->dfree_ret;
		goto out;
	}

	dfree_ret = sys_disk_free(conn, fname, bsize, dfree, dsize);

	if (dfree_ret == (uint64_t)-1) {
		/* Don't cache bad data. */
		goto out;
	}

	DBG_DEBUG("Creating dfree cache entry for %s\n", key_path);
	dfc_new.bsize = *bsize;
	dfc_new.dfree = *dfree;
	dfc_new.dsize = *dsize;
	dfc_new.dfree_ret = dfree_ret;
	dfc_new.last_dfree_time = conn->lastused;
	memcache_add(smbd_memcache(),
		     DFREE_CACHE,
		     key,
		     data_blob_const(&dfc_new, sizeof(dfc_new)));

out:
	TALLOC_FREE(to_free);
	return dfree_ret;
}

void flush_dfree_cache(void)
{
	memcache_flush(smbd_memcache(), DFREE_CACHE);
}
