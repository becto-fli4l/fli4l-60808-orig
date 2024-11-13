#include "defs.h"
#include "msg_ifinfo.h"
#include "type_basic.h"
#include "type_address.h"

static void handle_ifinfo_flags(__u32 const flags, struct env_t **const env)
{
	/* IFF_* in linux/if.h */
	handle_flag(flags & IFF_UP, env, "link_net_event_flag_up");
	handle_flag(flags & IFF_BROADCAST, env, "link_net_event_flag_broadcast");
	handle_flag(flags & IFF_DEBUG, env, "link_net_event_flag_debug");
	handle_flag(flags & IFF_LOOPBACK, env, "link_net_event_flag_loopback");
	handle_flag(flags & IFF_POINTOPOINT, env, "link_net_event_flag_pointopoint");
	handle_flag(flags & IFF_NOTRAILERS, env, "link_net_event_flag_notrailers");
	handle_flag(flags & IFF_RUNNING, env, "link_net_event_flag_running");
	handle_flag(flags & IFF_NOARP, env, "link_net_event_flag_noarp");
	handle_flag(flags & IFF_PROMISC, env, "link_net_event_flag_promisc");
	handle_flag(flags & IFF_ALLMULTI, env, "link_net_event_flag_allmulti");
	handle_flag(flags & IFF_MASTER, env, "link_net_event_flag_master");
	handle_flag(flags & IFF_SLAVE, env, "link_net_event_flag_slave");
	handle_flag(flags & IFF_MULTICAST, env, "link_net_event_flag_multicast");
	handle_flag(flags & IFF_PORTSEL, env, "link_net_event_flag_portsel");
	handle_flag(flags & IFF_AUTOMEDIA, env, "link_net_event_flag_automedia");
	handle_flag(flags & IFF_DYNAMIC, env, "link_net_event_flag_dynamic");
	handle_flag(flags & IFF_LOWER_UP, env, "link_net_event_flag_lower_up");
	handle_flag(flags & IFF_DORMANT, env, "link_net_event_flag_dormant");
	handle_flag(flags & IFF_ECHO, env, "link_net_event_flag_echo");
}

static void handle_af_inet6_flags(struct rtattr const *rta, struct env_t **const env)
{
	if (RTA_PAYLOAD(rta) >= sizeof(__u32))
	{
		__u32 const flags = *(__u32 *) RTA_DATA(rta);
		/* IF_* in net/if_inet6.h (kernel header) */
		handle_flag(flags & IF_READY, env, "link_net_event_inet6_flag_ready");
		handle_flag(flags & IF_RS_SENT, env, "link_net_event_inet6_flag_rs_sent");
		handle_flag(flags & IF_RA_RCVD, env, "link_net_event_inet6_flag_ra_rcvd");
		handle_flag(flags & IF_RA_MANAGED, env, "link_net_event_inet6_flag_ra_managed");
		handle_flag(flags & IF_RA_OTHERCONF, env, "link_net_event_inet6_flag_ra_otherconf");
	}
}

static void handle_af_inet6_attributes(struct rtattr const *const rta, struct env_t **const env)
{
	size_t nested_len = rta->rta_len - NLMSG_ALIGN(sizeof(struct rtattr));
	struct rtattr const *nested_rta;
	for (nested_rta = (struct rtattr const *) (RTA_DATA(rta));
			RTA_OK(nested_rta, nested_len);
			nested_rta = RTA_NEXT(nested_rta, nested_len)) {
		switch (nested_rta->rta_type) {
		case IFLA_INET6_FLAGS :
			handle_af_inet6_flags(nested_rta, env);
			break;
		}
	}
}

static void handle_af_spec_attributes(struct rtattr const *const rta, struct env_t **const env)
{
	size_t family_len = rta->rta_len - NLMSG_ALIGN(sizeof(struct rtattr));
	struct rtattr const *family_rta;
	for (family_rta = (struct rtattr const *) (RTA_DATA(rta));
			RTA_OK(family_rta, family_len);
			family_rta = RTA_NEXT(family_rta, family_len)) {
		switch (family_rta->rta_type) {
		case AF_INET6 :
			handle_af_inet6_attributes(family_rta, env);
			break;
		}
	}
}

int handle_ifinfo_message(struct ifinfomsg const *message, size_t const message_len, struct env_t **const env)
{
	struct rtattr const *rta;
	size_t len = message_len;

	add_env_entry(env, "net_event_family", "%u", message->ifi_family);      /* AF_* in sys/socket.h (typically AF_UNSPEC) */
	add_env_entry(env, "bound_net_event_index", "%d", message->ifi_index);
	add_env_entry(env, "link_net_event_linktype", "%u", message->ifi_type); /* ARPHRD_* from linux/if_arp.h */

	handle_ifinfo_flags(message->ifi_flags, env);

	for (rta = IFLA_RTA(message); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
		switch (rta->rta_type) {
		case IFLA_ADDRESS :
			handle_layer2_address(message->ifi_type, rta, env, "link_net_event_address");
			break;
		case IFLA_BROADCAST :
			handle_layer2_address(message->ifi_type, rta, env, "link_net_event_broadcast");
			break;
		case IFLA_IFNAME :
			handle_string(rta, env, "link_net_event_ifname");
			break;
		case IFLA_MTU :
			handle_u32(rta, env, "link_net_event_mtu");
			break;
		case IFLA_LINK :
			handle_u32(rta, env, "link_net_event_link");
			break;
		case IFLA_OPERSTATE : /* IF_OPER_* in linux/if.h */
			handle_u8(rta, env, "link_net_event_operstate");
			break;
		case IFLA_LINKMODE : /* IF_LINK_MODE_* in linux/if.h */
			handle_u8(rta, env, "link_net_event_linkmode");
			break;
		case IFLA_AF_SPEC :
			handle_af_spec_attributes(rta, env);
			break;
		}
	}

	return TRUE;
}
