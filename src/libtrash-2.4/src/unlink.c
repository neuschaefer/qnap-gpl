/* Copyright 2001, 2002, 2003, 2004 Manuel Arriaga
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/stat.h>

#include "trash.h"

#ifdef QNAPNAS
#ifdef DEBUG
#define __dbg(format, arg...)            \
        do {\
                FILE *fp; \
                fp = fopen("/debug","a+"); \
                fprintf(fp, format , ## arg); \
                fclose(fp); \
        } while (0);
#else
#define __dbg(format, arg...)    do{} while(0);
#endif
#endif

static int unlink_handle_error(const char *pathname, int (*real_unlink) (const char*),
			       int in_case_of_failure);
static int convert_team_folder_original_path(char **ptr_abs_path);
static int file_is_exist(const char *filename);

/* What this version of unlink() does: if everything is OK, we just rename() the given file instead of
 * unlink()ing it, putting it under absolute_trash_can.
 *
 * We also take care so that, if we fail, errno is either zero (if a
 * "libtrash-specific" error occurred) or has a meaningful value which the
 * caller should know how to interpret after a call to unlink(). This way,
 * we avoid confusing the caller with a errno set by some other GNU libc
 * function used by the libtrash wrapper.
 *
 */

int unlink(const char *pathname)
{
   struct stat path_stat;

   char *absolute_path = NULL;

   int symlink = 0;

   int error = 0;

   int retval = 0;

   int file_should = 0;

   /* Create a config structure, in which all configuration settings will be placed: */

   config cfg;

#ifdef DEBUG
   fprintf(stderr, "\nEntering unlink().\n");
#endif

   /* Run init(), which sets all the global variables: */
#ifdef RECYCLE_EX
    __dbg("%s %d file %s pid %d\n", __func__, __LINE__, pathname, getpid());
#endif
   init(&cfg);

   /* We always call fini() before quitting. */

   /* Isn't a pointer to GNU libc's unlink() available? In that case, there's nothing we can do: */

   if (cfg.real_unlink_unavailable)
     {
#ifdef DEBUG
	fprintf(stderr, "real_unlink_unavailable is set. unlink() returning error code.\n");
#endif
	errno = 0;

	fini(&cfg);
	return -1; /* errno set to 0 in order to avoid confusing the caller. */
     }

   /* If libtrash_off is set to true or intercept_unlink set to false, the user has asked us to become temporarily inactive an let the real
    * unlink() perform its task.
    Alternatively, if we were passed a NULL pointer we also call the real unlink: */

   if (cfg.libtrash_off || !cfg.intercept_unlink || pathname == NULL)
     {
#ifdef DEBUG
       if (!pathname)
	 fprintf(stderr, "unlink was passed a NULL pointer, calling real unlink.\n");
       else
	 fprintf(stderr, "Passing request to unlink %s to the real unlink because libtrash_off = true or intercept_unlink = false.\n", pathname);
#endif
       fini(&cfg);
       return (*cfg.real_unlink) (pathname); /* real unlink() sets errno */
     }

   /* If general_failure is set, something went wrong while initializing and we should just invoke the real function: */

   if (cfg.general_failure)
     {
#ifdef RECYCLE_EX
    __dbg("%s %d pid %d: general failure\n", __func__, __LINE__, getpid());
#endif

#ifdef DEBUG
	fprintf(stderr, "general_failure is set in unlink(), invoking unlink_handle_error().\n"
		"in_case_of_failure has value %d.\n", cfg.in_case_of_failure);
#endif
	fini(&cfg);
	return unlink_handle_error(pathname, cfg.real_unlink, cfg.in_case_of_failure); /* If in_case_of_failure is set to PROTECT, we return -1 with errno set to 0;
											otherwise, the real unlink() sets errno. */
     }

   /* First of all: has the user mistakenly asked us to remove either a missing file, a special file or a directory?
    * In any of these cases we, just let the normal unlink() complain about it and save ourselves the
    * extra trouble: */

   error = lstat(pathname, &path_stat);

   if (( error && errno == ENOENT)              ||
       (!error && S_ISDIR(path_stat.st_mode))   ||
       (!error && !S_ISREG(path_stat.st_mode) && !S_ISLNK(path_stat.st_mode)) )
     {
#ifdef RECYCLE_EX
	__dbg("%s %d pid %d [%s] either doesn't exit, or is a special file (non-symlink) or is a directory \n", __func__, __LINE__, getpid(), pathname);
#endif
#ifdef DEBUG
	fprintf(stderr, "%s either doesn't exit, or is a special file (non-symlink) or is a directory.\nCalling the \"real\" unlink().\n", pathname);
#endif
	fini(&cfg);
	return (*cfg.real_unlink) (pathname); /* real unlink() sets errno. */
     }

   /* The empty file (zero size) will call real unlink */
   if (path_stat.st_size == 0)
     {
#ifdef DEBUG
	fprintf(stderr, "%s is zero size file.\nCalling the \"real\" unlink().\n", pathname);
#endif
	fini(&cfg);
	return (*cfg.real_unlink) (pathname); /* real unlink() sets errno. */
     }

   /* If this is a symlink, we set symlink. We don't call the real unlink() immediately because we don't remove
    * symlinks under protected directories:
    */

   if (S_ISLNK(path_stat.st_mode))
     symlink = YES;
   else
     symlink = NO;

   /* Let us begin by building an absolute path, if we weren't passed one: */

   /* [If no error occurred, the memory absolute_path now points to was dynamically allocated.
    That is the reason why we free absolute_path before returning.] */

   /* We have a problem: if the pathname contains references to symlinks to directories (e.g.,
    /tmp/syml/test, where /tmp/syml is a link to /var/syml, the real
    canonical path being /var/syml/test), we want to resolve those (because
    the real unlink follows them), but if the filename in pathname (i.e.,
    the last "component" - "test", in the example above) is a symlink
    itself, then we DON'T want to resolve it because the real unlink will
    delete it, not the file it points at. For that reason, rather than
    directly invoking canonicalize_file_name(), we call
    build_absolute_path(), which takes care of these complications for us:
    */

   absolute_path = build_absolute_path(pathname);

   if (!absolute_path)
     {
#ifdef RECYCLE_EX
	__dbg("%s %d pid %d Unable to build absolute_path \n", __func__, __LINE__, getpid());
#endif
#ifdef DEBUG
	fprintf(stderr, "Unable to build absolute_path.\nInvoking unlink_handle_error().\n");
#endif
	fini(&cfg);
	return unlink_handle_error(pathname, cfg.real_unlink, cfg.in_case_of_failure); /* about errno: the same as in the other call to unlink_handle_error() above. */
     }

#ifdef RECYCLE_EX
    /* generate abs trash can */
    __dbg("%s %d absolute_path[%s] absolute_trash_can[%s] relative_trash_can[%s] cfg.home[%s] pid[%d]\n", 
	__func__, __LINE__, absolute_path, cfg.absolute_trash_can, cfg.relative_trash_can , cfg.home, getpid());
    int number_of_slash = 0, i = 0;
	char *teamfolder=NULL;
	if(NULL != (teamfolder = strstr(absolute_path, "/.team_folder/")))
	{
		char *homes=NULL;
		if(NULL != (homes = strstr(pathname, "/homes/")))
		{
			char *tmp=NULL;
			*teamfolder = 0;
			tmp = (char*)malloc(strlen(absolute_path)+strlen(homes)+1);
			sprintf(tmp, "%s%s", absolute_path, homes);
			free(absolute_path);
			absolute_path = tmp;
		}else{
			/* 
			    convert /share/HDA_DATA/.team_folder/$id/xxx to 
				    /share/CACHEDEV1_DATA/homes/admin/.Qsync/$team_folder/xxx 
			*/
			convert_team_folder_original_path(&absolute_path);
	
		}
	}
	
    /* i.e. /share/HDA_DATA/Download/xxxx */
    for(number_of_slash = 0, i = 0; i < strlen(absolute_path); i++){
	if(absolute_path[i] == '/')
	    number_of_slash++;
	if(number_of_slash == 4){
	    absolute_path[i] = '\0';
	    free(cfg.absolute_trash_can);
	    free(cfg.home);
	    cfg.home = malloc(strlen(absolute_path) + 1);
	    sprintf(cfg.home, "%s", absolute_path);
	    cfg.absolute_trash_can = malloc(strlen(absolute_path) + 1 + strlen(cfg.relative_trash_can) + 1);
	    sprintf(cfg.absolute_trash_can, "%s/%s", absolute_path, cfg.relative_trash_can);
	    absolute_path[i] = '/';
	    __dbg("%s %d [%s]\n", __func__, __LINE__, cfg.absolute_trash_can);
            break;
	}
    }
    if(number_of_slash != 4)
    {
	__dbg("%s %d the file not belong to share folders, call real unlink\n", __func__, __LINE__);
	fini(&cfg);
	free(absolute_path);
	return (*cfg.real_unlink) (pathname); /* real unlink() sets errno */
    }

#endif

   /* Independently of the way in which the argument was written, absolute_path now holds the absolute
    * path to the file, free of any "/../" or "/./" and with the any symlinks (except for the filename itself,
    if it is one) resolved.
    */

#ifdef DEBUG
   fprintf(stderr, "CLEANSED ABSOLUTE_PATH:   |%s|\n", absolute_path);
#endif

   /* By now we want to know whether this file "qualifies" to be stored in the trash can rather than deleted (another
    possible option is this file being considered "unremovable"). This decision is taken according to the user's preferences
    by the function decide_action(): */

   file_should = decide_action(absolute_path, &cfg);

   switch (file_should)
     {

      case BE_REMOVED:
#ifdef RECYCLE_EX
	__dbg("%s %d BE_REMOVED %s\n", __func__, __LINE__, pathname);
#endif
#ifdef DEBUG
	fprintf(stderr, "decide_action() told unlink() to permanently destroy file %s.\n", absolute_path);
#endif

	retval = (*cfg.real_unlink) (pathname); /* real unlink() sets errno. */

	break;

      case BE_LEFT_UNTOUCHED:
#ifdef RECYCLE_EX
	__dbg("%s %d BE_LEFT_UNTOUCHED %s\n", __func__, __LINE__, pathname);
#endif

#ifdef DEBUG
	fprintf(stderr, "decide_action() told unlink() to leave file %s untouched.\n", absolute_path);
#endif

	retval = -1;

	/* Set errno to EACCESS, which the caller will interpret as "insufficient permissions": */

	errno = EACCES;

	break;

      case BE_SAVED:
#ifdef RECYCLE_EX
	__dbg("%s %d BE_SAVED %s\n", __func__, __LINE__, absolute_path);
#endif

#ifdef DEBUG
	fprintf(stderr, "decide_action() told unlink() to save a copy of file %s.\n", absolute_path);
#endif

	/* But if it is a symlink we refuse to "save" it nonetheless: */

	if (symlink)
	  {
#ifdef DEBUG
	     fprintf(stderr, " but its suggestion is being ignored because %s is just a symlink.\n", absolute_path);
#endif
	     retval = (*cfg.real_unlink) (pathname); /* real unlink() sets errno. */
	  }
	else
	  {
	     /* (See below for information on this code.) */

	     /* see (0) */
#ifdef RECYCLE_EX 
	/* Due to performance consider and per-share recycle path is not static path. 
           We check recycle bin folder instead of init function */
	    //__dbg("%s %d can[%s] abs[%s] home[%s]\n",__func__,__LINE__, cfg.absolute_trash_can, absolute_path, cfg.home);
	    if(!dir_ok_ex(cfg.absolute_trash_can, NULL, &cfg)){
		__dbg("%s %d pid %d dir_ok fail[%s]\n", __func__, __LINE__, getpid(), cfg.absolute_trash_can);
		free(absolute_path);
		fini(&cfg);
		return unlink_handle_error(pathname, cfg.real_unlink, cfg.in_case_of_failure);
	    }
	    make_home_symblink(absolute_path, &cfg);
	    gen_windows_icon(cfg.absolute_trash_can);
#endif
	     if (found_under_dir(absolute_path, cfg.home))
	       retval = graft_file(cfg.absolute_trash_can, absolute_path, cfg.home, &cfg);
	     else
	       retval = graft_file(cfg.absolute_trash_system_root, absolute_path, NULL, &cfg);
	    
#ifdef RECYCLE_EX
		__dbg("%s %d pid %d ret = %d\n", __func__, __LINE__, getpid(), retval);
#endif

	     if (retval == -2) /* see (1) */
	       retval = -1;
	     else           /* see (2) */
	       errno = 0;

#ifdef DEBUG
	     fprintf(stderr, "graft_file(), called by unlink(), returned %d.\n", retval);
#endif
	  }

	break;

     }

   /* Notes on case BE_SAVED:
    *
    *
    * (0) if the file lies under the user's home directory, we "graft" it
    * into the user's trash can; if the file lies outside of the user's
    * home directory and global_protection is set we graft it to the
    * directory SYSTEM_ROOT under the user's trash can.
    *
    *
    * (1) graft_file() failed due to the lack of write-access (either
    * to the directory which holds this file or to the file itself
    * - the latter if we tried to "manually" move() the file). In
    * these situations, we leave errno untouched because it
    * contains meaningful information (EACCES, EPERM or EROFS),
    * just converting retval to -1 (which is the correct error code
    * for unlink()
    *
    * (2) if graft_file() either succeeded or failed for some other reason:
    * just zero errno and return retval, which contains a valid (and correct)
    * error code.
    */

   /* Free memory before quitting: */

   free(absolute_path);

   fini(&cfg);
   return retval;
}

