/*****************************************************************************
 *  @file flicapi/flicapi.h
 *  Generic functions on top of the ISDN/CAPI layer.
 *
 *  Copyright (c) 2012-2016 The fli4l team
 *
 *  This file is part of fli4l.
 *
 *  fli4l is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  fli4l is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with fli4l.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Last Update: $Id$
 *****************************************************************************
 */

#ifndef FLICAPI_H_
#define FLICAPI_H_

#include <sys/types.h>
#include <linux/capi.h>
#include <capi20.h>

#ifndef FALSE
/**
 * The boolean data type.
 */
typedef int BOOL;
/**
 * Represents the boolean value 'false'.
 */
#define FALSE 0
/**
 * Represents the boolean value 'true'.
 */
#define TRUE 1
#endif

/**
 * @name flicapi error codes
 * @{
 */
#define ERR_NO_CAPI_LAYER 1
#define ERR_PROFILE_RETRIEVAL_FAILED 2
#define ERR_NO_CAPI_CONTROLLERS 3
#define ERR_CAPI_REGISTRATION_FAILED 4
#define ERR_CAPI_DEREGISTRATION_FAILED 5
#define ERR_LISTEN_FAILED 6
#define ERR_WAITFORMESSAGE_FAILED 7
#define ERR_GET_MESSAGE_FAILED 8
#define ERR_NO_HANDLER_FOUND 9
/**
 * @}
 */

/**
 * Represents an idle handler.
 */
typedef void (*flicapi_idle_func_t)(void);

/**
 * Represents a CAPI message handler.
 *
 * @param msg
 *  The message to process.
 * @return
 *  Zero on success, a non-zero value on failure which terminates the
 *  application immediately.
 */
typedef int (*flicapi_handler_func_t)(_cmsg *msg);

/**
 * Combines a CAPI message type with a handler processing messages of this
 * type.
 *
 * @param command
 *  The command of the messages the handler is supposed to process.
 * @param subcommand
 *  The subcommand (CAPI_CONF, CAPI_IND) of the messages the handler is
 *  supposed to process.
 * @param handler
 *  The handler which should process messages of the type described above.
 */
typedef struct {
    /**
     * The command to be handled.
     */
    unsigned command;
    /**
     * The subcommand (CAPI_CONF, CAPI_IND) to be handled.
     */
    unsigned subcommand;
    /**
     * The handler to be called.
     */
    flicapi_handler_func_t handler;
} flicapi_handler_t;

/**
 * Initializes the flicapi layer.
 *
 * @param app
 *  The name of the application.
 * @param applId
 *  Receives the application identifier needed for other functions.
 * @param capiCtrl
 *  A pointer to an array of CAPI controller numbers to use. The last entry
 *  must be zero to mark the end of the array. If the pointer is NULL, all
 *  available CAPI controllers will be used.
 * @param use_syslog
 *  TRUE if log messages should be sent to syslog, FALSE if to be sent to
 *  stdout.
 * @return
 *  Zero on success, a non-zero value on failure. In the latter case, it is
 *  neither necessary nor allowed to call flicapi_fini().
 */
int
flicapi_init(char const *app, unsigned *applId, unsigned *capiCtrl, BOOL use_syslog);

/**
 * Processes all outstanding CAPI messages.
 *
 * @param applId
 *  The identifier of the current application, as returned by flicapi_init().
 * @param handlers
 *  The application's message handlers to use.
 * @return
 *  Zero on success, a non-zero value on failure.
 */
int
flicapi_process_messages(unsigned applId, flicapi_handler_t *handlers);

/**
 * Finalizes the flicapi layer.
 *
 * @param applId
 *  The identifier of the current application, as returned by flicapi_init().
 * @return
 *  Zero on success, a non-zero value on failure.
 */
int
flicapi_fini(unsigned applId);

/**
 * Logs a message.
 *
 * @param fmt
 *  The format of the message to be logged.
 * @param ...
 *  Additional arguments, if any.
 */
void
flicapi_log(char const *fmt, ...);

/**
 * Splits a CAPI structure into its length and its data part.
 *
 * @param str
 *  The CAPI structure as part of a CAPI message.
 * @param len
 *  Receives on return the length of the structure's data part in bytes.
 * @return
 *  A pointer to a dynamically allocated memory block containing the
 *  structure's data part. Use free() to release it after use.
 */
char *
flicapi_get_capi_struct(_cstruct str, unsigned *len);

#endif
