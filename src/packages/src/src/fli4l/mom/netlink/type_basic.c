#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "type_basic.h"

void handle_flag(int const flag, struct env_t **const env, char const *const var)
{
	if (flag) {
		add_env_entry(env, var, "1");
	}
}

void handle_u8(struct rtattr const *const rta, struct env_t **const env, char const *const var)
{
	if (RTA_PAYLOAD(rta) >= sizeof(__u8))
	{
		add_env_entry(env, var, "%u", *(__u8 *) RTA_DATA(rta));
	}
}

void handle_u32(struct rtattr const *const rta, struct env_t **const env, char const *const var)
{
	if (RTA_PAYLOAD(rta) >= sizeof(__u32))
	{
		add_env_entry(env, var, "%u", *(__u32 *) RTA_DATA(rta));
	}
}

void handle_x32(struct rtattr const *const rta, struct env_t **const env, char const *const var)
{
	if (RTA_PAYLOAD(rta) >= sizeof(__u32))
	{
		add_env_entry(env, var, "%x", *(__u32 *) RTA_DATA(rta));
	}
}

void handle_string(struct rtattr const *rta, struct env_t **const env, char const *const var)
{
	const size_t buflen = RTA_PAYLOAD(rta);
	char *const buf = (char *) malloc(buflen);
	strncpy(buf, RTA_DATA(rta), buflen);
	buf[buflen - 1] = '\0';
	add_env_entry(env, var, "%s", buf);
	free(buf);
}
