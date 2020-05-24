/*
  Copyright (C) 2006, 2008, 2009, 2010  Novell, Inc.
  Written by Andreas Gruenbacher <agruen@suse.de>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2, or (at your option) any
  later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this library; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

/*
 * FIXME:
 * Make ls show a `+' for richacls (in coreutils).
 * Add a way to show only expicitly set acls, and hide inherited ones.
 * Convert a non-Automatic-Inheritance tree into an Automatic Inheritance one?
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/xattr.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

#include "richacl.h"
#include "string_buffer.h"


#define E_DELETE_NOACL		1
#define E_DELETE_NOINDEX	2
#define E_DELETE_INHERITED_ACE	3




static struct richacl *get_richacl(const char *file, mode_t mode);
static int set_richacl(const char *path, struct richacl *acl);
int modify_richacl(struct richacl **acl2, struct richacl *acl, int acl_has, int isor);

static const char *progname;
int opt_repropagate;

void printf_stderr(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

#define richacl_for_each_entry_continue(_ace, _acl) \
	for ((_ace)++; \
	     (_ace) != (_acl)->a_entries + (_acl)->a_count; \
	     (_ace)++)

static inline int richace_is_inherited(const struct richace *ace)
{
	return ace->e_flags & ACE4_INHERITED_ACE;
}

static void compute_masks(struct richacl *acl, int acl_has)
{
	unsigned int owner_mask = acl->a_owner_mask;
	unsigned int group_mask = acl->a_group_mask;
	unsigned int other_mask = acl->a_other_mask;

	if ((acl_has & RICHACL_TEXT_OWNER_MASK) &&
	    (acl_has & RICHACL_TEXT_GROUP_MASK) &&
	    (acl_has & RICHACL_TEXT_OTHER_MASK) &&
	    (acl_has & RICHACL_TEXT_FLAGS))
		return;

	if (!(acl_has & RICHACL_TEXT_FLAGS))
		acl->a_flags &= ~ACL4_MASKED;
	richacl_compute_max_masks(acl);
	if (acl_has & RICHACL_TEXT_OWNER_MASK) {
		if (!(acl_has & RICHACL_TEXT_FLAGS) &&
		    (acl->a_owner_mask & ~owner_mask))
			acl->a_flags |= ACL4_MASKED;
		acl->a_owner_mask = owner_mask;
	}
	if (acl_has & RICHACL_TEXT_GROUP_MASK) {
		if (!(acl_has & RICHACL_TEXT_FLAGS) &&
		    (acl->a_group_mask & ~group_mask))
			acl->a_flags |= ACL4_MASKED;
		acl->a_group_mask = group_mask;
	}
	if (acl_has & RICHACL_TEXT_OTHER_MASK) {
		if (!(acl_has & RICHACL_TEXT_FLAGS) &&
		    (acl->a_other_mask & ~other_mask))
			acl->a_flags |= ACL4_MASKED;
		acl->a_other_mask = other_mask;
	}
}


/* 
 * Save the inheritable aces to the files recursively
 */
int apply_inheritable_aces(struct richacl *file_acl,
	struct richacl *dir_acl, const char *dirname)
{
	DIR *dir = NULL;
	struct dirent *entry = NULL;
	char path[MAX_PATHNAME_LENGTH] = {'\0'};
	int len = 0;
	int error = 0;

	if (!file_acl || !dir_acl || !dirname)
		return -1;

	if (!(dir = opendir(dirname))) {
		error = -1;
		goto fail;
	}
	if (!(entry = readdir(dir))) {
		error = -1;
		goto fail;
	}

	/* Save the inherited aces to the files recursively */
	do {
		if (entry->d_type == DT_DIR) {
			len = snprintf(path, sizeof(path), "%s/%s",
					dirname, entry->d_name);
			path[len] = '\0';

			if (strncmp(entry->d_name, ".", 1) == 0 ||
				strncmp(entry->d_name, "..", 2) == 0)
                                continue;

			if (removexattr(path, "system.richacl")) {
				if (errno == ENOTSUP)
					continue;
			}

			if (set_richacl(path, dir_acl))
				fprintf(stderr, "%s: fail to save richacl fail\n", path);

			apply_inheritable_aces(file_acl, dir_acl, path);

		} else {
			len = snprintf(path, sizeof(path), "%s/%s",
					dirname, entry->d_name);
			path[len] = '\0';

			if (removexattr(path, "system.richacl")) {
				if (errno == ENOTSUP)
					continue;
			}
			
			if (set_richacl(path, file_acl))
				fprintf(stderr, "%s: fail to save richacl\n", path);
                }
        } while ((entry = readdir(dir)));

	
fail:
	closedir(dir);
	return error;
}


