/**
 * @file env.h
 * Environment structure and associated routines.
 */
#if !defined(MOM_NETLINK_ENV_H_)
#define MOM_NETLINK_ENV_H_

/**
 * Represents a variable assignment. The entries are stored in a linked list.
 */
struct env_t {
	char const *name; /**< name of variable */
	char *value; /**< value of variable, dynamically allocated by malloc() */
	struct env_t *next; /**< pointer to next entry */
};

/**
 * Adds a variable to the process's environment and remembers the variable
 * assignment.
 * @param env the variable assignment to modify
 * @param name the name of the variable
 * @param fmt the value of the variable, can contain format specifiers
 * @param ... the arguments to the format specifiers (if any)
 * @return TRUE on success, FALSE otherwise
 */
int add_env_entry(struct env_t **env, char const *name, char const *fmt, ...)
#if defined(__GNUC__)
__attribute__ ((format (printf, 3, 4)))
#endif
;

/**
 * Clears a variable assignment and removes its variables from the process's
 * environment.
 * @param env the variable assignment
 */
void clear_env(struct env_t **env);

#endif
