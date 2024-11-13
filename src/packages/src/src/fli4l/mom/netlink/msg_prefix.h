#if !defined(MOM_NETLINK_MSG_PREFIX_H_)
#define MOM_NETLINK_MSG_PREFIX_H_

#include "netlink.h"
#include "env.h"

int handle_prefix_message(struct prefixmsg const *message, size_t const message_len, struct env_t **const env);

#endif
