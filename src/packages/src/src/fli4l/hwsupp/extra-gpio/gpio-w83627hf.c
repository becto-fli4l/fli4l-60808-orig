/*
 *  gpio-w83627hf.c - GPIO support for w83627hf chip
 *
 *  Copyright (c) 2014  Carsten Spiess <fli4l at carsten-spiess.de>
 *
 *  based on source from
 *  Copyright (c) 2010-2011  Rodolfo Giometti <giometti at linux.it>
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

#define DRVNAME "gpio-w83627hf"
#define GPIO_NAME "w83627hf/thf/hg/dhg gpio"

/* Constants specified below */
/* logical device numbers for superio_select (below) */
#define W83627HF_LD_FDC		0x00
#define W83627HF_LD_PRT		0x01
#define W83627HF_LD_UART1	0x02
#define W83627HF_LD_UART2	0x03
#define W83627HF_LD_KBC		0x05
#define W83627HF_LD_CIR		0x06 /* w83627hf only */
#define W83627HF_LD_GAME	0x07
#define W83627HF_LD_MIDI	0x07
#define W83627HF_LD_GPIO1	0x07
#define W83627THF_LD_GPIO5	0x07 /* w83627thf only */
#define W83627HF_LD_GPIO2	0x08
#define W83627HF_LD_GPIO3	0x09
#define W83627THF_LD_GPIO4	0x09 /* w83627thf only */
#define W83627HF_LD_ACPI	0x0a
#define W83627HF_LD_HWM		0x0b

/* registers */
#define W83627HF_REG_LD		0x07 /* Logical device select */
#define W83627HF_REG_DEVID	0x20 /* Device ID */

/* known device ID's */
#define W83627HF_DEVID		0x52
#define W83627THF_DEVID		0x82

/* type declarations */
/* supported chip types */
enum chips { w83627hf, w83627thf};

/* command sequences */
struct w83627hf_cmd_seq {
	int ld; /* logical device */
	int reg; /* register */
	u8 clr; /* clr bits */
	u8 set; /* set bits */
};

/* port configuration */
struct w83627hf_port_data {
	int ngpio; /* number of gpio's per port */
	u8 gpio_mask; /* bit mask for used gpio bits */
	int ld; /* logical device */
	int dir; /* direction register */
	int data; /* i/o register */
	const struct w83627hf_cmd_seq *enable; /* cmd sequenece to enable port */
	const struct w83627hf_cmd_seq *disable; /* cmd sequenece to disable port */
};

/* chip specific config */
struct w83627hf_config_data {
	const char *name;
	struct w83627hf_port_data *port_data;
};

/* sio data (passed as container data) */
struct w83627hf_sio_data {
	struct gpio_chip chip;
	int sioaddr;
	enum chips type;
	u8 status [8];
};

/* function forward declarations */
static int w83627hf_gpio_probe (struct platform_device *pdev);
static int w83627hf_gpio_remove (struct platform_device *pdev);

/* W83627HF enable/disable sequences */
/* W83627HF GP1x */
static const struct w83627hf_cmd_seq w83627hf_port1_enable [] = {
	{-1, 0x2a, 0xfc, 0xfc},
	{W83627HF_LD_GPIO1, 0x30, 0x03, 0x01},
	{-1, -1} /* last entry must set register -1 */
};
static const struct w83627hf_cmd_seq w83627hf_port1_disable [] = {
	{W83627HF_LD_GPIO1, 0x30, 0x03, 0x00},
	{-1, -1} /* last entry must set register -1 */
};

/* W83627HF GP2x */
static const struct w83627hf_cmd_seq w83627hf_port2_enable [] = {
	{-1, 0x2a, 0, 0x01},
	{-1, 0x2b, 0, 0xff},
	{W83627HF_LD_GPIO2, 0x30, 0x01, 0x01},
	{-1, -1} /* last entry must set register -1 */
};
static const struct w83627hf_cmd_seq w83627hf_port2_disable [] = {
	{W83627HF_LD_GPIO2, 0x30, 0x01, 0x00},
	{-1, -1} /* last entry must set register -1 */
};

