/*!
 * $Id$
 *
 * @file    hwsuppd.h
 * @brief   definitions for hwsuppd deamon
 *
 * @author  Carsten Spie&szlig; fli4l@carsten-spiess.de
 * @date    17.08.2013
 *
 * @note
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 *
 * I don't accept liability nor provide warranty for this Software.
 * This material is provided "AS-IS" and at no charge.
 */

#ifndef _HWSUPPD_H_
#define _HWSUPPD_H_

#include <stdbool.h>

struct _buttonT;
struct _deviceT;
struct libevdev;

/*!
 * @brief       Type of "destroy" operation on buttons.
 * @param[in]   button
 *              the button to destroy
 */
typedef void (*destroyButtonT)(struct _buttonT *button);

/*!
 * @struct      buttonT
 * @brief       describes a generic button
 */
typedef struct _buttonT
{
    struct _deviceT *dev;           /*!< underlying button device           */
    unsigned         nIndex;        /*!< button index (zero-based)          */
    struct _buttonT *next;          /*!< next button descriptor             */
    destroyButtonT   doDestroy;     /*!< destroy button                     */
} buttonT;

/*!
 * @struct      gpioButtonT
 * @brief       describes a button provided by a GPIO pin
 */
typedef struct _gpioButtonT
{
    buttonT          button;        /*!< generic button description         */
    bool             state;         /*!< false if pressed, true if released */
} gpioButtonT;

/*!
 * @struct      evdevButtonT
 * @brief       describes a button provided by an evdev input device
 */
typedef struct _evdevButtonT
{
    buttonT          button;        /*!< generic button description         */
    unsigned         key;           /*!< the key to handle                  */
} evdevButtonT;

/*!
 * @enum        deviceTypeT
 * @brief       input device discriminator
 */
typedef enum _deviceTypeT
{
    DEVICE_GPIO,
    DEVICE_EVDEV
} deviceTypeT;

/*!
 * @brief       Type of "read from device" operation on devices.
 * @param[in]   dev
 *              the device to read from
 * @details     The function not only reads from the device but also processes
 *              the data by calling execute_command() if pressing a button or
 *              key has been identified.
 */
typedef void (*readFromDeviceT)(struct _deviceT *dev);

/*!
 * @brief       Type of "destroy" operation on devices.
 * @param[in]   dev
 *              the device to destroy
 */
typedef void (*destroyDeviceT)(struct _deviceT *dev);

/*!
 * @struct      deviceT
 * @brief       describes a generic input device
 */
typedef struct _deviceT
{
    deviceTypeT      type;          /*!< type of input device               */
    char            *inputDev;      /*!< path to input device               */
    buttonT         *firstButton;   /*!< points to the first button         */
    struct _deviceT *next;          /*!< next device descriptor             */
    readFromDeviceT  doRead;        /*!< read from device                   */
    destroyDeviceT   doDestroy;     /*!< destroy device                     */
} deviceT;

/*!
 * @struct      deviceT
 * @brief       describes a GPIO input device
 */
typedef struct _gpioDeviceT
{
    deviceT          dev;           /*!< common part of device descriptor   */
} gpioDeviceT;

/*!
 * @struct      deviceT
 * @brief       describes an evdev input device
 */
typedef struct _evdevDeviceT
{
    deviceT          dev;           /*!< common part of device descriptor   */
    int              fd;            /*!< file descriptor of input device    */
    struct libevdev *evdev;         /*!< pointer to libevdev device object  */
} evdevDeviceT;

/*!
 * @struct      dataT
 * @brief       global data structure
 */
typedef struct _dataT
{
    deviceT        *firstDevice;    /*!< points to the first device         */
    int             pid;            /*!< process id for executed command    */
    int             fExecute;       /*!< main loop execute flag             */
    int             fForeground;    /*!< forground flag                     */
    int             fDaemonized;    /*!< daemonized flag                    */
    int             fDebug;         /*!< debug flag                         */
} dataT;

extern dataT g_data;

/* callbacks for button */
void execute_command (int num);

/* log functions */
void log_printf(int level, const char *fmt, ...) __attribute__ ((format(printf, 2, 3)));

/* device functions */
gpioDeviceT *gpio_createDevice(char const *inputDev);
evdevDeviceT *evdev_createDevice(char const *inputDev);

/* button functions */
gpioButtonT *gpio_createButton(gpioDeviceT *device, unsigned nIndex);
evdevButtonT *evdev_createButton(evdevDeviceT *device, unsigned nIndex, unsigned key);

#endif /* _HWSUPPD_H_ */
/*_oOo_*/
