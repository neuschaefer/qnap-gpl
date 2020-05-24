/*
 * Store streams in xattrs
 *
 * Copyright (C) Volker Lendecke, 2008
 *
 * Partly based on James Peach's Darwin module, which is
 *
 * Copyright (C) James Peach 2006-2007
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "includes.h"
#include "smbd/smbd.h"
#include "system/filesys.h"
#include "../lib/crypto/md5.h"

#define APPLE_USERTAG	"kMDItemUserTags"
#define USERTAG_EA_NETATALK	"user.com.apple.metadata:_kMDItemUserTags"

struct stream_io {
	char *base;
	char *xattr_name;
	void *fsp_name_ptr;
	files_struct *fsp;
	vfs_handle_struct *handle;
};

static bool is_apple_usertag_stream(const struct smb_filename *smb_fname)
{
	if (strstr_m(smb_fname->stream_name, APPLE_USERTAG) != NULL) {
		return true;
	}
	return false;
}

static SMB_INO_T stream_inode(const SMB_STRUCT_STAT *sbuf, const char *sname)
{
	MD5_CTX ctx;
	unsigned char hash[16];
	SMB_INO_T result;
	char *upper_sname;

	DEBUG(10, ("stream_inode called for %lu/%lu [%s]\n",
		   (unsigned long)sbuf->st_ex_dev,
		   (unsigned long)sbuf->st_ex_ino, sname));

	upper_sname = talloc_strdup_upper(talloc_tos(), sname);
	SMB_ASSERT(upper_sname != NULL);

	MD5Init(&ctx);
	MD5Update(&ctx, (const unsigned char *)&(sbuf->st_ex_dev),sizeof(sbuf->st_ex_dev));
	MD5Update(&ctx, (const unsigned char *)&(sbuf->st_ex_ino),sizeof(sbuf->st_ex_ino));
	MD5Update(&ctx, (unsigned char *)upper_sname,talloc_get_size(upper_sname)-1);
	MD5Final(hash, &ctx);

	TALLOC_FREE(upper_sname);

	/* Hopefully all the variation is in the lower 4 (or 8) bytes! */
	memcpy(&result, hash, sizeof(result));

	DEBUG(10, ("stream_inode returns %lu\n", (unsigned long)result));

	return result;
}

static ssize_t get_xattr_size(connection_struct *conn,
				const struct smb_filename *smb_fname,
				const char *xattr_name)
{
	NTSTATUS status;
	struct ea_struct ea;
	ssize_t result;

	status = get_ea_value(talloc_tos(), conn, NULL, smb_fname,
			      xattr_name, &ea);

	if (!NT_STATUS_IS_OK(status)) {
		return -1;
	}

	result = ea.value.length;
	TALLOC_FREE(ea.value.data);
	return result;
}

static bool del_specific_stream(TALLOC_CTX *mem_ctx, unsigned int *num_streams,
			     struct stream_struct **streams,
			     const char *name)
{
	struct stream_struct *tmp = *streams;
	int i;

	if (*num_streams == 0) {
		return true;
	}

	for (i = 0; i < *num_streams; i++) {
		if (strstr_m(tmp[i].name,name)) {
			break;
		}
	}

	if (i == *num_streams) {
		return true;
	}

	TALLOC_FREE(tmp[i].name);
	if (*num_streams - 1 > i) {
		memmove(&tmp[i], &tmp[i+1],
			(*num_streams - i - 1) * sizeof(struct stream_struct));
	}

	*num_streams -= 1;

	return true;
}