/* W83627HF GP3x */
static const struct w83627hf_cmd_seq w83627hf_port3_enable [] = {
	{-1, 0x29, 0, 0xfc},
	{W83627HF_LD_GPIO3, 0x30, 0x01, 0x01},
	{-1, -1} /* last entry must set register -1 */
};
static const struct w83627hf_cmd_seq w83627hf_port3_disable [] = {
	{W83627HF_LD_GPIO3, 0x30, 0x01, 0x01},
	{-1, -1} /* last entry must set register -1 */
};

/* W83627THF enable/disable sequences */
/* W83627THF GP1x */
static const struct w83627hf_cmd_seq w83627thf_port1_enable [] = {
	{-1, 0x29, 0xc0, 0x40},
	{W83627HF_LD_GPIO1, 0x30, 0x07, 0x01},
	{-1, -1} /* last entry must set register -1 */
};
static const struct w83627hf_cmd_seq w83627thf_port1_disable [] = {
	{W83627HF_LD_GPIO1, 0x30, 0x07, 0x00},
	{-1, -1} /* last entry must set register -1 */
};

/* W83627THF GP2x */
static const struct w83627hf_cmd_seq w83627thf_port2_enable [] = {
	{-1, 0x29, 0x03, 0x01},
	{-1, 0x2a, 0xfe, 0x7e},
	{W83627HF_LD_GPIO2, 0x30, 0x01, 0x01},
	{-1, -1} /* last entry must set register -1 */
};
static const struct w83627hf_cmd_seq w83627thf_port2_disable [] = {
	{W83627HF_LD_GPIO2, 0x30, 0x01, 0x00},
	{-1, -1} /* last entry must set register -1 */
};

/* W83627THF GP3x */
static const struct w83627hf_cmd_seq w83627thf_port3_enable [] = {
	{-1, 0x2b, 0x7f, 0x10},
	{-1, 0x2c, 0xfc, 0x04},
	{W83627HF_LD_GPIO3, 0x30, 0x01, 0x01},
	{-1, -1} /* last entry must set register -1 */
};
static const struct w83627hf_cmd_seq w83627thf_port3_disable [] = {
	{W83627HF_LD_GPIO3, 0x30, 0x01, 0x00},
	{-1, -1} /* last entry must set register -1 */
};

/* W83627THF GP4x */
static const struct w83627hf_cmd_seq w83627thf_port4_enable [] = {
	{-1, 0x2d, 0xff, 0xfe},
	{W83627THF_LD_GPIO4, 0x30, 0x02, 0x02},
	{-1, -1} /* last entry must set register -1 */
};
static const struct w83627hf_cmd_seq w83627thf_port4_disable [] = {
	{W83627THF_LD_GPIO4, 0x30, 0x02, 0x00},
	{-1, -1} /* last entry must set register -1 */
};

/* W83627THF GP5x */
static const struct w83627hf_cmd_seq w83627thf_port5_enable [] = {
	{-1, 0x29, 0x20, 0},
	{W83627THF_LD_GPIO5, 0x30, 0x08, 0x08},
	{-1, -1} /* last entry must set register -1 */
};
static const struct w83627hf_cmd_seq w83627thf_port5_disable [] = {
	{W83627THF_LD_GPIO5, 0x30, 0x08, 0x00},
	{-1, -1} /* last entry must set register -1 */
};

/* W83627THF config data */
static struct w83627hf_port_data w83627hf_port_data[] = {
	{
		.ngpio = 8,
		. gpio_mask = 0xff,
		.ld = W83627HF_LD_GPIO1,
		.dir = 0xf0,
		.data = 0xf1,
		w83627hf_port1_enable,
		w83627hf_port1_disable,
	},
	{
		.ngpio = 8,
		. gpio_mask = 0xff,
		.ld = W83627HF_LD_GPIO2,
		.dir = 0xf0,
		.data = 0xf1,
		w83627hf_port2_enable,
		w83627hf_port2_disable,
	},
	{
		.ngpio = 6,
		. gpio_mask = 0x3f,
		.ld = W83627HF_LD_GPIO3,
		.dir = 0xf0,
		.data = 0xf1,
		w83627hf_port3_enable,
		w83627hf_port3_disable,
	},
	{0} /* last entry must be empty! */
};

