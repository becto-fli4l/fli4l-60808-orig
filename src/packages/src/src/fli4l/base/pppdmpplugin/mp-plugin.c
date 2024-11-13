/*
 *
 * $Id$
 *
 * mp-plugin.c - pppd plugin which runs a user-configurable script whenever
 *               a MP link goes online or offline
 *
 * Copyright (c) 2013 Christoph Schulz <fli4l@kristov.de> and the fli4l team
 * Revision: $Id$
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 */
#define HAVE_MULTILINK
#include <pppd/pppd.h>
#include <pppd/patchlevel.h>
#include <pppd/pathnames.h>

#define _PATH_MPLINKUP   _ROOT_PATH "/etc/ppp/link-up"
#define _PATH_MPLINKDOWN _ROOT_PATH "/etc/ppp/link-down"

#ifdef VERSION
char pppd_version[] = VERSION;
#endif

/*
 * This is used to ensure that we don't start a link-up/down
 * script while one is already running.
 */
enum script_state {
    s_down,
    s_up
};

static enum script_state mplink_state = s_down;
static enum script_state mplink_script_state = s_down;
static pid_t mplink_script_pid = 0;

static void (*old_multilink_join_hook)(void) = NULL;
static void (*old_multilink_exit_hook)(void) = NULL;

static void mplink_script(void);
static void mplink_script_done(void *);

static void
mplink_script_done(void *arg)
{
    (void) arg;
    mplink_script_pid = 0;
    switch (mplink_script_state) {
    case s_up:
        if (mplink_state == s_down) {
            mplink_script_state = s_down;
            mplink_script();
        }
        break;
    case s_down:
        if (mplink_state == s_up) {
            mplink_script_state = s_up;
            mplink_script();
        }
        break;
    }
}

static void mplink_script(void)
{
    char *argv[5];

    if (mplink_script_state == s_up) {
        argv[0] = _PATH_MPLINKUP;
        argv[1] = ifname;
        argv[2] = ipparam;
        /* 1 == $BUNDLE_MASTER_TYPE_REAL, see /etc/rc.d/bundle-common */
        argv[3] = multilink_master ? "1" : "0";
        argv[4] = NULL;
    }
    else {
        argv[0] = _PATH_MPLINKDOWN;
        argv[1] = ifname;
        argv[2] = ipparam;
        argv[3] = NULL;
    }

    mplink_script_pid = run_program(argv[0], argv, 0, mplink_script_done, NULL, 0);
}

static void mplink_join_hook(void)
{
    if (old_multilink_join_hook) {
        (*old_multilink_join_hook)();
    }

    mplink_state = s_up;
    if (mplink_script_state == s_down && mplink_script_pid == 0) {
        mplink_script_state = s_up;
        mplink_script();
    }
}

static void mplink_exit_hook(void)
{
    mplink_state = s_down;
    if (mplink_script_state == s_up && mplink_script_pid == 0) {
        mplink_script_state = s_down;
        mplink_script();
    }

    if (old_multilink_exit_hook) {
        (*old_multilink_exit_hook)();
    }
}

void plugin_init(void)
{
    old_multilink_join_hook = multilink_join_hook;
    multilink_join_hook = mplink_join_hook;

    old_multilink_exit_hook = multilink_exit_hook;
    multilink_exit_hook = mplink_exit_hook;

    info("Multilink PPP plugin");
}
