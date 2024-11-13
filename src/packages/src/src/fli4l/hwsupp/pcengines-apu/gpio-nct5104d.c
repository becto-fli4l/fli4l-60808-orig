/*
 *  gpio-nct5104d.c - GPIO support for nct5104d chip
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/version.h>

#define DRVNAME "gpio-nct5104d"

/* Constants specified below */
/* logical device numbers for superio_select (below) */
#define NCT5104D_LD_UARTA	0x02
#define NCT5104D_LD_UARTB	0x03
#define NCT5104D_LD_GPIO1	0x07
#define NCT5104D_LD_WDT1	0x08
#define nct5104d_LD_GPIO2	0x0F
#define NCT5104D_LD_UARTC	0x10
#define NCT5104D_LD_UARTD	0x11
#define nct5104d_LD_PORT80	0x14

/* registers */
/* common */
#define NCT5104D_REG_LD		0x07 /* Logical device select */
#define NCT5104D_REG_IDH	0x20 /* Chip ID high byte */
#define NCT5104D_REG_IDL	0x21 /* Chip ID low byte */

/* GPIO - LD 0x07 */

/* known device ID's */
#define NCT5104D_CHIPID			0x1061 /* standard Chip ID */
#define PCENGINES_APU_CHIPID	0xc452 /* PC engines Chip ID */
#define PCENGINES_APU_CHIPID2	0xc453 /* PC engines Chip ID APU-3 */

/* type declarations */
/* supported chip types */
enum chips { nct5104d};

/* command sequences */
struct nct5104d_cmd_seq {
	int ld; /* logical device */
	int reg; /* register */
	u8 clr; /* clr bits */
	u8 set; /* set bits */
};

/* port configuration */
struct nct5104d_port_data {
	int ngpio; /* number of gpio's per port */
	u8 gpio_mask; /* bit mask for used gpio bits */
	int ld; /* logical device */
	int dir; /* direction register */
	int data; /* i/o register */
	const struct nct5104d_cmd_seq *enable; /* cmd sequenece to enable port */
	const struct nct5104d_cmd_seq *disable; /* cmd sequenece to disable port */
};

/* chip specific config */
struct nct5104d_config_data {
	const char *name;
	struct nct5104d_port_data *port_data;
};

/* sio data (passed as container data) */
struct nct5104d_sio_data {
	struct gpio_chip chip;
	int sioaddr;
	enum chips type;
	u8 status [8];
};

/* function forward declarations */
static int nct5104d_gpio_probe (struct platform_device *pdev);
static int nct5104d_gpio_remove (struct platform_device *pdev);

/* nct5104d enable/disable sequences */
/* nct5104d GP0x */
static const struct nct5104d_cmd_seq nct5104d_port0_enable [] = {
	{NCT5104D_LD_GPIO1, 0x30, 0x01, 0x01},
	{-1, -1} /* last entry must set register -1 */
};
static const struct nct5104d_cmd_seq nct5104d_port0_disable [] = {
	{NCT5104D_LD_GPIO1, 0x30, 0x01, 0x00},
	{-1, -1} /* last entry must set register -1 */
};

/* nct5104d GP1x */
static const struct nct5104d_cmd_seq nct5104d_port1_enable [] = {
	{NCT5104D_LD_GPIO1, 0x30, 0x02, 0x02},
	{-1, -1} /* last entry must set register -1 */
};
static const struct nct5104d_cmd_seq nct5104d_port1_disable [] = {
	{NCT5104D_LD_GPIO1, 0x30, 0x02, 0x00},
	{-1, -1} /* last entry must set register -1 */
};

/* W83627THF config data */
static struct nct5104d_port_data nct5104d_port_data[] = {
	{
		.ngpio = 8,
		. gpio_mask = 0xff,
		.ld = NCT5104D_LD_GPIO1,
		.dir = 0xE0,
		.data = 0xE1,
		nct5104d_port0_enable,
		nct5104d_port0_disable,
	},
	{
		.ngpio = 8,
		. gpio_mask = 0xff,
		.ld = NCT5104D_LD_GPIO1,
		.dir = 0xE4,
		.data = 0xE5,
		nct5104d_port1_enable,
		nct5104d_port1_disable,
	},
	{0} /* last entry must be empty! */
};


/* array of chip specific config data */
static const struct nct5104d_config_data nct5104d_config_data[] = {
	[nct5104d] = {
		.name = "nct5104d",
		.port_data = nct5104d_port_data
	},
};

/* the one and only device pointer */
static struct platform_device *nct5104d_gpio_pdev;