/* W83627THF config data */
static struct w83627hf_port_data w83627thf_port_data[] = {
	{
		.ngpio = 8,
		. gpio_mask = 0xff,
		.ld = W83627HF_LD_GPIO1,
		.dir = 0xf0,
		.data = 0xf1,
		w83627thf_port1_enable,
		w83627thf_port1_disable,
	},
	{
		.ngpio = 7,
		. gpio_mask = 0x7f,
		.ld = W83627HF_LD_GPIO2,
		.dir = 0xf0,
		.data = 0xf1,
		w83627thf_port2_enable,
		w83627thf_port2_disable,
	},
	{
		.ngpio = 7,
		. gpio_mask = 0xbf,
		.ld = W83627HF_LD_GPIO3,
		.dir = 0xf0,
		.data = 0xf1,
		w83627thf_port3_enable,
		w83627thf_port3_disable,
	},
	{
		.ngpio = 8,
		. gpio_mask = 0xff,
		.ld = W83627THF_LD_GPIO4,
		.dir = 0xf4,
		.data = 0xf5,
		w83627thf_port4_enable,
		w83627thf_port4_disable,
	},
	{
		.ngpio = 6,
		. gpio_mask = 0x3f,
		.ld = W83627THF_LD_GPIO5,
		.dir = 0xf3,
		.data = 0xf4,
		w83627thf_port5_enable,
		w83627thf_port5_disable,
	},
	{0} /* last entry must be empty! */
};

/* array of chip specific config data */
static const struct w83627hf_config_data w83627hf_config_data[] = {
	[w83627hf] = {
		.name = "w83627hf",
		.port_data = w83627hf_port_data
	},
	[w83627thf] = {
		.name = "w83627thf",
		.port_data = w83627thf_port_data
	},
};

/* the one and only device pointer */
static struct platform_device *w83627hf_gpio_pdev;

/* driver structure */
static struct platform_driver w83627hf_gpio_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= DRVNAME,
	},
	.probe		= w83627hf_gpio_probe,
	.remove		= w83627hf_gpio_remove,
};

/* module parameters */
static unsigned short force_id;
module_param (force_id, ushort, 0);
MODULE_PARM_DESC (force_id, "Override the detected device ID");


/* local functions */
/* super io in/out */
static inline int superio_enter (struct w83627hf_sio_data *sio_data)
{
	if (!request_muxed_region (sio_data->sioaddr, 2, GPIO_NAME))
		return -EBUSY;

	outb (0x87, sio_data->sioaddr);
	outb (0x87, sio_data->sioaddr);
	return 0;
}

static inline void superio_exit (struct w83627hf_sio_data *sio_data)
{
	outb (0xAA, sio_data->sioaddr);
	release_region (sio_data->sioaddr, 2);
}

static inline void superio_select (struct w83627hf_sio_data *sio_data, u8 ld)
{
	outb (W83627HF_REG_LD, sio_data->sioaddr);
	outb (ld,  sio_data->sioaddr + 1);
}

static inline void superio_outb (struct w83627hf_sio_data *sio_data, u8 reg, u8 val)
{
	outb (reg, sio_data->sioaddr);
	outb (val, sio_data->sioaddr + 1);
}

static inline int superio_inb (struct w83627hf_sio_data *sio_data, u8 reg)
{
	outb (reg, sio_data->sioaddr);
	return inb (sio_data->sioaddr + 1);
}

