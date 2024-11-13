/**
 * @file    src/error.c
 * NetSrv: Error handling.
 * @author  Christoph Schulz
 * @since   2004-11-08
 * @version $Id$
 */

#include "netsrv/error.h"
#include <stdio.h>

int
netsrv_scerror (const char *message, int errorcode)
{
	perror (message);
	return errorcode;
}

int
netsrv_error (const char *message, int errorcode)
{
	fprintf (stderr, "%s\n", message);
	return errorcode;
}