static bool qnap_macea_recheck(struct stream_io *sio)
{
	char *xattr_name = USERTAG_EA_NETATALK;

	if (sio->fsp->fsp_name == sio->fsp_name_ptr) {
		return true;
	}

	if (sio->fsp->fsp_name->stream_name == NULL) {
		/* how can this happen */
		errno = EINVAL;
		return false;
	}

	TALLOC_FREE(sio->xattr_name);
	TALLOC_FREE(sio->base);
	sio->xattr_name = talloc_strdup(VFS_MEMCTX_FSP_EXTENSION(sio->handle, sio->fsp),
					xattr_name);
	sio->base = talloc_strdup(VFS_MEMCTX_FSP_EXTENSION(sio->handle, sio->fsp),
				  sio->fsp->fsp_name->base_name);
	sio->fsp_name_ptr = sio->fsp->fsp_name;

	if ((sio->xattr_name == NULL) || (sio->base == NULL)) {
		return false;
	}

	return true;
}

/**
 * Helper to stat/lstat the base file of an smb_fname.
 */
static int qnap_macea_stat_base(vfs_handle_struct *handle,
				   struct smb_filename *smb_fname,
				   bool follow_links)
{
	char *tmp_stream_name;
	int result;

	tmp_stream_name = smb_fname->stream_name;
	smb_fname->stream_name = NULL;
	if (follow_links) {
		result = SMB_VFS_NEXT_STAT(handle, smb_fname);
	} else {
		result = SMB_VFS_NEXT_LSTAT(handle, smb_fname);
	}
	smb_fname->stream_name = tmp_stream_name;
	return result;
}

static int qnap_macea_fstat(vfs_handle_struct *handle, files_struct *fsp,
			       SMB_STRUCT_STAT *sbuf)
{
	struct smb_filename *smb_fname_base = NULL;
	int ret = -1;
	struct stream_io *io = (struct stream_io *)
		VFS_FETCH_FSP_EXTENSION(handle, fsp);

	if (io == NULL || fsp->base_fsp == NULL) {
		return SMB_VFS_NEXT_FSTAT(handle, fsp, sbuf);
	}

	DEBUG(10, ("qnap_macea_fstat called for %s\n", fsp_str_dbg(io->fsp)));

	if (!qnap_macea_recheck(io)) {
		return -1;
	}

	/* Create an smb_filename with stream_name == NULL. */
	smb_fname_base = synthetic_smb_fname(talloc_tos(),
					io->base,
					NULL,
					NULL,
					fsp->fsp_name->flags);
	if (smb_fname_base == NULL) {
		errno = ENOMEM;
		return -1;
	}

	if (smb_fname_base->flags & SMB_FILENAME_POSIX_PATH) {
		ret = SMB_VFS_LSTAT(handle->conn, smb_fname_base);
	} else {
		ret = SMB_VFS_STAT(handle->conn, smb_fname_base);
	}
	*sbuf = smb_fname_base->st;

	if (ret == -1) {
		TALLOC_FREE(smb_fname_base);
		return -1;
	}

	sbuf->st_ex_size = get_xattr_size(handle->conn,
					smb_fname_base, io->xattr_name);
	if (sbuf->st_ex_size == -1) {
		TALLOC_FREE(smb_fname_base);
		SET_STAT_INVALID(*sbuf);
		return -1;
	}

	sbuf->st_ex_ino = stream_inode(sbuf, io->xattr_name);
	sbuf->st_ex_mode &= ~S_IFMT;
	sbuf->st_ex_mode |= S_IFREG;
	sbuf->st_ex_blocks = sbuf->st_ex_size / STAT_ST_BLOCKSIZE + 1;

	TALLOC_FREE(smb_fname_base);
	return 0;
}

static int qnap_macea_stat(vfs_handle_struct *handle,
			      struct smb_filename *smb_fname)
{
	NTSTATUS status;
	int result = -1;
	char *xattr_name = USERTAG_EA_NETATALK;

	if (!is_ntfs_stream_smb_fname(smb_fname)) {
		return SMB_VFS_NEXT_STAT(handle, smb_fname);
	}

	if (!is_apple_usertag_stream(smb_fname)) {
		return SMB_VFS_NEXT_STAT(handle, smb_fname);
	}

