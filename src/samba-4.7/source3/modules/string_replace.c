/*
 * Unix SMB/CIFS implementation.
 *
 * Copyright (C) Volker Lendecke, 2005
 * Copyright (C) Aravind Srinivasan, 2009
 * Copyright (C) Guenter Kukkukk, 2013
 * Copyright (C) Ralph Boehme, 2017
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "includes.h"
#include "smbd/smbd.h"
#include "string_replace.h"

#define MAP_SIZE        0xFF
#define MAP_NUM         0x101 /* max unicode charval / MAP_SIZE */
#define T_OFFSET(_v_)   ((_v_ % MAP_SIZE))
#define T_START(_v_)    (((_v_ / MAP_SIZE) * MAP_SIZE))
#define T_PICK(_v_)     ((_v_ / MAP_SIZE))

struct char_mappings {
	smb_ucs2_t entry[MAP_SIZE][2];
};

static bool build_table(struct char_mappings **cmaps, int value)
{
	int i;
	int start = T_START(value);

	(*cmaps) = talloc_zero(NULL, struct char_mappings);

	if (!*cmaps)
		return False;

	for (i = 0; i < MAP_SIZE;i++) {
		(*cmaps)->entry[i][vfs_translate_to_unix] = start + i;
		(*cmaps)->entry[i][vfs_translate_to_windows] = start + i;
	}

	return True;
}

static void set_tables(struct char_mappings **cmaps,
		       long unix_map,
		       long windows_map)
{
	int i;

	/* set unix -> windows */
	i = T_OFFSET(unix_map);
	cmaps[T_PICK(unix_map)]->entry[i][vfs_translate_to_windows] = windows_map;

	/* set windows -> unix */
	i = T_OFFSET(windows_map);
	cmaps[T_PICK(windows_map)]->entry[i][vfs_translate_to_unix] = unix_map;
}

static bool build_ranges(struct char_mappings **cmaps,
			 long unix_map,
			 long windows_map)
{

	if (!cmaps[T_PICK(unix_map)]) {
		if (!build_table(&cmaps[T_PICK(unix_map)], unix_map))
			return False;
	}

	if (!cmaps[T_PICK(windows_map)]) {
		if (!build_table(&cmaps[T_PICK(windows_map)], windows_map))
			return False;
	}

	set_tables(cmaps, unix_map, windows_map);

	return True;
}

struct char_mappings **string_replace_init_map(const char **mappings)
{
	int i;
	char *tmp;
	fstring mapping;
	long unix_map, windows_map;
	struct char_mappings **cmaps = NULL;

	if (mappings == NULL) {
		return NULL;
	}

	cmaps = TALLOC_ZERO(NULL, MAP_NUM * sizeof(struct char_mappings *));
	if (cmaps == NULL) {
		return NULL;
	}

	/*
	 * catia mappings are of the form :
	 * UNIX char (in 0xnn hex) : WINDOWS char (in 0xnn hex)
	 *
	 * multiple mappings are comma separated in smb.conf
	 */

	for (i = 0; mappings[i]; i++) {
		fstrcpy(mapping, mappings[i]);
		unix_map = strtol(mapping, &tmp, 16);
		if (unix_map == 0 && errno == EINVAL) {
			DEBUG(0, ("INVALID CATIA MAPPINGS - %s\n", mapping));
			continue;
		}
		windows_map = strtol(++tmp, NULL, 16);
		if (windows_map == 0 && errno == EINVAL) {
			DEBUG(0, ("INVALID CATIA MAPPINGS - %s\n", mapping));
			continue;
		}

		if (!build_ranges(cmaps, unix_map, windows_map)) {
			DEBUG(0, ("TABLE ERROR - CATIA MAPPINGS - %s\n", mapping));
			continue;
		}
	}

	return cmaps;
}

NTSTATUS string_replace_allocate(connection_struct *conn,
				 const char *name_in,
				 struct char_mappings **cmaps,
				 TALLOC_CTX *mem_ctx,
				 char **mapped_name,
				 enum vfs_translate_direction direction)
{
	static smb_ucs2_t *tmpbuf = NULL;
	smb_ucs2_t *ptr = NULL;
	struct char_mappings *map = NULL;
	size_t converted_size;
	bool ok;