int apply_modified_inheritable_aces(struct richacl *file_acl,
	struct richacl *dir_acl, const char *dirname, 
	int acl_has, int non_propagation)
{
	DIR *dir = NULL;
	struct dirent *entry = NULL;
	char path[MAX_PATHNAME_LENGTH] = {'\0'};
	struct richacl *acl = NULL;
	int len = 0;
	struct stat st;
	int error = 0;

	if (!file_acl || !dir_acl || !dirname)
		return -1;

        if (!(dir = opendir(dirname))) {
		error = -1;
                goto fail;
	}
        if (!(entry = readdir(dir))) {
		error = -1;
                goto fail;
	}

	do {
		if (entry->d_type == DT_DIR) {
			len = snprintf(path, sizeof(path), "%s/%s",
						dirname, entry->d_name);
			path[len] = '\0';

			if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0)
				continue;

			/* Check status of file */
			if (stat(path, &st)) {
				fprintf(stderr, "%s: fail\n", path);
				continue;
			}

			/* Get richacl of file */
			acl = get_richacl(path, st.st_mode);
			if (!acl) {
				fprintf(stderr, "%s: fail\n", path);
				continue;
			}

			/* Modify the richacl of file */
			if (modify_richacl(&acl, dir_acl, acl_has, 0)) {
				if (acl) {
					richacl_free(acl);
					acl = NULL;
				}
				fprintf(stderr, "%s: fail\n", path);
				continue;
			}

			if (set_richacl(path, acl)) {
				richacl_free(acl);
				acl = NULL;
				fprintf(stderr, "%s: fail\n", path);
				continue;
			}

			richacl_free(acl);
			acl = NULL;

			/* If the inheritable aces is non-propagation, then stop.
 			 * Else, modify the acl recursively
 			 */
			if (!non_propagation)
				apply_modified_inheritable_aces(file_acl, dir_acl,
								path, acl_has, 0);

                } else {
                        len = snprintf(path, sizeof(path), "%s/%s",
					dirname, entry->d_name);
			path[len] = '\0';

			if (stat(path, &st)) {
				fprintf(stderr, "%s: fail\n", path);
				continue;
			}

			acl = get_richacl(path, st.st_mode);

			if (!acl) {
				fprintf(stderr, "%s: fail\n", path);
				continue;
			}

			if (modify_richacl(&acl, file_acl, acl_has, 0)) {
				fprintf(stderr, "%s: fail\n", path);
				if (acl) {
					richacl_free(acl);
					acl = NULL;
				}
				continue;
			}

			if (set_richacl(path, acl)) {
				fprintf(stderr, "%s: fail\n", path);
				richacl_free(acl);
				acl = NULL;
				continue;
			}

			richacl_free(acl);
			acl = NULL;
                }
        } while ((entry = readdir(dir)));

fail:
	closedir(dir);
        return error;
}


int qnap_apply_inheritage(struct richacl *acl,
			const char *dirname,
			int is_modified,
			int acl_has)
{
	struct richacl *file_acl = NULL, *dir_acl = NULL;       /* propagation inheritable acls     */
	struct richacl *np_file_acl = NULL, *np_dir_acl = NULL; /* non-propagation inheritable acls */
	struct richace *ace = NULL, *current_ace = NULL;
	int error = 0;

	file_acl = richacl_alloc(0);
	dir_acl = richacl_alloc(0);
	np_file_acl = richacl_alloc(0);
	np_dir_acl = richacl_alloc(0);

	if (!file_acl || !dir_acl || !np_file_acl || !np_dir_acl) {
		error = -1;
		goto clean;
	}

	/* propagation */
	struct richacl_alloc file_x = {
		.acl = file_acl,
		.count = file_acl->a_count,
	};	

	struct richacl_alloc dir_x = {
		.acl = dir_acl,
		.count = dir_acl->a_count,
	};

	/* non-propagation */
        struct richacl_alloc np_file_x = {
                .acl = np_file_acl,
                .count = np_file_acl->a_count,
        };

        struct richacl_alloc np_dir_x = {
                .acl = np_dir_acl,
                .count = np_dir_acl->a_count,
        };
	
	/* For propagation inheritable ace */
	richacl_for_each_entry (ace, acl) {

		if (!(ace->e_flags & ACE4_NO_PROPAGATE_INHERIT_ACE)) {
			if ((ace->e_flags & ACE4_FILE_INHERIT_ACE) 
					&& (ace->e_flags & ACE4_DIRECTORY_INHERIT_ACE)) {
				current_ace = richacl_append_entry(&file_x);
				richace_copy(current_ace, ace);
				current_ace->e_flags &= ~ACE4_FILE_INHERIT_ACE;
				current_ace->e_flags &= ~ACE4_DIRECTORY_INHERIT_ACE;
				current_ace->e_flags &= ~ACE4_INHERIT_ONLY_ACE;
				current_ace->e_flags |= ACE4_INHERITED_ACE;

				current_ace = richacl_append_entry(&dir_x);
				richace_copy(current_ace, ace);
				current_ace->e_flags &= ~ACE4_INHERIT_ONLY_ACE;
				current_ace->e_flags |= ACE4_INHERITED_ACE;
			}

			if (!(ace->e_flags & ACE4_DIRECTORY_INHERIT_ACE) 
					&& (ace->e_flags & ACE4_FILE_INHERIT_ACE)) {
				current_ace = richacl_append_entry(&file_x);
				richace_copy(current_ace, ace);
				current_ace->e_flags &= ~ACE4_FILE_INHERIT_ACE;
				current_ace->e_flags &= ~ACE4_DIRECTORY_INHERIT_ACE;
				current_ace->e_flags &= ~ACE4_INHERIT_ONLY_ACE;
				current_ace->e_flags |= ACE4_INHERITED_ACE;

				current_ace = richacl_append_entry(&dir_x);
				richace_copy(current_ace, ace);
				current_ace->e_flags |= ACE4_INHERIT_ONLY_ACE;
				current_ace->e_flags |= ACE4_INHERITED_ACE;
			}

			if ((ace->e_flags & ACE4_DIRECTORY_INHERIT_ACE) 
					&& !(ace->e_flags & ACE4_FILE_INHERIT_ACE)) {
				current_ace = richacl_append_entry(&dir_x);
				richace_copy(current_ace, ace);
				current_ace->e_flags |= ACE4_INHERITED_ACE;
			}
		}
	}

	file_acl = file_x.acl;
	dir_acl = dir_x.acl;

	if (!is_modified) {
		apply_inheritable_aces(file_acl, dir_acl, dirname);
	} else {
		apply_modified_inheritable_aces(file_acl, dir_acl, dirname, acl_has, 0);
	}


	/* For non-propogation ace */
        richacl_for_each_entry (ace, acl) {

		if (ace->e_flags & ACE4_NO_PROPAGATE_INHERIT_ACE) {
			if (ace->e_flags & ACE4_FILE_INHERIT_ACE) {
				current_ace = richacl_append_entry(&np_file_x);
				richace_copy(current_ace, ace);
				current_ace->e_flags &= ~ACE4_FILE_INHERIT_ACE;
				current_ace->e_flags &= ~ACE4_DIRECTORY_INHERIT_ACE;
				current_ace->e_flags &= ~ACE4_INHERIT_ONLY_ACE;
				current_ace->e_flags &= ~ACE4_NO_PROPAGATE_INHERIT_ACE;
				current_ace->e_flags |= ACE4_INHERITED_ACE;
			}
                        if (ace->e_flags & ACE4_DIRECTORY_INHERIT_ACE) {
				current_ace = richacl_append_entry(&np_dir_x);
				richace_copy(current_ace, ace);
				current_ace->e_flags &= ~ACE4_FILE_INHERIT_ACE;
				current_ace->e_flags &= ~ACE4_DIRECTORY_INHERIT_ACE;
				current_ace->e_flags &= ~ACE4_INHERIT_ONLY_ACE;
				current_ace->e_flags &= ~ACE4_NO_PROPAGATE_INHERIT_ACE;
				current_ace->e_flags |= ACE4_INHERITED_ACE;
			}
		}
	}

	np_file_acl = np_file_x.acl;
	np_dir_acl = np_dir_x.acl;

	/* After the process of the file_acl and dir_acl,
 	 * each file has richacl. So in this step, only 
 	 * use apply_modified_inheritable_aces
 	 */
	apply_modified_inheritable_aces(np_file_acl, np_dir_acl, dirname, 0, 1);

clean:
	if (file_acl) {
		richacl_free(file_acl);
		file_acl = NULL;
	}
	if (dir_acl) {
		richacl_free(dir_acl);
		dir_acl = NULL;
	}
	if (np_file_acl) {
		richacl_free(np_file_acl);
		np_file_acl = NULL;
	}
	if (np_dir_acl) {
		richacl_free(np_dir_acl);
		np_dir_acl = NULL;
	}

	return error;
}

