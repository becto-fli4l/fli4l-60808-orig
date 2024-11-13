#include "defs.h"
#include "msg_rt.h"
#include "type_basic.h"
#include "type_address.h"

#if 0 /* not interesting */
static void handle_rt_flags(__u32 const flags, struct env_t **const env)
{
	/* RTM_F_* in linux/rtnetlink.h */
	handle_flag(flags & RTM_F_NOTIFY, env, "rt_net_event_flag_notify");
	handle_flag(flags & RTM_F_EQUALIZE, env, "rt_net_event_flag_equalize");
	handle_flag(flags & RTM_F_PREFIX, env, "rt_net_event_flag_prefix");
}
#endif

int handle_rt_message(struct rtmsg const *const message, size_t const message_len, struct env_t **const env)
{
	struct rtattr const *rta;
	size_t len = message_len;

	/* ignore cloned routes */
	if (message->rtm_flags & RTM_F_CLONED) {
		return FALSE;
	}

	add_env_entry(env, "net_event_family", "%u", message->rtm_family);           /* AF_* in sys/socket.h */
	add_env_entry(env, "rt_net_event_routetype", "%u", message->rtm_type);    /* RTN_* in linux/rtnetlink.h */
	add_env_entry(env, "rt_net_event_protocol", "%u", message->rtm_protocol); /* RTPROT_* in linux/rtnetlink.h */
	add_env_entry(env, "rt_net_event_scope", "%u", message->rtm_scope);       /* RT_SCOPE_* in linux/rtnetlink.h */
	add_env_entry(env, "rt_net_event_dst_len", "%u", message->rtm_dst_len);
	add_env_entry(env, "rt_net_event_src_len", "%u", message->rtm_src_len);
	add_env_entry(env, "rt_net_event_tos", "%u", message->rtm_tos);
	add_env_entry(env, "rt_net_event_table", "%u", message->rtm_table);

#if 0 /* not interesting */
	handle_rt_flags(message->rtm_flags, env);
#endif

	for (rta = RTM_RTA(message); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
		switch (rta->rta_type) {
		case RTA_SRC :
			handle_layer3_address(message->rtm_family, rta, env, "rt_net_event_src");
			break;
		case RTA_DST :
			handle_layer3_address(message->rtm_family, rta, env, "rt_net_event_dst");
			break;
		case RTA_GATEWAY :
			handle_layer3_address(message->rtm_family, rta, env, "rt_net_event_gateway");
			break;
		case RTA_IIF :
			handle_u32(rta, env, "rt_net_event_iif");
			break;
		case RTA_OIF :
			handle_u32(rta, env, "rt_net_event_oif");
			break;
		case RTA_PRIORITY :
			handle_u32(rta, env, "rt_net_event_priority");
			break;
		}
	}

	return TRUE;
}
