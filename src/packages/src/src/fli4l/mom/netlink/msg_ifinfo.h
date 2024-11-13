#if !defined(MOM_NETLINK_MSG_IFINFO_H_)
#define MOM_NETLINK_MSG_IFINFO_H_

#include "netlink.h"
#include "env.h"

int handle_ifinfo_message(struct ifinfomsg const *message, size_t const message_len, struct env_t **const env);

#endif
