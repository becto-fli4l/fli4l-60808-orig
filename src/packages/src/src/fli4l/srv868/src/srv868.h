/**
 * @file    srv868.h
 * Global macros and public module operations.
 * @author  Christoph Schulz
 * @since   2004-11-08
 * @version $Id: srv868.h 233 2009-01-18 22:02:34Z kristov $
 */

#ifndef INC_SRV868_H_
#define INC_SRV868_H_

#include <stddef.h>
#include <errno.h>

/**********************
 * module time.c
 **********************/
/**
 * The main 'workhorse' of the application. Determines the current time and
 * sends it according to RFC 868 to the peer.
 * @param conn The connection handle.
 */
void time_handle_rfc868_request (void *conn);

#endif