	ok = push_ucs2_talloc(talloc_tos(), &tmpbuf, name_in,
			      &converted_size);
	if (!ok) {
		return map_nt_error_from_unix(errno);
	}

	for (ptr = tmpbuf; *ptr; ptr++) {
		if (*ptr == 0) {
			break;
		}
		if (cmaps == NULL) {
			continue;
		}
		map = cmaps[T_PICK((*ptr))];
		if (map == NULL) {
			/* nothing to do */
			continue;
		}

		*ptr = map->entry[T_OFFSET((*ptr))][direction];
	}

#ifdef QNAPNAS
	char *temp_map = NULL;
	ok = pull_ucs2_talloc(mem_ctx, &temp_map, tmpbuf,
			  &converted_size);
#else
	ok = pull_ucs2_talloc(mem_ctx, mapped_name, tmpbuf,
			  &converted_size);
#endif

	TALLOC_FREE(tmpbuf);
	if (!ok) {
		return map_nt_error_from_unix(errno);
	}

#ifdef QNAPNAS
	/* Now Catia mapping existed, check if Qmapping is necessary */
	if (IsClientMacOs() && IsQmappingEnable() && access(temp_map, F_OK) == -1) {
		DEBUG(10, ("vfs_catia: It's MacOS client and filename not found in NFC format.\n"));
		char *result = RunQmapping(temp_map);
		if (result == NULL)
			*mapped_name = talloc_strdup(mem_ctx, temp_map);
		else {
			*mapped_name = talloc_strdup(mem_ctx, result);
			free(result);
		}
	}
	else
		*mapped_name = talloc_strdup(mem_ctx, temp_map);
	
	TALLOC_FREE(temp_map);
	
	if (!*mapped_name) {
		errno = ENOMEM;
		return NT_STATUS_NO_MEMORY;
	}
#endif /* QNAPNAS */

	return NT_STATUS_OK;
}


#ifdef QNAPNAS

/*
 * Qnap function. Check if Qmapping is enable.
 */
bool IsQmappingEnable()
{
	bool mappings = lp_parm_bool(-1, "catia", "Qmappings", true);

	DEBUG(10, ("vfs_catia: Qmappings is %s\n", mappings ? "on":"off"));

	return mappings;
}


/*
 * Qnap function. Check if samba client is macOS.
 */
bool IsClientMacOs()
{
	if (get_remote_arch() == RA_OSX)
		return True;
	return False;
}


/*
 * Qnap function. Convert input string to specified normalization form.
 *
 * @param nf                 	The normalization form: UNINORM_NFD or UNINORM_NFC
 * @param src                   Input source filename string
 * @return NULL                 Error happened
 * @return string               Output string in specified normalization
 */
uint8_t* u8_norm_converter(uninorm_t nf, const uint8_t *src)
{
	uint8_t *result = NULL;
	size_t len = 0;

	// convert to UNINORM_NFD
	result = u8_normalize(nf, src, strlen(src)+1, NULL, &len);
	if (result == NULL) // fail
		return NULL;

	return result;
}


/*
 * Qnap function. Convert input filename string and check if it's existed.
 *
 * @param name_in             	Input source filename string
 * @return NULL                 Error or filename not found
 * @return string               Output string in specified normalization
 */
char* RunQmapping(const char *name_in)
{
	uint8_t *result = NULL;

	result = u8_norm_converter(UNINORM_NFD, (uint8_t*)name_in);
	if (result == NULL)
		return NULL;

	if (access(result, F_OK) == -1) { // if NFD format Not found
		free(result);
		DEBUG(10, ("vfs_catia: NFC/NFD filename NOT found! It's a new file! Return NULL\n"));
		return NULL;
	}
	else {
		DEBUG(10, ("vfs_catia: NFD Filename:%s found!\n", result));
		return (char*)result;
	}

	return NULL;
}

#endif /* QNAPNAS */


