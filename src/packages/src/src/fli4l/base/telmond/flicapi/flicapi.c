/*****************************************************************************
 *  @file flicapi/flicapi.c
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

#include <flicapi/flicapi.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/select.h>

#define MAX_LOGICAL_CONNECTIONS 1
#define MAX_DATA_BLOCKS 2
#define MAX_DATA_BLOCKSIZE 128

#define MAX_GET_MESSAGE_TIMEOUT 100

/**
 * The application's name.
 */
static char const *app_name = "";
/**
 * TRUE if logging to syslog has been requested, FALSE otherwise.
 */
static BOOL log_to_syslog = FALSE;

/**
 * Returns the length of a CAPI structure.
 *
 * @param str
 *  A pointer to a CAPI structure. On return it points to the first byte of the
 *  structure's data part.
 * @return
 *  The length of the structure in bytes.
 */
static unsigned
extract_capi_struct_len(_cstruct *str)
{
    unsigned char first = *(*str)++;
    if (first == 0xff) {
        unsigned result = *(_cword *)(*str);
        (*str) += sizeof(_cword);
        return result;
    }
    else {
        return first;
    }
}

char *
flicapi_get_capi_struct(_cstruct str, unsigned *len)
{
    char *buf;
    *len = extract_capi_struct_len(&str);
    buf = (char *) malloc(*len);
    if (!buf) {
        *len = 0;
        return NULL;
    }
    else {
        memcpy(buf, str, *len);
        return buf;
    }
}

