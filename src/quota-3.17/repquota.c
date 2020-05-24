/*
 *
 *	Utility for reporting quotas
 *
 *	Based on old repquota.
 *	Jan Kara <jack@suse.cz> - Sponsored by SuSE CZ
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <getopt.h>

#include "pot.h"
#include "common.h"
#include "quotasys.h"
#include "quotaio.h"

#define PRINTNAMELEN 9	/* Number of characters to be reserved for name on screen */
#define MAX_CACHE_DQUOTS 1024	/* Number of dquots in cache */

#define FL_USER 1
#define FL_GROUP 2
#define FL_VERBOSE 4
#define FL_ALL 8		/* Dump quota files on all filesystems */
#define FL_TRUNCNAMES 16	/* Truncate names to fit into the screen */
#define FL_SHORTNUMS 32	/* Try to print space in appropriate units */
#define FL_NONAME 64	/* Don't translate ids to names */
#define FL_NOCACHE 128	/* Don't cache dquots before resolving */
#define FL_NOAUTOFS 256	/* Ignore autofs mountpoints */
#define FL_RAWGRACE 512	/* Print grace times in seconds since epoch */
#ifdef QNAPNAS
#define FL_QNAP 8192 /* Print XML for QNAP NAS */
//#define QNAP_MIN_DOMAIN_UID 30001		/* this value defined in /etc/smb.conf "idmap uid" */
#define QNAP_MIN_DOMAIN_UID 0		/* No limited local users won't be shown */
#define QNAP_MAX_DOMAIN_USER 200000	/* the max domain users QNAP want to support */

#define APACHE_USER     "httpdusr"

qsize_t g_volume_free = 0;	// volume free space in MB
char *g_domain_user_printed = NULL;		/* record which domain user is printed out. index means 'uid-QNAP_MIN_DOMAIN_UID' */
extern int qflag;
#endif

int flags, fmt = -1;
char **mnt;
int mntcnt;
int cached_dquots;
struct dquot dquot_cache[MAX_CACHE_DQUOTS];
char *progname;

static void usage(void)
{
	errstr(_("Utility for reporting quotas.\nUsage:\n%s [-vugsi] [-c|C] [-t|n] [-F quotaformat] (-a | mntpoint)\n\n\
-v, --verbose               display also users/groups without any usage\n\
-u, --user                  display information about users\n\
-g, --group                 display information about groups\n\
-s, --human-readable        show numbers in human friendly units (MB, GB, ...)\n\
-t, --truncate-names        truncate names to 8 characters\n\
-p, --raw-grace             print grace time in seconds since epoch\n\
-n, --no-names              do not translate uid/gid to name\n\
-i, --no-autofs             avoid autofs mountpoints\n\
-c, --batch-translation     translate big number of ids at once\n\
-C, --no-batch-translation  translate ids one by one\n\
-F, --format=formatname     report information for specific format\n\
-h, --help                  display this help message and exit\n\
-V, --version               display version information and exit\n\n"), progname);
#ifdef QNAPNAS
	errstr(_("-Q, --qnap=volume_free_size_MB	print out XML for QNAP NAS\n\n"));
#endif
	fprintf(stderr, _("Bugs to %s\n"), MY_EMAIL);
	exit(1);
}