	/* Note if lp_posix_paths() is true, we can never
	 * get here as is_ntfs_stream_smb_fname() is
	 * always false. So we never need worry about
	 * not following links here. */

	/* If the default stream is requested, just stat the base file. */
	if (is_ntfs_default_stream_smb_fname(smb_fname)) {
		return qnap_macea_stat_base(handle, smb_fname, true);
	}

	/* Populate the stat struct with info from the base file. */
	if (qnap_macea_stat_base(handle, smb_fname, true) == -1) {
		return -1;
	}

	/* Augment the base file's stat information before returning. */
	smb_fname->st.st_ex_size = get_xattr_size(handle->conn,
						  smb_fname,
						  xattr_name);
	if (smb_fname->st.st_ex_size == -1) {
		SET_STAT_INVALID(smb_fname->st);
		errno = ENOENT;
		result = -1;
		goto fail;
	}

	smb_fname->st.st_ex_ino = stream_inode(&smb_fname->st, xattr_name);
	smb_fname->st.st_ex_mode &= ~S_IFMT;
	smb_fname->st.st_ex_mode |= S_IFREG;
	smb_fname->st.st_ex_blocks = smb_fname->st.st_ex_size / STAT_ST_BLOCKSIZE + 1;

	result = 0;
 fail:
	return result;
}

static int qnap_macea_lstat(vfs_handle_struct *handle,
			       struct smb_filename *smb_fname)
{
	NTSTATUS status;
	int result = -1;
	char *xattr_name = USERTAG_EA_NETATALK;

	if (!is_ntfs_stream_smb_fname(smb_fname)) {
		return SMB_VFS_NEXT_LSTAT(handle, smb_fname);
	}

	if (!is_apple_usertag_stream(smb_fname)) {
		return SMB_VFS_NEXT_LSTAT(handle, smb_fname);
	}

	/* If the default stream is requested, just stat the base file. */
	if (is_ntfs_default_stream_smb_fname(smb_fname)) {
		return qnap_macea_stat_base(handle, smb_fname, false);
	}

	/* Populate the stat struct with info from the base file. */
	if (qnap_macea_stat_base(handle, smb_fname, false) == -1) {
		return -1;
	}

	/* Augment the base file's stat information before returning. */
	smb_fname->st.st_ex_size = get_xattr_size(handle->conn,
						  smb_fname,
						  xattr_name);
	if (smb_fname->st.st_ex_size == -1) {
		SET_STAT_INVALID(smb_fname->st);
		errno = ENOENT;
		result = -1;
		goto fail;
	}

	smb_fname->st.st_ex_ino = stream_inode(&smb_fname->st, xattr_name);
	smb_fname->st.st_ex_mode &= ~S_IFMT;
	smb_fname->st.st_ex_mode |= S_IFREG;
	smb_fname->st.st_ex_blocks = smb_fname->st.st_ex_size / STAT_ST_BLOCKSIZE + 1;

	result = 0;

 fail:
	return result;
}

