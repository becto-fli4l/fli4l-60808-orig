/*
 *  keys-apu.c - Button support for PC-Engines APU board
 *
 *  Copyright (c) 2015 Christoph Schulz <develop@kristov.de>
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
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/gpio/consumer.h>
#include <linux/input.h>
#include <linux/version.h>

static struct gpio_keys_button apu_buttons[] = {
	{
		.code			= KEY_RESTART,
		.gpio			= -1,
		.desc			= "Reset button",
		.type			= EV_KEY,
		.wakeup			= 0,
		.debounce_interval	= 100,
		.can_disable		= 0,
	},
};

static struct gpio_keys_platform_data apu_buttons_data = {
	.buttons			= apu_buttons,
	.nbuttons			= ARRAY_SIZE(apu_buttons),
	.poll_interval			= 20,
};

static struct platform_device apu_buttons_dev = {
	.name				= "gpio-keys-polled",
	.id				= 1,
	.dev = {
		.platform_data		= &apu_buttons_data,
	}
};

static int __init keys_apu_init(void)
{
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(apu_buttons); ++i) {
		struct gpio_desc *btn = gpiod_get_index(NULL, "apu-button", i
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0)
			, GPIOD_ASIS
#endif
		);
		if (!IS_ERR(btn)) {
			int gpio = desc_to_gpio(btn);
			bool active_low = gpiod_is_active_low(btn);
			gpiod_put(btn);
			if (gpio >= 0) {
				apu_buttons[i].gpio = gpio;
				apu_buttons[i].active_low = active_low;
				pr_info("apu-button %d mapped to GPIO %d (active-%s)\n",
					i, gpio,
					active_low ? "low" : "high");
			} else {
				pr_err("cannot map GPIO descriptor for apu-button %d to GPIO index\n",
					i);
			}
		} else {
			pr_warn("apu-button %d not available\n", i);
		}
	}

	ret = platform_device_register(&apu_buttons_dev);
	if (ret) {
		pr_err("registration of platform device failed\n");
	}
	return ret;
}

static void __exit keys_apu_exit(void)
{
	platform_device_unregister(&apu_buttons_dev);
}

MODULE_AUTHOR("Christoph Schulz <develop@kristov.de>");
MODULE_DESCRIPTION("GPIO button driver for PC Engines APU");
MODULE_LICENSE("GPL");

module_init(keys_apu_init);
module_exit(keys_apu_exit);
