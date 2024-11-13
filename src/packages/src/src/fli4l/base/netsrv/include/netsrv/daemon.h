/**
 * @file    include/netsrv/daemon.h
 * NetSrv: Daemonizing.
 * @author  Christoph Schulz
 * @since   2004-11-08
 * @version $Id$
 */

#ifndef INC_NETSRV_DAEMON_H_
#define INC_NETSRV_DAEMON_H_

#include <stddef.h>
#include <errno.h>
#include <netsrv/defs.h>

/**
 * Daemonizes the application.
 * @return Zero if successful, an error code otherwise.
 */
int
netsrv_daemonize (void);

#endif
