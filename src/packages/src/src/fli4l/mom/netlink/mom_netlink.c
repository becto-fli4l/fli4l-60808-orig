/**
 * mom_netlink: translates netlink events to fli4l events
 *
 * Copyright © 2012 The fli4l team
 *
 * Created:  2012-02-14 by kristov (fli4l@kristov.de)
 * Revision: $Id$
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "defs.h"
#include "netlink.h"
#include "env.h"
#include "msg_ifinfo.h"
#include "msg_ifaddr.h"
#include "msg_prefix.h"
#include "msg_rt.h"

/* wait this time between initial requests */
#define INIT_REQ_DELAY (500 * 1000)
/* the shell script to execute when a netlink event arrives */
#define MOM_BROADCAST "mom_broadcast"

/* associates initial dump requests with multicast subscriptions */
struct req_mgrp_assoc_t {
	__u16 request; /* netlink request, can be NLMSG_NOOP */
	__u16 flags;   /* flags, should be NLM_F_DUMP */
	__u8 family;   /* address family for RTnetlink request */
	__u16 group;   /* multicast group to subscribe to */
};
static struct req_mgrp_assoc_t const req_mgrp_assoc[] = {
	{ RTM_GETLINK, NLM_F_DUMP, AF_UNSPEC, RTNLGRP_LINK, },
	{ RTM_GETADDR, NLM_F_DUMP, AF_INET, RTNLGRP_IPV4_IFADDR, },
	{ RTM_GETADDR, NLM_F_DUMP, AF_INET6, RTNLGRP_IPV6_IFADDR, },
	{ RTM_GETROUTE, NLM_F_DUMP, AF_INET, RTNLGRP_IPV4_ROUTE, },
	{ RTM_GETROUTE, NLM_F_DUMP, AF_INET6, RTNLGRP_IPV6_ROUTE, },
	{ NLMSG_NOOP, 0, AF_INET6, RTNLGRP_IPV6_PREFIX, },
};
static size_t const req_mgrp_assoc_num = COUNT_OF(req_mgrp_assoc);

sig_atomic_t do_exit = FALSE;

/**
 * Signals program termination.
 * @param sig the signal number (not used)
 */
static void termination_handler(int const sig)
{
	do_exit = TRUE;
	(void) sig;
}

/**
 * Processes a RTnetlink message.
 * @param nl_socket the underlying socket
 * @param nlh a pointer to the message header
 * @param env the environment containing attribute assignments
 * @return the type of event to broadcast or NULL otherwise (if e.g. message is unknown)
 */
static char const *process_message(int const nl_socket, struct nlmsghdr const *nlh, struct env_t **const env)
{
	switch (nlh->nlmsg_type) {
	case RTM_NEWLINK :
		if (handle_ifinfo_message((struct ifinfomsg const *) NLMSG_DATA(nlh), IFLA_PAYLOAD(nlh), env)) {
			return "new_link_net_event";
		} else {
			return NULL;
		}
	case RTM_DELLINK :
		if (handle_ifinfo_message((struct ifinfomsg const *) NLMSG_DATA(nlh), IFLA_PAYLOAD(nlh), env)) {
			return "del_link_net_event";
		} else {
			return NULL;
		}
	case RTM_NEWADDR :
		if (handle_ifaddr_message((struct ifaddrmsg const *) NLMSG_DATA(nlh), IFA_PAYLOAD(nlh), env)) {
			return "new_addr_net_event";
		} else {
			return NULL;
		}
	case RTM_DELADDR :
		if (handle_ifaddr_message((struct ifaddrmsg const *) NLMSG_DATA(nlh), IFA_PAYLOAD(nlh), env)) {
			return "del_addr_net_event";
		} else {
			return NULL;
		}
	case RTM_NEWROUTE :
		if (handle_rt_message((struct rtmsg const *) NLMSG_DATA(nlh), RTM_PAYLOAD(nlh), env)) {
			return "new_rt_net_event";
		} else {
			return NULL;
		}
	case RTM_DELROUTE :
		if (handle_rt_message((struct rtmsg const *) NLMSG_DATA(nlh), RTM_PAYLOAD(nlh), env)) {
			return "del_rt_net_event";
		} else {
			return NULL;
		}
	case RTM_NEWPREFIX :
		if (handle_prefix_message((struct prefixmsg const *) NLMSG_DATA(nlh), PREFIX_PAYLOAD(nlh), env)) {
			return "new_prefix_net_event";
		} else {
			return NULL;
		}
	case NLMSG_DONE :   /* ACK sent after NLM_F_DUMP message */
	case NLMSG_ERROR :  /* ACK sent after normal message */
		if (nlh->nlmsg_seq <= req_mgrp_assoc_num) {
			__u16 const group = req_mgrp_assoc[nlh->nlmsg_seq - 1].group;
			if (netlink_subscribe_to_multicast_group(nl_socket, group) < 0) {
				syslog(LOG_ERR, "subscribing to multicast group %u failed: %s", group, strerror(errno));
			}
			if (nlh->nlmsg_seq == req_mgrp_assoc_num) {
				wait(NULL);
			}
		}
		return NULL;
	default :
		syslog(LOG_WARNING, "unknown message received: %u", nlh->nlmsg_type);
		return NULL;
	}
}

