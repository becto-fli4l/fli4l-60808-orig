#include "defs.h"
#include "msg_prefix.h"
#include "type_basic.h"
#include "type_address.h"

static void handle_prefix_flags(__u32 const flags, struct env_t **const env)
{
	/* IF_PREFIX_* in net/if_inet6.h (kernel header) */
	handle_flag(flags & IF_PREFIX_ONLINK, env, "prefix_net_event_flag_onlink");
	handle_flag(flags & IF_PREFIX_AUTOCONF, env, "prefix_net_event_flag_autoconf");
}

int handle_prefix_message(struct prefixmsg const *message, size_t const message_len, struct env_t **const env)
{
	struct rtattr const *rta;
	size_t len = message_len;

	add_env_entry(env, "net_event_family", "%u", message->prefix_family);  /* AF_* in sys/socket.h */
	add_env_entry(env, "bound_net_event_index", "%d", message->prefix_ifindex);
	add_env_entry(env, "prefix_net_event_prefixlen", "%u", message->prefix_len);

	handle_prefix_flags(message->prefix_flags, env);

	for (rta = PREFIX_RTA(message); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
		switch (rta->rta_type) {
		case PREFIX_ADDRESS :
			handle_layer3_address(message->prefix_family, rta, env, "prefix_net_event_address");
			break;
		}
	}

	return TRUE;
}
