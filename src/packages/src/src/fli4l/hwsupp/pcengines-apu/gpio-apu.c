/*
 *  gpio-apu.c - AMD A55 fch GPIO support for PC-Engines APU board
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/dmi.h>
#include <linux/gpio.h>
#include <linux/gpio/machine.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/version.h>

#define DEVNAME                 "gpio-apu"

#define FCH_IO_PM_INDEX_REG     0xCD6
#define FCH_IO_PM_DATA_REG      0xCD7
#define FCH_PM_MMIO_BASE        0x24
#define FCH_PM_IOPORTS_SIZE     2
#define FCH_GPIO_OFFSET         0x100
#define FCH_GPIO_SIZE           0x100
#define APU_NUM_GPIO            4

#define GPIO_BIT_DIR            5
#define GPIO_BIT_WRITE          6
#define GPIO_BIT_READ           7

/* internal variables */
static u32 mmio_base;
static struct pci_dev *gpio_apu_pci;
static DEFINE_SPINLOCK ( gpio_lock);

/* the watchdog platform device */
static struct platform_device *gpio_apu_platform_device;

static const struct pci_device_id gpio_apu_pci_tbl[] ={
	{PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_SBX00_SMBUS, PCI_ANY_ID, PCI_ANY_ID},
	{ 0, } /* End of list */
};
MODULE_DEVICE_TABLE ( pci, gpio_apu_pci_tbl);

static u8 gpio_offset[APU_NUM_GPIO] = { 187, 189, 190, 191};

static struct gpiod_lookup_table gpio_apu_lookup_tbl = {
	.table = {
		GPIO_LOOKUP_IDX("gpio-apu", 0, "apu-button", 0, GPIO_ACTIVE_LOW),
		GPIO_LOOKUP_IDX("gpio-apu", 1, "apu-led", 0, GPIO_ACTIVE_LOW),
		GPIO_LOOKUP_IDX("gpio-apu", 2, "apu-led", 1, GPIO_ACTIVE_LOW),
		GPIO_LOOKUP_IDX("gpio-apu", 3, "apu-led", 2, GPIO_ACTIVE_LOW),
		{ },
	},
};

static void __iomem *gpio_addr[APU_NUM_GPIO] = {NULL, NULL, NULL, NULL};

static int gpio_apu_get_dir (struct gpio_chip *chip, unsigned offset)
{
	u8 val;

	val = ioread8 (gpio_addr[offset]);

	return (val >> GPIO_BIT_DIR) & 1;
}

static int gpio_apu_dir_in (struct gpio_chip *gc, unsigned offset)
{
	u8 val;

	spin_lock_bh (&gpio_lock);

	val = ioread8 (gpio_addr[offset]);
	val |= BIT(GPIO_BIT_DIR);
	iowrite8 (val, gpio_addr[offset]);

	spin_unlock_bh (&gpio_lock);
	return 0;
}

static int gpio_apu_dir_out (struct gpio_chip *chip, unsigned offset,
		int value)
{
	u8 val;

	spin_lock_bh (&gpio_lock);

	val = ioread8 (gpio_addr[offset]);

	val &= ~BIT(GPIO_BIT_DIR);

	iowrite8 (val, gpio_addr[offset]);

	spin_unlock_bh (&gpio_lock);
	return 0;
}

static int gpio_apu_get_data (struct gpio_chip *chip, unsigned offset)
{
	u8 val;

	val = ioread8(gpio_addr[offset]);

	return (val >> GPIO_BIT_READ) & 1;
}

static void gpio_apu_set_data (struct gpio_chip *chip, unsigned offset, int value)
{
	u8 val;

	spin_lock_bh (&gpio_lock);

	val = ioread8 (gpio_addr[offset]);

	if (value)
		val |= BIT(GPIO_BIT_WRITE);
	else
		val &= ~BIT(GPIO_BIT_WRITE);

	iowrite8 (val, gpio_addr[offset]);

	spin_unlock_bh (&gpio_lock);
}

static struct gpio_chip gpio_apu_chip = {
	.label = DEVNAME,
	.owner = THIS_MODULE,
	.base = -1,
	.ngpio = APU_NUM_GPIO,
	.get_direction = gpio_apu_get_dir,
	.direction_input = gpio_apu_dir_in,
	.direction_output = gpio_apu_dir_out,
	.get = gpio_apu_get_data,
	.set = gpio_apu_set_data,
};

static bool gpio_apu_test_dmi (void)
{
	const char *vendor, *product;

	vendor = dmi_get_system_info(DMI_SYS_VENDOR);
	product = dmi_get_system_info(DMI_PRODUCT_NAME);

	if (!vendor || !product)
		return false;

	pr_info("dmi vendor:\"%s\" product:\"%s\"\n", vendor, product);

	if (strcmp(vendor, "PC Engines"))
		return false;

	if (strcmp(product, "APU"))
		return false;

	return true;
}

