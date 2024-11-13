/*!
 * $Id$
 *
 * @file        hwsuppd.h
 * @brief       implementation of hwsuppd deamon
 * @details     program to control push button switches
 *
 *              see @ref usage for command line parameters
 *
 * @author  Carsten Spie&szlig; fli4l@carsten-spiess.de
 *          and the fli4l developer-team
 * @date    17.08.2013
 *
 * @note
 * portions based on
 * (c) 2005 by Torsten Georg <fli4l<at>foddy.de>
 *
 * (c) Copyright 2006 Stefan Krister <stefan@fli4l.de>
 * and the fli4l developer-team, All Rights Reserved.
 *
 * @note
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 *
 * I don't accept liability nor provide warranty for this Software.
 * This material is provided "AS-IS" and at no charge.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>

#include "hwsuppd.h"

static void usage (char *argv);
static int parse_args (int argc, char **argv);
static void main_loop (void);
static void sig_handler(int signr);
static void check_for_zombies (void);

/*!
 * @var         g_data
 * @brief       global data structure
 */
dataT g_data = {
        .firstDevice = NULL,
        .fExecute = 1,
        .fForeground = 0,
        .fDaemonized = 0,
        .fDebug = 0
};

/*!
 * @fn          static void add_device(deviceT **head, deviceT *dev)
 * @brief       adds a device to a singly-linked list of devices
 * @param[in]   head
 *              points to the head pointer of the linked list of devices
 * @param[in]   dev
 *              device to be added; it is put at the front of the list
 */
static void add_device(deviceT **head, deviceT *dev)
{
    dev->next = *head;
    *head = dev;
}

/*!
 * @fn          static deviceT *find_device(deviceT *head, deviceTypeT type, char const *inputDev)
 * @brief       searches for and returns a device for a given input device file
 * @param[in]   head
 *              points to the head of the linked list of devices
 * @param[in]   type
 *              the type of device to search for
 * @param[in]   inputDev
 *              path to the input device file
 * @return      a pointer to the device or NULL if not found
 */
