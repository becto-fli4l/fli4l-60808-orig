/*
 *  leds-simu.c - LED simulation
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
#include <linux/platform_device.h>

#define DRVNAME "leds-simu"

static struct platform_device *pdev;

static void leds_simu_brightness_set(struct led_classdev *led_cdev,
		enum led_brightness value);

static struct led_classdev leds_simu_leds[] = {
	{
		.name = "simu::1",
		.brightness_set = leds_simu_brightness_set,
		.default_trigger = "none",
		.flags = LED_CORE_SUSPENDRESUME,
	},
	{
		.name = "simu::2",
		.brightness_set = leds_simu_brightness_set,
		.default_trigger = "none",
		.flags = LED_CORE_SUSPENDRESUME,
	},
	{
		.name = "simu::3",
		.brightness_set = leds_simu_brightness_set,
		.default_trigger = "none",
		.flags = LED_CORE_SUSPENDRESUME,
	},
	{
		.name = "simu::4",
		.brightness_set = leds_simu_brightness_set,
		.default_trigger = "none",
		.flags = LED_CORE_SUSPENDRESUME,
	},
	{
		.name = "simu::5",
		.brightness_set = leds_simu_brightness_set,
		.default_trigger = "none",
		.flags = LED_CORE_SUSPENDRESUME,
	},
	{
		.name = "simu::6",
		.brightness_set = leds_simu_brightness_set,
		.default_trigger = "none",
		.flags = LED_CORE_SUSPENDRESUME,
	},
	{
		.name = "simu::7",
		.brightness_set = leds_simu_brightness_set,
		.default_trigger = "none",
		.flags = LED_CORE_SUSPENDRESUME,
	},
	{
		.name = "simu::8",
		.brightness_set = leds_simu_brightness_set,
		.default_trigger = "none",
		.flags = LED_CORE_SUSPENDRESUME,
	},
};

static void leds_simu_brightness_set(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	size_t index = (led_cdev - leds_simu_leds);
	dev_info(led_cdev->dev, "set LED %zu %s\n", index+1,
			(LED_OFF == value) ? "off" : "on");
}

static int leds_simu_probe(struct platform_device *pdev)
{
	int i, j, ret;

	for (i = 0; i < ARRAY_SIZE(leds_simu_leds); i++) {
		ret = led_classdev_register(&pdev->dev, &leds_simu_leds[i]);
		if (ret < 0)
			goto unregister;
	}

	dev_info(&pdev->dev, "loaded\n");

	return 0;

unregister:
	for (j = 0; j < i; i++)
		led_classdev_unregister(&leds_simu_leds[j]);

	return ret;
}

static int leds_simu_remove(struct platform_device *pdev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(leds_simu_leds); i++)
		led_classdev_unregister(&leds_simu_leds[i]);

	dev_info(&pdev->dev, "unloaded\n");

	return 0;
}

static struct platform_driver leds_simu_driver = {
	.probe = leds_simu_probe,
	.remove = leds_simu_remove,
	.driver = {
		.name = DRVNAME,
		.owner = THIS_MODULE,
	},
};

static int __init leds_simu_init(void)
{
	int ret;

	ret = platform_driver_register(&leds_simu_driver);
	if (ret < 0)
		goto exit;

	pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		ret = PTR_ERR(pdev);
		platform_driver_unregister(&leds_simu_driver);
		goto exit;
	}

exit:
	return ret;
}

static void __exit leds_simu_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&leds_simu_driver);
}

MODULE_AUTHOR("Carsten Spiess <fli4l at carsten-spiess.de>");
MODULE_DESCRIPTION("simulation LED driver");
MODULE_LICENSE("GPL");

module_init(leds_simu_init);
module_exit(leds_simu_exit);