static int read_pm_address (u32 pm_reg, u32 *mmio_addr)
{
	u32 addr = 0;

	if (!request_region (FCH_IO_PM_INDEX_REG, FCH_PM_IOPORTS_SIZE, DEVNAME))
	{
		pr_err ("I/O address 0x%04x already in use\n", FCH_IO_PM_INDEX_REG);
		return -EBUSY;
	}

	/*
	 * First, Find address from indirect I/O.
	 */

	outb (pm_reg + 3, FCH_IO_PM_INDEX_REG);
	addr = inb (FCH_IO_PM_DATA_REG);
	outb (pm_reg + 2, FCH_IO_PM_INDEX_REG);
	addr = addr << 8 | inb (FCH_IO_PM_DATA_REG);
	outb (pm_reg + 1, FCH_IO_PM_INDEX_REG);
	addr = addr << 8 | inb (FCH_IO_PM_DATA_REG);
	outb (pm_reg + 0, FCH_IO_PM_INDEX_REG);
	/* Low three bits of BASE are reserved */
	addr = addr << 8 | (inb (FCH_IO_PM_DATA_REG) & 0xf8);

	pr_info ("Got 0x%04x from indirect I/O\n", addr);

	release_region (FCH_IO_PM_INDEX_REG, FCH_PM_IOPORTS_SIZE);

	*mmio_addr = addr;

	return 0;
}

static int gpio_apu_probe (struct platform_device *dev)
{
	int ret = 0;
	int i;
	struct pci_dev *pci_dev = NULL;

	/* Match the PCI device */
	for_each_pci_dev (pci_dev) {
		if (pci_match_id (gpio_apu_pci_tbl, pci_dev) != NULL) {
			gpio_apu_pci = pci_dev;
			break;
		}
	}

	if (!gpio_apu_pci)
		return -ENODEV;

	pr_info ("PCI Revision ID: 0x%x\n", gpio_apu_pci->revision);

	/* Determine type of southbridge chipset. */
	if (gpio_apu_pci->revision < 0x40) {
		return -EACCES;
	}

	/* test for PC Engines APU-1 */
	if (!gpio_apu_test_dmi()) {
		pr_err ("not PC Engines APU-1\n");
		return -ENXIO;
	}

	/* Request the IO ports used by this driver */
	ret = read_pm_address (FCH_PM_MMIO_BASE, &mmio_base);
	if (ret) {
		pr_err ("get MMIO address failed\n");
		return ret;
	}

	/* Request memory region for GPIO's */
	if (!devm_request_mem_region (&dev->dev, mmio_base + FCH_GPIO_OFFSET,
			FCH_GPIO_SIZE, DEVNAME)) {
		pr_err ("request GPIO mem region failed\n");
		return -ENXIO;
	}

	/* Map IO's for GPIO's */
	for (i = 0; i < APU_NUM_GPIO; i++) {
		gpio_addr[i] = devm_ioremap (&dev->dev,
				mmio_base + FCH_GPIO_OFFSET + gpio_offset[i], 1);
		if (!gpio_addr[i]) {
			pr_err ("map GPIO[%02x] address failed\n", gpio_offset[i]);
			return -ENXIO;
		}
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0)
	gpio_apu_chip.dev = &dev->dev;
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0) */
	gpio_apu_chip.parent = &dev->dev;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0) */
	ret = gpiochip_add (&gpio_apu_chip);
	if (ret) {
		pr_err ("adding gpiochip failed\n");
	}

	gpiod_add_lookup_table (&gpio_apu_lookup_tbl);

	return ret;
}

static int gpio_apu_remove (struct platform_device *dev)
{
	gpiod_remove_lookup_table (&gpio_apu_lookup_tbl);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
	int ret;
	ret = gpiochip_remove (&gpio_apu_chip);
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0) */
	gpiochip_remove (&gpio_apu_chip);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0) */
	return 0;
}

static struct platform_driver gpio_apu_driver = {
	.probe = gpio_apu_probe,
	.remove = gpio_apu_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = DEVNAME
	}
};

static int __init gpio_apu_init (void)
{
	int err;

	pr_info ("load APU GPIO driver module\n");

	err = platform_driver_register (&gpio_apu_driver);
	if (err)
		goto exit;

	gpio_apu_platform_device = platform_device_register_simple (DEVNAME, -1, NULL, 0);
	if (IS_ERR(gpio_apu_platform_device)) {
		err = PTR_ERR(gpio_apu_platform_device);
		goto exit_driver;
	}

	pr_info ("APU GPIO driver module loaded\n");
	return 0;

exit_driver:
	platform_driver_unregister (&gpio_apu_driver);
exit:
	return err;
}

static void __exit gpio_apu_exit (void)
{
	platform_device_unregister (gpio_apu_platform_device);
	platform_driver_unregister (&gpio_apu_driver);
	pr_info ("APU GPIO driver module unloaded\n");
}

MODULE_AUTHOR ("Carsten Spiess <fli4l at carsten-spiess.de>");
MODULE_DESCRIPTION("GPIO driver for AMD A55 FCH on PC-Engines APU");
MODULE_LICENSE("GPL");

module_init (gpio_apu_init);
module_exit (gpio_apu_exit);
