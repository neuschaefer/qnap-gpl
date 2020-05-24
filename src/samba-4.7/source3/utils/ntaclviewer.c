#include <sys/stat.h>

#include "includes.h"
#include "smbd/smbd.h"
#include "librpc/gen_ndr/xattr.h"
#include "librpc/gen_ndr/ndr_xattr.h"
#include "librpc/gen_ndr/idmap.h"
#include "../libcli/security/security.h"
#include "../lib/crypto/sha256.h"
#include "auth.h"
#include "libwbclient/wbclient.h"

static bool wbinfo_lookupsid_type(const char *sid_str,enum wbcSidType *SidType)
{
	wbcErr wbc_status;
	struct wbcDomainSid sid;
	char *domain;
	char *name;
	enum wbcSidType type;

	/* Send off request */
 	wbc_status = wbcStringToSid(sid_str, &sid);
 	if (!WBC_ERROR_IS_OK(wbc_status)) {
		d_fprintf(stderr, "failed to call wbcStringToSid: %s\n", wbcErrorString(wbc_status));
		return False;
	}

	wbc_status = wbcLookupSid(&sid, &domain, &name, &type);
	if (!WBC_ERROR_IS_OK(wbc_status)) {
		//d_fprintf(stderr, "failed to call wbcLookupSid: %s\n", wbcErrorString(wbc_status));
		return False;
	}

	*SidType = type;

	wbcFreeMemory(domain);
	wbcFreeMemory(name);

	return True;
}

/* 
 * Pull the ntacl xattr from the file
 * @*path: name of file
 * @*pblob
 * return: 0 for success, -1 for fail
 */
static int pull_ntacl_into_blob(char *path, DATA_BLOB *pblob)
{
	ssize_t length = 0;
	void *val = NULL;

	if (path == NULL || pblob == NULL) {
		return -1;
	}

	length = getxattr(path, XATTR_NTACL_NAME, NULL, 0);
	if (length == -1) {
		return -1;
	}

	val = malloc(length);
	if (!val) {
		perror("malloc");
		return -1;
	}

	length = getxattr(path, XATTR_NTACL_NAME, val, length);
	if (length == -1) {
		if (val) {
			free(val);
			val = NULL;
		}
		fprintf(stderr, "Error: getxattr(..., NULL, 0) (%s, %s)\n",
			__FUNCTION__, path);
		return -1;
	}
		
	pblob->data = val;
	pblob->length = length;

	return 0;
}

static void nt_ace_mask_racl(uint32_t nt_mask)
{
	struct mask_flag_struct {
		char		e_char;
		uint32_t	e_mask;
		const char	*e_name;
	};
	struct mask_flag_struct mask_flags[] = {
		{'r', SEC_FILE_READ_DATA, "read_data"},
		{'w', SEC_FILE_WRITE_DATA, "write_data"},
		{'p', SEC_FILE_APPEND_DATA, "append_data"},
		{'x', SEC_FILE_EXECUTE, "execute"},
		/* DELETE_CHILD is only meaningful for directories but it might also
		   be set in an ACE of a file, so print it in file context as well.  */
		{'d', SEC_DIR_DELETE_CHILD, "delete_child"},
		{'D', SEC_STD_DELETE, "delete"},
		{'a', SEC_FILE_READ_ATTRIBUTE, "read_attributes"},
		{'A', SEC_FILE_WRITE_ATTRIBUTE, "write_attributes"},
		{'R', SEC_FILE_READ_EA, "read_xattr"},
		{'W', SEC_FILE_WRITE_EA, "write_xattr"},
		{'c', SEC_STD_READ_CONTROL, "read_acl"},
		{'C', SEC_STD_WRITE_DAC, "write_acl"},
		{'o', SEC_STD_WRITE_OWNER, "write_owner"},
		{'S', SEC_STD_SYNCHRONIZE, "synchronize"},
	};

	int i = 0;

	for (; i < ARRAY_SIZE(mask_flags); i++) {
		if (nt_mask & mask_flags[i].e_mask) {
			printf("%c", mask_flags[i].e_char);
		} else {
			printf("-");
		}
	}
}

static void nt_ace_flags_racl(uint8_t nt_flags, bool is_group)
{
	#define SEC_ACE_FLAG_IDENTIFIER_GROUP ( 0x20 )
	
	static struct {
		char		e_char;
		uint8_t		e_flag;
		const char	*e_name;
	} ace_flag_bits[] = {
		{'f', SEC_ACE_FLAG_OBJECT_INHERIT, "file_inherit"},
		{'d', SEC_ACE_FLAG_CONTAINER_INHERIT, "dir_inherit"},
		{'n', SEC_ACE_FLAG_NO_PROPAGATE_INHERIT, "no_propagate"},
		{'i', SEC_ACE_FLAG_INHERIT_ONLY, "inherit_only"},
		{'g', SEC_ACE_FLAG_IDENTIFIER_GROUP, "identifier_group"},
		{'a', SEC_ACE_FLAG_INHERITED_ACE, "inherited"},
	};
	int i = 0;

	if (is_group) {
		nt_flags |= SEC_ACE_FLAG_IDENTIFIER_GROUP;
	}

	for (; i < ARRAY_SIZE(ace_flag_bits); i++) {
		if (nt_flags & ace_flag_bits[i].e_flag) {
			printf("%c", ace_flag_bits[i].e_char);
		} else {
			printf("-");
		}
	}
}

