/**
 * @file env.h
 * General definitions.
 */
#if !defined(MOM_NETLINK_DEFS_H_)
#define MOM_NETLINK_DEFS_H_

#include <signal.h>

#define FALSE 0 /**< boolean false */
#define TRUE 1  /**< boolean true */

/**
 * Calculates the number of elements in an array.
 * @param x the array
 * @return the number of elements in the array
 */
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

/**
 * Set to TRUE if SIGINT is received. Initially set to FALSE.
 */
extern sig_atomic_t do_exit;

#endif
