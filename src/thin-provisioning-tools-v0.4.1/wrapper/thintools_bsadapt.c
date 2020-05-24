#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

/**
 * Vaild commands supported by this wrapper program.
 */
char _commands[][20] = {"thin_dump", "thin_check", "thin_repair"};
static const int _num_commands = sizeof(_commands) / sizeof(_commands[0]);

#define MAX_ARG_COUNT 30

/**
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
 *
 * Original from LVM2.2.02.106.
 *
 * Create verbose string with list of parameters
 */
static char *_verbose_args(const char *const argv[], char *buf, size_t sz)
{
	int pos = 0;
	int len;
	unsigned i;

	buf[0] = '\0';
	for (i = 0; argv[i]; i++) {
		if ((len = snprintf(buf + pos, sz - pos,
				       " %s", argv[i])) < 0)
			/* Truncated */
			break;
		pos += len;
	}

	return buf;
}

/**
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
 *
 * Original from LVM2.2.02.106.
 *
 * Execute and wait for external command
 * Returns 1 if the subprocess executed successfully (errno==0), 0 if failed.
 */
static int exec_cmd(const char *const argv[], int *rstatus)
{
	pid_t pid;
	int status;
	char buf[PATH_MAX * 2];

	if (!argv[0]) {
		fprintf(stderr, "Missing command.\n");
		return 0;
	}

	if (rstatus)
		*rstatus = -1;

	_verbose_args(argv, buf, sizeof(buf));

	if ((pid = fork()) == -1) {
		return 0;
	}

	if (!pid) {
		/* Child */
		execvp(argv[0], (char **) argv);
		fprintf(stderr, "execvp %s\n", argv[0]);
		_exit(errno);
	}

	/* Parent */
	if (wait4(pid, &status, 0, NULL) != pid) {
		fprintf(stderr, "wait4 child process %u failed: %s\n", pid,
			  strerror(errno));
		return 0;
	}

	if (!WIFEXITED(status)) {
		fprintf(stderr, "Child %u exited abnormally\n", pid);
		return 0;
	}

	if (WEXITSTATUS(status)) {
		if (rstatus) {
			*rstatus = WEXITSTATUS(status);
			fprintf(stderr, "%s failed: %u\n", argv[0], *rstatus);
		} else
			fprintf(stderr, "%s failed: %u\n", argv[0], WEXITSTATUS(status));
		return 0;
	}

	if (rstatus)
		*rstatus = 0;

	return 1;
}

/**
 * Copyright (C) 2007 Red Hat, Inc.
 *
 * Return the address of the last file name component of NAME.
 * If NAME ends in a slash, return the empty string.
 *
 * Original from LVM2.2.02.106.
 */
static inline const char *last_path_component(char const *name)
{
	char const *slash = strrchr(name, '/');

	return (slash) ? slash + 1 : name;
}

static char* find_command(const char *name)
{
	int i;
	const char *base;

	base = last_path_component(name);

	for (i = 0; i < _num_commands; i++) {
		if (!strcmp(base, _commands[i]))
			break;
	}

	if (i >= _num_commands)
		return NULL;

	return _commands[i];
}

/* Argument handling method is similar to lvm2_main() */
int main(int argc, char* argv[])
{
	int ret = 0;
	int rstatus = 0;
	const char* cmd = NULL;
	const char* adaptee_argv[MAX_ARG_COUNT] = {0};
	int i = 0;

	// Skip the first argument if the program is invoked directly, not invoked through a symbolic link.
	// i.e., # thintools_bsadapt thin_check <arg1> <arg2> ...
	if (!strcmp(last_path_component(argv[0]), "thintools_bsadapt"))
	{
		if (argc < 2)
		{
			fprintf(stderr, "Usage: thintools_bsadapt <command> <arg1> <arg2> ...\n");
			ret = EINVAL;
			goto MAIN_END;
		}
		--argc;
		++argv;
	}

	if (argc >= MAX_ARG_COUNT)
	{
		fprintf(stderr, "Too many arguments\n");
		ret = E2BIG;
		goto MAIN_END;
	}

	cmd = find_command(argv[0]);
	if (!cmd)
	{
		fprintf(stderr, "Unknown command\n");
		ret = ENOENT;
		goto MAIN_END;
	}

	/* Try to execute the version of blocksize=8192 */
	adaptee_argv[0] = "/sbin/pdata_tools_8192";
	adaptee_argv[1] = cmd;
	for (i = 1; i < argc; ++i)
		adaptee_argv[i + 1] = argv[i];
	adaptee_argv[i + 1] = NULL;
	ret = exec_cmd(adaptee_argv, &rstatus);
	if (!ret || rstatus != 0)
	{
		fprintf(stderr, "%s with block_size = 8192 failed\n", cmd);
		ret = rstatus;
	}
	else
	{
		ret = rstatus;
		goto MAIN_END;
	}

	/* Try to execute the version of blocksize=4096 */
	adaptee_argv[0] = "/sbin/pdata_tools_4096";
	adaptee_argv[1] = cmd;
	for (i = 1; i < argc; ++i)
		adaptee_argv[i + 1] = argv[i];
	adaptee_argv[i + 1] = NULL;
	ret = exec_cmd(adaptee_argv, &rstatus);
	if (!ret || rstatus != 0)
	{
		fprintf(stderr, "%s with block_size = 4096 failed\n", cmd);
		ret = rstatus;
	}
	else
	{
		ret = rstatus;
		goto MAIN_END;
	}

MAIN_END:
	return ret;
}


