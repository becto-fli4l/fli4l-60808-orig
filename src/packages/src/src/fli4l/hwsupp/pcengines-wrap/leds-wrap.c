/*
 *  leds-wrap.c - LED support for PC-Engines WRAP board
 *
 *  Copyright (c) 2014  Carsten Spiess <fli4l at carsten-spiess.de>
 *  Copyright (c) 2015  Christoph Schulz <develop at kristov.de>
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

static struct gpio_led wrap_leds[] = {
	{
		.name = "wrap::1",
		.gpio = 2,
		.default_trigger = "none",
	},
	{
		.name = "wrap::2",
		.gpio = 3,
		.default_trigger = "none",
	},
	{
		.name = "wrap::3",
		.gpio = 18,
		.default_trigger = "none",
	},
};

static struct gpio_led_platform_data wrap_leds_data = {
	.num_leds = ARRAY_SIZE(wrap_leds),
	.leds = wrap_leds,
};

static struct platform_device wrap_leds_dev = {
	.name = "leds-gpio",
	.id = -1,
	.dev = {
		.platform_data = &wrap_leds_data,
	}
};

static int __init leds_wrap_init(void)
{
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(wrap_leds); ++i) {
		struct gpio_desc *led = gpiod_get_index(NULL, "wrap-led", i
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0)
			, GPIOD_ASIS
#endif
		);
		if (!IS_ERR(led)) {
			int gpio = desc_to_gpio(led);
			bool active_low = gpiod_is_active_low(led);
			gpiod_put(led);
			if (gpio >= 0) {
				wrap_leds[i].gpio = gpio;
				wrap_leds[i].active_low = active_low;
				pr_info("wrap-led %d mapped to GPIO %d (active-%s)\n",
					i, gpio,
					active_low ? "low" : "high");
			} else {
				pr_err("cannot map GPIO descriptor for wrap-led %d to GPIO index\n",
					i);
			}
		} else {
			pr_warn("wrap-led %d not available\n", i);
		}
	}

	ret = platform_device_register(&wrap_leds_dev);
	if (ret) {
		pr_err("registration of platform device failed\n");
	}
	return ret;
}

static void __exit leds_wrap_exit (void)
{
	platform_device_unregister (&wrap_leds_dev);
}

MODULE_AUTHOR("Carsten Spiess <fli4l at carsten-spiess.de>");
MODULE_DESCRIPTION("GPIO LED driver for PC Engines WRAP");
MODULE_LICENSE("GPL");

module_init(leds_wrap_init);
module_exit(leds_wrap_exit);
