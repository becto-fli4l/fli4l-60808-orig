#if !defined(MOM_NETLINK_TYPE_ADDRESS_H_)
#define MOM_NETLINK_TYPE_ADDRESS_H_

#include "netlink.h"
#include "env.h"

void handle_layer2_address(unsigned const type, struct rtattr const *const rta, struct env_t **const env, char const *const var);
void handle_layer2_ethernet_address(struct rtattr const *const rta, struct env_t **const env, char const *const var);
void handle_layer3_address(unsigned const family, struct rtattr const *const rta, struct env_t **const env, char const *const var);

#endif
