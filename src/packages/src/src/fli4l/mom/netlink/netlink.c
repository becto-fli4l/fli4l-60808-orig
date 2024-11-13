/**
 * @file netlink.c
 * Netlink I/O.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "defs.h"
#include "netlink.h"

/* should suffice, I hope... */
#define MAX_PAYLOAD 65536

static char g_message_buffer[MAX_PAYLOAD];
static __u32 nlh_next_seq = 1; /* start index, may not be zero */

int netlink_create_socket(void)
{
	int nl_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (nl_socket < 0) {
		perror("create_socket");
		return -1;
	} else {
		struct sockaddr_nl sa;
		memset(&sa, 0, sizeof sa);
		sa.nl_family = AF_NETLINK;
		sa.nl_pid = getpid();
		if (bind(nl_socket, (struct sockaddr *) &sa, sizeof sa) < 0) {
			perror("bind_socket");
			close(nl_socket);
			return -1;
		} else {
			return nl_socket;
		}
	}
}

int netlink_subscribe_to_multicast_group(int const nl_socket, __u32 const group)
{
	return setsockopt(nl_socket, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &group, sizeof group);
}

struct nlmsghdr const *netlink_read_messages(int const nl_socket, size_t *const len)
{
	struct iovec iov;
	struct msghdr msgh;

	memset(&iov, 0, sizeof iov);
	iov.iov_base = g_message_buffer;
	iov.iov_len = sizeof g_message_buffer;

	memset(&msgh, 0, sizeof msgh);
	msgh.msg_name = NULL;
	msgh.msg_namelen = 0;
	msgh.msg_iov = &iov;
	msgh.msg_iovlen = 1;

	while (TRUE) {
		ssize_t bytes = recvmsg(nl_socket, &msgh, 0);
		if (bytes < 0 && (errno != EINTR || do_exit)) {
			return NULL;
		} else if (bytes >= 0) {
			if (len) {
				*len = (size_t) bytes;
			}
			return (struct nlmsghdr const *) g_message_buffer;
		}
	}
}

int netlink_send_request(int const nl_socket, __u16 const type, __u16 const flags, void const *const data, size_t const datalen)
{
	struct sockaddr_nl sa;
	struct iovec iov;
	struct msghdr msgh;
	struct nlmsghdr *msg;

	msg = (struct nlmsghdr *) malloc(NLMSG_SPACE(datalen));
	memset(msg, 0, sizeof(struct nlmsghdr));
	msg->nlmsg_len = NLMSG_SPACE(sizeof(struct rtgenmsg));
	msg->nlmsg_type = type;
	msg->nlmsg_pid = getpid();
	msg->nlmsg_seq = nlh_next_seq++;
	msg->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
	memcpy(NLMSG_DATA(msg), data, datalen);

	memset(&sa, 0, sizeof sa);
	sa.nl_family = AF_NETLINK;
	sa.nl_pid = 0; /* kernel is target */

	memset(&iov, 0, sizeof iov);
	iov.iov_base = msg;
	iov.iov_len = msg->nlmsg_len;

	memset(&msgh, 0, sizeof msgh);
	msgh.msg_name = &sa;
	msgh.msg_namelen = sizeof sa;
	msgh.msg_iov = &iov;
	msgh.msg_iovlen = 1;

	return sendmsg(nl_socket, &msgh, 0);
}
int rtnetlink_send_request(int const nl_socket, __u16 type, __u16 flags, __u8 family)
{
	struct rtgenmsg gen;
	memset(&gen, 0, sizeof gen);
	gen.rtgen_family = family;
	return netlink_send_request(nl_socket, type, flags, &gen, sizeof gen);
}