/* 
 * Convert the inherited aces to exlicit aces
 */
int qnap_convert_richacl(struct richacl **acl)
{
	struct richace *ace = NULL;
	struct richacl *inherited_acl = NULL;
	struct richacl_alloc x;
	int count = 0, idx = 0, i = 0;
	int *inherited_idx_table = NULL; /* Record the index of the inherited aces */
	int error = 0;

	if (!(*acl))
		return -1;

	/* How many inherited aces ? */
	richacl_for_each_entry(ace, *acl) {
		if (richace_is_inherited(ace))
			count ++;
	}
	if (count == 0)
		return 0;

	/* Create a new acl which stores the inherited aces */
	inherited_acl = richacl_alloc(count);
	if (!inherited_acl)
		return -1;

	inherited_idx_table = (int *)malloc(sizeof(int) * count);	
	if (!inherited_idx_table) {
		error = -1;
		goto clean;
	}

	i = 0;
	richacl_for_each_entry(ace, *acl) {
		if (richace_is_inherited(ace)) {
			richace_copy(inherited_acl->a_entries + idx, ace);
			(inherited_acl->a_entries + idx)->e_flags &= ~ACE4_INHERITED_ACE;
			*(inherited_idx_table + idx) = i;
			idx++;
		}
		i++;
	}
	/* Delete the inherited aces from the original acl */
	for (i = 0; i < inherited_acl->a_count; i++) {
		x.acl = *acl;
		x.count = (*acl)->a_count;
		ace = x.acl->a_entries + (*(inherited_idx_table + i) - i);
		richacl_delete_entry(&x, &ace);
	}
	*acl = x.acl;
	/* Merge the original and the new acl */
	if (modify_richacl(acl, inherited_acl, 0, 1))
		error = -1;
clean:
	if (inherited_idx_table) {
		free(inherited_idx_table);
		inherited_idx_table = NULL;
	}	

	if (inherited_acl) {
		richacl_free(inherited_acl);
		inherited_acl = NULL;
	}
	return error;
}


int qnap_delete_richace(struct richacl *acl, int index)
{
	struct richace *ace = NULL;
	int error = 0;

	if (!acl) {
		error = E_DELETE_NOACL;
		goto fail;
	}
	
	if (index <= 0 || index > acl->a_count) {
		error = E_DELETE_NOINDEX;
		goto fail;
	}

	ace = acl->a_entries + index - 1 ;

	if (ace->e_flags & ACE4_INHERITED_ACE) {
		error = E_DELETE_INHERITED_ACE;
		goto fail;
	}

	struct richacl_alloc x = {
		.acl = acl,
		.count = acl->a_count,
	};

	richacl_delete_entry(&x, &ace);

	return 0;

fail:
	return error;
}


