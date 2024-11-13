/**
 * @file    include/netsrv/net.h
 * NetSrv: Network framework.
 * @author  Christoph Schulz
 * @since   2004-11-08
 * @version $Id$
 */

#ifndef INC_NETSRV_NET_H_
#define INC_NETSRV_NET_H_

#include <stddef.h>
#include <errno.h>
#include <netsrv/defs.h>

/**********************
 * module net.c
 **********************/
/**
 * Typedef for connection handlers.
 * @param conn The connection handle, needed for calls to netsrv_net_read() or
 * netsrv_net_write().
 */
typedef void (*netsrv_net_conn_handler_t) (void *conn);
/**
 * Initializes the network module.
 * @return Zero if successful, an error code otherwise.
 */
int netsrv_net_init (void);
/**
 * Adds a network service.
 * @param port The port number to listen to (in host order).
 * @param is_tcp If 0, a UDP service is created, else a TCP service.
 * @param handler The connection handler to call when a connection has been
 *   established (or, in the case of UDP connections, a datagram has been
 *   received).
 * @return Zero if successful, an error code otherwise.
 */
int netsrv_net_add_service (
	int port, int is_tcp,
	netsrv_net_conn_handler_t handler
);
/**
 * Runs the main loop of the application, waiting for incoming connections.
 * Services are run in separate processes (fork()s).
 */
int netsrv_net_run (void);
/**
 * Finalizes the network module.
 */
void netsrv_net_done (void);
/**
 * Receives an amount of bytes from the remote peer.
 * @param conn The connection handle, passed to the service.
 * @param buf The buffer to receive the data requested.
 * @param count The number of bytes to read.
 * @return Zero if successful, an error code otherwise.
 */
int netsrv_net_read (void *conn, void *buf, size_t count);
/**
 * Receives some data from the remote peer.
 * @param conn The connection handle, passed to the service.
 * @param buf Receives the buffer with the data read.
 * @param count The number of bytes read.
 * @return Zero if successful, an error code otherwise.
 */
int netsrv_net_read_some (void *conn, void **buf, size_t *count);
/**
 * Sends an amount of bytes to the remote peer.
 * @param conn The connection handle, passed to the service.
 * @param buf The buffer containing the data to be sent.
 * @param count The number of bytes to write.
 * @return Zero if successful, an error code otherwise.
 */
int netsrv_net_write (void *conn, const void *buf, size_t count);

#endif
