/**
 * @file    include/netsrv/error.h
 * NetSrv: Error handling.
 * @author  Christoph Schulz
 * @since   2004-11-08
 * @version $Id$
 */

#ifndef INC_NETSRV_ERROR_H_
#define INC_NETSRV_ERROR_H_

#include <stddef.h>
#include <errno.h>
#include <netsrv/defs.h>
#include <netsrv/except.h>

/**
 * @def SYSCALL(var, exp, errorcode)
 * Evaluates exp. In case of an error (== -1) it evaluates errno, prints a
 * corresponding error message, and returns errorcode. Otherwise, the result is
 * stored in var.
 * @param var The variable to store the result of the expression to.
 * @param exp The expression to be evaluated. Most notably a system call.
 * @param errorcode The error code to return if the expression evaluates to -1.
 */
/**
 * @def SYSCALLINTR(var, exp, errorcode)
 * Similar to SYSCALL. The difference: EINTR is not seen as an error.
 * @see SYSCALL
 */
/**
 * @def ERROR(message, errorcode)
 * Prints message and returns errorcode. In non-debug mode, nothing is printed.
 */

#ifdef DEBUG
#define SYSCALL(var, exp, errorcode) do { var = (exp); if (var == -1) THROW (netsrv_scerror (#exp, errorcode)); } while (0)
#define SYSCALLINTR(var, exp, errorcode) do { var = (exp); if (var == -1 && errno != EINTR) THROW (netsrv_scerror (#exp, errorcode)); } while (0)
#define ERROR(message, errorcode) THROW (netsrv_error (message, errorcode))
#else
#define SYSCALL(var, exp, errorcode) do { var = (exp); if (var == -1) THROW (netsrv_scerror ("", errorcode)); } while (0)
#define SYSCALLINTR(var, exp, errorcode) do { var = (exp); if (var == -1 && errno != EINTR) THROW (netsrv_scerror ("", errorcode)); } while (0)
#define ERROR(message, errorcode) THROW (errorcode)
#endif

/**
 * Prints message and returns errorcode. The message is printed by the means of
 * perror().
 * @param message The message to print.
 * @param errorcode The error code to return.
 */
int netsrv_scerror (const char *message, int errorcode);

/**
 * Prints message and returns errorcode.
 * @param message The message to print.
 * @param errorcode The error code to return.
 */
int netsrv_error (const char *message, int errorcode);

/** Error codes returned by various operations. */
enum
{
	ERROR_FORK1 = 1,
	ERROR_SETSID,
	ERROR_FORK2,
	ERROR_CHDIR,
	ERROR_SIGACTION,
	ERROR_ADD_SERVICE,
	ERROR_SOCKET,
	ERROR_SETSOCKOPT,
	ERROR_BIND,
	ERROR_LISTEN,
	ERROR_SELECT,
	ERROR_ACCEPT,
	ERROR_ACCEPT_FORK,
	ERROR_WAITPID,
	ERROR_READ,
	ERROR_WRITE
};

#endif