int qnap_copy_richacl(const char *src_path, const char* dst_path)
{
	struct stat src_st;
	struct stat dst_st;
	struct richacl *acl = NULL;
	struct richace *ace = NULL;

	if (!src_path || !dst_path)
		goto fail;

	if (stat(dst_path, &dst_st))
		goto fail;

	if (stat(src_path, &src_st))
		goto fail;

	if ((!S_ISDIR(src_st.st_mode) &&  S_ISDIR(dst_st.st_mode)) || 
		(S_ISDIR(src_st.st_mode) &&  !S_ISDIR(dst_st.st_mode)))
		goto fail;

	acl = get_richacl(src_path, src_st.st_mode);

	if (!acl)
		goto fail;	

	richacl_for_each_entry(ace, acl) {
		ace->e_flags &= ~ACE4_INHERITED_ACE;
	}	

	if (set_richacl(dst_path, acl)) {
		if (acl) {
			richacl_free(acl);
			acl = NULL;
		}
		goto fail;
	}

	if (acl) {
		richacl_free(acl);
		acl = NULL;
	}

	return 0;

fail:
	return -1;	
}


int modify_richacl(struct richacl **acl2, struct richacl *acl, int acl_has, int isor)
{
	struct richace *ace2, *ace;

	if (richacl_apply_masks(acl2))
		return -1;

	richacl_for_each_entry(ace, acl) {
		struct richacl *acl3;
		struct richace *ace3;

		richacl_for_each_entry(ace2, *acl2) {
			if (ace2->e_type == ace->e_type &&
			    richace_is_inherited(ace2) == richace_is_inherited(ace) &&
			    richace_is_same_identifier(ace, ace2)) {
				if (!isor) {
					ace2->e_mask = ace->e_mask;
					ace2->e_flags = ace->e_flags;
				} else {
					ace2->e_mask |= ace->e_mask;
					ace2->e_flags |= ace->e_flags;
				}
				goto next_change;
			}
		}

		acl3 = richacl_alloc((*acl2)->a_count + 1);
		if (!acl3)
			return -1;
		acl3->a_flags = (*acl2)->a_flags;
		ace3 = acl3->a_entries;
		if (!(ace->e_flags & ACE4_INHERITED_ACE)) {
			if (richace_is_deny(ace)) {
				/*
				 * Insert the new deny entry after the existing
				 * initial non-inherited deny entries.
				 */
				richacl_for_each_entry(ace2, *acl2) {
					if (!richace_is_deny(ace2) ||
					    richace_is_inherited(ace2))
						break;
					richace_copy(ace3++, ace2);
				}
			} else {
				/*
				 * Append the new allow entry at the end of the
				 * non-inherited aces.
				 */
				richacl_for_each_entry(ace2, *acl2) {
					if (richace_is_inherited(ace2))
						break;
					richace_copy(ace3++, ace2);
				}
			}
			richace_copy(ace3++, ace);
			ace2--;
			richacl_for_each_entry_continue(ace2, *acl2)
				richace_copy(ace3++, ace2);
		} else {
			struct richace *last_inherited;

			last_inherited = (*acl2)->a_entries + (*acl2)->a_count;
			while (last_inherited > (*acl2)->a_entries &&
			       richace_is_inherited(last_inherited - 1))
				last_inherited--;

			richacl_for_each_entry(ace2, *acl2) {
				if (ace2 == last_inherited)
					break;
				richace_copy(ace3++, ace2);
			}
			if (richace_is_deny(ace)) {
				/*
				 * Insert the new deny entry after the existing
				 * initial inherited deny entries.
				 */
				ace2--;
				richacl_for_each_entry_continue(ace2, *acl2) {
					if (!richace_is_deny(ace2))
						break;
					richace_copy(ace3++, ace2);
				}
			} else {
				/*
				 * Append the new allow entry at the end of the
				 * inherited aces.
				 */
				ace2--;
				richacl_for_each_entry_continue(ace2, *acl2)
					richace_copy(ace3++, ace2);
			}
			richace_copy(ace3++, ace);
			ace2--;
			richacl_for_each_entry_continue(ace2, *acl2)
				richace_copy(ace3++, ace2);
		}

		richacl_free(*acl2);
		*acl2 = acl3;

	next_change:
		/* gcc is unhappy without a statement behind the label ... */ ;
	}

	if (acl_has & RICHACL_TEXT_FLAGS)
		(*acl2)->a_flags = acl->a_flags;
	if (acl_has & RICHACL_TEXT_OWNER_MASK)
		(*acl2)->a_owner_mask = acl->a_owner_mask;
	if (acl_has & RICHACL_TEXT_GROUP_MASK)
		(*acl2)->a_group_mask = acl->a_group_mask;
	if (acl_has & RICHACL_TEXT_OTHER_MASK)
		(*acl2)->a_other_mask = acl->a_other_mask;
	compute_masks(*acl2, acl_has);

	return 0;
}

