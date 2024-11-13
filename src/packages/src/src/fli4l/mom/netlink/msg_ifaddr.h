#if !defined(MOM_NETLINK_MSG_IFADDR_H_)
#define MOM_NETLINK_MSG_IFADDR_H_

#include "netlink.h"
#include "env.h"

int handle_ifaddr_message(struct ifaddrmsg const *message, size_t const message_len, struct env_t **const env);

#endif
