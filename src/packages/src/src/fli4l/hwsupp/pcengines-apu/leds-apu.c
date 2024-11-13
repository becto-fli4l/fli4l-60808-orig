/*
 *  leds-apu.c - LED support for PC-Engines APU board
 *
 *  Copyright (c) 2014  Carsten Spiess <fli4l at carsten-spiess.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/version.h>

static struct gpio_led apu_leds[] = {
	{
		.name = "apu::1",
		.gpio = -1,
		.default_trigger = "none",
	},
	{
		.name = "apu::2",
		.gpio = -1,
		.default_trigger = "none",
	},
	{
		.name = "apu::3",
		.gpio = -1,
		.default_trigger = "none",
	},
};

static struct gpio_led_platform_data apu_leds_data = {
	.num_leds = ARRAY_SIZE(apu_leds),
	.leds = apu_leds,
};

static struct platform_device apu_leds_dev = {
	.name = "leds-gpio",
	.id = -1,
	.dev = {
		.platform_data = &apu_leds_data,
	}
};

static int __init leds_apu_init(void)
{
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(apu_leds); ++i) {
		struct gpio_desc *led = gpiod_get_index(NULL, "apu-led", i
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0)
			, GPIOD_ASIS
#endif
		);
		if (!IS_ERR(led)) {
			int gpio = desc_to_gpio(led);
			bool active_low = gpiod_is_active_low(led);
			gpiod_put(led);
			if (gpio >= 0) {
				apu_leds[i].gpio = gpio;
				apu_leds[i].active_low = active_low;
				pr_info("apu-led %d mapped to GPIO %d (active-%s)\n",
					i, gpio,
					active_low ? "low" : "high");
			} else {
				pr_err("cannot map GPIO descriptor for apu-led %d to GPIO index\n",
					i);
			}
		} else {
			pr_warn("apu-led %d not available\n", i);
		}
	}

	ret = platform_device_register(&apu_leds_dev);
	if (ret) {
		pr_err("registration of platform device failed\n");
	}
	return ret;
}

static void __exit leds_apu_exit(void)
{
	platform_device_unregister(&apu_leds_dev);
}

MODULE_AUTHOR("Carsten Spiess <fli4l at carsten-spiess.de>");
MODULE_DESCRIPTION("GPIO LED driver for PC Engines APU");
MODULE_LICENSE("GPL");

module_init(leds_apu_init);
module_exit(leds_apu_exit);
