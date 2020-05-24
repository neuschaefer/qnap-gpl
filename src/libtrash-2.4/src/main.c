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

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pwd.h>
#include <unistd.h>
#include <dlfcn.h>

/* Wrapper and helper functions are defined in this header file: */

#include "trash.h"
#ifdef QNAPNAS
#include <sys/stat.h>
#define __dbg(format, arg...)            \
        do {\
                FILE *fp; \
                fp = fopen("/debug","a+"); \
                fprintf(fp, format , ## arg); \
                fclose(fp); \
        } while (0);
#endif

/* These are the buffers which hold the compile-time configuration defaults: */

static char default_ignore_re[] = IGNORE_RE;

static char default_ignore_extensions[] = IGNORE_EXTENSIONS;

#ifdef RECYCLE_EX
static char default_share_folders[] = SHARE_FOLDERS;
static char default_admins_folders[] = ADMINS_FOLDERS;
#endif

static char default_unremovable_dirs[] = UNREMOVABLE_DIRS;

/* uncover_dirs doesn't show up here because there's no sense in having exceptions to unremovable_dirs by default... :-) */

static char default_temporary_dirs[] = TEMPORARY_DIRS;

static char default_user_temporary_dirs[] = USER_TEMPORARY_DIRS;

static char default_removable_media_mount_points[] = REMOVABLE_MEDIA_MOUNT_POINTS;

static char default_exceptions[] = EXCEPTIONS;

static char default_relative_trash_can[] = TRASH_CAN;

static char default_relative_trash_system_root[] = TRASH_SYSTEM_ROOT;

/* calling init() is the first thing every wrapper function does. It is used
 to gather necessary user information and preferences (if any),
 perform some tests and "turn off" libtrash if it finds any problems.
 This is done by setting the variable general_failure, the value of
 which the wrapped functions check before running.  This function
 used to be called _init() so that it was automatically invoked by
 the linker whenever the library was loaded, but that caused two
 different problems: (i) the initialization of other libraries (e.g.,
 libproc when running 'ps') can occur before libtrash is initialized,
 and the functions we override must already be available at that
 time; (ii) from the moment in which the library is initialized to
 the point when the wrapper functions actually run the information
 _init() collected had already become out of date (e.g., processes
 which change UIDs (<-Samba), etc). */

/* init() is passed a pointer to a cfg structure (defined in trash.h), which it
 * uses to communicate to the wrapper which called it the current configuration settings: */

void init(config *cfg)
{

   /* Variables: */

   struct passwd *userinfo = NULL;

   char *tmp = NULL;

   cfg->in_case_of_failure = IN_CASE_OF_FAILURE;

   /* Holds a regular expression which causes files matching this r.e. to be
    * ignored by libtrash:
    */

   cfg->ignore_re = default_ignore_re;

   /* Holds a list of file name extensions which cause files of these types to be ignored
    * by libtrash:
    */

   cfg->ignore_extensions = default_ignore_extensions;

#ifdef RECYCLE_EX 
   cfg->share_folders = default_share_folders;
   //__dbg("%s %d share folder:[%s]\n", __func__, __LINE__, cfg->share_folders);
   cfg->admins_folders = default_admins_folders;
#endif

   /* Controls whether hidden files (or files under hidden directories) should be ignored
    * (i.e., really destroyed) or handled normally:
    */

   cfg->ignore_hidden = IGNORE_HIDDEN;

   /* Controls whether files whose names end in a tilde ('~') should be ignored (i.e., really destroyed)
    or handled normally: */

   cfg->ignore_editor_backup = IGNORE_EDITOR_BACKUP;

   /* Controls whether files whose names begin with a cardinal ('#') should be ignored: */

   cfg->ignore_editor_temporary = IGNORE_EDITOR_TEMPORARY;

   /* Controls what we do when asked to destroy files under the user's TRASH_CAN: */

   cfg->protect_trash = PROTECT_TRASH;

   /* Controls whether attempts to destroy files outside of the user's home dir are intercepted
    * (see detailed explanation above): */

   cfg->global_protection = GLOBAL_PROTECTION;

   /* Controls whether WARNING_STRING is printed after the execution of each program when libtrash
    * is disabled:
    */

   cfg->should_warn = SHOULD_WARN;

   /* Controls whether the user's libtrash configuration file may be destroyed while libtrash is active: */

   cfg->libtrash_config_file_unremovable = LIBTRASH_CONFIG_FILE_UNREMOVABLE;

   /* 2- Directories: */

   /* This points to a list of directories (separated by semi-colons) which contain
    * files that we should refuse to destroy if running as root (or NULL, if
    * this feature should be disabled by default): */

   cfg->unremovable_dirs = default_unremovable_dirs;

   /* This points to a list of exceptions from the above list of unremovable dirs (set to NULL at this point,
    * because it doesn't have a default value -- it is meant to be used only at run-time): */

   cfg->uncovered_dirs = NULL;

   /* This points to a semi-colon delimited list of directories under which the "real" functions
    * are always used, because they hold temporary files: */

   cfg->temporary_dirs = default_temporary_dirs;

   /* This points to a semi-colon delimited list of directories *relative* to the user's home dir, under which
    * the "real" functions are always used, because they hold temporary files: */

   cfg->user_temporary_dirs = default_user_temporary_dirs;

   /* This points to a semi-colon delimited list of directories under which the "real" functions are
    * always used, because files under them are stored on removable media,
    * something which leads you to consider it not so worthwhile to keep a
    * copy of them around: (functionally equivalent to temporary_dirs) */

   cfg->removable_media_mount_points = default_removable_media_mount_points;

   /* This points to a list of paths (separated by semi-colons) which lists exceptions
    * to libtrash protection: files which otherwise be considered either worthy of being saved
    * in the trash can or unremovable lose their protection if listed here. This also applies to
    * any files with a pathname which *starts* with a path listed in this list: */

   cfg->exceptions = default_exceptions;

   /* 3- Paths: */

   /* Points to the absolute path of the directory in the user's home dir to which "deleted" files are moved: */

   cfg->absolute_trash_can = NULL;

   /* If global_protection is set to YES, files outside of the user's home dir which are destroyed have
    * a copy of themselves stored under absolute_trash_system_root:
    */

   cfg->absolute_trash_system_root = NULL;

   /* (The following paths need only be accessed by init() and get_config_from_file().) */

   /* Name of directory under the user's home directory in which we store deleted files (i.e., "trash can"): */

   cfg->relative_trash_can = default_relative_trash_can;

   /* Name of directory under the user's trash can in which we store files deleted outside of the user's home
    * directory if global_protection is set: */

   cfg->relative_trash_system_root = default_relative_trash_system_root;

   /* 4- User-related: ------------------------------------------------ */

   /* This one points to the user's home directory: */

   cfg->home = NULL;

   /* 5- Others: ------------------------------------------------------ */

   /* Switches which individually turn on/off each of the wrapped functions: */

   cfg->intercept_unlink = INTERCEPT_UNLINK;

   cfg->intercept_rename = INTERCEPT_RENAME;

   cfg->intercept_fopen  = INTERCEPT_FOPEN;

   cfg->intercept_freopen = INTERCEPT_FREOPEN;

   cfg->intercept_open   = INTERCEPT_OPEN;

   /* Variables which "remember" if a pointer to a given GNU libc function is available: */

   cfg->real_unlink_unavailable = YES;

   cfg->real_rename_unavailable = YES;

   cfg->real_fopen_unavailable = YES;

   cfg->real_freopen_unavailable = YES;

   cfg->real_open_unavailable = YES;

   /* Used by init() to signal to the called functions that libtrash is disabled either at the request of the
    user or because a serious error occurred: */

   cfg->libtrash_off = NO;

   cfg->general_failure = NO;

   /* These are pointers to the GNU libc functions we'll be overriding: */

   cfg->real_unlink = NULL;
   cfg->real_rename = NULL;
   cfg->real_fopen = NULL;
   cfg->real_freopen = NULL;
   cfg->real_open = NULL;

   /*  ----------------------------------------------- */

   /* Get a pointer to GNU libc's original functions: */

   dlerror();
				
   cfg->real_unlink = dlsym(RTLD_NEXT, "unlink");

   if (!dlerror())
     cfg->real_unlink_unavailable = NO;

   dlerror();
   
   cfg->real_rename = dlsym(RTLD_NEXT, "rename");

   if (!dlerror())
     cfg->real_rename_unavailable = NO;
   
   dlerror();
   
   cfg->real_fopen = dlsym(RTLD_NEXT, "fopen"); /* funny interaction fixed! when dlsym() was used instead of dlvsym(), GNU libc would give us a pointer to an older version of fopen() and subsequently crash if the calling code tried to use, e.g., getwc(). */
   
   if (!dlerror())
     cfg->real_fopen_unavailable = NO;

   dlerror();
   
   cfg->real_freopen = dlsym(RTLD_NEXT, "freopen");

   if (!dlerror())
     cfg->real_freopen_unavailable = NO;

   dlerror();
   
   cfg->real_open = dlsym(RTLD_NEXT, "open");

   if (!dlerror())
     cfg->real_open_unavailable = NO;

   /* If any of these calls to dlsym() failed, we'll never be able to invoke the GNU libc function it
    * should return a pointer to.
    *
    * If that happens with rename(), that means that graft_file() - the
    * function we use inside all wrappers to move files to the trash can -
    * won't work, because it relies on the availability of GNU libc's
    * rename(). On the other hand, the inability to access one of the other
    * GNU libc functions mentioned above only affects that function's
    * wrapper. For this reason, if the call to dlsym(RTLD_NEXT, "rename")
    * fails we set general_failure (thereby completely disactivating libtrash),
    * while in the other cases we just selectively disable the wrapper for
    * that function. */

   if (cfg->real_rename_unavailable)
     {
#ifdef DEBUG
	fprintf(stderr, "MAJOR PROBLEM: real_rename_unavailable == YES!\ngeneral_failure set.\n");
#endif
	cfg->general_failure = YES;

	return;
     }

   /* Has the user asked us to become temporarily inactive by setting the environmental variable TRASH_OFF to
    YES? */

   tmp = getenv("TRASH_OFF");

   if (tmp && !strcmp(tmp, "YES"))
     {
#ifdef DEBUG
	fprintf(stderr, "TRASH_OFF is set to YES in the environment, setting libtrash_off.\n");
#endif
	cfg->libtrash_off = YES;

	return;
     }

#ifdef QNAPNAS
    struct stat status;

    if (0 != stat("/var/qfunc/recyclebin.enable", &status)){
	cfg->libtrash_off = YES;
    }
#endif

   /* Has the user set UNCOVER_DIRS in her environment? */

   tmp = getenv("UNCOVER_DIRS");

   if (tmp)
     {
#ifdef DEBUG
	fprintf(stderr, "UNCOVER_DIRS is set, directories %s won't be considered \"unremovable\".\n", tmp);
#endif
	cfg->uncovered_dirs = malloc(strlen(tmp) + 1);

	if (cfg->uncovered_dirs) /* if malloc() failed, there's nothing we can do so
				  * we just leave the pointer set to NULL -- that will
				  * be interpreted as meaning that there are no uncovered dirs */
	  {
	     strcpy(cfg->uncovered_dirs, tmp);
	  }
     }

   /* ------------------------------------------- */

   /* Override compile-time defaults with values from the user-specific configuration file: */

#ifdef RECYCLE_EX
    //__dbg("%s %d share folder:[%s] pid[%d]\n",__func__,__LINE__,cfg->share_folders, getpid());
#endif
   get_config_from_file(cfg);

   /* Translate the relative paths in cfg->user_temporary_dirs into absolute paths: */

   if (strlen(cfg->user_temporary_dirs) > 0)
     {
	tmp = convert_relative_into_absolute_paths(cfg->user_temporary_dirs);

	if (tmp)
	  cfg->user_temporary_dirs = tmp;
	else  //  if convert_relative_... fails we signal failure
	  {
#ifdef DEBUG
	     fprintf(stderr, "convert_relative_into_absolute_paths failed.\n");
#endif

	/* Free memory allocated inside get_config_from_files() which should be free()d inside this function: */
#ifndef RECYCLE_EX 
	     if (cfg->relative_trash_can != default_relative_trash_can)
	       free(cfg->relative_trash_can);
#endif

	     if (cfg->relative_trash_system_root != default_relative_trash_system_root)
	       free(cfg->relative_trash_system_root);

	     cfg->general_failure = YES;

	     return;
	  }
     }

   /* (All the configuration variables are by now set. The allocated memory which needs to be free()d will
    be deallocated in fini(), except for relative_trash_(can|system_root), which will be free()d before
    this function (init()) returns.) */

#ifdef DEBUG
   fprintf(stderr,
	   "TRASH_CAN:                         %s\n"
	   "IN_CASE_OF_FAILURE:                %d\n"
	   "SHOULD_WARN:                       %d\n"
	   "IGNORE_HIDDEN:                     %d\n"
	   "IGNORE_EDITOR_BACKUP:              %d\n"
	   "IGNORE_EDITOR_TEMPORARY:           %d\n"
	   "PROTECT_TRASH:                     %d\n"
	   "GLOBAL_PROTECTION:                 %d\n"
	   "TRASH_SYSTEM_ROOT:                 %s\n"
	   "TEMPORARY_DIRS:                    %s\n"
	   "USER_TEMPORARY_DIRS:               %s\n"
	   "UNREMOVABLE_DIRS:                  %s\n"
	   "IGNORE_EXTENSIONS:                 %s\n"
	   "INTERCEPT_UNLINK:                  %d\n"
	   "INTERCEPT_RENAME:                  %d\n"
	   "INTERCEPT_FOPEN:                   %d\n"
	   "INTERCEPT_FREOPEN:                 %d\n"
	   "INTERCEPT_OPEN:                    %d\n"
	   "LIBTRASH_CONFIG_FILE_UNREMOVABLE:  %d\n"
	   "REMOVABLE_MEDIA_MOUNT_POINTS:      %s\n"
	   "EXCEPTIONS:                        %s\n"
	   "IGNORE_RE:                         %s\n\n",
	   cfg->relative_trash_can, cfg->in_case_of_failure, cfg->should_warn, cfg->ignore_hidden,
	   cfg->ignore_editor_backup, cfg->ignore_editor_temporary, cfg->protect_trash, cfg->global_protection,
	   cfg->relative_trash_system_root, cfg->temporary_dirs, cfg->user_temporary_dirs, cfg->unremovable_dirs,
	   cfg->ignore_extensions,
	   cfg->intercept_unlink, cfg->intercept_rename, cfg->intercept_fopen, cfg->intercept_freopen, cfg->intercept_open,
	   cfg->libtrash_config_file_unremovable,
	   cfg->removable_media_mount_points,
	   cfg->exceptions,
	   cfg->ignore_re);
#endif

   /* ------------------------------------------------------ */

   /* Things left to do:
    * Get information about the user, form two strings containing the absolute
    * pathname of the trash can and the relative_trash_system_root under it (if global_protection
    * is set), see if those dirs exist (try to create it them they don't) and store the user's
    * login and home directory in memory.
    */

   /* I got this from (2) getlogin()'s man page: */

   userinfo = getpwuid(geteuid());

   if (!userinfo)
     {
#ifdef DEBUG
	fprintf(stderr, "Unable to determine necessary data about the user.\ngeneral_failure set.\n");
#endif
	/* Free memory allocated inside get_config_from_files() which should be free()d inside this function: */

	if (cfg->relative_trash_can != default_relative_trash_can)
	  free(cfg->relative_trash_can);

	if (cfg->relative_trash_system_root != default_relative_trash_system_root)
	  free(cfg->relative_trash_system_root);

	cfg->general_failure = YES;
	return;
     }

   /* Information which the functions we will be overriding need: home, absolute_trash_can and
    * possibly absolute_trash_system_root. */

   /* This memory will be free()d in fini() : */

   /*Add by JimmyChang 2006/9/8 for change the default trash can path*/

   userinfo->pw_dir = '\0';
   
   userinfo->pw_dir = "/share";  
   
   /*End*/

   cfg->home = malloc(strlen(userinfo->pw_dir) + 1);

   cfg->absolute_trash_can = malloc(strlen(userinfo->pw_dir) + 1 + strlen(cfg->relative_trash_can) + 1);

   if (cfg->global_protection)
     cfg->absolute_trash_system_root = malloc(strlen(userinfo->pw_dir) + 1 + strlen(cfg->relative_trash_can)
					      + 1 + strlen(cfg->relative_trash_system_root) + 1);

   if (!cfg->home || !cfg->absolute_trash_can ||
       (cfg->global_protection && !cfg->absolute_trash_system_root))
     {
#ifdef DEBUG
	fprintf(stderr, "Unable to allocate sufficient memory.\ngeneral_failure set.\n");
#endif
	/* Free any successfully allocated memory and point the respective pointer to NULL, so that fini() doesn't
	 try to free() that memory again: */

	if (cfg->home)
	  {
	     free(cfg->home);
	     cfg->home = NULL;
	  }

	if (cfg->absolute_trash_can)
	  {
	     free(cfg->absolute_trash_can);
	     cfg->absolute_trash_can = NULL;
	  }

	if (cfg->absolute_trash_system_root)
	  {
	     free(cfg->absolute_trash_system_root);
	     cfg->absolute_trash_system_root = NULL;
	  }

	/* Free memory previously allocated inside get_config_from_files() (no need to point it to NULL since fini() never
	 tries to free() the memory these pointers point at): */

	if (cfg->relative_trash_can != default_relative_trash_can)
	  free(cfg->relative_trash_can);

	if (cfg->relative_trash_system_root != default_relative_trash_system_root)
	  free(cfg->relative_trash_system_root);

	cfg->general_failure = YES;

	return;
     }

   strcpy(cfg->home, userinfo->pw_dir);

   strcpy(cfg->absolute_trash_can, cfg->home);
   strcat(cfg->absolute_trash_can, "/");
   strcat(cfg->absolute_trash_can, cfg->relative_trash_can);
#ifdef RECYCLE_EX
    //__dbg("%s %d [%s] [%s] [%s] pid[%d]\n", __func__,__LINE__,cfg->absolute_trash_can, cfg->home, default_relative_trash_can, getpid());
#endif
   if (cfg->global_protection)
     {
	strcpy(cfg->absolute_trash_system_root, cfg->absolute_trash_can);
	strcat(cfg->absolute_trash_system_root, "/");
	strcat(cfg->absolute_trash_system_root, cfg->relative_trash_system_root);
     }

   /* This is a good time to free() the memory pointed to by relative_trash_can and relative_trash_system_root,
    * if they are pointing to dynamically allocated memory: we have already used the information they contain. */
#ifndef RECYCLE_EX 
   if (cfg->relative_trash_can != default_relative_trash_can)
     free(cfg->relative_trash_can);
#endif

   if (cfg->relative_trash_system_root != default_relative_trash_system_root)
     free(cfg->relative_trash_system_root);

   /* Now we will check the existence and permissions of absolute_trash_can and, if
    * global_protection is set, absolute_trash_system_root, and create them if they don't
    * already exist:
    */
#ifndef RECYCLE_EX 
   if (!dir_ok(cfg->absolute_trash_can, NULL))
     {
#ifdef DEBUG
	fprintf(stderr, "The TRASH_CAN dir (%s) either doesn't exist (and its creation failed) or has insufficient "
		"permissions which we were unable to change.\ngeneral_failure set.\n", cfg->absolute_trash_can);
#endif
	cfg->general_failure = YES;
	return;
     }
#endif
   if (cfg->global_protection && !dir_ok(cfg->absolute_trash_system_root, NULL))
     {
#ifdef DEBUG
	fprintf(stderr, "The TRASH_CAN/SYSTEM_ROOT dir (%s) either doesn't exist (and its creation failed) or has insufficient "
		"permissions which we were unable to change.\ngeneral_failure set.\n", cfg->absolute_trash_system_root);
#endif
	cfg->general_failure = YES;
	return;
     }

   /* We know everything we need to know. init() is done. */

   return;
}

/* -------------------------------  */

/* Just like init(), this function is always invoked by the wrapper functions before quitting. Its
 only task is freeing malloc()ed memory: */

void fini(config *cfg)
{

   /* If libtrash is disabled and the user wishes to be informed, tell him about it: */

   if (cfg->libtrash_off && cfg->should_warn)
     fprintf(stderr, "%s\n", WARNING_STRING);

   /* The only thing we need to do is free() the dynamically allocated buffers: */

   if (cfg->absolute_trash_can != NULL)
     free(cfg->absolute_trash_can);

   if (cfg->absolute_trash_system_root != NULL)
     free(cfg->absolute_trash_system_root);
#ifdef RECYCLE_EX 
   if (cfg->relative_trash_can != default_relative_trash_can)
     free(cfg->relative_trash_can);
#endif

   if (cfg->home != NULL)
     free(cfg->home);

   if (cfg->temporary_dirs != default_temporary_dirs)
     free(cfg->temporary_dirs);

   if (cfg->user_temporary_dirs != default_user_temporary_dirs)
     free(cfg->user_temporary_dirs);

   if (cfg->unremovable_dirs != default_unremovable_dirs)
     free(cfg->unremovable_dirs);

   if (cfg->uncovered_dirs != NULL)
     free(cfg->uncovered_dirs);

   if (cfg->removable_media_mount_points != default_removable_media_mount_points)
     free(cfg->removable_media_mount_points);

   if (cfg->exceptions != default_exceptions)
     free(cfg->exceptions);

   if (cfg->ignore_extensions != default_ignore_extensions)
     free(cfg->ignore_extensions);

#ifdef RECYCLE_EX
   if (cfg->share_folders != default_share_folders)
     free(cfg->share_folders);
   if (cfg->admins_folders != default_admins_folders)
     free(cfg->admins_folders);
#endif

   if (cfg->ignore_re != default_ignore_re)
     free(cfg->ignore_re);

   /* DON'T free() relative_trash_(can|system_root)! The memory they point to has already been free()d by
    * init() before returning. */

   return;
}

/* -------------------------------- */

