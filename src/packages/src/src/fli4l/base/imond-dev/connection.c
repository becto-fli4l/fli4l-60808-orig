/*
 * IMonD - deamon to monitor/control network connections
 *
 * Copyright © 2000-2004 Frank Meyer <frank@fli4l.de> & the fli4l team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/**
 * \file
 * \brief Client connection managment
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>				/* decl of inet_addr()	    */
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include "log.h"
#include "imond.h"
#include "user.h"

/** This is an array with all connected clients */
struct client_t client[MAX_CLIENTS];
/** This is the set of all active connections that should be honored by
 * select(3) */
static fd_set    conn_fdset;
/** This is the value that is actually passed to select(3), because it's value
 * is changed by the call */
static fd_set    current_fdset;
/** The maximum file descriptor that is used */
static int       conn_max_fd;

int conn_accept (struct client_t * c);
int handle_client_req (struct client_t * c);

static int alarm_triggered; /**< flag if alarm signal was triggered */

/**
 * Signal handler function used to catch hanging connections
 * \param sig signal number (man signal)
 */
static void
catch_alarm (int sig)
{
    alarm_triggered = 1;
}

/**
 * Initialize client connection array
 */
static void conn_init_register (void)
{
    int i; 
    for (i = 0; i< MAX_CLIENTS; i++)
    {
	client[i].fd = -1;
    }
    FD_ZERO (&conn_fdset);
    conn_max_fd = 0;
}

/**
 * Initialize connection handling
 */
void conn_init (void)
{
    struct sigaction act;

    /* install alarm handler */
    act.sa_handler = catch_alarm;
    sigemptyset (&act.sa_mask);
    act.sa_flags = 0;
    act.sa_restorer = NULL;
    sigaction (SIGALRM, &act, NULL);
    
    conn_init_register ();
}

/**
 * Register a new client connection, set up client structure
 * \param new_fd file descriptor of the new connection
 * \param ip the ip of the new client
 * \param password_state initial state of access
 * \param conn_handler pointer to a function handling an event on a file descriptor
 * \param handler_name name of the handler, used for debugging
 * \retval 0 on success
 * \retval -1 when there is no space left for a new client
 */
int conn_register_fd (int new_fd, const char * ip, int password_state, 
		      int (*conn_handler)(struct client_t * client), 
		      char * handler_name)
{
    int i; 
    for (i = 0; i< MAX_CLIENTS; i++)
    {
	if (client[i].fd == -1)
	{
	    client[i].fd = new_fd;
	    strncpy (client[i].ip, ip, 15);
	    client[i].password_state = password_state;
	    client[i].salt[0] = '\0';
	    client[i].conn_handler = conn_handler;
	    client[i].handler_name = handler_name;
	    FD_SET (new_fd, &conn_fdset);
	    if (new_fd > conn_max_fd)
	    {
		conn_max_fd = new_fd;
	    }
	    return 0;
	}
    }
    return -1;
}

/**
 * Unregister a client connection
 * \param fd file descriptor of the connection to unregister
 */
void conn_unregister_fd (int fd)
{
    int i; 
    conn_max_fd = 0;
    for (i = 0; i< MAX_CLIENTS; i++)
    {
	if (client[i].fd == fd)
	{
	    FD_CLR (fd, &conn_fdset);
	    client[i].fd = -1;
	}
	else if (client[i].fd > conn_max_fd)
	{
	    conn_max_fd = client[i].fd;
	}
    }
}

/**
 * Unregister a client connection
 * \param client client structure of the conection to unregister
 */
void conn_unregister (struct client_t * client)
{
    conn_unregister_fd (client->fd);
}

/**
 * Wait for an activity on any of the registered file descriptors
 * \retval 0 timeouton success
 * \retval > 0 number of ready file descriptors
 */
int conn_select (void)
{
    int ret;
    struct timeval		tv;

    tv.tv_sec   = POLLING_INTERVAL;	 /* 10 sec timeout for select */
    tv.tv_usec  = 0;

    current_fdset = conn_fdset;
    ret = select (conn_max_fd + 1, 
		  &current_fdset, (fd_set *) 0, (fd_set *) 0,
		  &tv);
    
    if (ret < 0)
    {
	if (errno != EINTR)
	{
	    imond_syslog (LOG_ERR, "select: %s\n", strerror (errno));
	    exit (1);
	}
	/* pretend that it was a timeout */
	ret = 0;
    }
    return ret;
}

/**
 * function called to handle active file descriptors; iterates over
 * all registered file descriptors and calls their handler functions
 * if there is something to do.
 */
void conn_handle_fd (void)
{
    int i; 
    for (i = 0; i< MAX_CLIENTS; i++)
    {
	if (client[i].fd >= 0 && FD_ISSET (client[i].fd, &current_fdset))
	{
#if 0
		imond_syslog (LOG_DEBUG, "invoking %s () for fd %d\n", 
			  client[i].handler_name, client[i].fd);
#endif
	    client[i].conn_handler (client+i);
	}
    }
    
}

/**
 * This function listens on a port on all given ip addresses.
 * 
 * \param ips array of ips to listen on
 * \param port the port to bind to
 * \retval 0 on success
 * \retval -1 on any error
 */