static int qnap_macea_open(vfs_handle_struct *handle,
			      struct smb_filename *smb_fname,
			      files_struct *fsp, int flags, mode_t mode)
{
	NTSTATUS status;
	struct smb_filename *smb_fname_base = NULL;
	struct stream_io *sio = NULL;
	struct ea_struct ea;
	char *xattr_name = USERTAG_EA_NETATALK;
	int baseflags;
	int hostfd = -1;

	DEBUG(10, ("qnap_macea_open called for %s with flags 0x%x\n",
		   smb_fname_str_dbg(smb_fname), flags));

	if (!is_ntfs_stream_smb_fname(smb_fname)) {
		return SMB_VFS_NEXT_OPEN(handle, smb_fname, fsp, flags, mode);
	}

	if (!is_apple_usertag_stream(smb_fname)) {
		return SMB_VFS_NEXT_OPEN(handle, smb_fname, fsp, flags, mode);
	}

	/* If the default stream is requested, just open the base file. */
	if (is_ntfs_default_stream_smb_fname(smb_fname)) {
		char *tmp_stream_name;
		int ret;

		tmp_stream_name = smb_fname->stream_name;
		smb_fname->stream_name = NULL;

		ret = SMB_VFS_NEXT_OPEN(handle, smb_fname, fsp, flags, mode);

		smb_fname->stream_name = tmp_stream_name;

		return ret;
	}

	/* Create an smb_filename with stream_name == NULL. */
	smb_fname_base = synthetic_smb_fname(
		talloc_tos(), smb_fname->base_name, NULL, NULL, smb_fname->flags);
	if (smb_fname_base == NULL) {
		errno = ENOMEM;
		goto fail;
	}

	/*
	* We use baseflags to turn off nasty side-effects when opening the
	* underlying file.
	*/
	baseflags = flags;
	baseflags &= ~O_TRUNC;
	baseflags &= ~O_EXCL;
	baseflags &= ~O_CREAT;

	hostfd = SMB_VFS_OPEN(handle->conn, smb_fname_base, fsp, baseflags, mode);

	TALLOC_FREE(smb_fname_base);

	/* It is legit to open a stream on a directory, but the base
	* fd has to be read-only.
	*/
	if ((hostfd == -1) && (errno == EISDIR)) {
		baseflags &= ~O_ACCMODE;
		baseflags |= O_RDONLY;
		hostfd = SMB_VFS_OPEN(handle->conn, smb_fname, fsp, baseflags,mode);
	}

	if (hostfd == -1) {
		goto fail;
	}

	status = get_ea_value(talloc_tos(), handle->conn, NULL,
			      smb_fname, xattr_name, &ea);

	DEBUG(10, ("qnap_macea_open: get_ea_value returned %s\n", nt_errstr(status)));

	if (!NT_STATUS_IS_OK(status)
	    && !NT_STATUS_EQUAL(status, NT_STATUS_NOT_FOUND)) {
		/*
		 * The base file is not there. This is an error even if we got
		 * O_CREAT, the higher levels should have created the base
		 * file for us.
		 */
		DEBUG(3, ("qnap_macea_open: base file %s not around, "
			   "returning ENOENT\n", smb_fname->base_name));
		errno = ENOENT;
		goto fail;
	}

	if ((!NT_STATUS_IS_OK(status) && (flags & O_CREAT)) ||
	    (flags & O_TRUNC)) {
		/*
		 * The attribute does not exist or needs to be truncated
		 */

		/*
		 * Darn, xattrs need at least 1 byte
		 */
		char null = '\0';

		DEBUG(3, ("qnap_macea_open: creating or truncating attribute %s on file %s\n",
			   xattr_name, smb_fname->base_name));

		int ret = SMB_VFS_SETXATTR(fsp->conn,
				       smb_fname,
				       xattr_name,
				       &null, sizeof(null),
				       flags & O_EXCL ? XATTR_CREATE : 0);
		if (ret != 0) {
			goto fail;
		}
	}

	sio = (struct stream_io *)VFS_ADD_FSP_EXTENSION(handle, fsp, struct stream_io,NULL);
	if (sio == NULL) {
		errno = ENOMEM;
		goto fail;
	}

	sio->xattr_name = talloc_strdup(VFS_MEMCTX_FSP_EXTENSION(handle, fsp),xattr_name);
	sio->base = talloc_strdup(VFS_MEMCTX_FSP_EXTENSION(handle, fsp),fsp->fsp_name->base_name);
	sio->fsp_name_ptr = fsp->fsp_name;
	sio->handle = handle;
	sio->fsp = fsp;

	if ((sio->xattr_name == NULL) || (sio->base == NULL)) {
		errno = ENOMEM;
		goto fail;
	}
	
	return hostfd;

 fail:
	if (hostfd >= 0) {
		/*
		 * BUGBUGBUG -- we would need to call fd_close_posix here, but
		 * we don't have a full fsp yet
		 */
		fsp->fh->fd = hostfd;
		SMB_VFS_CLOSE(fsp);
	}

	return -1;
}