static void parse_options(int argcnt, char **argstr)
{
	int ret;
	int cache_specified = 0;
	struct option long_opts[] = {
		{ "version", 0, NULL, 'V' },
		{ "all", 0, NULL, 'a' },
		{ "verbose", 0, NULL, 'v' },
		{ "user", 0, NULL, 'u' },
		{ "group", 0, NULL, 'g' },
		{ "help", 0, NULL, 'h' },
		{ "truncate-names", 0, NULL, 't' },
		{ "raw-grace", 0, NULL, 'p' },
		{ "human-readable", 0, NULL, 's' },
		{ "no-names", 0, NULL, 'n' },
		{ "cache", 0, NULL, 'c' },
		{ "no-cache", 0, NULL, 'C' },
		{ "no-autofs", 0, NULL, 'i' },
		{ "format", 1, NULL, 'F' },
#ifdef QNAPNAS
		{ "qnap", 1, NULL, 'Q' },
#endif
		{ NULL, 0, NULL, 0 }
	};

#ifdef QNAPNAS
	while ((ret = getopt_long(argcnt, argstr, "VavughtspncCiF:Q:", long_opts, NULL)) != -1) {
#else
	while ((ret = getopt_long(argcnt, argstr, "VavughtspncCiF:", long_opts, NULL)) != -1) {
#endif
		switch (ret) {
#ifdef QNAPNAS
			case 'Q':
				flags |= FL_QNAP;
				g_volume_free = atoll(optarg);
				qflag = 1;
				break;
#endif
			case '?':
			case 'h':
				usage();
			case 'V':
				version();
				exit(0);
			case 'u':
				flags |= FL_USER;
				break;
			case 'g':
				flags |= FL_GROUP;
				break;
			case 'v':
				flags |= FL_VERBOSE;
				break;
			case 'a':
				flags |= FL_ALL;
				break;
			case 't':
				flags |= FL_TRUNCNAMES;
				break;
			case 'p':
				flags |= FL_RAWGRACE;
				break;
			case 's':
				flags |= FL_SHORTNUMS;
				break;
			case 'C':
				flags |= FL_NOCACHE;
				cache_specified = 1;
				break;
			case 'c':
				cache_specified = 1;
				break;
			case 'i':
				flags |= FL_NOAUTOFS;
				break;
			case 'F':
				if ((fmt = name2fmt(optarg)) == QF_ERROR)
					exit(1);
				break;
			case 'n':
				flags |= FL_NONAME;
				break;

		}
	}

	if ((flags & FL_ALL && optind != argcnt) || (!(flags & FL_ALL) && optind == argcnt)) {
		fputs(_("Bad number of arguments.\n"), stderr);
		usage();
	}
	if (fmt == QF_RPC) {
		fputs(_("Repquota cannot report through RPC calls.\n"), stderr);
		exit(1);
	}
	if (flags & FL_NONAME && flags & FL_TRUNCNAMES) {
		fputs(_("Specified both -n and -t but only one of them can be used.\n"), stderr);
		exit(1);
	}
	if (!(flags & (FL_USER | FL_GROUP)))
		flags |= FL_USER;
	if (!(flags & FL_ALL)) {
		mnt = argstr + optind;
		mntcnt = argcnt - optind;
	}
	if (!cache_specified && !(flags & FL_NONAME) && passwd_handling() == PASSWD_DB)
		flags |= FL_NOCACHE;
}

/* Are we over soft or hard limit? */
static char overlim(qsize_t usage, qsize_t softlim, qsize_t hardlim)
{
	if ((usage > softlim && softlim) || (usage > hardlim && hardlim))
		return '+';
	return '-';
}

#ifdef QNAPNAS
#define SZ_USRQUOTACONF_PATH "/etc/config/usr_quota.conf"
qsize_t global_quota_size = 0;
static qsize_t getQnapQuotaSize(void)
{
	FILE *f;
	char buf[1024], *value;
	qsize_t quota_size = 0;

	if (!(f = fopen(SZ_USRQUOTACONF_PATH, "r")))
		return quota_size;
	while (fgets(buf, sizeof(buf), f)) {
		if (strncasecmp(buf, "User Size =", 11) == 0) {
			value = buf+11;
			quota_size = atoll(value);
			break;
		}
	}
	fclose(f);
	return quota_size;
}

static qsize_t getQnapUserQuotaSize(char *username)
{
	FILE *f;
	char buf[1024], *value, session_name[256];
	int is_find = 0, session_len = 0;
	qsize_t quota_size = global_quota_size;

	if (!(f = fopen(SZ_USRQUOTACONF_PATH, "r")))
		return quota_size;
	snprintf(session_name, sizeof(session_name), "[%s]", username);
	session_len = strlen(session_name);
	if(session_len <= 0)
		return quota_size;
	while (fgets(buf, sizeof(buf), f)) {
		if(is_find){
			if(buf[0] == '['){
				//user end
				break;
			}
			if (strncasecmp(buf, "Size =", 6) == 0) {
				value = buf+6;
				quota_size = atoll(value);
			}
		}
		if (strncmp(buf, session_name, session_len) == 0) {
			is_find = 1;
		}
	}
	fclose(f);
	return quota_size;
}

int printCount = 0;
/*FORMAT: status1/status2/vt_size/vf_size/user_name */
static void printLine(struct dquot *dquot, char *name)
{
	struct util_dqblk *entry = &dquot->dq_dqb;
	int no_limit = 0;
	qsize_t used_MB = entry->dqb_curspace/1024/1024;
//	qsize_t limit_MB = entry->dqb_bsoftlimit/1024;
	double used_dMB = ((double)entry->dqb_curspace)/1024/1024;
//	double limit_dMB = ((double)entry->dqb_bsoftlimit)/1024;
	qsize_t limit_MB = getQnapUserQuotaSize(name)/1024;
	double limit_dMB = (double)limit_MB;

//	if (printCount > 10000) exit(0) ;
//	printCount ++;
	if (dquot->dq_id == 0) {	// for the users whose uid == 0 (like administrator)
		printf("IEI_NAS_QUOTA11//");	//no limit for status 1/2
		no_limit = 1;
	}
	else {	// Not administrator
		if (limit_MB == 0) {
			//no limit
			printf("IEI_NAS_QUOTA11//");	//no limit for status 1/2
			no_limit = 1;
		}
		else if (limit_MB > used_MB){
			qsize_t avai_size_MB = limit_MB - used_MB;
			printf("IEI_NAS_QUOTA9/");	//"Available"
			if (g_volume_free >= avai_size_MB)
				printf("%.2f MB/", limit_dMB-used_dMB); //"Available"
			else
				printf("%.2f MB/", (double)g_volume_free); //"Available"
		}
		else {
			printf("IEI_NAS_QUOTA10//");	//"Quota exceeded"
		}
	}
	if (no_limit)
		printf("--/");	//quota size in MB
	else
		printf("%.2f MB/", limit_dMB);	//quota size in MB
	printf("%.2f MB/", used_dMB);	//used size in MB
	printf("%s\n", name);	//user_name
}

static void printXML(struct dquot *dquot, char *name)
{
	struct util_dqblk *entry = &dquot->dq_dqb;
	int no_limit = 0;
	qsize_t used_MB = entry->dqb_curspace/1024/1024;
	qsize_t limit_MB = entry->dqb_bsoftlimit/1024;
	double used_dMB = ((double)entry->dqb_curspace)/1024/1024;
	double limit_dMB = ((double)entry->dqb_bsoftlimit)/1024;

//	if (printCount > 1000) exit(0) ;
//	printCount ++;
	printf("<QuotaList>");
	if (dquot->dq_id == 0) {	// for the users whose uid == 0 (like administrator)
		printf("<status1><![CDATA[IEI_NAS_QUOTA11]]></status1>\n");	//no limit
		no_limit = 1;
	}
	else {	// Not administrator
		if (limit_MB == 0) {
			//no limit
			printf("<status1><![CDATA[IEI_NAS_QUOTA11]]></status1>\n");	//no limit
			no_limit = 1;
		}
		else if (limit_MB > used_MB){
			qsize_t avai_size_MB = limit_MB - used_MB;
			if (g_volume_free >= avai_size_MB)
				printf("<status2><![CDATA[%.2f MB]]></status2>\n", limit_dMB-used_dMB); //"Available"
			else
				printf("<status2><![CDATA[%.2f MB]]></status2>\n", (double)g_volume_free); //"Available"
			printf("<status1><![CDATA[IEI_NAS_QUOTA9]]></status1>\n");	//"Available"
		}
		else {
			printf("<status1><![CDATA[IEI_NAS_QUOTA10]]></status1>\n");	//"Quota exceeded"
		}
	}
	printf("<user_name><![CDATA[%s]]></user_name>\n", name);	//user_name
	if (no_limit)
		printf("<vt_size><![CDATA[--]]></vt_size>\n");	//quota size in MB
	else
		printf("<vt_size><![CDATA[%.2f MB]]></vt_size>\n", ((double)entry->dqb_bsoftlimit)/1024);	//quota size in MB
	printf("<vf_size><![CDATA[%.2f MB]]></vf_size>\n", ((double)entry->dqb_curspace)/1024/1024);	//used size in MB
	printf("</QuotaList>\n");
}
#endif
/* Print one quota entry */
static void print(struct dquot *dquot, char *name)
{
	char pname[MAXNAMELEN];
	char time[MAXTIMELEN];
	char numbuf[3][MAXNUMLEN];
	
	struct util_dqblk *entry = &dquot->dq_dqb;

	if (!entry->dqb_curspace && !entry->dqb_curinodes && !(flags & FL_VERBOSE))
		return;
#ifdef QNAPNAS
	if (flags & FL_QNAP) {
		printLine(dquot, name);
		return ;
	}
#endif
	sstrncpy(pname, name, sizeof(pname));
	if (flags & FL_TRUNCNAMES)
		pname[PRINTNAMELEN] = 0;
	if (entry->dqb_bsoftlimit && toqb(entry->dqb_curspace) >= entry->dqb_bsoftlimit)
		if (flags & FL_RAWGRACE)
			sprintf(time, "%llu", (unsigned long long)entry->dqb_btime);
		else
			difftime2str(entry->dqb_btime, time);
	else
		if (flags & FL_RAWGRACE)
			strcpy(time, "0");
		else
			time[0] = 0;
	space2str(toqb(entry->dqb_curspace), numbuf[0], flags & FL_SHORTNUMS);
	space2str(entry->dqb_bsoftlimit, numbuf[1], flags & FL_SHORTNUMS);
	space2str(entry->dqb_bhardlimit, numbuf[2], flags & FL_SHORTNUMS);
	printf("%-*s %c%c %7s %7s %7s %6s", PRINTNAMELEN, pname,
	       overlim(qb2kb(toqb(entry->dqb_curspace)), qb2kb(entry->dqb_bsoftlimit), qb2kb(entry->dqb_bhardlimit)),
	       overlim(entry->dqb_curinodes, entry->dqb_isoftlimit, entry->dqb_ihardlimit),
	       numbuf[0], numbuf[1], numbuf[2], time);
	if (entry->dqb_isoftlimit && entry->dqb_curinodes >= entry->dqb_isoftlimit)
		if (flags & FL_RAWGRACE)
			sprintf(time, "%llu", (unsigned long long)entry->dqb_itime);
		else
			difftime2str(entry->dqb_itime, time);
	else
		if (flags & FL_RAWGRACE)
			strcpy(time, "0");
		else
			time[0] = 0;
	number2str(entry->dqb_curinodes, numbuf[0], flags & FL_SHORTNUMS);
	number2str(entry->dqb_isoftlimit, numbuf[1], flags & FL_SHORTNUMS);
	number2str(entry->dqb_ihardlimit, numbuf[2], flags & FL_SHORTNUMS);
	printf(" %7s %5s %5s %6s\n", numbuf[0], numbuf[1], numbuf[2], time);
}

/* Print all dquots in the cache */
static void dump_cached_dquots(int type)
{
	int i;
	char namebuf[MAXNAMELEN];

	if (!cached_dquots)
		return;
	if (type == USRQUOTA) {
		struct passwd *pwent;

		setpwent();
		while ((pwent = getpwent())) {
			for (i = 0; i < cached_dquots && pwent->pw_uid != dquot_cache[i].dq_id; i++);
			if (i < cached_dquots && !(dquot_cache[i].dq_flags & DQ_PRINTED)) {
				print(dquot_cache+i, pwent->pw_name);
				dquot_cache[i].dq_flags |= DQ_PRINTED;
#ifdef QNAPNAS	// The joined AD users won't be set quota aotumatically, so we must print out the non-joined domain users later.
				if (pwent->pw_uid >= QNAP_MIN_DOMAIN_UID) {
					unsigned long index = pwent->pw_uid - QNAP_MIN_DOMAIN_UID;
					if (index < QNAP_MAX_DOMAIN_USER)
						g_domain_user_printed[index] = 1;		//Record it is printed, so we won't print it out later.
				}
#endif
			}
#ifdef QNAPNAS
//			else
//				print(dquot_cache+i, pwent->pw_name);
#endif
		}
		endpwent();
	}
	else {
		struct group *grent;

		setgrent();
		while ((grent = getgrent())) {
			for (i = 0; i < cached_dquots && grent->gr_gid != dquot_cache[i].dq_id; i++);
			if (i < cached_dquots && !(dquot_cache[i].dq_flags & DQ_PRINTED)) {
				print(dquot_cache+i, grent->gr_name);
				dquot_cache[i].dq_flags |= DQ_PRINTED;
			}
		}
		endgrent();
	}
#ifndef QNAPNAS		//We don't need print out #UID
	for (i = 0; i < cached_dquots; i++)
		if (!(dquot_cache[i].dq_flags & DQ_PRINTED)) {
			sprintf(namebuf, "#%u", dquot_cache[i].dq_id);
			print(dquot_cache+i, namebuf);
		}
#endif
	cached_dquots = 0;
}

/* Callback routine called by scan_dquots on each dquot */
static int output(struct dquot *dquot, char *name)
{
	if (flags & FL_NONAME) {	/* We should translate names? */
		char namebuf[MAXNAMELEN];

		sprintf(namebuf, "#%u", dquot->dq_id);
		print(dquot, namebuf);
	}
	else if (name || flags & FL_NOCACHE) {	/* We shouldn't do batched id->name translations? */
		char namebuf[MAXNAMELEN];

		if (!name) {
#ifdef QNAPNAS	//We don't need print out #UID because id2name return non-zero means namebuf is "#UID"
			if (id2name(dquot->dq_id, dquot->dq_h->qh_type, namebuf) != 0)
				return 0;
#else
			id2name(dquot->dq_id, dquot->dq_h->qh_type, namebuf);
#endif
			name = namebuf;
		}
		print(dquot, name);
	}
	else {	/* Lets cache the dquot for later printing */
		memcpy(dquot_cache+cached_dquots++, dquot, sizeof(struct dquot));
		if (cached_dquots >= MAX_CACHE_DQUOTS)
			dump_cached_dquots(dquot->dq_h->qh_type);
	}
	return 0;
}

/* Dump information stored in one quota file */
static void report_it(struct quota_handle *h, int type)
{
	char bgbuf[MAXTIMELEN], igbuf[MAXTIMELEN];

#ifdef QNAPNAS
	if (flags & FL_QNAP) goto SKIP_TITLE;
#endif
	printf(_("*** Report for %s quotas on device %s\n"), type2name(type), h->qh_quotadev);
	time2str(h->qh_info.dqi_bgrace, bgbuf, TF_ROUND);
	time2str(h->qh_info.dqi_igrace, igbuf, TF_ROUND);
	printf(_("Block grace time: %s; Inode grace time: %s\n"), bgbuf, igbuf);
	printf(_("                        Block limits                File limits\n"));
	printf(_("%-9s       used    soft    hard  grace    used  soft  hard  grace\n"), (type == USRQUOTA)?_("User"):_("Group"));
	printf("----------------------------------------------------------------------\n");
#ifdef QNAPNAS
SKIP_TITLE:
#endif

	if (h->qh_ops->scan_dquots(h, output) < 0)
		return;
	dump_cached_dquots(type);
	if (h->qh_ops->report) {
		putchar('\n');
		h->qh_ops->report(h, flags & FL_VERBOSE);
		putchar('\n');
	}
}

static void report(int type)
{
	struct quota_handle **handles;
	int i;

	if (flags & FL_ALL)
		handles = create_handle_list(0, NULL, type, fmt, IOI_READONLY | IOI_OPENFILE, MS_LOCALONLY | (flags & FL_NOAUTOFS ? MS_NO_AUTOFS : 0));
	else
		handles = create_handle_list(mntcnt, mnt, type, fmt, IOI_READONLY | IOI_OPENFILE, MS_LOCALONLY | (flags & FL_NOAUTOFS ? MS_NO_AUTOFS : 0));
	for (i = 0; handles[i]; i++)
		report_it(handles[i], type);
	dispose_handle_list(handles);
}

int main(int argc, char **argv)
{
	gettexton();
	progname = basename(argv[0]);

	parse_options(argc, argv);
	init_kernel_interface();
#ifdef QNAPNAS
	if (flags & FL_USER) {
		struct passwd *pwent;
		g_domain_user_printed = malloc(QNAP_MAX_DOMAIN_USER);
		if (!g_domain_user_printed)
			return ENOMEM;
		memset(g_domain_user_printed, 0, QNAP_MAX_DOMAIN_USER);

		global_quota_size = getQnapQuotaSize();
		report(USRQUOTA);

		if (flags & FL_QNAP) {
			qsize_t quota_size = 0;
			double quota_size_MB = 0;
			//Print out the non quota joined domain users.
			setpwent();
			while ((pwent = getpwent())) {
//				printf("<QNAP>%u:%s\n", pwent->pw_uid, pwent->pw_name);
				if (pwent->pw_uid >= QNAP_MIN_DOMAIN_UID) {
					unsigned long index = pwent->pw_uid - QNAP_MIN_DOMAIN_UID;
					if (index < QNAP_MAX_DOMAIN_USER && g_domain_user_printed[index] == 0 && strcmp(pwent->pw_name, APACHE_USER)) {
						quota_size = getQnapUserQuotaSize(pwent->pw_name);
						quota_size_MB = ((double)quota_size)/1024;
						printf("IEI_NAS_QUOTA9/%.2f MB/%.2f MB/0.00 MB/%s\n", quota_size_MB, quota_size_MB, pwent->pw_name);
//						printf("IEI_NAS_QUOTA11//--/0/%s\n", pwent->pw_name);
					}
				}
			}
			endpwent();
		}
		free(g_domain_user_printed);
	}
#else
	if (flags & FL_USER)
		report(USRQUOTA);
#endif

	if (flags & FL_GROUP)
		report(GRPQUOTA);

	return 0;
}
