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
 * \brief Public functions for date parsing
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#ifndef IMOND_DATE_H
#define IMOND_DATE_H

/** first day of a unix week */
#define SUNDAY		 0
/** hours per day */
#define HOURS_PER_DAY	24
/** days per week */
#define DAYS_PER_WEEK	 7

int convert_week_day_to_day (char * week_day);
void add_days (int start_day, int start_month, int start_year, int n_days,
	       int * new_day_p, int * new_month_p);

int is_holiday (int day, int month, int year);
char * get_weekday (int nr);
#endif