/* ------------------------------------------------------------------------------------ */

/* This function is called by unlink() in case an error is spotted. It does what the user
 * told us to do in these situations (through the argument in_case_of_failure), and unlink() just
 * returns the value passed by unlink_trash_error():
 */

static int unlink_handle_error(const char *pathname, int (*real_unlink) (const char*), int in_case_of_failure)
{
   if (in_case_of_failure == ALLOW_DESTRUCTION)
     return (*real_unlink) (pathname); /* the real unlink() sets errno */
   else /* if (in_case_of_failure == PROTECT) */
     {
	errno = 0;
	return -1; /* errno is set to zero to avoid confusing the caller. */
     }

}

/*
 * Desc: Check file is exist 
 * @return: 1 for success
 */
static int file_is_exist(const char *filename)
{
    struct stat status;

    if (0 == stat(filename, &status))
        return 1;
    return 0;
}

static int convert_team_folder_original_path(char **ptr_abs_path)
{
    char *abs_path = NULL;
    char info_path[256];
    int len = 0;

    abs_path = *ptr_abs_path;

    if(!abs_path)
	return 0;

    /* /share/CACHEDEV1_DATA/.team_folder/8bf2a6ab926ff10011f3c5004b01e930119ffcae */

    char *ptr = strstr(abs_path, "/.team_folder/");
    if(!ptr)
	return 0;

    len = (ptr - abs_path + 54); 

    if(len > strlen(abs_path))
	return 0;
    
    /* avoid /share/CACHEDEV1_DATA/.team_folder/8bf2a6ab926ff10011f3c5004b01e930119ffcae.perm */
    if(abs_path[len] != '/'){
	return 0;
    }

    /* get original path */
    strncpy(info_path, abs_path, len);
    strcpy(info_path + len , ".info");
    if(!file_is_exist(info_path)){
	return 0;
    }

    /* 
	info content:
	[global]
	path = /share/CACHEDEV1_DATA/homes/admin/.Qsync/aabbbb
    */     

    FILE *fp = fopen(info_path, "r");
    char buf[4096] = "", full_path[4096] = "";
    int found = 0;
    if(!fp)
	return -1;

    while(fgets(buf, sizeof(buf), fp)){
	if(strncmp(buf, "path = ", 7) == 0){
	    found = 1;
	    break;
	}
    }

    fclose(fp);

    if(!found)
	return 0;

    if(buf[strlen(buf)] == '\n')
	buf[strlen(buf)] = '\0';
    else if( buf[strlen(buf) - 1] == '\n')
	buf[strlen(buf) - 1] = '\0';


    /* combine fullpath */
    snprintf(full_path, sizeof(full_path), "%s%s", buf + 7, abs_path + len);

    free(abs_path);
    /* overwirte abs_path */
    *ptr_abs_path=strdup(full_path);

    return 0;
}
