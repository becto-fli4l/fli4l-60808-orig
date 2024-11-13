#if !defined(MOM_NETLINK_TYPE_BASIC_H_)
#define MOM_NETLINK_TYPE_BASIC_H_

#include "netlink.h"
#include "env.h"

void handle_flag(int const flag, struct env_t **const env, char const *const var);
void handle_u8(struct rtattr const *const rta, struct env_t **const env, char const *const var);
void handle_u32(struct rtattr const *const rta, struct env_t **const env, char const *const var);
void handle_x32(struct rtattr const *const rta, struct env_t **const env, char const *const var);
void handle_string(struct rtattr const *rta, struct env_t **const env, char const *const var);

#endif
