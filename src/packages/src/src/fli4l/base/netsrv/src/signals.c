/**
 * @file    src/signals.c
 * NetSrv: Signal handling.
 * @author  Christoph Schulz
 * @since   2004-11-08
 * @version $Id$
 */

#include "netsrv/signals.h"
#include "netsrv/error.h"
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * The last signal received. -1 means no signal since program start or the last
 * call to signals_reset().
 */
static volatile sig_atomic_t g_termination_requested = 0;

/**
 * The SIGTERM handler of the application. Sets g_termination_requested to one.
 * @param signum The signal received (not used).
 */
static void
netsrv_sigterm_handler (int signum)
{
	UNUSED (signum);
	g_termination_requested = 1;
}

/**
 * The SIGCHLD handler of the application. Calls waitpid(-1).
 * @param signum The signal received (not used).
 */
static void
netsrv_sigchld_handler (int signum)
{
	UNUSED (signum);
	waitpid (-1, (int *) NULL, WNOHANG);
}

int
netsrv_signals_init (void)
{
	int rc;
	struct sigaction sa;
	EXCEPT_AWARE (int);

	memset (&sa, 0, sizeof sa);
	sa.sa_flags = SA_RESTART; /* we test for EINTR in some calls anyway... */

	/* all the following signals will be ignored */
	sa.sa_handler = SIG_IGN;
	SYSCALL (rc, sigaction (SIGINT, &sa, (struct sigaction *) NULL),
		ERROR_SIGACTION);
	SYSCALL (rc, sigaction (SIGQUIT, &sa, (struct sigaction *) NULL),
		ERROR_SIGACTION);
	SYSCALL (rc, sigaction (SIGHUP, &sa, (struct sigaction *) NULL),
		ERROR_SIGACTION);
	SYSCALL (rc, sigaction (SIGPIPE, &sa, (struct sigaction *) NULL),
		ERROR_SIGACTION);
	SYSCALL (rc, sigaction (SIGALRM, &sa, (struct sigaction *) NULL),
		ERROR_SIGACTION);

	/* the SIGTERM signal sets g_termination_requested to a non-zero value */
	sa.sa_handler = &netsrv_sigterm_handler;
	SYSCALL (rc, sigaction (SIGTERM, &sa, (struct sigaction *) NULL),
		ERROR_SIGACTION);

	/* the SIGCHLD signal calls waitpid(-1) */
	sa.sa_handler = &netsrv_sigchld_handler;
	SYSCALL (rc, sigaction (SIGCHLD, &sa, (struct sigaction *) NULL),
		ERROR_SIGACTION);

	return 0;
}

void
netsrv_signals_done (void)
{
	/* nothing to do */
}

int
netsrv_signals_termination_requested (void)
{
	return g_termination_requested ? 1 : 0;
}
