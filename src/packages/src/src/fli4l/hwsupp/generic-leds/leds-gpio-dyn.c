/*
 *  leds-gpio-dyn.c - generic gpio LED driver
 *
 *  Copyright(c) 2014  Carsten Spiess <fli4l at carsten-spiess.de>
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
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <linux/gpio_keys.h>
#include <linux/platform_device.h>
#include <linux/version.h>

#define DRVNAME "leds-gpio-dyn"

static struct platform_device *leds_gpio_dyn_pdev;


struct leds_gpio_dyn_data {
	char name[16];
	struct led_classdev led_cdev;
	unsigned gpio;
	struct work_struct work;
	bool active_low;
	bool can_sleep;
	int new_value;
};

struct leds_gpio_dyn_data *leds_gpio_dyn_list[ARCH_NR_GPIOS];
static DEFINE_SPINLOCK(leds_gpio_dyn_lock);

static void leds_gpio_dyn_work(struct work_struct *work)
{
	struct leds_gpio_dyn_data *data =
		container_of(work, struct leds_gpio_dyn_data, work);

	gpio_set_value_cansleep(data->gpio, data->new_value);
}

static void leds_gpio_dyn_set(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	int gpio_value;
	struct leds_gpio_dyn_data *data =
			container_of(led_cdev, struct leds_gpio_dyn_data,
					led_cdev);

	gpio_value = (LED_OFF == value) ? 0 : 1;

	if (data->active_low)
		gpio_value ^= 1;

	if (data->can_sleep) {
		data->new_value = gpio_value;
		schedule_work(&data->work);
	} else
		gpio_set_value(data->gpio, gpio_value);
}

static int leds_gpio_dyn_create(unsigned gpio, int active_low)
{
	int ret;
	struct leds_gpio_dyn_data *data;

	if (gpio >= ARCH_NR_GPIOS)
		return -EINVAL;

	data = kzalloc(sizeof(struct leds_gpio_dyn_data), GFP_KERNEL);

	if (!data)
		return -ENOMEM;

	ret = gpio_request(gpio, DRVNAME);
	if (ret)
		goto exit_mem_free;

	snprintf(data->name, sizeof(data->name), "gpio::%d", gpio);
	data->gpio = gpio;
	data->led_cdev.name = data->name;
	data->led_cdev.brightness_set = leds_gpio_dyn_set;
	data->led_cdev.default_trigger = "none";
	data->led_cdev.flags = LED_CORE_SUSPENDRESUME;

	INIT_WORK(&data->work, leds_gpio_dyn_work);
	data->active_low = active_low;
	data->can_sleep = gpio_cansleep(gpio);

	ret = gpio_direction_output(gpio, active_low);
	if (ret)
		goto exit_gpio_free;

	ret = led_classdev_register(&leds_gpio_dyn_pdev->dev, &data->led_cdev);
	if (ret)
		goto exit_gpio_free;

	spin_lock(&leds_gpio_dyn_lock);
	if (leds_gpio_dyn_list[gpio]) {
		spin_unlock(&leds_gpio_dyn_lock);
		ret = -EBUSY;
		goto exit_led_free;
	}
	leds_gpio_dyn_list[gpio] = data;
	spin_unlock(&leds_gpio_dyn_lock);

	return 0;

exit_led_free:
	led_classdev_unregister(&data->led_cdev);
exit_gpio_free:
	gpio_free(gpio);
exit_mem_free:
	kfree(data);

	return ret;
}

static int leds_gpio_dyn_delete(unsigned gpio)
{
	struct leds_gpio_dyn_data *data;

	if (gpio >= ARCH_NR_GPIOS)
		return -EINVAL;

	spin_lock(&leds_gpio_dyn_lock);
	if (!leds_gpio_dyn_list[gpio]) {
		spin_unlock(&leds_gpio_dyn_lock);
		return -EINVAL;
	}

	data = leds_gpio_dyn_list[gpio];
	leds_gpio_dyn_list[gpio] = NULL;
	spin_unlock(&leds_gpio_dyn_lock);

	led_classdev_unregister(&data->led_cdev);
	gpio_free(data->gpio);
	kfree(data);

	return 0;
}

static ssize_t leds_gpio_dyn_attach(struct class *class,
				struct class_attribute *attr,
				const char *buf, size_t len)
{
	int ret = 0;
	unsigned gpio;

	ret = kstrtouint(buf, 0, &gpio);
	if (ret < 0)
		goto exit;

	ret = leds_gpio_dyn_create(gpio, 1);

exit:
	return ret ? : len;
}

static ssize_t leds_gpio_dyn_attach_inv(struct class *class,
				struct class_attribute *attr,
				const char *buf, size_t len)
{
	int ret = 0;
	unsigned gpio;

	ret = kstrtouint(buf, 0, &gpio);
	if (ret < 0)
		goto exit;

	ret = leds_gpio_dyn_create(gpio, 0);

exit:
	return ret ? : len;
}

static ssize_t leds_gpio_dyn_detach(struct class *class,
				struct class_attribute *attr,
				const char *buf, size_t len)
{
	int ret = 0;
	long gpio;

	ret = kstrtol(buf, 0, &gpio);
	if (ret < 0)
		goto exit;

	ret = leds_gpio_dyn_delete(gpio);

exit:
	return ret ? : len;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,13,0)
static struct class_attribute leds_gpio_dyn_class_attrs[] = {
	__ATTR(attach, S_IWUSR, NULL, leds_gpio_dyn_attach),
	__ATTR(attach-inv, S_IWUSR, NULL, leds_gpio_dyn_attach_inv),
	__ATTR(detach, S_IWUSR, NULL, leds_gpio_dyn_detach),
	__ATTR_NULL,
};
#else
static struct class_attribute class_attr_leds_gpio_dyn_attach =
	__ATTR(attach, S_IWUSR, NULL, leds_gpio_dyn_attach);
static struct class_attribute class_attr_leds_gpio_dyn_attach_inv =
	__ATTR(attach-inv, S_IWUSR, NULL, leds_gpio_dyn_attach_inv);
static struct class_attribute class_attr_leds_gpio_dyn_detach =
	__ATTR(detach, S_IWUSR, NULL, leds_gpio_dyn_detach);

static struct attribute* leds_gpio_dyn_class_attrs[] = {
	&class_attr_leds_gpio_dyn_attach.attr,
	&class_attr_leds_gpio_dyn_attach_inv.attr,
	&class_attr_leds_gpio_dyn_detach.attr,
	NULL,
};
ATTRIBUTE_GROUPS(leds_gpio_dyn_class);
#endif

static int leds_gpio_dyn_probe(struct platform_device *pdev)
{
	return 0;
}

static int leds_gpio_dyn_remove(struct platform_device *pdev)
{
	unsigned gpio;

	for (gpio = 0; gpio < ARCH_NR_GPIOS; gpio++)
		leds_gpio_dyn_delete(gpio);

	return 0;
}

static struct platform_driver leds_gpio_dyn_driver = {
	.probe = leds_gpio_dyn_probe,
	.remove = leds_gpio_dyn_remove,
	.driver = {
		.name = DRVNAME,
		.owner = THIS_MODULE,
	},
};

static struct class leds_gpio_dyn_class = {
	.name = DRVNAME,
	.owner = THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,13,0)
	.class_attrs = leds_gpio_dyn_class_attrs,
#else
	.class_groups = leds_gpio_dyn_class_groups,
#endif
};

static int __init leds_gpio_dyn_init(void)
{
	int ret;
	unsigned gpio;

	for (gpio = 0; gpio < ARCH_NR_GPIOS; gpio++)
		leds_gpio_dyn_list[gpio] = NULL;

	ret = class_register(&leds_gpio_dyn_class);
	if (ret)
		goto exit;

	ret = platform_driver_register(&leds_gpio_dyn_driver);
	if (ret < 0)
		goto exit_class_unregister;

	leds_gpio_dyn_pdev = platform_device_register_simple(DRVNAME, -1,
			NULL, 0);
	if (IS_ERR(leds_gpio_dyn_pdev)) {
		ret = PTR_ERR(leds_gpio_dyn_pdev);
		goto exit_platform_unregister;
	}

	return 0;

exit_platform_unregister:
	platform_driver_unregister(&leds_gpio_dyn_driver);

exit_class_unregister:
	class_unregister(&leds_gpio_dyn_class);

exit:
	return ret;

}

static void __exit leds_gpio_dyn_exit(void)
{
	unsigned gpio;

	for (gpio = 0; gpio < ARCH_NR_GPIOS; gpio++)
		leds_gpio_dyn_delete(gpio);

	platform_device_unregister(leds_gpio_dyn_pdev);
	platform_driver_unregister(&leds_gpio_dyn_driver);
	class_unregister(&leds_gpio_dyn_class);
}

MODULE_AUTHOR("Carsten Spiess <fli4l at carsten-spiess.de>");
MODULE_DESCRIPTION("generic gpio LED driver");
MODULE_LICENSE("GPL");

module_init(leds_gpio_dyn_init);
module_exit(leds_gpio_dyn_exit);


