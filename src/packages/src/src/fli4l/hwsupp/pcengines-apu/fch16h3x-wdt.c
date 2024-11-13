/*
 *  fch16h3x-wdt : watchdog timer driver for AMD Family 16h Model 30h-3Fh FCH
 *
 *  Copyright (c) 2016  Carsten Spiess <fli4l at carsten-spiess.de>
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
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define DEVNAME                 "fch16h3x-wdt"

#define FCH_ACPI_MMIO_BASE      0xFED80000
#define FCH_WDT_BASE            (FCH_ACPI_MMIO_BASE + 0xB00)
#define FCH_WDT_SIZE            0x08

#define FCH_WDT_CONTROL(base)   ((base) + 0x00) /* Watchdog Control */
#define FCH_WDT_COUNT(base)     ((base) + 0x04) /* Watchdog Count */

#define FCH_WDT_BIT_RUN_STOP    0
#define FCH_WDT_BIT_FIRED       1
#define FCH_WDT_BIT_ACTION      2
#define FCH_WDT_BIT_DISABLE     3
#define FCH_WDT_BIT_TRIGGER     7

/* internal variables */
static struct pci_dev *watchdog_pci = NULL;
static DEFINE_SPINLOCK(watchdog_lock);
static void __iomem *watchdog_base = NULL;
static volatile unsigned long watchdog_timer_alive = 0;
static bool watchdog_expect_close = false;;

/* the watchdog platform device */
static struct platform_device *watchdog_platform_device;

static const struct pci_device_id watchdog_pci_tbl[] ={
	{PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_HUDSON2_SMBUS, PCI_ANY_ID, PCI_ANY_ID},
	{ 0, } /* End of list */
};
MODULE_DEVICE_TABLE ( pci, watchdog_pci_tbl);

/* module parameters */

#define WATCHDOG_HEARTBEAT 60	/* 60 sec default heartbeat. */
static int heartbeat = WATCHDOG_HEARTBEAT;  /* in seconds */
module_param(heartbeat, int, 0);
MODULE_PARM_DESC(heartbeat, "Watchdog heartbeat in seconds. (default="
		 __MODULE_STRING(WATCHDOG_HEARTBEAT) ")");

static bool nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started."
		" (default=" __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

/*
 * Some TCO specific functions
 */
static void watchdog_start(void)
{
	u32 val;
	unsigned long flags;

	spin_lock_irqsave(&watchdog_lock, flags);

	val = ioread32(FCH_WDT_CONTROL(watchdog_base));
	val |= BIT(FCH_WDT_BIT_RUN_STOP);
	iowrite32(val, FCH_WDT_CONTROL(watchdog_base));

	spin_unlock_irqrestore(&watchdog_lock, flags);
}

static void watchdog_stop(void)
{
	u32 val;
	unsigned long flags;

	spin_lock_irqsave(&watchdog_lock, flags);

	val = ioread32(FCH_WDT_CONTROL(watchdog_base));
	val &= ~BIT(FCH_WDT_BIT_RUN_STOP);
	iowrite32(val, FCH_WDT_CONTROL(watchdog_base));

	spin_unlock_irqrestore(&watchdog_lock, flags);
}

static void watchdog_keepalive(void)
{
	u32 val;
	unsigned long flags;

	spin_lock_irqsave(&watchdog_lock, flags);

	val = ioread32(FCH_WDT_CONTROL(watchdog_base));
	val |= BIT(FCH_WDT_BIT_TRIGGER);
	iowrite32(val, FCH_WDT_CONTROL(watchdog_base));

	spin_unlock_irqrestore(&watchdog_lock, flags);
}

static int watchdog_set_heartbeat(int t)
{
	unsigned long flags;

	if (t < 0 || t > 0xffff)
		return -EINVAL;

	/* Write new heartbeat to watchdog */
	spin_lock_irqsave(&watchdog_lock, flags);

	iowrite32(t, FCH_WDT_COUNT(watchdog_base));

	spin_unlock_irqrestore(&watchdog_lock, flags);

	heartbeat = t;
	return 0;
}

