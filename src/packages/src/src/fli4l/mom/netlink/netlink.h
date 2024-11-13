/**
 * @file netlink.h
 * Netlink I/O.
 */
#if !defined(MOM_NETLINK_NETLINK_H_)
#define MOM_NETLINK_NETLINK_H_

#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <linux/if.h>
#include <linux/if_link.h>
#include <linux/if_arp.h>

/* linux/socket.h, missing from userspace headers */
#if !defined(SOL_NETLINK)
#define SOL_NETLINK	270
#endif

/* linux/netlink.h, missing from userspace headers */
#if !defined(NETLINK_ADD_MEMBERSHIP)
#define NETLINK_ADD_MEMBERSHIP	1
#endif

/* linux/rtnetlink.h, missing (forgotten?) */
#if !defined(PREFIX_RTA)
#define PREFIX_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct prefixmsg))))
#define PREFIX_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct prefixmsg))
#endif

/* linux/if_link.h, missing from older kernel headers */
#define IFLA_AF_SPEC 26

/* net/if_inet6.h, missing from userspace headers */
#if !defined(IF_READY)
#define IF_RA_OTHERCONF	0x80
#define IF_RA_MANAGED	0x40
#define IF_RA_RCVD	0x20
#define IF_RS_SENT	0x10
#define IF_READY	0x80000000
#endif

/* net/if_inet6.h, missing from userspace headers */
#if !defined(IF_PREFIX_ONLINK)
#define IF_PREFIX_ONLINK	0x01
#define IF_PREFIX_AUTOCONF	0x02
#endif

/**
 * Creates and binds a netlink socket.
 * @return the socket created or -1 if an error occurs
 */
int netlink_create_socket(void);

/**
 * Subscribes process to a netlink multicast group.
 * @param nl_socket the underlying socket
 * @param group the group to subscribe to
 * @return zero on success, -1 on failure
 */
int netlink_subscribe_to_multicast_group(int nl_socket, __u32 group);

/**
 * Reads a bunch of netlink messages.
 * @param nl_socket the underlying socket
 * @param len receives the length of the datagram read
 * @return a pointer to the first message or NULL if an error occurs; the
 *         messages (if any) can be retrieved by using the NLMSG_OK and
 *         NLMSG_NEXT macros
 */
struct nlmsghdr const *netlink_read_messages(int nl_socket, size_t *len);

/**
 * Sends a netlink request.
 * @param nl_socket the underlying socket
 * @param type the type of the request
 * @param flags the netlink flags (NLM_F_REQUEST and NLM_F_ACK will be added)
 * @param data additional data sent with the request (may be NULL)
 * @param datalen length of the data sent (may be zero)
 * @return number of characters sent or -1 on failure
 */
int netlink_send_request(int nl_socket, __u16 type, __u16 flags, void const *data, size_t datalen);

/**
 * Sends a RTnetlink request.
 * @param nl_socket the underlying socket
 * @param type the type of the request
 * @param flags the netlink flags (NLM_F_REQUEST and NLM_F_ACK will be added)
 * @param family the RTnetlink address family (may be AF_UNSPEC)
 * @return number of characters sent or -1 on failure
 */
int rtnetlink_send_request(int nl_socket, __u16 type, __u16 flags, __u8 family);

#endif
