/*
 *  leds-alix.c - LED support for PC-Engines ALIX board
 *
 *  Copyright(c) 2014  Carsten Spiess <fli4l at carsten-spiess.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/leds.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/console_struct.h>
#include <linux/vt_kern.h>

#include <linux/platform_device.h>

#define DRVNAME "leds-keyboard"

static struct platform_device *pdev;
static u8 ledstate;

static void leds_keyboard_brightness_set(struct led_classdev *led_cdev,
		enum led_brightness value);

static struct led_classdev leds_keyboard_leds[] = {
	{
		.name = "keyboard::scroll",
		.brightness_set = leds_keyboard_brightness_set,
		.default_trigger = "none",
		.flags = LED_CORE_SUSPENDRESUME,
	},
	{
		.name = "keyboard::num",
		.brightness_set = leds_keyboard_brightness_set,
		.default_trigger = "none",
		.flags = LED_CORE_SUSPENDRESUME,
	},
	{
		.name = "keyboard::caps",
		.brightness_set = leds_keyboard_brightness_set,
		.default_trigger = "none",
		.flags = LED_CORE_SUSPENDRESUME,
	},
};

static int leds_keyboard_kbsetled(u8 leds)
{
	int ret = -ENODEV;
	int i;

	for (i = 0; i < MAX_NR_CONSOLES; i++) {
		if (!vc_cons[i].d)
			break;
		if (!vc_cons[i].d->port.tty)
			continue;
		if (!vc_cons[i].d->port.tty->driver)
			continue;
		if (!vc_cons[i].d->port.tty->driver->ops)
			continue;
		if (!vc_cons[i].d->port.tty->driver->ops->ioctl)
			continue;

		(vc_cons[i].d->port.tty->driver->ops->ioctl)(
				vc_cons[i].d->port.tty, KDSETLED, leds);
		ret = 0;
	}

	return ret;
}

static void leds_keyboard_brightness_set(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	size_t index = (led_cdev - leds_keyboard_leds);
	if (value)
		ledstate |= (1 << index);
	else
		ledstate &= ~(1 << index);

	leds_keyboard_kbsetled(ledstate);
}

static int leds_keyboard_probe(struct platform_device *pdev)
{
	int i, j, ret;

	ret = leds_keyboard_kbsetled(ledstate);
	if (ret) {
		dev_info(&pdev->dev, "no console found\n");
		goto exit;
	}

	for (i = 0; i < ARRAY_SIZE(leds_keyboard_leds); i++) {
		ret = led_classdev_register(&pdev->dev, &leds_keyboard_leds[i]);
		if (ret < 0)
			goto unregister;
	}

	dev_info(&pdev->dev, "loaded\n");

	return 0;

unregister:
	for (j = 0; j < i; i++)
		led_classdev_unregister(&leds_keyboard_leds[j]);

	leds_keyboard_kbsetled(0xFF);

exit:
	return ret;
}

static int leds_keyboard_remove(struct platform_device *pdev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(leds_keyboard_leds); i++)
		led_classdev_unregister(&leds_keyboard_leds[i]);

	leds_keyboard_kbsetled(0xFF);

	dev_info(&pdev->dev, "unloaded\n");

	return 0;
}

static struct platform_driver leds_keyboard_driver = {
	.probe		= leds_keyboard_probe,
	.remove		= leds_keyboard_remove,
	.driver		= {
		.name		= DRVNAME,
		.owner		= THIS_MODULE,
	},
};

static int __init leds_keyboard_init(void)
{
	int ret;

	ret = platform_driver_register(&leds_keyboard_driver);
	if (ret < 0)
		goto exit;

	pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		ret = PTR_ERR(pdev);
		platform_driver_unregister(&leds_keyboard_driver);
		goto exit;
	}

exit:
	return ret;
}

static void __exit leds_keyboard_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&leds_keyboard_driver);
}

MODULE_AUTHOR("Carsten Spiess <fli4l at carsten-spiess.de>");
MODULE_DESCRIPTION("LED driver for PC Keyboard");
MODULE_LICENSE("GPL");

module_init(leds_keyboard_init);
module_exit(leds_keyboard_exit);


