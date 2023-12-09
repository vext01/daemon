#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int
main(int argc, char **argv)
{
	if (argc == 1) {
		fprintf(stderr, "usage: daemon <program> [args...]\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * As explained below, we can't report a failure of execvp(3). We can at
	 * least check a few things about the path the user has asked to invoke.
	 *
	 * Of course these checks are all racy, but that's all we've got.
	 */
	if (argv[1][0] != '/')
		errx(EXIT_FAILURE, "path must be absolute");

	struct stat st;
	if (stat(argv[1], &st) == -1)
		err(EXIT_FAILURE, "stat");

	if (!S_ISREG(st.st_mode))
		errx(EXIT_FAILURE, "not a regular file");

	if (access(argv[1], X_OK) == -1)
		errx(EXIT_FAILURE, "no permission to execute");

	/*
	 * Looks like there's a good chance this will work. Daemonise!
	 *
	 * XXX: make daemon(3) params a command-line option.
	 */
	if (daemon(0, 1) == -1)
		err(EXIT_FAILURE, "daemon");

	execvp(argv[1], &argv[1]);

	/*
	 * If we get here, execvp(3) failed.
	 *
	 * Although we exit with error status, there will be no one to witness the
	 * failure. daemon(3) forked, we are the child, and the parent exited
	 * already.
	 */
	exit(EXIT_FAILURE);
}