int conn_listen (char **ips, int port)
{
    struct sockaddr_in  my_listen_addr;
    struct in_addr	my_ip;
    int                 so_reuseaddr;
    int			sock_fd;
    
    while (*ips)
    {
	if ((sock_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    imond_syslog(LOG_ERR, "socket: %s\n", strerror (errno));
	    return (-1);
	}

	so_reuseaddr = 1;               /* re-use port immediately  */
	(void) setsockopt (sock_fd, SOL_SOCKET, SO_REUSEADDR, 
			   &so_reuseaddr, sizeof (so_reuseaddr));

	(void) memset ((char *) &my_listen_addr, sizeof (my_listen_addr), 0);
	(void) memset ((char *) &my_ip, sizeof(my_ip), 0);

	// try to use specified IP address
	if (inet_aton(*ips, &my_ip))
	{
	    my_listen_addr.sin_family      = AF_INET;
	    my_listen_addr.sin_addr        = my_ip;
	    my_listen_addr.sin_port        = htons (port);
	    
	    if (bind (sock_fd, (struct sockaddr *) &my_listen_addr,
		      sizeof (my_listen_addr)) < 0)
	    {
		(void) close (sock_fd);
		imond_syslog(LOG_ERR, "bind: %s while binding to %s:%d\n", 
			     strerror (errno), *ips, port);
		return (-1);
	    }
	    imond_syslog(LOG_ERR, "bind: bound to %s:%d\n", 
			 *ips, port);
	    listen (sock_fd, 5);
	    conn_register_fd (sock_fd, *ips, 0, 
			      conn_accept, "accept_connection");
	}
	else
	{
	    close (sock_fd);
	    imond_syslog (LOG_ERR,"ignoring invalid ip-address: %s\n",
			  *ips);
	}
	ips++;
    }
    return 0;
}

/**
 * Accept a connection from a client, this is one of the connection
 * handling functions which can can be registered via
 * conn_register_fd. If the accept is successful, the newly
 * established connection is registered with handle_client_req as the
 * connection handling function
 * \param c client structure associated with this connection
 */
int conn_accept (struct client_t * c)
{
    int new_fd;
    struct sockaddr_in my_listen_addr;
    int my_listen_size = sizeof (my_listen_addr);
    int password_state = PASSWORD_STATE_NONE;
    
    char ip [16];

    new_fd = accept (c->fd, (struct sockaddr *) &my_listen_addr,
		     &my_listen_size);
    
    if (new_fd >= 0)
    {
	unsigned char a[4];
	(void) memcpy (a, &(my_listen_addr.sin_addr.s_addr), 4);
	(void) sprintf (ip, "%d.%d.%d.%d", a[0], a[1], a[2], a[3]);
	
	if (! strcmp (ip, "127.0.0.1") ||
#ifdef TEST
	    1
#else
	    0
#endif
	    )
	{   /* local connection: no password needed */
	    password_state |= PASSWORD_STATE_NORMAL | PASSWORD_STATE_ADMIN;
	}
	else
	{
	    if (! (password_mode & PASS_REQUIRED))
	    {
		password_state |= PASSWORD_STATE_NORMAL;
		
		if (! (password_mode & ADMIN_PASS_REQUIRED))
		{
		    password_state |= PASSWORD_STATE_ADMIN;
		}
	    }
	}
	conn_register_fd (new_fd, ip, password_state, 
			  handle_client_req, "handle_client_request");
	return 0;
    }
    else
    {			    /* sorry, too many clients */
	close (new_fd);
	return -1;
    }
}


/**
 * Read a line from the client
 * \param fd file descriptor to read from
 * \param buf buffer to write the line to
 * \param maxlen maximal length of the line to read
 * \retval 0 when everything is all right
 * \retval -1 on error (client timeout)
 */
static int
read_line (int fd, char * buf, int maxlen)
{
    char *	p = buf;
    int		i;
    int		rtc;
    int		ret = 0;

    /* set alarm clock to be able to handle crashing or malicious clients */
    alarm_triggered = 0;
    alarm (5);
    for (p = buf, i = 1; i < maxlen; i++, p++)
    {
	rtc = read (fd, p, 1);

	if (rtc != 1)
	{
	    if (errno == EINTR && alarm_triggered)
	    {
		imond_syslog (LOG_INFO, "timeout while trying to"
			" read request from client\n");
	    }
	    ret = -1;
	    *p = '\0';
	    break;
	}

	if (*p == '\n')
	{
	    break;
	}
    }
    /* stop alarm clock */
    alarm (0);

    p++;
    *p = '\0';
    return ret;
} /* read_line (fd, buf, maxlen) */


/**
 * Read a line from the client, this is one of the connection handling
 * functions which can can be registered via conn_register_fd
 * \param c client structure associated with this connection
 */
int handle_client_req (struct client_t * c)
{
    char buf[256];
    if (read_line (c->fd, buf, 255) == 0)
    {
	char * answer = parse_command (c-client, buf);
	
	if (answer)
	{
	    int	n = strlen (answer);

	    if (write (c->fd, answer, n) == n)
	    {
		return 0;
	    }
	}
    }
    close (c->fd);
    conn_unregister (c);
    return -1;
}