static int auto_inherit(const char *dirname, struct richacl *dir_acl)
{
	DIR *dir;
	struct richacl *dir_inheritable, *file_inheritable;
	struct dirent *dirent;
	char *path = NULL;
	size_t dirname_len;
	int status = 0;

	dir = opendir(dirname);
	if (!dir) {
		if (errno == ENOTDIR)
			return 0;
		return -1;
	}

	dirname_len = strlen(dirname);
	path = malloc(dirname_len + 2);
	if (!path)
		goto fail;
	sprintf(path, "%s/", dirname);

	errno = 0;
	file_inheritable = richacl_inherit(dir_acl, 0);
	if (!file_inheritable && errno != 0)
		goto fail;
	dir_inheritable = richacl_inherit(dir_acl, 1);
	if (!dir_inheritable && errno != 0)
		goto fail;

	while ((errno = 0, dirent = readdir(dir))) {
		struct richacl *old_acl = NULL, *new_acl = NULL;
		int isdir;
		char *p;

		if (!strcmp(dirent->d_name, ".") ||
		    !strcmp(dirent->d_name, ".."))
			continue;

		p = realloc(path, strlen(dirname) + strlen(dirent->d_name) + 2);
		if (!p)
			goto fail;
		path = p;
		strcpy(path + dirname_len + 1, dirent->d_name);

		if (dirent->d_type == DT_UNKNOWN) {
			struct stat st;

			if (lstat(path, &st))
				goto fail2;
			dirent->d_type = IFTODT(st.st_mode);
		}
		if (dirent->d_type == DT_LNK)
			continue;
		isdir = (dirent->d_type == DT_DIR);

		old_acl = richacl_get_file(path);
		if (!old_acl) {
			if (errno == ENODATA || errno == ENOTSUP || errno == ENOSYS)
				goto next;
			goto fail2;
		}
		if (!richacl_is_auto_inherit(old_acl))
			goto next;
		if (old_acl->a_flags & ACL4_PROTECTED) {
			if (!opt_repropagate)
				goto next;
			new_acl = old_acl;
			old_acl = NULL;
		} else {
			int equal;
			new_acl = richacl_auto_inherit(old_acl,
					isdir ? dir_inheritable :
						file_inheritable);
			equal = !richacl_compare(old_acl, new_acl);
			if (equal && !opt_repropagate)
				goto next;
			if (!equal && richacl_set_file(path, new_acl))
				goto fail2;
		}

		if (isdir)
			if (auto_inherit(path, new_acl))
				goto fail2;

	next:
		free(old_acl);
		free(new_acl);
		continue;

	fail2:
		perror(path);
		free(old_acl);
		free(new_acl);
		free(path);
		status = -1;
	}
	if (errno != 0) {
		perror(dirname);
		status = -1;
	}
	free(path);
	closedir(dir);
	return status;

fail:
	perror(basename(progname));
	free(path);
	closedir(dir);
	return -1;
}

static struct richacl *get_richacl(const char *file, mode_t mode)
{
	struct richacl *acl;

	acl = richacl_get_file(file);
	if (!acl) {
		if (errno == ENOTSUP &&
		    (getxattr(file, "system.posix_acl_access", NULL, 0) >= 0 ||
		    (S_ISDIR(mode) &&
		     getxattr(file, "system.posix_acl_default", NULL, 0) >= 0))) {
			fprintf(stderr, "%s: POSIX ACL(s) exist\n", file);
			errno = 0;
			return NULL;
		} else if (errno == ENODATA || errno == ENOTSUP || errno == ENOSYS)
			acl = richacl_from_mode(mode);
	}
	return acl;
}

static int set_richacl(const char *path, struct richacl *acl)
{
	struct richace *ace;
	struct richacl_alloc x;
	int count = 0;
	int *blank_ace_table = NULL;
	int idx = 0, i = 0;

	richacl_for_each_entry(ace, acl) {
		if (ace->e_mask == 0)
			count++;
	}

	if (count != 0) {	
		blank_ace_table = malloc(sizeof(int) * count);
	
        	richacl_for_each_entry(ace, acl) {
                	if (ace->e_mask == 0) {
                        	*(blank_ace_table + idx) = i;
				idx++;
			}
			i++;
        	}
	
		for (i = 0; i < count; i++) {
                	x.acl = acl;
                	x.count = acl->a_count;
                	ace = x.acl->a_entries + (*(blank_ace_table + i) - i);
                	richacl_delete_entry(&x, &ace);
        	}

		acl = x.acl;
	}

	if (richacl_set_file(path, acl)) {
		struct stat st;

		if (stat(path, &st))
			return -1;
		if (!richacl_equiv_mode(acl, &st.st_mode))
			return chmod(path, st.st_mode);
		/* FIXME: We could try POSIX ACLs here as well. */
		return -1;
	}
	
	return 0;
}

int format_for_mode(mode_t mode)
{
	if (S_ISDIR(mode))
		return RICHACL_TEXT_DIRECTORY_CONTEXT;
	else
		return RICHACL_TEXT_FILE_CONTEXT;
}

static int print_richacl(const char *file, struct richacl **acl,
			 struct stat *st, int fmt)
{
	char *text;

	if (!(fmt & RICHACL_TEXT_SHOW_MASKS)) {
		if (richacl_apply_masks(acl))
			goto fail;
	}
	text = richacl_to_text(*acl, fmt | format_for_mode(st->st_mode));
	if (!text)
		goto fail;
	printf("%s:\n", file);
	puts(text);
	free(text);
	return 0;

fail:
	return -1;
}

void remove_filename(struct string_buffer *buffer)
{
	char *c, *end;

	for (c = buffer->buffer, end = buffer->buffer + buffer->offset;
	     c != end;
	     c++) {
		if (*c == '\n')
			break;
		if (*c == ':') {
			c++;
			if (c == end)
				break;
			if (*c != '\n')
				continue;
			c++;
			memmove(buffer->buffer, c, end - c + 1);
			buffer->offset = end - c;
		}
	}
}

static struct option long_options[] = {
	{"get-acl",		0, 0, 'g'},
	{"set-acl",             1, 0, 's'},
	{"modify-acl",          1, 0, 'm'},
	{"delete-ace",          1, 0, 'd'}, 	
	{"convert-aces",        0, 0, 'c'}, 
	{"effective-perms",     1, 0, 'e'}, 
	{"apply-inheritage",	0, 0, 'a'},
	{"copy-acl",            1, 0, 'C'},
	{"long",		0, 0, 'l'},
	{"raw",			0, 0,  1 },
	{"numeric-ids",		0, 0,  5 },
	{"version",		0, 0, 'v'},
	{"help",		0, 0, 'h'},
	{ NULL,			0, 0,  0 }
	
};

