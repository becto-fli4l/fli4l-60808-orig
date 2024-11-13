/**
 * @file env.c
 * Environment structure and associated routines.
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "defs.h"
#include "env.h"

int add_env_entry(struct env_t **const env, char const *const name, char const *const fmt, ...)
{
	va_list ap;
	struct env_t *const e = (struct env_t *) malloc(sizeof(struct env_t));
	int rc;
	e->name = name;
	e->next = *env;
	va_start(ap, fmt);
	rc = vasprintf(&e->value, fmt, ap);
	va_end(ap);
	if (rc < 0) {
		free(e);
		return FALSE;
	} else {
		setenv(e->name, e->value, TRUE);
		*env = e;
		return TRUE;
	}
}

void clear_env(struct env_t **const env)
{
	while (*env) {
		struct env_t *e = *env;
		unsetenv(e->name);
		free(e->value);
		*env = e->next;
		free(e);
	}
}
