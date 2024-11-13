/*!
 * $Id$
 *
 * @file    button_gpio.c
 * @brief   implementation of GPIO button handler for hwsuppd deamon
 *
 * @author  Carsten Spie&szlig; fli4l@carsten-spiess.de
 * @date    17.08.2013
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>

#include "hwsuppd.h"

/*!
 * @fn          static int get_button (int num)
 * @brief       get button status
 * @details     read from file matching inputDev
 * @param[in]   inputDev
 *              path to the GPIO value
 * @param[in]   num
 *              button number
 * @return      button status
 * @retval      true
 *              button released
 * @retval      false
 *              button pressed
 */
static bool get_button(char const *inputDev, gpioButtonT *btn)
{
    int fd;
    unsigned char state;

    if ((fd = open(inputDev, O_RDONLY)) < 0)
    {
        log_printf(LOG_ERR, "error opening %s: %s, ignoring button", inputDev, strerror(errno));
        return btn->state;
    }

    if (read(fd, &state, sizeof(state)) < 0)
    {
        close(fd);
        log_printf(LOG_ERR, "error reading button %d: %s, ignoring button", btn->button.nIndex, strerror(errno));
        return btn->state;
    }

    close(fd);
    return state != '0';
}

/*!
 * @fn          void button_read (int num)
 * @brief       read button
 * @details     read button state and execute command when button is pressed
 * @param[in]   num
 *              button number
 */
static void gpio_readButton(char const *inputDev, gpioButtonT *btn)
{
    // read status of pushbutton switch
    // true = released
    // false = pressed
    bool old_status = btn->state;

    btn->state = get_button(inputDev, btn);
    if (old_status && !btn->state)
    {
        execute_command (btn->button.nIndex);
    }
}

/**
 * @fn          static void gpio_readFromDevice(deviceT *dev)
 * @brief       checks whether a button attached via a GPIO line has been
 *              pressed
 * @param[in]   dev
 *              the device object
 * @details     calls gpio_readButton() to do the real work
 */
static void gpio_readFromDevice(deviceT *dev)
{
    buttonT *btn = dev->firstButton;
    if (btn != NULL) {
        gpio_readButton(dev->inputDev, (gpioButtonT *) btn);
    }
}

/**
 * @fn          static void gpio_destroyDevice(deviceT *dev)
 * @brief       destroys the device object
 * @param[in]   dev
 *              the device object
 */
static void gpio_destroyDevice(deviceT *dev)
{
    free(dev->inputDev);
    free((gpioDeviceT *) dev);
}

/**
 * @fn          gpioDeviceT *gpio_createDevice(char const *inputDev)
 * @brief       creates a GPIO device object
 * @param[in]   inputDev
 *              path to the /sys/class/gpio/.../value file; this string is
 *              copied so the caller may reuse the memory after returning
 * @return      the GPIO device object or NULL if an error occurs
 */
gpioDeviceT *gpio_createDevice(char const *inputDev)
{
    gpioDeviceT *device = (gpioDeviceT *) malloc(sizeof(gpioDeviceT));
    if (device == NULL) {
        return NULL;
    }

    memset(device, 0, sizeof(*device));
    device->dev.type      = DEVICE_GPIO;
    device->dev.inputDev  = strdup(inputDev);
    device->dev.doRead    = gpio_readFromDevice;
    device->dev.doDestroy = gpio_destroyDevice;
    return device;
}

/**
 * @fn          static void gpio_destroyButton(buttonT *button)
 * @brief       destroys the button object
 * @param[in]   button
 *              the button object
 */
static void gpio_destroyButton(buttonT *button)
{
    free((gpioButtonT *) button);
}

/**
 * @fn          gpioButtonT *gpio_createButton(gpioDeviceT *device, unsigned nIndex)
 * @brief       creates a button object attached to a GPIO device
 * @param[in]   device
 *              the GPIO device object
 * @param[in]   nIndex
 *              the absolute index of the button (zero-based)
 * @return      the button device object or NULL if an error occurs
 */
gpioButtonT *gpio_createButton(gpioDeviceT *device, unsigned nIndex)
{
    gpioButtonT *button = (gpioButtonT *) malloc(sizeof(gpioButtonT));
    if (button == NULL) {
        return NULL;
    }

    memset(button, 0, sizeof(*button));
    button->button.dev       = &device->dev;
    button->button.nIndex    = nIndex;
    button->button.doDestroy = gpio_destroyButton;
    button->state            = true;
    return button;
}

/*_oOo*/