/* driver structure */
static struct platform_driver nct5104d_gpio_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= DRVNAME,
	},
	.probe		= nct5104d_gpio_probe,
	.remove		= nct5104d_gpio_remove,
};

/* module parameters */
static unsigned short force_id;
module_param (force_id, ushort, 0);
MODULE_PARM_DESC (force_id, "Override the detected device ID");


/* local functions */
/* super io in/out */
static inline int superio_enter (struct nct5104d_sio_data *sio_data)
{
	if (!request_muxed_region (sio_data->sioaddr, 2, DRVNAME))
		return -EBUSY;

	outb (0x87, sio_data->sioaddr);
	outb (0x87, sio_data->sioaddr);
	return 0;
}

static inline void superio_exit (struct nct5104d_sio_data *sio_data)
{
	outb (0xAA, sio_data->sioaddr);
	release_region (sio_data->sioaddr, 2);
}

static inline void superio_select (struct nct5104d_sio_data *sio_data, u8 ld)
{
	outb (NCT5104D_REG_LD, sio_data->sioaddr);
	outb (ld,  sio_data->sioaddr + 1);
}

static inline void superio_outb (struct nct5104d_sio_data *sio_data, u8 reg, u8 val)
{
	outb (reg, sio_data->sioaddr);
	outb (val, sio_data->sioaddr + 1);
}

static inline int superio_inb (struct nct5104d_sio_data *sio_data, u8 reg)
{
	outb (reg, sio_data->sioaddr);
	return inb (sio_data->sioaddr + 1);
}

static inline int superio_inw(struct nct5104d_sio_data *sio_data, u8 reg)
{
	int val;
	outb (reg, sio_data->sioaddr);
	val = inb (sio_data->sioaddr + 1) << 8;
	outb (reg+1, sio_data->sioaddr);
	val |= inb (sio_data->sioaddr + 1);

	return val;
}

/* get port_data port and bit number from offset */
static int nct5104d_gpio_get_port_bit (struct nct5104d_sio_data *sio_data,
		unsigned int offset,
		struct nct5104d_port_data **_port_data,
		int* _port, int* _bit)
{
	int port, bit, test;
	int ngpio = 0, last;

	for (port = 0; nct5104d_config_data[sio_data->type].port_data[port].ngpio > 0; port++) {
		last = ngpio;
		ngpio += nct5104d_config_data[sio_data->type].port_data[port].ngpio;
		if (offset >= ngpio) {
			continue;
		}
		for (test=0, bit=0; bit<8; bit++) {
			if (nct5104d_config_data[sio_data->type].port_data[port].gpio_mask & (1<<bit)) {
				if (test == (offset - last)) {
					*_port_data = &nct5104d_config_data[sio_data->type].port_data[port];
					*_port = port;
					*_bit = bit;

					return 0;
				}
				test++;
			}
		}
	}

	return -ENODEV;
}

/* execute command sequence */
static void nct5104d_gpio_exec_cmd_seq (struct nct5104d_sio_data *sio_data,
		const struct nct5104d_cmd_seq *cmd)
{
	u8 val;

	if (!cmd)
		return;

	if (superio_enter (sio_data))
		return;

	while (cmd->reg >= 0) {
		if (cmd->ld >= 0) {
			superio_select (sio_data, cmd->ld);
		}

		val = superio_inb (sio_data, cmd->reg);
		val &= ~cmd->clr;
		val |= cmd->set;
		superio_outb (sio_data, cmd->reg, val);
		cmd++;
	};

	superio_exit (sio_data);
}

/* GPIOs methods */
/* request gpio access */
static int nct5104d_gpio_request (struct gpio_chip *chip, unsigned offset)
{
	int ret, port, bit;
	struct nct5104d_sio_data *sio_data =
			container_of (chip, struct nct5104d_sio_data, chip);

	struct nct5104d_port_data *port_data;

	if ((ret = nct5104d_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit))) {
		BUG ();
		return ret;
	}

	nct5104d_gpio_exec_cmd_seq (sio_data, port_data->enable);
	sio_data->status[port] |= (1<<bit);

	return 0;
}

/* release gpio access */
static void nct5104d_gpio_free (struct gpio_chip *chip, unsigned offset)
{
	int port, bit;
	struct nct5104d_sio_data *sio_data =
			container_of (chip, struct nct5104d_sio_data, chip);
	struct nct5104d_port_data *port_data;

	if (nct5104d_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit)) {
		BUG ();
		return;
	}

	sio_data->status[port] &= ~ (1<<bit);
	if (0 == sio_data->status[port])
	{
		nct5104d_gpio_exec_cmd_seq (sio_data, port_data->disable);
	}
}

