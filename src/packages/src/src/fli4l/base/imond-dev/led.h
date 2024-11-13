/*
 * IMonD - deamon to monitor/control network connections
 *
 * Copyright © 2000-2004 Frank Meyer <frank@fli4l.de> & the fli4l team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/**
 * \file
 * \brief Public functions for the serial LED
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#ifndef IMOND_LED_H
#define IMOND_LED_H

extern int led_fd;

/** switch the LED on */
#define led_on() do { if (led_fd < 0 && globals.led_device) 		\
				led_fd = open (globals.led_device, 0); 	\
		    } while (0);

/** switch the LED off */
#define led_off() do { if (led_fd >= 0) 				\
				close (led_fd), led_fd = -1;		\
		     } while (0);

#endif /* IMOND_LED_H */