static int qnap_macea_unlink(vfs_handle_struct *handle,
				const struct smb_filename *smb_fname)
{
	int ret = -1;

	DEBUG(10, ("qnap_macea_unlink called for %s \n", smb_fname_str_dbg(smb_fname)));

	if (!is_ntfs_stream_smb_fname(smb_fname)) {
		return SMB_VFS_NEXT_UNLINK(handle, smb_fname);
	}

	if (!is_apple_usertag_stream(smb_fname)) {
		return SMB_VFS_NEXT_UNLINK(handle, smb_fname);
	}

	/* If the default stream is requested, just open the base file. */
	if (is_ntfs_default_stream_smb_fname(smb_fname)) {
		struct smb_filename *smb_fname_base = NULL;

		smb_fname_base = cp_smb_filename(talloc_tos(), smb_fname);
		if (smb_fname_base == NULL) {
			errno = ENOMEM;
			return -1;
		}

		ret = SMB_VFS_NEXT_UNLINK(handle, smb_fname_base);

		TALLOC_FREE(smb_fname_base);
		return ret;
	}
	/* Ignores deletes on the User tag stream like AFP */
#if 0
	char *xattr_name = USERTAG_EA_NETATALK;
	ret = SMB_VFS_REMOVEXATTR(handle->conn, smb_fname->base_name, xattr_name);

	if ((ret == -1) && (errno == ENOATTR)) {
		errno = ENOENT;
		goto fail;
	}
#endif
	ret = 0;

 fail:
	return ret;
}

static NTSTATUS walk_xattr_streams(vfs_handle_struct *handle,
				files_struct *fsp,
				const struct smb_filename *smb_fname,
				bool (*fn)(struct ea_struct *ea,
					void *private_data),
				void *private_data)
{
	NTSTATUS status;
	char **names;
	size_t i, num_names;
	bool ea_support_value;

	/* KS-Redmine#23511 */
	ea_support_value = lp_ea_support(SNUM(handle->conn));
	lp_do_parameter(SNUM(handle->conn), "ea support", "yes");
	status = get_ea_names_from_file(talloc_tos(),
				handle->conn,
				fsp,
				smb_fname,
				&names,
				&num_names);
	lp_do_parameter(SNUM(handle->conn), "ea support", (ea_support_value)? "yes":"no");

	if (!NT_STATUS_IS_OK(status)) {
		/* Bug#99510 (KS-Redmine#24152) */
		num_names = 0;
	}

	for (i=0; i<num_names; i++) {
		struct ea_struct ea;
		 
		if (strstr_m(names[i], APPLE_USERTAG) == NULL) {
			continue;
		}

		if (samba_private_attr_name(names[i])) {
			continue;
		}

		status = get_ea_value(names,
					handle->conn,
					NULL,
					smb_fname,
					names[i],
					&ea);
		if (!NT_STATUS_IS_OK(status)) {
			DEBUG(0, ("Could not get ea %s for file %s: %s\n",
				names[i],
				smb_fname->base_name,
				nt_errstr(status)));
			continue;
		}

		ea.name = talloc_asprintf(
			ea.value.data, ":%s%s",ea.name,":$DATA");
		if (ea.name == NULL) {
			DEBUG(0, ("talloc failed\n"));
			continue;
		}

		if (!fn(&ea, private_data)) {
			TALLOC_FREE(ea.value.data);
			return NT_STATUS_OK;
		}

		TALLOC_FREE(ea.value.data);
	}

	TALLOC_FREE(names);
	return NT_STATUS_OK;
}

