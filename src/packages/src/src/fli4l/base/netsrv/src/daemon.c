/**
 * @file    src/daemon.c
 * NetSrv: Daemonizing.
 * @author  Christoph Schulz
 * @since   2004-11-08
 * @version $Id$
 */

#include "netsrv/daemon.h"
#include "netsrv/error.h"
#include <stdlib.h>
#include <unistd.h>

int
netsrv_daemonize (void)
{
	pid_t pid; /* for system calls related to process management */
	int rc;    /* for other system calls */
	EXCEPT_AWARE (int);

	/* detach from console to never be a process group leader */
	SYSCALL (pid, fork (), ERROR_FORK1);
	if (pid != 0) exit (0);

	/* become a session group leader without associated tty */
	SYSCALL (pid, setsid (), ERROR_SETSID);

	/* become a non-session group leader for never regaining a tty */
	SYSCALL (pid, fork (), ERROR_FORK2);
	if (pid != 0) exit (0);

	/* change to root directory */
	SYSCALL (rc, chdir ("/"), ERROR_CHDIR);

	return 0;
}