/* get GPIO direction */
static int nct5104d_gpio_get_dir (struct gpio_chip *chip, unsigned offset)
{
	int ret, port, bit;
	u8 val;
	struct nct5104d_sio_data *sio_data =
			container_of (chip, struct nct5104d_sio_data, chip);
	struct nct5104d_port_data *port_data;

	if ((ret = nct5104d_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit))) {
		BUG ();
		return ret;
	}

	if ((ret = superio_enter (sio_data)))
		return ret;

	superio_select (sio_data, port_data->ld);

	val = superio_inb (sio_data, port_data->dir);

	superio_exit (sio_data);

	return (val >> bit) & 1;
}

/* GPIO as input */
static int nct5104d_gpio_dir_in (struct gpio_chip *chip, unsigned offset)
{
	int ret, port, bit;
	u8 val;
	struct nct5104d_sio_data *sio_data =
			container_of (chip, struct nct5104d_sio_data, chip);
	struct nct5104d_port_data *port_data;

	if ((ret = nct5104d_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit))) {
		BUG ();
		return ret;
	}

	if ((ret = superio_enter (sio_data)))
		return ret;

	superio_select (sio_data, port_data->ld);

	val = superio_inb (sio_data, port_data->dir);
	val |= (1 << bit);
	superio_outb (sio_data, port_data->dir, val);

	superio_exit (sio_data);

	return 0;
}

/* GPIO as output */
static int nct5104d_gpio_dir_out (struct gpio_chip *chip,
						unsigned offset, int value)
{
	int ret, port, bit;
	u8 val;
	struct nct5104d_sio_data *sio_data =
			container_of (chip, struct nct5104d_sio_data, chip);
	struct nct5104d_port_data *port_data;

	if ((ret = nct5104d_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit))) {
		BUG ();
		return ret;
	}

	if ((ret = superio_enter (sio_data)))
		return ret;

	superio_select (sio_data, port_data->ld);

	val = superio_inb (sio_data, port_data->dir);
	val &= ~ (1 << bit);
	superio_outb (sio_data, port_data->dir, val);

	val = superio_inb (sio_data, port_data->data);
	if (value) {
		val |= (1 << bit);
	}
	else {
		val &= ~(1 << bit);
	}
	superio_outb (sio_data, port_data->data, val);

	superio_exit (sio_data);

	return 0;
}

/* get GPIO value */
static int nct5104d_gpio_get_data (struct gpio_chip *chip, unsigned offset)
{
	int ret, port, bit;
	u8 val;
	struct nct5104d_sio_data *sio_data =
			container_of (chip, struct nct5104d_sio_data, chip);
	struct nct5104d_port_data *port_data;

	if ((ret = nct5104d_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit))) {
		BUG ();
		return ret;
	}

	if ((ret = superio_enter (sio_data)))
		return ret;

	superio_select (sio_data, port_data->ld);

	val = superio_inb (sio_data, port_data->data);

	superio_exit (sio_data);

	return (val >> bit) & 1;
}

/* set GPIO data */
static void nct5104d_gpio_set_data (struct gpio_chip *chip, unsigned offset, int value)
{
	int port, bit;
	u8 val;
	struct nct5104d_sio_data *sio_data =
			container_of (chip, struct nct5104d_sio_data, chip);
	struct nct5104d_port_data *port_data;

	if (nct5104d_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit)) {
		BUG ();
		return;
	}

	if (superio_enter (sio_data))
		return;

	superio_select (sio_data, port_data->ld);

	val = superio_inb (sio_data, port_data->data);
	if (value) {
		val |= (1 << bit);
	}
	else {
		val &= ~(1 << bit);
	}
	superio_outb (sio_data, port_data->data, val);

	superio_exit (sio_data);
}

/* probe device */
static int nct5104d_gpio_probe (struct platform_device *pdev)
{
	int err, ngpio, port;
	struct nct5104d_sio_data *sio_data = pdev->dev.platform_data;
	const struct nct5104d_config_data *config_data = &nct5104d_config_data[sio_data->type];

	for (port=0, ngpio=0; config_data->port_data[port].ngpio > 0; port++) {
		ngpio += config_data->port_data[port].ngpio;
	}
	if (0 == ngpio) {
		dev_err (&pdev->dev, "gpio support not available "
					"for this chip type\n");
		return -ENODEV;
	}

	platform_set_drvdata (pdev, sio_data);

	sio_data->chip.base = -1;
	sio_data->chip.ngpio = ngpio;
	sio_data->chip.label = config_data->name;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0)
	sio_data->chip.dev = &pdev->dev;
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0) */
	sio_data->chip.parent = &pdev->dev;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0) */

	sio_data->chip.owner = THIS_MODULE;
	sio_data->chip.request = nct5104d_gpio_request;
	sio_data->chip.free = nct5104d_gpio_free;
	sio_data->chip.get_direction = nct5104d_gpio_get_dir;
	sio_data->chip.direction_input = nct5104d_gpio_dir_in;
	sio_data->chip.direction_output = nct5104d_gpio_dir_out;
	sio_data->chip.get = nct5104d_gpio_get_data;
	sio_data->chip.set = nct5104d_gpio_set_data;
	sio_data->chip.can_sleep = 1;

	err = gpiochip_add (&sio_data->chip);
	if (err < 0) {
		dev_err (&pdev->dev, "could not register gpiochip, %d\n", err);
		goto exit;
	}

	dev_info (&pdev->dev, "GPIO support successfully loaded.\n");
	return 0;

