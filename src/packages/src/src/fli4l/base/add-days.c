/*----------------------------------------------------------------------------
 *  add-days.c - add some days to current date
 *
 *  should be started by fli4l-boot script /etc/rc if you have a Y2K problem
 *
 *  usage: add-days [--utc] number-of-days
 *
 *  Copyright (c) 2000-2001 - Frank Meyer <frank@fli4l.de>
 *  Copyright (c) 2001-2016 - fli4l-Team <team@fli4l.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Creation:	    27.05.2000  fm
 *  Last Update:    09.09.2001  fm
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

static long utc_offset ();

int
main (int argc, char ** argv)
{
    time_t	seconds;
    struct tm *	tm;
    int		utc = 0;
    char *	pgm_name = argv[0];

    if (argc == 3 && ! strcmp (argv[1], "--utc"))
    {
	utc = 1;
	argv++;
	argc--;
    }

    if (argc != 2)
    {
	fprintf (stderr, "usage: %s [-u] days-to-add\n", pgm_name);
	return (1);
    }

    seconds = atoi (argv[1]) * 86400;

    if (seconds != 0 || utc == 0)
    {
	seconds += time ((time_t *) NULL);

	if (stime (&seconds) != 0)
	{
	    perror (pgm_name);
	    return (1);
	}

	if (utc == 0)			    /* clock runs with local time   */
	{				    /* but we have timezone infos   */
	    struct tm *	tm_l;
	    struct tm *	tm_g;
	    long	diff_sec;

	    tm_l = localtime (&seconds);
	    tm_g = gmtime (&seconds);

	    /* get difference between utc and local time    */
	    diff_sec = utc_offset (tm_l, tm_g);

	    seconds += diff_sec;

	    if (stime (&seconds) != 0)	    /* correct sys time to utc	    */
	    {
		perror (pgm_name);
		return (1);
	    }
	}
    }

    seconds = time ((time_t *) 0);

    tm = localtime (&seconds);

    printf ("%s: new date: %02d.%02d.%4d %02d:%02d:%02d\r\n",
	     pgm_name, tm->tm_mday, tm->tm_mon + 1,
	     1900 + tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);


    return (0);
} /* main (argc, argv) */

static long
utc_offset (struct tm * tm_local, struct tm * tm_gmt)
{
    long	diff_sec;
    long	local_sec;
    long	gmt_sec;

    /* that's a lie, but works! */
    local_sec = (tm_local->tm_year - 70)    * 3600 * 24 * 31 * 12 +
		tm_local->tm_mon	    * 3600 * 24 * 31 +
		(tm_local->tm_mday - 1)	    * 3600 * 24 +
		tm_local->tm_hour	    * 3600 +
		tm_local->tm_min	    * 60 +
		tm_local->tm_sec;

    gmt_sec =	(tm_gmt->tm_year - 70)	* 3600 * 24 * 31 * 12 +
		tm_gmt->tm_mon		* 3600 * 24 * 31 +
		(tm_gmt->tm_mday - 1)	* 3600 * 24 +
		tm_gmt->tm_hour		* 3600 +
		tm_gmt->tm_min		* 60 +
		tm_gmt->tm_sec;

    diff_sec = gmt_sec - local_sec;

    diff_sec %= 86400;				    /* here's the trick :-) */

    return (diff_sec);
} /* utc_offset (tm_local, tm_gmt) */