/* get port_data port and bit number from offset */
static int w83627hf_gpio_get_port_bit (struct w83627hf_sio_data *sio_data,
		unsigned int offset,
		struct w83627hf_port_data **_port_data,
		int* _port, int* _bit)
{
	int port, bit, test;
	int ngpio = 0, last;

	for (port = 0; w83627hf_config_data[sio_data->type].port_data[port].ngpio > 0; port++) {
		last = ngpio;
		ngpio += w83627hf_config_data[sio_data->type].port_data[port].ngpio;
		if (offset >= ngpio) {
			continue;
		}
		for (test=0, bit=0; bit<8; bit++) {
			if (w83627hf_config_data[sio_data->type].port_data[port].gpio_mask & (1<<bit)) {
				if (test == (offset - last)) {
					*_port_data = &w83627hf_config_data[sio_data->type].port_data[port];
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
static void w83627hf_gpio_exec_cmd_seq (struct w83627hf_sio_data *sio_data,
		const struct w83627hf_cmd_seq *cmd)
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
static int w83627hf_gpio_request (struct gpio_chip *chip, unsigned offset)
{
	int ret, port, bit;
	struct w83627hf_sio_data *sio_data =
			container_of (chip, struct w83627hf_sio_data, chip);

	struct w83627hf_port_data *port_data;

	if ((ret = w83627hf_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit))) {
		BUG ();
		return ret;
	}

	w83627hf_gpio_exec_cmd_seq (sio_data, port_data->enable);
	sio_data->status[port] |= (1<<bit);

	return 0;
}

/* release gpio access */
static void w83627hf_gpio_free (struct gpio_chip *chip, unsigned offset)
{
	int port, bit;
	struct w83627hf_sio_data *sio_data =
			container_of (chip, struct w83627hf_sio_data, chip);
	struct w83627hf_port_data *port_data;

	if (w83627hf_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit)) {
		BUG ();
		return;
	}

	sio_data->status[port] &= ~ (1<<bit);
	if (0 == sio_data->status[port])
	{
		w83627hf_gpio_exec_cmd_seq (sio_data, port_data->disable);
	}
}

/* get GPIO direction */
static int w83627hf_gpio_get_dir (struct gpio_chip *chip, unsigned offset)
{
	int ret, port, bit;
	u8 val;
	struct w83627hf_sio_data *sio_data =
			container_of (chip, struct w83627hf_sio_data, chip);
	struct w83627hf_port_data *port_data;

	if ((ret = w83627hf_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit))) {
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
static int w83627hf_gpio_dir_in (struct gpio_chip *chip, unsigned offset)
{
	int ret, port, bit;
	u8 val;
	struct w83627hf_sio_data *sio_data =
			container_of (chip, struct w83627hf_sio_data, chip);
	struct w83627hf_port_data *port_data;

	if ((ret = w83627hf_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit))) {
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
static int w83627hf_gpio_dir_out (struct gpio_chip *chip,
						unsigned offset, int value)
{
	int ret, port, bit;
	u8 val;
	struct w83627hf_sio_data *sio_data =
			container_of (chip, struct w83627hf_sio_data, chip);
	struct w83627hf_port_data *port_data;

	if ((ret = w83627hf_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit))) {
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
static int w83627hf_gpio_get_data (struct gpio_chip *chip, unsigned offset)
{
	int ret, port, bit;
	u8 val;
	struct w83627hf_sio_data *sio_data =
			container_of (chip, struct w83627hf_sio_data, chip);
	struct w83627hf_port_data *port_data;

	if ((ret = w83627hf_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit))) {
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
static void w83627hf_gpio_set_data (struct gpio_chip *chip, unsigned offset, int value)
{
	int port, bit;
	u8 val;
	struct w83627hf_sio_data *sio_data =
			container_of (chip, struct w83627hf_sio_data, chip);
	struct w83627hf_port_data *port_data;

	if (w83627hf_gpio_get_port_bit (sio_data, offset, &port_data, &port, &bit)) {
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
static int w83627hf_gpio_probe (struct platform_device *pdev)
{
	int err, ngpio, port;
	struct w83627hf_sio_data *sio_data = pdev->dev.platform_data;
	const struct w83627hf_config_data *config_data = &w83627hf_config_data[sio_data->type];

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
	sio_data->chip.request = w83627hf_gpio_request;
	sio_data->chip.free = w83627hf_gpio_free;
	sio_data->chip.get_direction = w83627hf_gpio_get_dir;
	sio_data->chip.direction_input = w83627hf_gpio_dir_in;
	sio_data->chip.direction_output = w83627hf_gpio_dir_out;
	sio_data->chip.get = w83627hf_gpio_get_data;
	sio_data->chip.set = w83627hf_gpio_set_data;
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
static int w83627hf_gpio_remove (struct platform_device *pdev)
{
	struct w83627hf_sio_data *sio_data = platform_get_drvdata (pdev);
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

static int __init w83627hf_gpio_device_add (const struct w83627hf_sio_data *sio_data)
{
	int err;

	w83627hf_gpio_pdev = platform_device_alloc (DRVNAME, -1);
	if (!w83627hf_gpio_pdev) {
		err = -ENOMEM;
		pr_err ("Device allocation failed\n");
		goto exit;
	}

	err = platform_device_add_data (w83627hf_gpio_pdev, sio_data,
				       sizeof (struct w83627hf_sio_data));
	if (err) {
		pr_err ("Platform data allocation failed\n");
		goto exit_device_put;
	}

	err = platform_device_add (w83627hf_gpio_pdev);
	if (err) {
		pr_err ("Device addition failed (%d)\n", err);
		goto exit_device_put;
	}

	return 0;

exit_device_put:
	platform_device_put (w83627hf_gpio_pdev);
exit:
	return err;
}

/*
 * Module stuff
 */

static int __init w83627hf_gpio_device_find (int sioaddr,
				struct w83627hf_sio_data *sio_data)
{
	int err = -ENODEV, ret;
	u16 val;

	sio_data->sioaddr = sioaddr;
	ret = superio_enter (sio_data);
	if (ret)
		return ret;

	val = force_id ? force_id : superio_inb (sio_data, W83627HF_REG_DEVID);
	switch (val) {
	case W83627HF_DEVID:
		sio_data->type = w83627hf;
		break;
	case W83627THF_DEVID:
		sio_data->type = w83627thf;
		break;
	case 0xff:	/* No device at all */
		goto exit;
	default:
		pr_debug (DRVNAME ": Unsupported chip (DEVID=0x%02x)\n", val);
		goto exit;
	}

	err = 0;
	pr_info (DRVNAME ": Found %s chip at %#x\n",
		w83627hf_config_data[sio_data->type].name, sioaddr);

 exit:
	superio_exit (sio_data);
	return err;
}

static int __init gpio_w83627hf_init (void)
{
	int err;
	struct w83627hf_sio_data sio_data;

	memset (&sio_data, 0, sizeof (sio_data));

	if (w83627hf_gpio_device_find (0x2e, &sio_data)
	 && w83627hf_gpio_device_find (0x4e, &sio_data))
		return -ENODEV;

	err = platform_driver_register (&w83627hf_gpio_driver);
	if (err)
		goto exit;

	/* Sets global w83627hf_gpio_pdev as a side effect */
	err = w83627hf_gpio_device_add (&sio_data);
	if (err)
		goto exit_driver;

	return 0;

exit_driver:
	platform_driver_unregister (&w83627hf_gpio_driver);
exit:
	return err;
}

static void __exit gpio_w83627hf_exit (void)
{
	platform_device_unregister (w83627hf_gpio_pdev);
	platform_driver_unregister (&w83627hf_gpio_driver);
}

MODULE_AUTHOR ("Carsten Spiess <fli4l at carsten-spiess.de>");
MODULE_DESCRIPTION ("W83627HF/THF gpio driver");
MODULE_LICENSE ("GPL");

module_init (gpio_w83627hf_init);
module_exit (gpio_w83627hf_exit);
