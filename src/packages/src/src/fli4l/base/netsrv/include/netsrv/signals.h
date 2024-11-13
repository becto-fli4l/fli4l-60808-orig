/**
 * @file    include/netsrv/signals.h
 * NetSrv: Signal handling.
 * @author  Christoph Schulz
 * @since   2004-11-08
 * @version $Id$
 */

#ifndef INC_NETSRV_SIGNALS_H_
#define INC_NETSRV_SIGNALS_H_

#include <stddef.h>
#include <errno.h>
#include <netsrv/defs.h>

/**
 * Initializes the signal handling.
 * @return Zero if successful, an error code otherwise.
 */
int netsrv_signals_init (void);
/**
 * Finalizes the signal handling.
 */
void netsrv_signals_done (void);
/**
 * Returns a non-zero value if SIGTERM has been received.
 */
int netsrv_signals_termination_requested (void);

#endif