static bool add_one_stream(TALLOC_CTX *mem_ctx, unsigned int *num_streams,
			   struct stream_struct **streams,
			   const char *name, off_t size,
			   off_t alloc_size)
{
	struct stream_struct *tmp;

	tmp = talloc_realloc(mem_ctx, *streams, struct stream_struct,
				   (*num_streams)+1);
	if (tmp == NULL) {
		return false;
	}

	/* Bug#94029 (KS-Redmine#23506) 
	 *
	 * Reply correct name for 'UserTag' streams to client
	 */
	char stream_info_name[]={':',0x63,0x6f,0x6d,0x2e,0x61,0x70,0x70,0x6c,0x65,0x2e  \
                ,0x6d,0x65,0x74,0x61,0x64,0x61,0x74,0x61,0xffef,0xff80  \
                ,0xffa2,0x5f,0x6b,0x4d,0x44,0x49,0x74,0x65,0x6d,0x55,0x73,0x65  \
                ,0x72,0x54,0x61,0x67,0x73,':','%$','D','A','T','A','\0'};
	tmp[*num_streams].name = talloc_strdup(tmp, stream_info_name);
	if (tmp[*num_streams].name == NULL) {
		return false;
	}

	tmp[*num_streams].size = size;
	tmp[*num_streams].alloc_size = alloc_size;

	*streams = tmp;
	*num_streams += 1;
	return true;
}

struct streaminfo_state {
	TALLOC_CTX *mem_ctx;
	vfs_handle_struct *handle;
	unsigned int num_streams;
	struct stream_struct *streams;
	NTSTATUS status;
};

static bool collect_one_stream(struct ea_struct *ea, void *private_data)
{
	struct streaminfo_state *state =
		(struct streaminfo_state *)private_data;

	if (!add_one_stream(state->mem_ctx,
			    &state->num_streams, &state->streams,
			    ea->name, ea->value.length,
			    smb_roundup(state->handle->conn,
					ea->value.length))) {
		state->status = NT_STATUS_NO_MEMORY;
		return false;
	}

	return true;
}

static NTSTATUS qnap_macea_streaminfo(vfs_handle_struct *handle,
					 struct files_struct *fsp,
					 const struct smb_filename *smb_fname,
					 TALLOC_CTX *mem_ctx,
					 unsigned int *pnum_streams,
					 struct stream_struct **pstreams)
{
	SMB_STRUCT_STAT sbuf;
	int ret;
	NTSTATUS status;
	struct streaminfo_state state;

	status = SMB_VFS_NEXT_STREAMINFO(handle, fsp, smb_fname, mem_ctx,
					 pnum_streams, pstreams);
	if (!NT_STATUS_IS_OK(status)) {
		return status;
	}

	del_specific_stream(mem_ctx, pnum_streams, pstreams, APPLE_USERTAG);	

	ret = vfs_stat_smb_basename(handle->conn, smb_fname, &sbuf);
	if (ret == -1) {
		return map_nt_error_from_unix(errno);
	}

	state.streams = *pstreams;
	state.num_streams = *pnum_streams;
	state.mem_ctx = mem_ctx;
	state.handle = handle;
	state.status = NT_STATUS_OK;

	if (S_ISLNK(sbuf.st_ex_mode)) {
		/*
		 * Currently we do't have SMB_VFS_LLISTXATTR
		 * inside the VFS which means there's no way
		 * to cope with a symlink when lp_posix_pathnames().
		 * returns true. For now ignore links.
		 * FIXME - by adding SMB_VFS_LLISTXATTR. JRA.
		 */
		status = NT_STATUS_OK;
	} else {
		status = walk_xattr_streams(handle, fsp, smb_fname,
				    collect_one_stream, &state);
	}

	if (!NT_STATUS_IS_OK(status)) {
		TALLOC_FREE(state.streams);
		return status;
	}

	if (!NT_STATUS_IS_OK(state.status)) {
		TALLOC_FREE(state.streams);
		return state.status;
	}

	*pnum_streams = state.num_streams;
	*pstreams = state.streams;

