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
 * \brief Date parsing
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#include <string.h>
#ifdef TEST
#  include <stdio.h>
#endif

#include "date.h"

/** Well, true... */
#define TRUE			1
/** You guessed right: false */
#define FALSE			0

/** calulate if \a y is a leap year */
#define IS_LEAP_YEAR(y)	((((y) % 4) == 0) &&    \
			     ((((y) % 100) != 0) || (((y) % 400) == 0)))


/** integer function */
#define INT(x)	    (x)

/** describes one day of a year */
struct holidays
{
    int		day;	/**< The day of the month (1 == 1.) */
    int		month;	/**< The month (1 == January) */
};

/** number of currently defined holidays */
#define N_HOLIDAYS  9

/** dynamic struct for holidays of current year */
struct holidays holidays[N_HOLIDAYS];

/** This is a static struct for global definition of holidays. If the month is
 * == 0, this denotes an event \a day days after easter.  */
struct holidays holiday_table[N_HOLIDAYS] =
{
    {  1,  1 },				    /* Neujahr			    */
    {  1,  5 },				    /* Maifeiertag		    */
    {  3, 10 },				    /* Tag der deutschen Einheit    */
    { 25, 12 },				    /* 1. Weihnachtsfeiertag	    */
    { 26, 12 },				    /* 2. Weihnachtsfeiertag	    */

    { -2,  0 },				    /* Karfreitag		    */
    {  1,  0 },				    /* Ostermontag		    */
    { 39,  0 },				    /* Christi Himmelfahrt	    */
    { 50,  0 },				    /* Pfingstmontag		    */
#if 0
    { 60,  0 },				    /* Fronleichnam		    */
#endif
};

/** enumeration of the days per month */
static int  g_days_per_month[]    =
{
    31,					/* January			    */
    28,					/* February (may be changed to 29)  */
    31,					/* March			    */
    30,					/* April			    */
    31,					/* May				    */
    30,					/* June				    */
    31,					/* July				    */
    31,					/* August			    */
    30,					/* September			    */
    31,					/* October			    */
    30,					/* November			    */
    31					/* December			    */
};

/** enumeration of the german week days */
static char *	ge_wday_strings[DAYS_PER_WEEK] =
{
    "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"
};

/** enumeration of the englisch week days */
static char *	en_wday_strings[DAYS_PER_WEEK] =
{
    "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"
};

/**
 * This function returns the shortcut for the respecting weekday.
 *
 * \param index the index of the weekday, Sunday is 0, Monday is 1, etc.
 *
 * \returns pointer to the string that denotes the found weekday
 */
char*
get_weekday(int index)
{
    return en_wday_strings[index];
}

/**
 * This function converts a weekday to an index 0 - 6 (0 is Sunday).
 * It accepts German and English shortcuts (2 characters) for weekdays.
 *
 * \todo FIXME: add more error checking
 *
 * \param week_day string to convert
 *
 * \retval 0-6 if weekday is found
 * \retval -1 if weekday is not found
 */
int
convert_week_day_to_day (char * week_day)
{
    int	i;

    for (i = 0; i < DAYS_PER_WEEK; i++)
    {
	if (! strcmp (week_day, en_wday_strings[i]) ||
	    ! strcmp (week_day, ge_wday_strings[i]))
	{
	    return (i);
	}
    }

    return (-1);
} /* convert_week_day_to_day (week_day) */

/**
 * This function adds a specific number of days to a given date.
 *
 * \warning Since the year is not returned, it would not be a wise choice to
 * set \a n_days to more then 365 or less then -365, since there is no way to
 * find out in which year you landed...
 *
 * \param start_day day of month of the starting date (1 == 1.)
 * \param start_month month of year of the starting date (1 == January)
 * \param start_year year of the starting date (4-digit)
 * \param n_days number of days to add (or substract if negative)
 * \param new_day_p pointer to new value of day
 * \param new_month_p pointer to new value of month
 */
void
add_days (int start_day, int start_month, int start_year, int n_days,
	  int * new_day_p, int * new_month_p)
{
    int	days_this_month;

    while (n_days > 0)
    {
	start_day += n_days;

	days_this_month = g_days_per_month[start_month - 1];

	if (start_month == 1 && IS_LEAP_YEAR(start_year))
	{
	    days_this_month++;
	}

	if (start_day > days_this_month)
	{
	    n_days	= start_day - days_this_month - 1;
	    start_day	= 1;
	    start_month++;

	    if (start_month > 12)
	    {
		start_month = 1;
		start_year++;
	    }
	}
	else
	{
	    n_days = 0;
	}
    }

    while (n_days < 0)
    {
	start_day += n_days;

	if (start_day <= 0)
	{
	    n_days = start_day;

	    start_month--;

	    if (start_month == 0)
	    {
		start_month = 12;
		start_year--;
	    }

	    days_this_month = g_days_per_month[start_month - 1];

	    if (start_month == 1 && IS_LEAP_YEAR(start_year))
	    {
		days_this_month++;
	    }

	    start_day = days_this_month;
	}
	else
	{
	    n_days = 0;
	}
    }

    *new_day_p	    = start_day;
    *new_month_p    = start_month;

    return;
} /* add_days (start_day, start_month, start_year, n_days, new_day_p, new_month_p) */

/**
 * This function checks if the supplied day is a holiday.
 * 
 * \todo make the local vars more self-explain
 *
 * \param day the day to check (1 == 1.)
 * \param month the month to check (1 == January)
 * \param year the year to check (4-digit)
 *
 * \retval TRUE day is a holiday
 * \retval FALSE day is not a holiday
 */
int
is_holiday (int day, int month, int year)
{
    static int	last_year;
    static int	ostern_day, ostern_month;
    int		a, b, c, d, e;
    int		m, s, M, N, D;
    int		i;
    int		offset;

    if (last_year != year)
    {
	last_year = year;

	a = year % 19;
	b = year % 4;
	c = year % 7;

	m = INT ((8 * INT (year/100) + 13) / 25) - 2;
	s = INT (year/100) - INT (year/400) - 2;

	M = (15 + s - m) % 30;
	N = (6 + s) % 7;

	d = (M + 19 * a) % 30;

	if (d == 29)
	{
	    D = 28;
	}
	else if (d == 28 && a >= 11)
	{
	    D = 27;
	}
	else
	{
	    D = d;
	}

	e = (2 * b + 4 * c + 6 * D + N) % 7;

	offset = D + e + 1;

	ostern_day = 21 + offset;
	ostern_month = 3;

	while (ostern_day > 31)
	{
	    ostern_day -= 31;
	    ostern_month++;
	}

	for (i = 0; i < N_HOLIDAYS; i++)
	{
	    if (holiday_table[i].month == 0)
	    {
		add_days (ostern_day, ostern_month, year, holiday_table[i].day,
			  &(holidays[i].day), &(holidays[i].month));
	    }
	    else
	    {
		holidays[i].day	    = holiday_table[i].day;
		holidays[i].month   = holiday_table[i].month;
	    }
	}

#ifdef TEST
	printf ("Ostern: %02d.%02d.%04d\n", ostern_day, ostern_month, year);
#endif
    }

    for (i = 0; i < N_HOLIDAYS; i++)
    {
	if (holidays[i].day == day && holidays[i].month == month)
	{
	    return (TRUE);
	}
    }
    return (FALSE);
} /* is_holiday (day, month, year) */


