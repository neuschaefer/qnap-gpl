/*
 * Example of LUKS password dictionary search
 *
 * Run this as root, e.g. ./luks_dict test.img /usr/share/john/password.lst 4
 *
 * Copyright (C) 2012 Milan Broz <asi@ucw.cz>
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <libcryptsetup.h>

#define MAX_LEN 512

static void check(struct crypt_device *cd, const char *pwd_file, unsigned my_id, unsigned max_id)
{
	FILE *f;
	int len, r = -1;
	unsigned long line = 0;
	char pwd[MAX_LEN];

	if (fork())
		return;

	/* open password file, now in separate process */
	f = fopen(pwd_file, "r");
	if (!f) {
		printf("Cannot open %s.\n", pwd_file);
		exit(EXIT_FAILURE);
	}

	while (fgets(pwd, MAX_LEN, f)) {

		/* every process tries N-th line, skip others */
		if (line++ % max_id != my_id)
			continue;

		len = strlen(pwd);

		/* strip EOL - this is like a input from tty */
		if (len && pwd[len - 1] == '\n') {
			pwd[len - 1] = '\0';
			len--;
		}

		/* lines starting "#!comment" are comments */
		if (len >= 9 && !strncmp(pwd, "#!comment", 9)) {
			/* printf("skipping %s\n", pwd); */
			continue;
		}

		/* printf("%d: checking %s\n", my_id, pwd); */
		r = crypt_activate_by_passphrase(cd, NULL, CRYPT_ANY_SLOT, pwd, len, 0);
		if (r >= 0) {
			printf("Found passphrase for slot %d: \"%s\"\n", r, pwd);
			break;
		}
	}

	fclose(f);
	crypt_free(cd);
	exit(r >= 0 ? 2 : EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	int i, status, procs = 4;
	struct crypt_device *cd;

	if (argc < 3 || argc > 4) {
		printf("Use: %s <LUKS_device|file> <password file> [#processes] %d\n", argv[0], argc);
		exit(EXIT_FAILURE);
	}

	if (argc == 4 && (sscanf(argv[3], "%i", &procs) != 1 || procs < 1)) {
		printf("Wrong number of processes.\n");
		exit(EXIT_FAILURE);
	}

	/* crypt_set_debug_level(CRYPT_DEBUG_ALL); */

	/*
	 * Need to create temporary keyslot device-mapper devices and allocate loop if needed,
	 * so root is requried here.
	 */
	if (getuid() != 0) {
		printf("You must be root to run this program.\n");
                exit(EXIT_FAILURE);
	}

	/* signal all children if anything happens */
	prctl(PR_SET_PDEATHSIG, SIGHUP);
	setpriority(PRIO_PROCESS, 0, -5);

	/* we are not going to modify anything, so common init is ok */
	if (crypt_init(&cd, argv[1]) || crypt_load(cd, CRYPT_LUKS1, NULL)) {
		printf("Cannot open %s.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	/* run scan in separate processes, it is up to scheduler to assign CPUs inteligently */
	for (i = 0; i < procs; i++)
		check(cd, argv[2], i, procs);

	/* wait until at least one finishes with error or status 2 (key found) */
	while (wait(&status) != -1 && WIFEXITED(status)) {
		if (WEXITSTATUS(status) == EXIT_SUCCESS)
			continue;
		/* kill rest of processes */
		kill(0, SIGHUP);
		/* not reached */
		break;
	}
	exit(0);
}