	return status;
}

static ssize_t qnap_macea_pwrite(vfs_handle_struct *handle,
				    files_struct *fsp, const void *data,
				    size_t n, off_t offset)
{
	struct stream_io *sio =
		(struct stream_io *)VFS_FETCH_FSP_EXTENSION(handle, fsp);
	struct ea_struct ea;
	NTSTATUS status;
	struct smb_filename *smb_fname_base = NULL;
	int ret;

	DEBUG(10, ("qnap_macea_pwrite called for %d bytes, offset %.0f\n", (int)n,(double)offset));

	if (sio == NULL) {
		return SMB_VFS_NEXT_PWRITE(handle, fsp, data, n, offset);
	}

	if (!qnap_macea_recheck(sio)) {
		return -1;
	}

	/* Create an smb_filename with stream_name == NULL. */
	smb_fname_base = synthetic_smb_fname(talloc_tos(),
					sio->base,
					NULL,
					NULL,
					fsp->fsp_name->flags);
	if (smb_fname_base == NULL) {
		errno = ENOMEM;
		return -1;
	}

	status = get_ea_value(talloc_tos(), handle->conn, NULL,
			      smb_fname_base, sio->xattr_name, &ea);
	if (!NT_STATUS_IS_OK(status)) {
		return -1;
	}
	
	if ((offset + n) > ea.value.length) {
		uint8_t *tmp;

		tmp = talloc_realloc(talloc_tos(), ea.value.data, uint8_t,
					   offset + n);

		if (tmp == NULL) {
			TALLOC_FREE(ea.value.data);
			errno = ENOMEM;
			return -1;
		}
		ea.value.data = tmp;
		ea.value.length = offset + n;
		ea.value.data[offset+n] = 0;
	}

	memcpy(ea.value.data + offset, data, n);

	ret = SMB_VFS_SETXATTR(fsp->conn,
			       fsp->fsp_name,
			       sio->xattr_name,
			       ea.value.data, ea.value.length, 0);
	TALLOC_FREE(ea.value.data);

	if (ret == -1) {
		return -1;
	}

	return n;
}

static ssize_t qnap_macea_pread(vfs_handle_struct *handle,
				   files_struct *fsp, void *data,
				   size_t n, off_t offset)
{
	struct stream_io *sio =
		(struct stream_io *)VFS_FETCH_FSP_EXTENSION(handle, fsp);
	struct ea_struct ea;
	NTSTATUS status;
	size_t length, overlap;
	struct smb_filename *smb_fname_base = NULL;

	DEBUG(10, ("qnap_macea_pread: offset=%d, size=%d\n",
		   (int)offset, (int)n));

	if (sio == NULL) {
		return SMB_VFS_NEXT_PREAD(handle, fsp, data, n, offset);
	}

	if (!qnap_macea_recheck(sio)) {
		return -1;
	}

	/* Create an smb_filename with stream_name == NULL. */
	smb_fname_base = synthetic_smb_fname(talloc_tos(),
					sio->base,
					NULL,
					NULL,
					fsp->fsp_name->flags);
	if (smb_fname_base == NULL) {
		errno = ENOMEM;
		return -1;
	}

	status = get_ea_value(talloc_tos(), handle->conn, NULL,
			      smb_fname_base, sio->xattr_name, &ea);
	if (!NT_STATUS_IS_OK(status)) {
		return -1;
	}

	length = ea.value.length;

	DEBUG(10, ("qnap_macea_pread: get_ea_value returned %d bytes\n",
		   (int)length));

	/* Attempt to read past EOF. */
	if (length <= offset) {
		return 0;
	}

	overlap = (offset + n) > length ? (length - offset) : n;
	memcpy(data, ea.value.data + offset, overlap);

	TALLOC_FREE(ea.value.data);
	return overlap;
}

