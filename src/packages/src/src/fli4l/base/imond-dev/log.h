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
 * \brief Public logging functions
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#ifndef IMOND_LOG_H
#define IMOND_LOG_H

#include <syslog.h>

/*----------------------------------------------------------------------------
 * imond_do_open_log (int append)
 * 	- open imond logfile, either in append or write mode
 * imond_close_log (char * log_file)
 * 	- close and sync imond logfile
 * imond_reset_log (char * log_file)
 * 	- reset imond logfile
 * imond_write_record (...)
 * 	 - write an accounting record to imond log file
 */

int imond_open_syslog (void);
void imond_syslog (int log_level, const char *fmt, ...);

void set_imond_log_dir (const char * log_dir);
const char * get_imond_log_file (void);
void imond_reset_log (void);
void imond_write_record (char * circuit_name,
			 time_t online_start,
			 /* time_t now, */
			 /* long diff, */ 
			 long charge_time,
			 double charge_val,
			 unsigned long ibytes_overflow, 
			 unsigned long ibytes,
			 unsigned long ibytes_start,
			 unsigned long obytes_overflow, 
			 unsigned long obytes,
			 unsigned long obytes_start,
			 char * device,
			 int charge_int,
			 double charge_per_minute);


#endif