static deviceT *find_device(deviceT *head, deviceTypeT type, char const *inputDev)
{
    while (head != NULL) {
        if (head->type == type && !strcmp(head->inputDev, inputDev)) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

/*!
 * @fn          static void add_button(buttonT **head, buttonT *button)
 * @brief       adds a button to a singly-linked list of buttons
 * @param[in]   head
 *              points to the head pointer of the linked list of buttons
 * @param[in]   button
 *              button to be added; it is put at the front of the list
 */
static void add_button(buttonT **head, buttonT *button)
{
    button->next = *head;
    *head = button;
}

/*!
 * @fn          static int usage (char *argv)
 * @brief       print usage information
 * @param[in]   argv
 *              programm name
 */
static void usage (char *argv)
{
    printf("--------------------------------------------------------------\n");
    printf(" hwsuppd usage\n\n");
    printf(" Usage: %s [-d] [-f] <button-options...> \n",
            argv);
    printf("  -d:     debug\n");
    printf("  -f:     foreground\n");
    printf("  <button options...>: one of\n");
    printf("    -g <device>:        connect to GPIO <device>\n");
    printf("    -e <device>:<key>: connect to evdev input <device> and wait for <key>\n");
    printf("--------------------------------------------------------------\n");
}

/*!
 * @fn          int main (int argc, char **argv)
 * @brief       programm entry function
 * @param[in]   argc
 *              number of commandline arguments
 * @param[in]   argv
 *              commandline arguments
 * @return      result code
 */
int main (int argc, char **argv)
{
    int result;

    result =  parse_args (argc, argv);
    if (result < 0)
    {
        goto exit;
    }

    if (!g_data.fForeground)
    {
        if (daemon(1, 1) == -1)
        {
            log_printf(LOG_ERR, "cannot detach");
            exit(1);
        }
        g_data.fDaemonized = 1;
    }

    openlog("hwsuppd", LOG_PID, LOG_USER);

    if (0 == result)
    {
        main_loop ();
    }

exit:
    return result;
}

/*!
 * @fn          static void main_loop ()
 * @brief       main programm loop
 * @details     read buttons and execute external command
 */
static void main_loop (void)
{
    log_printf (LOG_INFO, "main loop entered");

    signal (SIGINT, sig_handler);
    signal (SIGTERM, sig_handler);

    while (g_data.fExecute)
    {
        deviceT *dev;

        check_for_zombies();

        // handle devices
        for (dev = g_data.firstDevice; dev != NULL; dev = dev->next)
        {
            dev->doRead(dev);
        }
        usleep(10000); // wait 10ms
    }
    log_printf (LOG_INFO, "main loop finished");
}

/*!
 * @fn          static void sig_handler(int signr)
 * @brief       signal handler
 * @details     terminate main loop on SIGINT and SIGTERM
 * @param       signr
 *              signal number
 */
static void sig_handler(int signr)
{
    log_printf (LOG_INFO, "signal %d received",signr);
    switch (signr)
    {
        case SIGINT:
        case SIGTERM:
            g_data.fExecute = 0;
            break;
    }
}

/*!
 * @fn          static int parse_args (int argc, char **argv)
 * @brief       parse command line arguments
 * @param[in]   argc
 *              number of commandline arguments
 * @param[in]   argv
 *              commandline arguments
 * @return      result code
 * @retval      0
 *              o.k.
 * @retval      -1
 *              failure, exit program
 */
static int parse_args (int argc, char **argv)
{
    int result = 0;
    char c;
    unsigned nIndex = 0;

    while ((c = getopt(argc, argv, "fdig:e:")) != -1)
    {
        deviceT *device;
        switch (c)
        {
            case 'd':
                g_data.fDebug = 1;
                break;
            case 'f':
                g_data.fForeground = 1;
                break;
            case 'g':
                if ((device = find_device(g_data.firstDevice, DEVICE_GPIO,
                        optarg)) != NULL) {
                    fprintf(stderr, "%s: GPIO device %s cannot be used more than once\n",
                            argv[0], optarg);
                } else {
                    gpioDeviceT *dev = gpio_createDevice(optarg);
                    gpioButtonT *btn = dev ? gpio_createButton(dev, nIndex)
                            : NULL;

                    if (dev == NULL) {
                        fprintf(stderr, "%s: Could not create GPIO device object for %s: %s\n",
                                argv[0], optarg, strerror(errno));
                    } else if (btn == NULL) {
                        fprintf(stderr, "%s: Could not create GPIO button object for %s: %s\n",
                                argv[0], optarg, strerror(errno));
                    } else {
                        add_device(&g_data.firstDevice, &dev->dev);
                        add_button(&dev->dev.firstButton, &btn->button);
                    }
                }
                nIndex++;
                break;
            case 'e':
                {
                    char *colon = strrchr(optarg, ':');
                    if (colon == NULL) {
                        fprintf(stderr, "%s: Key missing in EVDEV device: %s\n",
                                argv[0], optarg);
                    } else {
                        evdevDeviceT *dev;
                        evdevButtonT *btn;
                        unsigned key;

                        *colon = '\0';
                        key = atoi(colon + 1);
                        device = find_device(g_data.firstDevice, DEVICE_EVDEV,
                                optarg);
                        dev = device ? (evdevDeviceT *) device
                                : evdev_createDevice(optarg);
                        btn = dev ? evdev_createButton(dev, nIndex, key)
                                : NULL;

                        if (dev == NULL) {
                            fprintf(stderr, "%s: Could not create EVDEV device object for %s and key %u: %s\n",
                                    argv[0], optarg, key, strerror(errno));
                        } else if (btn == NULL) {
                            fprintf(stderr, "%s: Could not create EVDEV button object for %s and key %u: %s\n",
                                    argv[0], optarg, key, strerror(errno));
                        } else {
                            add_device(&g_data.firstDevice, &dev->dev);
                            add_button(&dev->dev.firstButton, &btn->button);
                        }
                    }
                }
                nIndex++;
                break;
            default:
                usage (argv[0]);
                result = -1;
                break;
        }
    }


    if (optind > argc)
    {
        usage (argv[0]);
        result = -1;
    }

    return result;
}

/*!
 * @fn          void execute_command (int num)
 * @brief       exeucte command on button press
 * @details     exceute /usr/bin/hwsupp_cmd <num>
 *              with button number <num>
 *              the user is responsible to check the arg <num> in the
 *              hwsupp_cmd script
 */
void execute_command (int num)
{
    int result;
    char param[10];
    char* command[3] = { "/usr/bin/hwsupp_cmd", param, NULL};
    snprintf(param, sizeof(param), "%d", num + 1);

    log_printf(LOG_INFO, "button %d pressed, executing command \"%s %s\"",
            num, command[0], command[1]);

    if (g_data.pid)
    {
        return;
    }

    g_data.pid = fork();
    switch (g_data.pid)
    {
        case -1:
            log_printf(LOG_ERR, "unable to fork: %s", strerror(errno));
            g_data.pid = 0;
            break;
        case 0:
            result = execvp(command[0], command);
            if (result < 0)
            {
                log_printf(LOG_ERR, "unable to exec %s: %s", command[0],
                    strerror(errno));
            }
            _exit (1);
    }
}

/*!
 * @fn          static void check_for_zombies (void)
 * @brief       check for unterminated execute processes
 */
static void check_for_zombies (void)
{
    int status, result;

    if (!g_data.pid)
        return;

    result = waitpid(-1, &status, WNOHANG);
    if (result < 0)
    {
        log_printf(LOG_ERR, "waitpid returned %s\n", strerror(errno));
    }
    else if (result > 0)
    {
        if (WIFEXITED(status))
        {
            log_printf(LOG_INFO, "%d terminated with %d\n", result,
                    WEXITSTATUS(status));
        }
        else
        {
            log_printf(LOG_INFO, "%d terminated abnormal\n", result);
        }
        g_data.pid = 0;
    }
}

/*_oOo_*/