static void synopsis(int help)
{
	FILE *file = help ? stdout : stderr;

	fprintf(file, "richacl\nSYNOPSIS: %s [options] {command} file ...\n",
		basename(progname));
	if (!help) {
		fprintf(file, "Try `%s --help' for more information.\n",
			basename(progname));
		exit(1);
	}
	fprintf(file,
"\n"
"Commands:\n"
"  --get-acl, -g\n" 
"          Display the ACL of file(s). Multiple ACL entries are separated by newline\n"
"          Example: richacl -g file.c\n"
"  --set-acl acl, -s acl\n"
"          Set the ACL of file(s)\n"
"          Example: richacl -s 'user1:rwd::allow user2:r::deny' file.c\n"
"  --modify-acl acl, -m acl\n"
"          Modify the ACL of file(s)\n"
"          Example: richacl -m 'user1:rwd::allow user2:r::deny' file.c\n"
"  --delete-ace i-th ACE, -d i-th ACE\n"
"          Delete the i-th ACE of the ACL of file(s).\n"
"          Example: richacl -d 3 file.c\n"
"  --effective-perms username/groupname, -e username/groupname\n"
"          List the effective permissions of the user/group\n"
"          Example: richacl -e admin file.c\n"
"          Example: richacl -e :group1 file.c\n"
"  --convert-aces, -c\n"
"          Convert the inherited ACEs to explicit ACEs of file(s)\n"
"          Example: richacl -c file.c\n"
"  --apply-inheritage, -a\n"
"          Apply the inheritable ACEs of a directory to its descendants\n"
"          Example: richacl -a dir1\n"
"  --copy-acl, -C\n"
"          Copy the ACL of file1/dir1 to file2/dir2\n"
"          Example: richacl -C file1 file2\n"
"          Example: richacl -C dir1 dir2\n"
"  --version, -v\n"
"          Display the version of %s and exit.\n\n"
"  --help, -h\n"
" \n"
"Options:\n"
"  --long, -l  Display access masks and flags in their long form.\n"
"  --raw       Show acls as stored on the file system including the file masks.\n"
"              Implies --full.\n"
"  --numeric-ids\n"
"              Display numeric user and group IDs instead of names.\n"
"\n"
"ACL entries are represented by colon separated <who>:<mask>:<flags>:<type>\n"
"fields. The <who> field may be \"owner@\", \"group@\", \"everyone@\", a user\n"
"name or ID, or a group name or ID. Groups have the identifier_group(g) flag\n"
"set in the <flags> field. The <type> field may be \"allow\" or \"deny\".\n"
"The <mask> and <flags> fields are lists of single-letter abbreviations or\n"
"slash-separated names, or a combination of both.\n"
"\n"
"ACL entry <mask> values are:\n"
"\tread_data (r), list_directory (r), write_data (w), add_file (w),\n"
"\texecute (x), append_data (p), add_subdirectory (p), delete_child (d),\n"
"\tdelete (D), read_attributes (a), write_attributes (A), read_xattr (R),\n"
"\twrite_xattr (W), read_acl (c), write_acl (C), write_owner(o),\n"
"\tsynchronize (S), write_retention (e), write_retention_hold (E)\n"
"\n"
"ACL entry <flags> values are:\n"
"\tfile_inherit (f), dir_inherit (d),\n"
"\tno_propagate (n), inherit_only (i),\n"
"\tidentifier_group (g), inherited (a)\n"
"\n"
"ACL flag values are:\n"
"\tmasked (m), protected (p), defaulted (d), posix_mapped (P)\n",
	basename(progname));
	exit(0);
}

