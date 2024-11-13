/**
 * @file    src/net.c
 * NetSrv: Network framework.
 * @author  Christoph Schulz
 * @since   2004-11-08
 * @version $Id$
 */

#include "netsrv/net.h"
#include "netsrv/signals.h"
#include "netsrv/error.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

/**
 * @def NETSRV_NET_MAX_SERVICES
 * Determines how many services (not connections!) we allow.
 */
#ifndef NETSRV_NET_MAX_SERVICES
#define NETSRV_NET_MAX_SERVICES 10 /* can be incremented if really needed */
#endif

/**
 * @def NETSRV_NET_SELECT_TIMEOUT
 * The number of seconds used as select() timeout.
 */
#ifndef NETSRV_NET_SELECT_TIMEOUT
#define NETSRV_NET_SELECT_TIMEOUT 5
#endif

/**
 * Encapsulates the information about a service.
 */
struct service_info
{
	int sock; /**< The listening socket. */
	int is_tcp; /**< != 0 if this is a TCP service, otherwise it's UDP. */
	netsrv_net_conn_handler_t handler; /**< The associated connection handler. */
};

/**
 * Encapsulates the information about a connection.
 */
struct conn_info
{
	int sock; /**< The connection socket. */
	struct service_info *service;  /**< The associated service. */
	struct sockaddr_in source;  /**< The address of the peer. */
	size_t count; /**< The size of the datagram in bytes (only for UDP) */
	char *data; /**< The data of the datagram (only for UDP) */
};

/**
 * The array containing information about each registered service.
 */
static struct service_info g_service_info [NETSRV_NET_MAX_SERVICES];
/**
 * The number of registered services.
 */
static int g_num_services = 0;
/**
 * A (cached) value denoting the highest socket created for listening. Needed
 * later for select().
 */
static int g_largest_socket = -1;
/**
 * The library version.
 */
char const g_version[] = NETSRV_VERSION;

/**
 * Accepts a TCP connection request.
 * @param service The service the input has been sent to.
 * @return Zero if successful, an error code otherwise.
 */
static int
netsrv_net_accept_tcp (struct service_info *service)
{
	int rc;
	int accepted;
	struct sockaddr_in source;
	socklen_t addrlen = sizeof source;
	EXCEPT_AWARE (int);

	/* accept incoming TCP connection, determine peer's address */
	SYSCALL (
		accepted,
		accept (service->sock, (struct sockaddr *) &source, &addrlen),
		ERROR_ACCEPT
	);

	/* fork */
	TRY(1)

		SYSCALL (rc, fork (), ERROR_ACCEPT_FORK);
		if (rc == 0)
		{
			/* initialize connection info structure */
			struct conn_info conninfo;
			memset (&conninfo, 0, sizeof conninfo);
			conninfo.sock = accepted;
			conninfo.service = service;
			conninfo.source = source;

			/* call connection handler */
			service->handler (&conninfo);

			/* shutdown connection and close socket */
			shutdown (accepted, SHUT_RDWR);
			close (accepted);

			/* terminate fork()'ed  process */
			exit (0);
		}
		else
		{
			/* close TCP server socket in *this* process */
			close (accepted);
			return 0;
		}

	EXCEPT(1)
		/* close TCP server socket in *this* process */
		close (accepted);
		RETHROW();

	ENDTRY(1)
}

/**
 * Accepts a UDP datagram.
 * @param service The service the input has been sent to.
 * @return Zero if successful, an error code otherwise.
 */
