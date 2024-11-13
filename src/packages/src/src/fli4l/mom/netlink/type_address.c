#include <arpa/inet.h>
#include <stdlib.h>

#include "defs.h"
#include "type_address.h"

void handle_layer2_address(unsigned const type, struct rtattr const *const rta, struct env_t **const env, char const *const var)
{
	switch (type) { /* ARPHRD_* from linux/if_arp.h */
	case ARPHRD_ETHER :
		handle_layer2_ethernet_address(rta, env, var);
		break;
	}
}

void handle_layer2_ethernet_address(struct rtattr const *const rta, struct env_t **const env, char const *const var)
{
	if (RTA_PAYLOAD(rta) >= 6)
	{
		unsigned char const *const mac = (unsigned char const *) RTA_DATA(rta);
		add_env_entry(env, var, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
}

void handle_layer3_address(unsigned const family, struct rtattr const *const rta, struct env_t **const env, char const *const var)
{
	if ((family == AF_INET && RTA_PAYLOAD(rta) >= 4) || (family == AF_INET6 && RTA_PAYLOAD(rta) >= 16))
	{
		const size_t buflen = family == AF_INET ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN;
		char *const buf = (char *) malloc(buflen);
		if (inet_ntop(family, RTA_DATA(rta), buf, buflen)) {
			add_env_entry(env, var, "%s", buf);
		}
		free(buf);
	}
}