static void print_ace(struct security_ace *ace)
{
	struct unixid unixid;
	bool is_group = False;

	/* name */
	if (dom_sid_equal(&ace->trustee, &global_sid_World)) {
		printf("everyone@:");
	} else {
		if (!sids_to_unixids(&ace->trustee, 1, &unixid)) {
			return;
		}

		if (unixid.type == ID_TYPE_BOTH) {
			enum wbcSidType SidType;
			if (!wbinfo_lookupsid_type(sid_string_dbg(&ace->trustee), &SidType)) {
				return;
			}

			if (SidType != WBC_SID_NAME_USER) {
				is_group = True;
			}
		} else if (unixid.type == ID_TYPE_UID) {
			is_group = False;
		} else if (unixid.type == ID_TYPE_GID) {
			is_group = True;
		} else {
			return;
		}

		if (is_group == False) {
			struct passwd *passwd = getpwuid(unixid.id);
			if (passwd) {
				printf("%s:", passwd->pw_name);
			} else {
				printf("%d:", unixid.id);
			}
		} else {
			struct group *group = getgrgid(unixid.id);
			if (group) {
				printf("%s:", group->gr_name);
			} else {
				printf("%d:", unixid.id);
			}
		}
	}

	/* mask */
	nt_ace_mask_racl(ace->access_mask);
	printf(":");

	/* flag*/
	nt_ace_flags_racl(ace->flags, is_group);
	printf(":");

	/* type */	
	if (ace->type == SEC_ACE_TYPE_ACCESS_ALLOWED) {
		printf("allow");
	} else if (ace->type == SEC_ACE_TYPE_ACCESS_DENIED) {
		printf("deny");
	} else {
		printf("%d", ace->type);
	}
}

static int qnap_ntacl(char *filename)
{
	DATA_BLOB pblob; /* store the ntacl */
	struct xattr_NTACL xacl;
	enum ndr_err_code ndr_err;
	TALLOC_CTX *frame = talloc_stackframe();
	struct security_descriptor *sd = NULL;
	int i;

	pblob.data = NULL;
	pblob.length = 0;

	if (!pull_ntacl_into_blob(filename, &pblob)) {
		if (pblob.length > 0) {
			ndr_err = ndr_pull_struct_blob(&pblob, frame, &xacl,
				(ndr_pull_flags_fn_t)ndr_pull_xattr_NTACL);

			if (!NDR_ERR_CODE_IS_SUCCESS(ndr_err)) {
				fprintf(stderr, "Error: failed to do ndr_pull_struct_blob\n");
				goto clean;
			}

			switch (xacl.version) {
				case 1:
					sd = xacl.info.sd;
					break;
				case 2:
					sd = xacl.info.sd_hs2->sd;
					break;
				case 3:
					sd = xacl.info.sd_hs3->sd;
					break;
				case 4:
					sd = xacl.info.sd_hs4->sd;
					break;
				default:
					break;
			}

			if (sd == NULL) {
				fprintf(stderr, "Error: xacl.version (%d)\n", xacl.version);
				goto clean;
			}

			/* Print aces */
			for (i = 0; sd->dacl && i < sd->dacl->num_aces; i++) {
				struct security_ace *ace = &sd->dacl->aces[i];
				print_ace(ace);
				printf("\n");
			}
		}
	}

clean:
	TALLOC_FREE(frame);
	if (pblob.data) {
		free(pblob.data);
		pblob.data = NULL;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	static const char *backend;
	char *filename = NULL;
	struct stat st;

	if (!lp_load_global(get_dyn_CONFIGFILE())) {
		fprintf(stderr, "Error: lp_load_global()\n");
		return -1;
	}

	if (!init_names()) {
		fprintf(stderr, "Error: init_names()\n");
		return -1;
	}

	backend = lp_passdb_backend();

	if (!backend) {
		fprintf(stderr, "Error: lp_passdb_backend()\n");
		return -1;
	}

	if (!initialize_password_db(False, NULL)) {
		fprintf(stderr, "Error: initialize_password_db()\n");
		return -1;
	}
	
	if (argc == 2) {
		filename = argv[1];
		if (stat(filename, &st) != 0) {
			printf("Failed to stat %s\n", filename);
			return 0;
		}
		qnap_ntacl(filename);
	}

	return 0;
}