static int
netsrv_net_accept_udp (struct service_info *service)
{
	int rc;
	struct sockaddr_in source;
	socklen_t addrlen = sizeof source;
	size_t count;
	char *data;
	EXCEPT_AWARE (int);

	if (ioctl(service->sock, FIONREAD, (int *) &count) != 0) {
		return ERROR_READ;
	}

	if (count == 0) {
		data = NULL;
	}
	else {
		data = (char *) malloc(count);
		if (!data) {
			return ERROR_READ;
		}

		TRY(1)
			SYSCALLINTR (rc,
				recvfrom (service->sock, data, count, 0,
							(struct sockaddr *) &source, &addrlen),
				ERROR_READ
			);

		EXCEPT(1)
			free(data);
			RETHROW();

		ENDTRY(1)
	}

	/* fork */
	TRY(2)
		SYSCALL (rc, fork (), ERROR_ACCEPT_FORK);
		if (rc == 0)
		{
			/* initialize connection info structure */
			struct conn_info conninfo;
			memset (&conninfo, 0, sizeof conninfo);
			conninfo.sock = service->sock;
			conninfo.service = service;
			conninfo.source = source;
			conninfo.count = count;
			conninfo.data = data;

			/* call connection handler */
			service->handler (&conninfo);

			/* terminate fork()'ed  process */
			exit (0);
		}
		else
		{
			/* free datagram */
			free(data);
			return 0;
		}

	EXCEPT(2)
		/* free datagram */
		free(data);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
		RETHROW();
#pragma GCC diagnostic pop

	ENDTRY(2)
}

/**
 * Accepts either a TCP connection request or a UDP datagram.
 * @param service The service the input has been sent to.
 * @return Zero if successful, an error code otherwise.
 */
static int
netsrv_net_accept (struct service_info *service)
{
	if (service->is_tcp)
		return netsrv_net_accept_tcp(service);
	else
		return netsrv_net_accept_udp(service);
}

int
netsrv_net_init (void)
{
	/* nothing to do, global variables are set to zero by the compiler */
	return 0;
}

int
netsrv_net_add_service (int port, int is_tcp, netsrv_net_conn_handler_t handler)
{
	int rc;
	struct service_info *service = (struct service_info *) NULL;
	int sockflag;
	struct sockaddr_in in;
	EXCEPT_AWARE (int);

	/* check some arguments */
	if (g_num_services == NETSRV_NET_MAX_SERVICES)
		ERROR ("netsrv_net_add_service: limit exceeded", ERROR_ADD_SERVICE);
	if (handler == (netsrv_net_conn_handler_t) NULL)
		ERROR ("netsrv_net_add_service: handler is NULL", ERROR_ADD_SERVICE);

	service = &g_service_info [g_num_services];
	service->is_tcp = is_tcp;
	/* create listening socket */
	SYSCALL (
		service->sock,
		socket (PF_INET, is_tcp ? SOCK_STREAM : SOCK_DGRAM, 0),
		ERROR_SOCKET
	);

	/* now mark entry as used so socket will be closed later */
	service->handler = handler;
	++g_num_services;

	TRY(1)

		/* set SO_REUSEADDR flag on socket */
		sockflag = 1;
		SYSCALL (rc,
			setsockopt (service->sock, SOL_SOCKET, SO_REUSEADDR, &sockflag,
				sizeof sockflag),
			ERROR_SETSOCKOPT
		);

		/* bind socket to all local addresses to passed port */
		in.sin_family = AF_INET;
		in.sin_port = htons ((uint16_t) port);
		in.sin_addr.s_addr = htonl (INADDR_ANY);
		SYSCALL (
			rc,
			bind (service->sock, (struct sockaddr *) &in, sizeof in),
			ERROR_BIND
		);

		/* for TCP services: set socket to listening mode */
		if (is_tcp)
			SYSCALL (rc, listen (service->sock, SOMAXCONN), ERROR_LISTEN);

		/* update g_largest_socket if needed */
		if (service->sock > g_largest_socket)
			g_largest_socket = service->sock;

		return 0;

	EXCEPT(1)

		/* remove service again due to an error */
		--g_num_services;
		close (service->sock);
		RETHROW();

	ENDTRY(1)
}