/**
 * Broadcasts an event.
 * @param event_type the type of event to broadcast
 * @param env the environment containing attribute assignments
 */
static void broadcast_event(char const *const event_type, struct env_t *const env)
{
	struct env_t *e;
	size_t len = 0;
	char *args;
	char *cmd = NULL;

	e = env;
	while (e) {
		len += strlen(e->name) + 1 + strlen(e->value) + 1;
		e = e->next;
	}

	args = (char *) malloc(len + 1);
	args[0] = '\0';
	e = env;
	while (e) {
		strcat(args, e->name);
		strcat(args, "=");
		strcat(args, e->value);
		strcat(args, " ");
		e = e->next;
	}

	syslog(LOG_INFO, "delivering %s %s", event_type, args);
	if (asprintf(&cmd, "%s %s", MOM_BROADCAST, event_type) < 0) {
		syslog(LOG_ERR, "%s not delivered: asprintf() failed", event_type);
	}
	else {
		if (system(cmd) < 0) {
			syslog(LOG_ERR, "%s not delivered: system() failed: %s", event_type, strerror(errno));
		}
		free(cmd);
	}
}

/**
 * Reads and processes RTnetlink messages.
 * @param nl_socket the underlying socket
 * @return FALSE if program exit requested, TRUE otherwise
 */
static int read_and_process_messages(int const nl_socket)
{
	size_t len = 0;
	struct nlmsghdr const *nlh = netlink_read_messages(nl_socket, &len);
	if (!nlh) {
		return FALSE;
	}

	for (; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
		struct env_t *env = NULL;
		char const *event_type = process_message(nl_socket, nlh, &env);
		if (event_type != NULL) {
			broadcast_event(event_type, env);
		}
		clear_env(&env);
	}

	return TRUE;
}

/**
 * Sends initial requests to get initial data and to subscribe to multicast
 * groups.
 * @param nl_socket the underlying socket
 */
static void send_initial_requests(int const nl_socket)
{
	size_t i;
	for (i = 0; i < req_mgrp_assoc_num; ++i) {
		__u16 const req = req_mgrp_assoc[i].request;
		if (rtnetlink_send_request(nl_socket, req, req_mgrp_assoc[i].flags, req_mgrp_assoc[i].family) < 0) {
			syslog(LOG_ERR, "message %u not sent: %s", req, strerror(errno));
		}
		usleep(INIT_REQ_DELAY);
	}
}

/**
 * Main program entry point.
 * @param argc the number of parameters
 * @param argv the parameters
 */
int main(int const argc, char *const argv [])
{
	struct sigaction siga;
	int nl_socket;
	pid_t child_pid;

	memset(&siga, 0, sizeof siga);
	siga.sa_handler = &termination_handler;
	sigaction(SIGINT, &siga, NULL);
	sigaction(SIGTERM, &siga, NULL);

	siga.sa_handler = SIG_IGN;
	sigaction(SIGHUP, &siga, NULL);
	sigaction(SIGPIPE, &siga, NULL);
	sigaction(SIGALRM, &siga, NULL);
	sigaction(SIGUSR1, &siga, NULL);
	sigaction(SIGUSR2, &siga, NULL);

	openlog(argc > 0 ? argv[0] : "mom_netlink", 0, LOG_LOCAL3);

	nl_socket = netlink_create_socket();
	if (nl_socket < 0) {
		return 1;
	}

	child_pid = fork();
	if (child_pid < 0) {
		perror("main");
	} else if (child_pid == 0) {
		send_initial_requests(nl_socket);
	} else {
		while (read_and_process_messages(nl_socket)) {
			;
		}
	}

	close(nl_socket);
	return 0;
}