void
flicapi_log(char const *fmt, ...)
{
    va_list ap;
    if (log_to_syslog) {
        va_start(ap, fmt);
        vsyslog(LOG_INFO, fmt, ap);
        va_end(ap);
    }
    else {
        printf("%s: ", app_name);
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
}

/**
 * Handles a LISTEN_CONF message. Such a message is received at the very
 * beginning for each ISDN controller the flicapi module has registered the
 * application with by sending a LISTEN_REQ message.
 *
 * @param cmsg
 *  The LISTEN_CONF message.
 */
static int
handleListenCONF(_cmsg *cmsg)
{
    _cword const info = LISTEN_CONF_INFO(cmsg);
    if (info != 0) {
        flicapi_log("LISTEN_CONF received for controller %u: %s\n",
                LISTEN_CONF_CONTROLLER(cmsg), capi_info2str(info));
        return ERR_LISTEN_FAILED;
    }
    else {
        flicapi_log("LISTEN_CONF received for controller %u.\n",
                LISTEN_CONF_CONTROLLER(cmsg));
        return 0;
    }
}

/**
 * Predefined handlers used by flicapi.
 */
static flicapi_handler_t flicapi_handlers[] = {
    { CAPI_LISTEN, CAPI_CONF, &handleListenCONF, },
};

static int
flicapi_process_handlers(_cmsg *msg, flicapi_handler_t *handlers)
{
    flicapi_handler_t *handler = handlers;
    while (handler && handler->handler) {
        if (handler->command == msg->Command
                && handler->subcommand == msg->Subcommand) {
            return (*handler->handler)(msg);
        }
        else {
            ++handler;
        }
    }

    return ERR_NO_HANDLER_FOUND;
}

int
flicapi_process_messages(unsigned applId, flicapi_handler_t *handlers)
{
    int ret = 0;

    while (ret == 0) {
        _cmsg cmsg;
        unsigned reason;
        struct timeval tv;

        tv.tv_sec = 0;
        tv.tv_usec = MAX_GET_MESSAGE_TIMEOUT * 1000;
        reason = CAPI20_WaitforMessage(applId, &tv);

        if (reason == CapiReceiveQueueEmpty) {
            return 0;
        }
        else if (reason != 0) {
            flicapi_log("WAITFORMESSAGE failed: %s\n",
                    capi_info2str(reason));
            return ERR_WAITFORMESSAGE_FAILED;
        }

        reason = CAPI_GET_CMSG(&cmsg, applId);
        if (reason != 0) {
            flicapi_log("GET_MESSAGE failed: %s\n",
                    capi_info2str(reason));
            return ERR_GET_MESSAGE_FAILED;
        }

        ret = flicapi_process_handlers(&cmsg, flicapi_handlers);
        if (ret == ERR_NO_HANDLER_FOUND) {
            ret = flicapi_process_handlers(&cmsg, handlers);
        }

        if (ret == ERR_NO_HANDLER_FOUND) {
            flicapi_log("Unhandled CAPI message: %u, %u.\n",
                    cmsg.Command, cmsg.Subcommand);
            ret = 0;
        }
    }

    return ret;
}

int
flicapi_init(char const *app, unsigned *applId, unsigned *capiCtrl, BOOL use_syslog)
{
    capi_profile profile;
    unsigned ctrl;
    unsigned reason;
    BOOL listenSucceeded = FALSE;

    app_name = app;
    log_to_syslog = use_syslog;

    if ((reason = CAPI20_ISINSTALLED()) != 0) {
        flicapi_log("No CAPI layer available: %s\n", capi_info2str(reason));
        return ERR_NO_CAPI_LAYER;
    }

    profile.ncontroller = 0;
    if ((reason = CAPI20_GET_PROFILE(0, (unsigned char*) &profile)) != 0) {
        flicapi_log("Failed to retrieve CAPI profile: %s\n",
                capi_info2str(reason));
        return ERR_PROFILE_RETRIEVAL_FAILED;
    }

    if (profile.ncontroller == 0) {
        flicapi_log("No CAPI controllers available.\n");
        return ERR_NO_CAPI_CONTROLLERS;
    }

    flicapi_log("%u controller%s available.\n",
            profile.ncontroller, profile.ncontroller == 1 ? "" : "s");

    if ((reason = CAPI20_REGISTER(MAX_LOGICAL_CONNECTIONS, MAX_DATA_BLOCKS,
                                    MAX_DATA_BLOCKSIZE, applId)) != 0) {
        flicapi_log("Failed to register application at CAPI layer: %s\n",
                capi_info2str(reason));
        return ERR_CAPI_REGISTRATION_FAILED;
    }

    for (ctrl = 1; ctrl <= profile.ncontroller; ++ctrl) {
        unsigned *entry = capiCtrl;
        while (entry && *entry && *entry != ctrl) {
            ++entry;
        }

        if (!entry || *entry == ctrl) {
            _cmsg listen_req;
            unsigned char cpn = 0;
            LISTEN_REQ_HEADER(&listen_req, *applId, ctrl, ctrl);
            LISTEN_REQ_INFOMASK(&listen_req) = 0x0;
            LISTEN_REQ_CIPMASK(&listen_req) = 0x1;
            LISTEN_REQ_CIPMASK2(&listen_req) = 0x0;
            LISTEN_REQ_CALLINGPARTYNUMBER(&listen_req) = &cpn;
            LISTEN_REQ_CALLINGPARTYSUBADDRESS(&listen_req) = &cpn;
            if ((reason = CAPI_PUT_CMSG(&listen_req)) != 0) {
                flicapi_log(
                    "Failed to listen for CAPI events on controller %u: %s\n",
                    ctrl,
                    capi_info2str(reason)
                );
            }
            else {
                flicapi_log("Listening for events on controller %u.\n", ctrl);
                listenSucceeded = TRUE;
            }
        }
    }

    if (listenSucceeded) {
        return 0;
    }
    else {
        flicapi_fini(*applId);
        return ERR_LISTEN_FAILED;
    }
}

int
flicapi_fini(unsigned applId)
{
    unsigned reason;
    if ((reason = CAPI20_RELEASE(applId)) != 0) {
        flicapi_log("Failed to deregister application at CAPI layer: %s\n",
                capi_info2str(reason));
        return ERR_CAPI_DEREGISTRATION_FAILED;
    }
    else {
        return 0;
    }
}