static int qnap_macea_ftruncate(struct vfs_handle_struct *handle,
					struct files_struct *fsp,
					off_t offset)
{
	int ret;
	uint8_t *tmp;
	struct ea_struct ea;
	NTSTATUS status;
	struct stream_io *sio =
		(struct stream_io *)VFS_FETCH_FSP_EXTENSION(handle, fsp);
	struct smb_filename *smb_fname_base = NULL;

	DEBUG(10, ("qnap_macea_ftruncate called for file %s offset %.0f\n",
		   fsp_str_dbg(fsp), (double)offset));

	if (sio == NULL) {
		return SMB_VFS_NEXT_FTRUNCATE(handle, fsp, offset);
	}

	if (!qnap_macea_recheck(sio)) {
		return -1;
	}

	/* Create an smb_filename with stream_name == NULL. */
	smb_fname_base = synthetic_smb_fname(talloc_tos(),
					sio->base,
					NULL,
					NULL,
					fsp->fsp_name->flags);
	if (smb_fname_base == NULL) {
		errno = ENOMEM;
		return -1;
	}

	status = get_ea_value(talloc_tos(), handle->conn, NULL,
			      smb_fname_base, sio->xattr_name, &ea);
	if (!NT_STATUS_IS_OK(status)) {
		return -1;
	}

	tmp = talloc_realloc(talloc_tos(), ea.value.data, uint8_t,
				   offset + 1);

	if (tmp == NULL) {
		TALLOC_FREE(ea.value.data);
		errno = ENOMEM;
		return -1;
	}

	/* Did we expand ? */
	DEBUG(10, ("qnap_macea_ftruncate: ea.value.length=%d\n", ea.value.length));
	if (ea.value.length < offset + 1) {
		memset(&tmp[ea.value.length], '\0',
			offset + 1 - ea.value.length);
	}

	ea.value.data = tmp;
	ea.value.length = offset + 1;
	ea.value.data[offset] = 0;

	ret = SMB_VFS_SETXATTR(fsp->conn,
			       fsp->fsp_name,
			       sio->xattr_name,
			       ea.value.data, ea.value.length, 0);
	TALLOC_FREE(ea.value.data);

	if (ret == -1) {
		return -1;
	}

	return 0;
}

static int qnap_macea_fallocate(struct vfs_handle_struct *handle,
					struct files_struct *fsp,
					uint32_t mode,
					off_t offset,
					off_t len)
{
        struct stream_io *sio =
		(struct stream_io *)VFS_FETCH_FSP_EXTENSION(handle, fsp);

	DEBUG(10, ("qnap_macea_fallocate called for file %s offset %.0f"
		"len = %.0f\n",
		fsp_str_dbg(fsp), (double)offset, (double)len));

	if (sio == NULL) {
		return SMB_VFS_NEXT_FALLOCATE(handle, fsp, mode, offset, len);
	}

	if (!qnap_macea_recheck(sio)) {
		return -1;
	}

	/* Let the pwrite code path handle it. */
	errno = ENOSYS;
	return -1;
}

static struct vfs_fn_pointers vfs_qnap_macea_fns = {
	.open_fn = qnap_macea_open,
	.stat_fn = qnap_macea_stat,
	.fstat_fn = qnap_macea_fstat,
	.lstat_fn = qnap_macea_lstat,
	.pread_fn = qnap_macea_pread,
	.pwrite_fn = qnap_macea_pwrite,
	.unlink_fn = qnap_macea_unlink,
	.streaminfo_fn = qnap_macea_streaminfo,
	.ftruncate_fn = qnap_macea_ftruncate,
	.fallocate_fn = qnap_macea_fallocate,
};

NTSTATUS vfs_qnap_macea_init(TALLOC_CTX *);
NTSTATUS vfs_qnap_macea_init(TALLOC_CTX *ctx)
{
	return smb_register_vfs(SMB_VFS_INTERFACE_VERSION, "qnap_macea",
				&vfs_qnap_macea_fns);
}