static void watchdog_enable(void)
{
	u32 val;
	unsigned long flags;

	spin_lock_irqsave(&watchdog_lock, flags);

	val = ioread32(FCH_WDT_CONTROL(watchdog_base));
	val &= ~BIT(FCH_WDT_BIT_DISABLE);
	val &= ~BIT(FCH_WDT_BIT_ACTION);
	iowrite32(val, FCH_WDT_CONTROL(watchdog_base));

	spin_unlock_irqrestore(&watchdog_lock, flags);
}

/*
 *	/dev/watchdog handling
 */
static int watchdog_open(struct inode *inode, struct file *file)
{
	/* /dev/watchdog can only be opened once */
	if (test_and_set_bit(0, &watchdog_timer_alive))
		return -EBUSY;

	/* Reload and activate timer */
	watchdog_start();
	watchdog_keepalive();
	return nonseekable_open(inode, file);
}

static int watchdog_release(struct inode *inode, struct file *file)
{
	/* Shut off the timer. */
	if (watchdog_expect_close) {
		watchdog_stop();
	} else {
		pr_crit("Unexpected close, not stopping watchdog!\n");
		watchdog_keepalive();
	}
	clear_bit(0, &watchdog_timer_alive);
	watchdog_expect_close = false;
	return 0;
}

static ssize_t watchdog_write(struct file *file, const char __user *data,
				size_t len, loff_t *ppos)
{
	/* See if we got the magic character 'V' and reload the timer */
	if (len) {
		if (!nowayout) {
			size_t i;

			watchdog_expect_close = false;

			for (i = 0; i != len; i++) {
				char c;
				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					watchdog_expect_close = true;
			}
		}

		watchdog_keepalive();
	}
	return len;
}

static long watchdog_ioctl(struct file *file, unsigned int cmd,
			     unsigned long arg)
{
	int new_options, retval = -EINVAL;
	int new_heartbeat;
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	static const struct watchdog_info ident = {
		.options =		WDIOF_SETTIMEOUT |
					WDIOF_KEEPALIVEPING |
					WDIOF_MAGICCLOSE,
		.firmware_version =	0,
		.identity =		DEVNAME,
	};

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		return copy_to_user(argp, &ident,
			sizeof(ident)) ? -EFAULT : 0;
	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		return put_user(0, p);
	case WDIOC_SETOPTIONS:
		if (get_user(new_options, p))
			return -EFAULT;
		if (new_options & WDIOS_DISABLECARD) {
			watchdog_stop();
			retval = 0;
		}
		if (new_options & WDIOS_ENABLECARD) {
			watchdog_start();
			watchdog_keepalive();
			retval = 0;
		}
		return retval;
	case WDIOC_KEEPALIVE:
		watchdog_keepalive();
		return 0;
	case WDIOC_SETTIMEOUT:
		if (get_user(new_heartbeat, p))
			return -EFAULT;
		if (watchdog_set_heartbeat(new_heartbeat))
			return -EINVAL;
		watchdog_keepalive();
		/* Fall through */
	case WDIOC_GETTIMEOUT:
		return put_user(heartbeat, p);
	default:
		return -ENOTTY;
	}
}

/* Kernel Interfaces */
static const struct file_operations watchdog_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.write = watchdog_write,
	.unlocked_ioctl = watchdog_ioctl,
	.open = watchdog_open,
	.release = watchdog_release,
};

static struct miscdevice watchdog_miscdev = {
	.minor = WATCHDOG_MINOR,
	.name = "watchdog",
	.fops = &watchdog_fops,
};