int main(int argc, char *argv[])
{
	int opt_remove = 0;
	int opt_get = 0, opt_set = 0, opt_modify = 0, opt_delete = 0;
	int opt_access = 0, opt_convert = 0, opt_apply = 0, opt_copy = 0;
	char *opt_user = NULL;
	char *acl_text = NULL, *acl_file = NULL;
	char *ace_index = NULL;
	char *dst_path = NULL, *src_path = NULL;
	int format = RICHACL_TEXT_SIMPLIFY | RICHACL_TEXT_ALIGN;
	uid_t user = -1;
	gid_t *groups = NULL;
	int n_groups = -1;
	int retval = 0;
	int c = 0;
	struct richacl *acl = NULL;
	struct richacl *acl2 = NULL;
	struct richace *ace = NULL;
	int acl_has;

	progname = argv[0];

	while ((c = getopt_long(argc, argv, "gs:m:d:cC:e:alvh",
				long_options, NULL)) != -1) {
		switch(c) {
			case 'g':
				opt_get = 1;
				break;
			case 's':
				opt_set = 1;
				acl_text = optarg;
				break;
			case 'm':
				opt_modify = 1;
				acl_text = optarg;
				break;
			case 'd':
				opt_delete = 1;
				ace_index = optarg;
				break;
			case 'e':
				opt_access = 1;
				opt_user = optarg;
				break;
			case 'c':
				opt_convert = 1; 
				break;
			case 'a':
				opt_apply = 1;
				break;
			case 'C':
				opt_copy = 1;
				src_path = optarg;
				break;
			case 'l':
				format |= RICHACL_TEXT_LONG;
				break;
			case 'v':
				printf("%s %s\n", basename(progname), VERSION);
				exit(0);

			case 'h':
				synopsis(1);
				break;

			case 1:  /* --raw */
				format |= RICHACL_TEXT_SHOW_MASKS;
				format &= ~RICHACL_TEXT_SIMPLIFY;
				break;
			case 5:  /* --numeric-ids */
				format |= RICHACL_TEXT_NUMERIC_IDS;
				break;

			default:
				fprintf(stderr, "Unsupported option! \n");
				break;
		}
	}

	/* Check unsupported option */
	if (opt_remove || acl_file) {
		fprintf(stderr, "Unsupported option! \n");
		synopsis(1);
	}
	
	/* Option checking */
	if ((opt_get + opt_set + opt_modify + opt_delete + 
		opt_access + opt_convert + opt_apply + opt_copy != 1) ||
		((acl_text ? 1 : 0) + (acl_file ? 1 : 0) > 1) ||
		optind == argc){
		synopsis(optind != argc);
	}

	/* Convert text format to richacl */
	if (acl_text) {
		acl = richacl_from_text(format, acl_text, &acl_has, printf_stderr);
		if (!acl) {
			fprintf(stderr, "Fail to convert text to richacl!\n");
			return -1;
		}
	}

	/* Compute all masks which haven't been set explicitly. */
	/* FIXME: how about --modify? */
	if (opt_set && acl)
		compute_masks(acl, acl_has);

	if (opt_user) {
		int n_groups_alloc;
		char *opt_groups;
		struct passwd *passwd = NULL;
		char *endp;

		opt_groups = strchr(opt_user, ':');
		if (opt_groups)
			*opt_groups++ = 0;
		user = strtoul(opt_user, &endp, 10);
		if (*endp) {
			passwd = getpwnam(opt_user);
			if (passwd == NULL) {
				fprintf(stderr, "%s: No such user\n", opt_user);
				exit(1);
			}
			user = passwd->pw_uid;
		} else
			user = -1;

		n_groups_alloc = 32;
		groups = malloc(sizeof(gid_t) * n_groups_alloc);
		if (!groups)
			goto fail;
		if (opt_groups) {
			char *tok;
			n_groups = 0;
			tok = strtok(opt_groups, ":");
			while (tok) {
				struct group *group;

				if (n_groups == n_groups_alloc) {
					gid_t *new_groups;
					n_groups_alloc *= 2;
					new_groups = realloc(groups, sizeof(gid_t) * n_groups_alloc);
					if (!new_groups)
						goto fail;
				}

				groups[n_groups] = strtoul(tok, &endp, 10);
				if (*endp) {
					group = getgrnam(tok);
					if (!group) {
						fprintf(stderr, "%s: No such group\n", tok);
						exit(1);
					}
					groups[n_groups] = group->gr_gid;
				}
				n_groups++;

				tok = strtok(NULL, ":");
			}
		} else {
			if (!passwd)
				passwd = getpwuid(user);
			if (passwd) {
				n_groups = n_groups_alloc;
				if (getgrouplist(passwd->pw_name, passwd->pw_gid,
					         groups, &n_groups) < 0) {
					free(groups);
					groups = malloc(sizeof(gid_t) * n_groups);
					if (getgrouplist(passwd->pw_name, passwd->pw_gid,
							 groups, &n_groups) < 0)
						goto fail;
				}
			} else
				n_groups = 0;
		}
	} else
		user = geteuid();


	for (; optind < argc; optind++) {
		const char *file = argv[optind];
		struct stat st;

		acl2 = NULL;

		/* Check input file */
		if (stat(file, &st)) {
			fprintf(stderr, "%s: %s\n", file, strerror(errno));
			goto clean;
		}

		/* Set the acl to file. If the file is a dir, then
		 * check if there are inheritable of aces, and save
		 * the aces to the child files/dirs
		 */
		if (opt_set) {
			int has_inheritable_ace = 0;

			acl2 = richacl_alloc(0);
			if (!acl2) {
				fprintf(stderr, "Allocate richacl fail\n");
				goto clean;
			}

			retval = removexattr(file, "system.richacl");

			if (modify_richacl(&acl2, acl, acl_has, 0)) {
				fprintf(stderr, "%s: fail to modify richacl\n", file);
				goto clean;
			}

			/* Save the richacl to file */
			if (set_richacl(file, acl2)) {
				fprintf(stderr, "%s: fail to save richacl fail\n", file);
				goto clean;
			}

			/* Applying inheritable ACEs to children */
			retval = stat(file, &st);
			if (!retval && S_ISDIR(st.st_mode)) {
				richacl_for_each_entry (ace, acl2) {
					if ((ace->e_flags & ACE4_FILE_INHERIT_ACE) ||
						(ace->e_flags & ACE4_DIRECTORY_INHERIT_ACE))
							has_inheritable_ace = 1;
				}

				if (has_inheritable_ace) {
					fprintf(stdout, "Applying inheritable ACEs to children...\n");
					if (qnap_apply_inheritage(acl2, file, 0, 0)) {
						fprintf(stdout, "Fail\n");
						goto clean;
					}
					fprintf(stdout, "Finish\n");
				}
			}
			
			if (acl) {
				richacl_free(acl2);
				acl2 = NULL;
			}

		} else if (opt_modify) {
			int has_inheritable_ace = 0;

			acl2 = get_richacl(file, st.st_mode);
			if (!acl2)
				goto clean;

			if (modify_richacl(&acl2, acl, acl_has, 0)) {
				fprintf(stderr, "%s: failied to modify richacl\n", file);
				goto clean;
			}
			
			if (set_richacl(file, acl2)) {
				fprintf(stderr, "%s: failed to save richacl\n", file);
				goto clean;
			}

			/* Applying inheritable ACEs to children */
			retval = stat(file, &st);
			if (!retval && S_ISDIR(st.st_mode)) {
				richacl_for_each_entry (ace, acl2) {
					if ((ace->e_flags & ACE4_FILE_INHERIT_ACE) || 
						(ace->e_flags & ACE4_DIRECTORY_INHERIT_ACE))
						has_inheritable_ace = 1;
				}

				if (has_inheritable_ace) {
					if (qnap_apply_inheritage(acl, file, 1, acl_has))
						goto clean;
				}
			}

			if (acl2) {
				richacl_free(acl2);
				acl2 = NULL;
			}

		} else if (opt_remove) {
			if (removexattr(file, "system.richacl")) {
				if (errno != ENODATA)
					goto clean;
			}
		} else if (opt_access) {
			int mask = 0;
			char *mask_text = NULL;

			mask = richacl_access(file, &st, user, groups, n_groups);
			if (mask < 0) {
				fprintf(stderr, "%s: fail to read effective permissions\n",file);
				goto clean;
			}

			mask_text = richacl_mask_to_text(mask,
					format | format_for_mode(st.st_mode));

			if (!mask_text) {
				fprintf(stderr, "%s: failed to read effective permissions\n",file);
				if (mask_text) {
					free(mask_text);
					mask_text = NULL;
				}
				goto clean;
			} else {
				printf("%s  %s\n", mask_text, file);
				if (mask_text) {
					free(mask_text);
					mask_text = NULL;
				}
			}

		} else if (opt_convert) {
			acl2 = get_richacl(file, st.st_mode);
			if (!acl2)
				goto clean;

			if (qnap_convert_richacl(&acl2)) {
				fprintf(stderr, "%s: failed to convert richacl\n", file);
				goto clean;
			}
			if (set_richacl(file, acl2)) {
				fprintf(stderr, "%s: failed to save richacl\n", file);
				goto clean;
			}
			
			if (acl2) {
				richacl_free(acl2);
				acl2 = NULL;
			}

		} else if (opt_delete) {
			int index = atoi(ace_index);

			acl2 = get_richacl(file, st.st_mode);
			if (!acl2)
				goto clean;	
				
			if (index > acl2->a_count || index <= 0) {
				fprintf(stderr, "%s: invalid index\n", file);
				goto clean;
			}

			/*
			 * If the ACE is inheritable ACE, then delete the 
 			 * inherited ACE of children
 			 */
			if (S_ISDIR(st.st_mode)) {
				struct richacl *acl3 = richacl_alloc(1);

				if (!acl3) {
					perror("malloc");
					goto clean;
				}

				richace_copy(&acl3->a_entries[0], &acl2->a_entries[index-1]);
				acl3->a_entries[0].e_mask = 0;
				/*
				 * Apply this delete to the children of the dir.
				 * the flag of subfolders/files with inherited flag will be
				 * removed
				 */
				if (qnap_apply_inheritage(acl3, file, 1, 0)) {
					if (acl3) {
						richacl_free(acl3);
						acl3 = NULL;
					}
					goto clean;
				}

				if (acl3) {
					richacl_free(acl3);
					acl3 = NULL;
				}
				
			}

			/* Delete ACE (given index) */
			retval = qnap_delete_richace(acl2, index);
			if (retval) {
				switch (retval) {
				case E_DELETE_INHERITED_ACE:
					fprintf(stderr, "%s: forbidden to delete inherited ACE\n", file);
					break;
				case E_DELETE_NOACL:
					fprintf(stderr, "%s: no existed ACL\n", file);
					break;
				case E_DELETE_NOINDEX:
					fprintf(stderr, "%s: no such index\n", file);
					break;
				}
				goto clean;
			}
			if (set_richacl(file, acl2)) {
				fprintf(stderr, "%s: failed to save richacl\n", file);
				goto clean;
			}
			if (acl2) {
				richacl_free(acl2);
				acl2 = NULL;
			}
		} else if (opt_apply) {
			if(!S_ISDIR(st.st_mode)) {
				fprintf(stderr, "%s: not a directory\n", file);
				goto clean;
			}

			acl2 = get_richacl(file, st.st_mode);
			if (!acl2)
				goto clean;

			if (qnap_apply_inheritage(acl2, file, 0, 0))
				goto clean;

			if (set_richacl(file, acl2)) {
				fprintf(stderr, "%s: failed to save richacl\n", file);
				goto clean;
			}

			if (acl2) {
				richacl_free(acl2);
				acl2 = NULL;
			}
		} else if (opt_copy) {
			dst_path = file;
			if (qnap_copy_richacl(src_path, dst_path)) {
				fprintf(stderr, "%s: failed to copy richacl\n", file);
				goto clean;
			}
		} else /* opt_get */ {
			acl2 = get_richacl(file, st.st_mode);
			if (!acl2) {
				fprintf(stderr, "%s: failed to get richacl\n", file);
				goto clean;
			}
			if (print_richacl(file, &acl2, &st, format)) {
				fprintf(stderr, "%s: failed to print richacl\n", file);
				goto clean;
			}

		}

		clean:
		if (acl2) {
			richacl_free(acl2);
			acl2 = NULL;
		}

		continue;
	}
	
	if (acl) {
		richacl_free(acl);
		acl = NULL;
	}

	return 0;

fail:
	if (acl) {
		richacl_free(acl);
		acl = NULL;
	}

	return -1;
}
