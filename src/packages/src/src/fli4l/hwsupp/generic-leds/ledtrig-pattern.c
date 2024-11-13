/*
 * LED Pattern Trigger
 *
 * Copyright(c) 2014  Carsten Spiess <fli4l at carsten-spiess.de>
 *
 * Based on ledtrig-heartbeat.c
 * Copyright(C) 2006 Atsushi Nemoto <anemo@mba.ocn.ne.jp>
 *
 * Based on Richard Purdie's ledtrig-timer.c and some arch's
 * CONFIG_HEARTBEAT code.

 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/timer.h>
#include <linux/version.h>

#if defined(timer_setup) && defined(from_timer)
#define HAVE_TIMER_SETUP
#endif

#define BLINK_DELAY msecs_to_jiffies(50)
struct ledtrig_pattern_data {
	unsigned long act;
	unsigned long next;
	unsigned int phase;
	struct timer_list timer;
#if defined(HAVE_TIMER_SETUP)
	struct led_classdev *led_cdev;
#endif
};

static inline void __led_set_brightness(struct led_classdev *led_cdev,
					enum led_brightness value)
{
	if (value > led_cdev->max_brightness)
		value = led_cdev->max_brightness;
	led_cdev->brightness = value;
	if (!(led_cdev->flags & LED_SUSPENDED))
		led_cdev->brightness_set(led_cdev, value);
}

#if defined(HAVE_TIMER_SETUP)
static void ledtrig_pattern_function(struct timer_list *t)
{
	struct ledtrig_pattern_data *data = from_timer(data, t, timer);
	struct led_classdev *led_cdev = data->led_cdev;
#else
static void ledtrig_pattern_function(unsigned long param)
{
	struct led_classdev *led_cdev = (struct led_classdev *) param;
	struct ledtrig_pattern_data *data = led_cdev->trigger_data;
#endif

	if (0 == data->phase) {
		data->act = data->next;
		__led_set_brightness(led_cdev,
				(data->act & 1) ? LED_FULL : LED_OFF);
	} else {
		if (((data->act >> data->phase)&1) !=
				((data->act >> (data->phase-1))&1))
			__led_set_brightness(led_cdev,
					((data->act >> data->phase)&1) ?
					LED_FULL : LED_OFF);
	}

	data->phase++;
	data->phase &= 0x1f;

	mod_timer(&data->timer, jiffies + BLINK_DELAY);
}

static ssize_t ledtrig_pattern_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ledtrig_pattern_data *data = led_cdev->trigger_data;

	return sprintf(buf, "%08lx\n", data->next);
}

static ssize_t ledtrig_pattern_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t len)
{
	int ret;
	unsigned long next;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct ledtrig_pattern_data *data = led_cdev->trigger_data;

	ret = kstrtoul(buf, 16, &next);
	if (ret)
		return ret;

	data->next = next;

	return len;
}

static DEVICE_ATTR(pattern, S_IWUSR|S_IRUGO,
		ledtrig_pattern_show, ledtrig_pattern_store);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
static void ledtrig_pattern_activate(struct led_classdev *led_cdev)
#else
static int ledtrig_pattern_activate(struct led_classdev *led_cdev)
#endif
{
	int ret;
	struct ledtrig_pattern_data *data;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->next = data->act = 0x0001;
#if defined(HAVE_TIMER_SETUP)
	data->led_cdev = led_cdev;
#endif

	ret = device_create_file(led_cdev->dev, &dev_attr_pattern);
	if (ret)
		goto exit;

	led_cdev->trigger_data = data;
	led_set_brightness(led_cdev, LED_OFF);

#if defined(HAVE_TIMER_SETUP)
	timer_setup(&data->timer, ledtrig_pattern_function, 0);
#else
	setup_timer(&data->timer,
			ledtrig_pattern_function, (unsigned long) led_cdev);
#endif
	mod_timer(&data->timer, jiffies + BLINK_DELAY);

	led_cdev->activated = true;

	return 0;

exit:
	kfree(data);
	return ret;
}

static void ledtrig_pattern_deactivate(struct led_classdev *led_cdev)
{
	struct ledtrig_pattern_data *data = led_cdev->trigger_data;

	if (led_cdev->activated) {
		del_timer_sync(&data->timer);
		kfree(data);
		led_cdev->activated = false;

		led_set_brightness(led_cdev, LED_OFF);
	}
}

static struct led_trigger ledtrig_pattern_trigger = {
	.name = "pattern",
	.activate = ledtrig_pattern_activate,
	.deactivate = ledtrig_pattern_deactivate,
};

static int __init ledtrig_pattern_init(void)
{
	return led_trigger_register(&ledtrig_pattern_trigger);
}

static void __exit ledtrig_pattern_exit(void)
{
	led_trigger_unregister(&ledtrig_pattern_trigger);
}

module_init(ledtrig_pattern_init);
module_exit(ledtrig_pattern_exit);

MODULE_AUTHOR("Carsten Spiess <fli4l at carsten-spiess.de>");
MODULE_DESCRIPTION("Blink sequence LED trigger");
MODULE_LICENSE("GPL");