int
netsrv_net_run (void)
{
	EXCEPT_AWARE (int);
	while (1)
	{
		int rc;
		int s;
		fd_set sockets;
		struct timeval tv;

		/* handle SIGTERM signal */
		if (netsrv_signals_termination_requested ())
			return 0;

		/* poll sockets */
		FD_ZERO (&sockets);
		for (s = 0; s < g_num_services; ++s)
			FD_SET (g_service_info [s].sock, &sockets);

		memset (&tv, 0, sizeof tv);
		tv.tv_sec = NETSRV_NET_SELECT_TIMEOUT;
		tv.tv_usec = 0;

		SYSCALLINTR (
			rc,
			select (g_largest_socket + 1, &sockets, (fd_set *) NULL,
				(fd_set *) NULL, &tv),
			ERROR_SELECT
		);

		if (rc > 0)
		{
			/* handle incoming connection requests */
			for (s = 0; s < g_num_services; ++s)
				if (FD_ISSET (g_service_info [s].sock, &sockets))
					(void) netsrv_net_accept (&g_service_info [s]);
		}
	}
}

void
netsrv_net_done (void)
{
	int i;
	for (i = 0; i < g_num_services; ++i)
	{
		/* shutdown and close all sockets */
		int sock = g_service_info [i].sock;
		shutdown (sock, SHUT_RDWR);
		close (sock);
	}
}

int
netsrv_net_read (void *conn, void *buf, size_t count)
{
	struct conn_info *info = (struct conn_info *) conn;
	int is_tcp = info->service->is_tcp;
	EXCEPT_AWARE (int);

	if (is_tcp) {
		/* loop because recv() can return less bytes than expected due
		   to signals */
		while (count > 0)
		{
			ssize_t bytesread;
			SYSCALLINTR (bytesread,
				recv (info->sock, buf, count, 0),
				ERROR_READ
			);
			count = count - (size_t) bytesread;
			buf = (char *) buf + bytesread;
		}
		return 0;
	}
	else {
		if (info->count < count) {
			return ERROR_READ;
		}
		else {
			buf = info->data;
			info->data += count;
			info->count -= count;
			return 0;
		}
	}
}

int
netsrv_net_read_some (void *conn, void **buf, size_t *count)
{
	struct conn_info *info = (struct conn_info *) conn;
	int is_tcp = info->service->is_tcp;
	EXCEPT_AWARE (int);

	if (is_tcp) {
		while (1)
		{
			int rc;
			fd_set sockets;
			struct timeval tv;

			/* handle SIGTERM signal */
			if (netsrv_signals_termination_requested ())
				return 0;

			/* poll sockets */
			FD_ZERO (&sockets);
			FD_SET (info->sock, &sockets);

			memset (&tv, 0, sizeof tv);
			tv.tv_sec = NETSRV_NET_SELECT_TIMEOUT;
			tv.tv_usec = 0;

			SYSCALLINTR (
				rc,
				select (info->sock + 1, &sockets, (fd_set *) NULL,
					(fd_set *) NULL, &tv),
				ERROR_SELECT
			);

			if (rc > 0)
			{
				char *p;
				size_t plen;

				if (ioctl(info->sock, FIONREAD, (int *) count) != 0) {
					*buf = NULL;
					*count = 0;
					return 0;
				}

				*buf = malloc(*count);
				if (!*buf) {
					return ERROR_READ;
				}

				p = (char *) *buf;
				plen = *count;

				/* loop because recv() can return less bytes than expected due
				   to signals */
				while (plen > 0)
				{
					ssize_t bytesread;
					SYSCALLINTR (bytesread,
						recv (info->sock, p, plen, 0),
						ERROR_READ
					);
					plen -= (size_t) bytesread;
					p += bytesread;
				}
				return 0;
			}
		}
	}
	else {
		*buf = info->data;
		*count = info->count;
		info->data = NULL;
		info->count = 0;
		return 0;
	}
}

int
netsrv_net_write (void *conn, const void *buf, size_t count)
{
	struct conn_info *info = (struct conn_info *) conn;
	int is_tcp = info->service->is_tcp;
	EXCEPT_AWARE (int);

	/* loop because send()/sendto() can send less bytes than expected due to
	   signals */
	while (count > 0)
	{
		ssize_t byteswritten;
		SYSCALLINTR (byteswritten,
			is_tcp
				? send (info->sock, buf, count, 0)
				: sendto (info->sock, buf, count, 0, (const struct sockaddr *)
					&info->source, sizeof info->source),
			ERROR_WRITE
		);
		count = count - (size_t) byteswritten;
		buf = (const char *) buf + byteswritten;
	}
	return 0;
}
