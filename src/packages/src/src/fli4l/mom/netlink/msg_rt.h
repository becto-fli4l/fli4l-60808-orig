#if !defined(MOM_NETLINK_MSG_RT_H_)
#define MOM_NETLINK_MSG_RT_H_

#include "netlink.h"
#include "env.h"

int handle_rt_message(struct rtmsg const *const message, size_t const message_len, struct env_t **const env);

#endif
