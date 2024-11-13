/*!
 * $Id$
 *
 * @file    button_evdev.c
 * @brief   implementation of evdev button handler for hwsuppd deamon
 *
 * @author  Christoph Schulz (develop@kristov.de)
*/

#include <libevdev/libevdev.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "hwsuppd.h"

/**
 * @fn          static void evdev_readFromDevice(deviceT *dev)
 * @brief       checks whether a button attached via evdev has been pressed.
 * @param[in]   dev
 *              the device object
 * @details     uses libevdev to retrieve keys and tries to find buttons
 *              attached to these keys
 */
static void evdev_readFromDevice(deviceT *dev)
{
    evdevDeviceT *evdev = (evdevDeviceT *) dev;
    buttonT *btn = dev->firstButton;
    struct input_event ev;
    int rc;

    if (libevdev_has_event_pending(evdev->evdev) <= 0) {
        return;
    }

    rc = libevdev_next_event(evdev->evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    if (rc == LIBEVDEV_READ_STATUS_SUCCESS && ev.type == EV_KEY && ev.value != 0) {
        while (btn != NULL) {
            evdevButtonT *evdevBtn = (evdevButtonT *) btn;
            if (evdevBtn->key == ev.code) {
                execute_command(btn->nIndex);
            }
            btn = btn->next;
        }
    }
}

/**
 * @fn          static void evdev_destroyDevice(deviceT *dev)
 * @brief       destroys the device object
 * @param[in]   dev
 *              the device object
 * @details     frees the underlying libevdev object and closes the file handle
 */
static void evdev_destroyDevice(deviceT *dev)
{
    evdevDeviceT *evdev = (evdevDeviceT *) dev;
    libevdev_free(evdev->evdev);
    close(evdev->fd);
    free(dev->inputDev);
    free(evdev);
}

/**
 * @fn          evdevDeviceT *evdev_createDevice(char const *inputDev)
 * @brief       creates an evdev device
 * @param[in]   inputDev
 *              path to the /dev/input/event... file; this string is copied so
 *              the caller may reuse the memory after returning
 * @return      the evdev device object or NULL if an error occurs
 * @details     the device is opened in non-blocking mode and attached to an
 *              evdev object
 */
evdevDeviceT *evdev_createDevice(char const *inputDev)
{
    evdevDeviceT *device;
    struct libevdev *evdev;

    int fd = open(inputDev, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        return NULL;
    }

    if ((libevdev_new_from_fd(fd, &evdev)) < 0) {
        close(fd);
        return NULL;
    }

    device = (evdevDeviceT *) malloc(sizeof(evdevDeviceT));
    if (device == NULL) {
        libevdev_free(evdev);
        close(fd);
        return NULL;
    }

    memset(device, 0, sizeof(*device));
    device->dev.type      = DEVICE_EVDEV;
    device->dev.inputDev  = strdup(inputDev);
    device->dev.doRead    = evdev_readFromDevice;
    device->dev.doDestroy = evdev_destroyDevice;
    device->fd            = fd;
    device->evdev         = evdev;
    return device;
}

/**
 * @fn          static void evdev_destroyButton(buttonT *button)
 * @brief       destroys the button object
 * @param[in]   button
 *              the button object
 */
static void evdev_destroyButton(buttonT *button)
{
    free((evdevButtonT *) button);
}

/**
 * @fn          evdevButtonT *evdev_createButton(evdevDeviceT *device, unsigned nIndex, unsigned key)
 * @brief       creates a button object attached to an evdev device
 * @param[in]   device
 *              the evdev device object
 * @param[in]   nIndex
 *              the absolute index of the button (zero-based)
 * @param[in]   key
 *              the evdev key to associate with this button (e.g. KEY_RESTART)
 * @return      the button device object or NULL if an error occurs
 */
evdevButtonT *evdev_createButton(evdevDeviceT *device, unsigned nIndex, unsigned key)
{
    evdevButtonT *button = (evdevButtonT *) malloc(sizeof(evdevButtonT));
    if (button == NULL) {
        return NULL;
    }

    memset(button, 0, sizeof(*button));
    button->button.dev       = &device->dev;
    button->button.nIndex    = nIndex;
    button->button.doDestroy = evdev_destroyButton;
    button->key              = key;
    return button;
}

/*_oOo*/