exit:
	platform_set_drvdata (pdev, NULL);

	return err;
}

/* remove device */
static int nct5104d_gpio_remove (struct platform_device *pdev)
{
	struct nct5104d_sio_data *sio_data = platform_get_drvdata (pdev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
	int err;

	err = gpiochip_remove (&sio_data->chip);
	if (err < 0) {
		dev_err (&pdev->dev, "could not deregister gpiochip, %d\n", err);
		return err;
	}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0) */
	gpiochip_remove (&sio_data->chip);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0) */

	platform_set_drvdata (pdev, NULL);

	return 0;
}

static int __init nct5104d_gpio_device_add (const struct nct5104d_sio_data *sio_data)
{
	int err;

	nct5104d_gpio_pdev = platform_device_alloc (DRVNAME, -1);
	if (!nct5104d_gpio_pdev) {
		err = -ENOMEM;
		pr_err ("Device allocation failed\n");
		goto exit;
	}

	err = platform_device_add_data (nct5104d_gpio_pdev, sio_data,
				       sizeof (struct nct5104d_sio_data));
	if (err) {
		pr_err ("Platform data allocation failed\n");
		goto exit_device_put;
	}

	err = platform_device_add (nct5104d_gpio_pdev);
	if (err) {
		pr_err ("Device addition failed (%d)\n", err);
		goto exit_device_put;
	}

	return 0;

exit_device_put:
	platform_device_put (nct5104d_gpio_pdev);
exit:
	return err;
}

/*
 * Module stuff
 */

static int __init nct5104d_gpio_device_find (int sioaddr,
				struct nct5104d_sio_data *sio_data)
{
	int err = -ENODEV, ret;
	u16 val;

	sio_data->sioaddr = sioaddr;
	ret = superio_enter (sio_data);
	if (ret)
		return ret;

	val = force_id ? force_id : superio_inw (sio_data, NCT5104D_REG_IDH);
	switch (val) {
	case NCT5104D_CHIPID:
	case PCENGINES_APU_CHIPID:
	case PCENGINES_APU_CHIPID2:
		sio_data->type = nct5104d;
		break;
	case 0xff:	/* No device at all */
		goto exit;
	default:
		pr_debug (DRVNAME ": Unsupported chip (DEVID=0x%02x)\n", val);
		goto exit;
	}

	err = 0;
	pr_info (DRVNAME ": Found %s chip at %#x\n",
		nct5104d_config_data[sio_data->type].name, sioaddr);

 exit:
	superio_exit (sio_data);
	return err;
}

static int __init gpio_nct5104d_init (void)
{
	int err;
	struct nct5104d_sio_data sio_data;

	memset (&sio_data, 0, sizeof (sio_data));

	if (nct5104d_gpio_device_find (0x2e, &sio_data)
	 && nct5104d_gpio_device_find (0x4e, &sio_data))
		return -ENODEV;

	err = platform_driver_register (&nct5104d_gpio_driver);
	if (err)
		goto exit;

	/* Sets global nct5104d_gpio_pdev as a side effect */
	err = nct5104d_gpio_device_add (&sio_data);
	if (err)
		goto exit_driver;

	return 0;

exit_driver:
	platform_driver_unregister (&nct5104d_gpio_driver);
exit:
	return err;
}

static void __exit gpio_nct5104d_exit (void)
{
	platform_device_unregister (nct5104d_gpio_pdev);
	platform_driver_unregister (&nct5104d_gpio_driver);
}

MODULE_AUTHOR ("Carsten Spiess <fli4l at carsten-spiess.de>");
MODULE_DESCRIPTION ("NCT5104D gpio driver");
MODULE_LICENSE ("GPL");

module_init (gpio_nct5104d_init);
module_exit (gpio_nct5104d_exit);
