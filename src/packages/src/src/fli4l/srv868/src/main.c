/**
 * @file    main.c
 * Main program. Initializes all other modules.
 * @author  Christoph Schulz
 * @since   2004-11-08
 * @version $Id: main.c 245 2009-03-21 16:20:02Z kristov $
 */

#include "srv868.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>      /* for SIGTERM */
#include <netinet/in.h>  /* for IPPORT_TIMESERVER */
#include <netsrv/daemon.h>
#include <netsrv/signals.h>
#include <netsrv/net.h>

/**
 * Version number.
 */
#define VERSION "1.1"

/**
 * Interprets the command line.
 * @param argc
 *   The number of arguments on the command line.
 * @param argv
 *   The command line arguments.
 */
static int
parse_options (int argc, char *argv[])
{
	if (argc == 1)
	{
		return 0;
	}

	if (argv [1][0] == '-')
	{
		if (strcmp (argv [1], "--version") == 0)
		{
			fprintf (stderr, "%s\n", VERSION);
			return 2;
		}
	}

	fprintf (stderr, "usage: srv868 [--help|--version]\n");
	return 1;
}

/**
 * Marks an initialization phase.
 * @param exp The expression to evaluate. If it fails, the macro jumps to the
 *   label 'exit'. Otherwise, a local variable 'phase' is incremented.
 */
#define PHASE_INIT(exp) \
	rc = (exp); if (rc) goto exit; else ++phase
/**
 * Marks a finalization phase.
 * @param p If p <= phase, the expression exp is evaluated.
 * @param exp The expression to evaluate. Normally, this constitutes the logical
 *   inverse to the expression in the corresponding initialization phase.
 */
#define PHASE_DONE(p, exp) \
	if (p <= phase) (exp)

/**
 * Main application.
 * @return Zero if successful, an error code otherwise.
 */
int
main (int argc, char *argv[])
{
	int rc;
	int phase = 0;

	/* parse command line */
	if ((rc = parse_options (argc, argv)) != 0)
	{
		return rc;
	}

	PHASE_INIT (netsrv_daemonize ());
	PHASE_INIT (netsrv_signals_init ());
	PHASE_INIT (netsrv_net_init ());
	PHASE_INIT (netsrv_net_add_service (IPPORT_TIMESERVER, 0, time_handle_rfc868_request));
	PHASE_INIT (netsrv_net_add_service (IPPORT_TIMESERVER, 1, time_handle_rfc868_request));

	rc = netsrv_net_run ();

exit:
	PHASE_DONE (3, netsrv_net_done ());
	PHASE_DONE (2, netsrv_signals_done ());

	return rc;
}
