#include "defs.h"
#include "msg_ifaddr.h"
#include "type_basic.h"
#include "type_address.h"

static void handle_ifaddr_flags(__u32 const flags, struct env_t **const env)
{
	/* IFA_F_* in linux/if_addr.h */
	handle_flag(flags & IFA_F_TEMPORARY, env, "addr_net_event_flag_temporary");
	handle_flag(flags & IFA_F_NODAD, env, "addr_net_event_flag_nodad");
	handle_flag(flags & IFA_F_OPTIMISTIC, env, "addr_net_event_flag_optimistic");
	handle_flag(flags & IFA_F_DADFAILED, env, "addr_net_event_flag_dadfailed");
	handle_flag(flags & IFA_F_HOMEADDRESS, env, "addr_net_event_flag_homeaddress");
	handle_flag(flags & IFA_F_DEPRECATED, env, "addr_net_event_flag_deprecated");
	handle_flag(flags & IFA_F_TENTATIVE, env, "addr_net_event_flag_tentative");
	handle_flag(flags & IFA_F_PERMANENT, env, "addr_net_event_flag_permanent");
}

int handle_ifaddr_message(struct ifaddrmsg const *message, size_t const message_len, struct env_t **const env)
{
	struct rtattr const *rta;
	size_t len = message_len;

	add_env_entry(env, "net_event_family", "%u", message->ifa_family); /* AF_* in sys/socket.h */
	add_env_entry(env, "bound_net_event_index", "%u", message->ifa_index);
	add_env_entry(env, "addr_net_event_prefixlen", "%u", message->ifa_prefixlen);
	add_env_entry(env, "addr_net_event_scope", "%u", message->ifa_scope);   /* RT_SCOPE_* in linux/rtnetlink.h */

	handle_ifaddr_flags(message->ifa_flags, env);

	for (rta = IFA_RTA(message); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
		switch (rta->rta_type) {
		case IFA_ADDRESS :
			handle_layer3_address(message->ifa_family, rta, env, "addr_net_event_address");
			break;
		case IFA_LOCAL :
			handle_layer3_address(message->ifa_family, rta, env, "addr_net_event_local");
			break;
		case IFA_BROADCAST :
			handle_layer3_address(message->ifa_family, rta, env, "addr_net_event_broadcast");
			break;
		case IFA_ANYCAST :
			handle_layer3_address(message->ifa_family, rta, env, "addr_net_event_nycast");
			break;
		case IFA_MULTICAST :
			handle_layer3_address(message->ifa_family, rta, env, "addr_net_event_multicast");
			break;
		case IFA_LABEL :
			handle_string(rta, env, "addr_net_event_label");
			break;
		}
	}

	return TRUE;
}
