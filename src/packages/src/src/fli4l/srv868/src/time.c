/**
 * @file    time.c
 * RFC 868 time service.
 * @author  Christoph Schulz
 * @since   2004-11-08
 * @version $Id: time.c 233 2009-01-18 22:02:34Z kristov $
 */

#include "srv868.h"
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netsrv/net.h>

/**
 * The number of seconds between 1900-01-01 00:00 and 1970-01-01 00:00.
 */
static const u_int32_t c_seconds_before_epoch = 2208988800u;

void
time_handle_rfc868_request (void *conn)
{
	/* time() returns the current time as the number of seconds since
	 * 1970-01-01 00:00; take that value and add correction value */
	uint32_t seconds = htonl (
		(uint32_t) time ((time_t *) NULL) + c_seconds_before_epoch
	);
	/* send seconds to the peer and return */
	netsrv_net_write (conn, &seconds, sizeof seconds);
}