static int watchdog_probe(struct platform_device *dev)
{
	int ret = 0;
	struct pci_dev *pci_dev = NULL;
	u32 fired;

	/* Match the PCI device */
	for_each_pci_dev(pci_dev) {
		if (pci_match_id(watchdog_pci_tbl, pci_dev) != NULL) {
			watchdog_pci = pci_dev;
			break;
		}
	}

	if (!watchdog_pci)
		return -ENODEV;

	pr_info("PCI Revision ID: 0x%x\n", watchdog_pci->revision);

	/* Determine type of southbridge chipset */
	if (watchdog_pci->revision < 0x40) {
		return -EACCES;
	}

	/* Request memory region for GPIO's */
	if (!devm_request_mem_region(&dev->dev, FCH_WDT_BASE,
			FCH_WDT_SIZE, DEVNAME))	{
		pr_err("request watchdog mem region failed\n");
		return -ENXIO;
	}

	/* Map IO's for watchdog timer  */
	watchdog_base = devm_ioremap (&dev->dev, FCH_WDT_BASE, FCH_WDT_SIZE);

	if (!watchdog_base) {
		pr_err ("map watchdog address failed\n");
		return -ENXIO;
	}

	/* Check to see if last reboot was due to watchdog timeout */
	fired = ioread32(FCH_WDT_CONTROL(watchdog_base)) & BIT(FCH_WDT_BIT_FIRED);
	pr_info("Last reboot was %striggered by watchdog.\n",
		fired ? "" : "not ");

	/* enable and stop watchdog */
	watchdog_enable();
	watchdog_stop();

	/*
	 * Check that the heartbeat value is within it's range.
	 * If not, reset to the default.
	 */
	if (watchdog_set_heartbeat(heartbeat)) {
		heartbeat = WATCHDOG_HEARTBEAT;
		watchdog_set_heartbeat(heartbeat);
	}

	ret = misc_register(&watchdog_miscdev);
	if (ret != 0) {
		pr_err("cannot register miscdev on minor=%d (err=%d)\n",
		       WATCHDOG_MINOR, ret);
		return ret;
	}

	clear_bit(0, &watchdog_timer_alive);

	/* Show module parameters */
	pr_info("initialized (0x%p). heartbeat=%d sec (nowayout=%d)\n",
		watchdog_base, heartbeat, nowayout);

	return 0;
}

static int watchdog_remove(struct platform_device *dev)
{
	/* Stop the timer before we leave */
	if (!nowayout) {
		if (watchdog_base) {
			watchdog_stop();
		}
	}

	/* Deregister */
	misc_deregister(&watchdog_miscdev);
	return 0;
}

static void watchdog_shutdown(struct platform_device *dev)
{
	watchdog_stop();
}

static struct platform_driver watchdog_driver = {
	.probe = watchdog_probe,
	.remove = watchdog_remove,
	.shutdown = watchdog_shutdown,
	.driver		= {
			.owner = THIS_MODULE,
			.name = DEVNAME
	},
};

static int __init fch16h3x_wdt_init (void)
{
	int err;

	pr_info ("load AMD Family 16h Model 3Xh FCH watchdog driver module\n");

	err = platform_driver_register (&watchdog_driver);
	if (err)
		goto exit;

	watchdog_platform_device = platform_device_register_simple (DEVNAME, -1, NULL, 0);
	if (IS_ERR(watchdog_platform_device)) {
		err = PTR_ERR(watchdog_platform_device);
		goto exit_driver;
	}

	pr_info ("AMD Family 16h Model 3Xh FCH watchdog driver module loaded\n");
	return 0;

exit_driver:
	platform_driver_unregister (&watchdog_driver);
exit:
	return err;
}

static void __exit fch16h3x_wdt_exit (void)
{
	platform_device_unregister (watchdog_platform_device);
	platform_driver_unregister (&watchdog_driver);
	pr_info ("AMD Family 16h Model 3Xh FCH watchdog module unloaded\n");
}

MODULE_AUTHOR("Carsten Spiess");
MODULE_DESCRIPTION("Watchdog timer driver for AMD Family 16h Model 3Xh FCH");
MODULE_LICENSE("GPL");

module_init(fch16h3x_wdt_init);
module_exit(fch16h3x_wdt_exit);

