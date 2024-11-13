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
 * \brief Logging code
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#include <stdarg.h>
#include <sys/time.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "log.h"

int imond_do_open_log (int append);
void imond_close_log (void);

/** size of log message buffer (maximum length of a log message */
#define LOG_BUF_SIZE 1024
/** maximum length of the name of the imond log file */
#define LOG_NAME_SIZE 255

/** file descriptor for imond log file */
static FILE * imond_log_fp = NULL;
/** may imond write its log to syslog or to stdout? */
static int log_to_syslog = 0;
/** name of the imond log file */
static char logfile_name[LOG_NAME_SIZE+1] = "";

/**
 * This function opens syslog and signals that we use syslog.
 */
int imond_open_syslog (void)
{
  log_to_syslog = 1;
  openlog ("imond", LOG_PID|LOG_PERROR, LOG_USER);
  return 0;
}

/**
 * This function logs a  message depending on the value of log_to_syslog.
 * 
 * \param log_level the log level for syslog(3)
 * \param fmt printf(3) format for syslog message
 * \param ... parameters for the syslog message
 */
void imond_syslog (int log_level, const char *fmt, ...)
{
    va_list 	ap;
    char	msg_buf[LOG_BUF_SIZE];
    
    va_start (ap, fmt);
    if (log_to_syslog)
    {
	vsprintf (msg_buf, fmt, ap);
	syslog (log_level, "%s", msg_buf);
    }
    else
    {
	vfprintf (stderr, fmt, ap);
    }
    va_end (ap);
}

/**
 * This function generates a log file name and saves it in logfile_name.
 *
 * \param logdir directory for log file
 */
void set_imond_log_dir (const char * logdir)
{
    snprintf (logfile_name, LOG_NAME_SIZE, "%s/%s", logdir, "imond.log");
}

/**
 * This function returns the name of the imond log file.
 *
 * \return NULL if there is no log file, the name of the log file otherwise
 */
const char * get_imond_log_file (void)
{
    if (*logfile_name)
	return logfile_name;
    else
	return NULL;
}

/**
 * This function opens the imond log file for overwriting or appending and
 * does some simple error handling.
 *
 * \param append if true, append to the log file, overwrite otherwise
 *
 * \retval 1 log file was succesfully opened
 * \retval 0 there was an error opening the log file
 */
int imond_do_open_log (int append)
{
    if (logfile_name[0])
    {
	imond_log_fp = fopen (logfile_name, append ? "a" : "w");
	if (imond_log_fp)
	{
	    return 1;
	}
	else
	{
	    imond_syslog (LOG_ERR, "Error opening imond logfile '%s': %s\n",
			  logfile_name, strerror (errno));
	}
    }
    return 0;
}

/**
 * This fuction closes the imond log file and forks a new process to sync it
 * to disk.
 */
void imond_close_log ()
{
    if (imond_log_fp)
    {
	/* Execute sync call as child process to prevent lockups */
	if (fork () == 0)
	{
	    fsync (fileno (imond_log_fp));
	    exit (0);
	}
	fclose (imond_log_fp);
	imond_log_fp = NULL;
    }
}

/**
 * This resets the logfile by opening it for overwriting and closing it right
 * afterwards.
 */
void imond_reset_log ()
{
    if (logfile_name[0] && access (logfile_name, 0) == 0)
    {
	imond_do_open_log (0);
	imond_close_log ();
    }
}

/**
 * This function returns date and time of the supplied UNIX time stamp as a
 * formatted string.
 * 
 * The exact format of the retuned string is "YYYY/MM/DD HH:MM:SS", no locales
 * are used, this is normally used in log files.
 *
 * \param seconds The timestamp to convert
 *
 * \returns converted string
 */
static char *
date_time (time_t seconds)
{
    static char buf[64];
    struct tm *	tm;

    tm = localtime (&seconds);

    sprintf (buf, "%04d/%02d/%02d %02d:%02d:%02d",
	     1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
	     tm->tm_hour, tm->tm_min, tm->tm_sec);
    return (buf);
} /* date_time (seconds) */

/**
 * This function writes a record to the imond log file.
 *
 * \param circuit_name name of the used circuit
 * \param online_start time stamp from the beginning of the connection
 * \param charge_time time that was charged
 * \param charge_val cost of this online session
 * \param ibytes_overflow nuber of 2GB overflows in the input byte counter
 * \param ibytes current input byte counter
 * \param ibytes_start start value of the input byte counter
 * \param obytes_overflow nuber of 2GB overflows in the output byte counter
 * \param obytes current output byte counter
 * \param obytes_start start value of the output byte counter
 * \param device name of the connection device
 * \param charge_int the charge interval
 * \param charge_per_minute cost of the connection per minute
 * 
 */
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
			 double charge_per_minute)
{
    time_t  now		= time ((time_t *) 0);
    time_t  diff	= now - online_start;

    if (ibytes_start >= ibytes)
    {
	ibytes_overflow--;
    }
    ibytes -= ibytes_start;

    if (obytes_start >= obytes)
    {
	obytes_overflow--;
    }
    obytes -= obytes_start;

    /* we assume, logfile_name is the logfile */
    if (imond_do_open_log (1))
    {
	fprintf (imond_log_fp, "%-20s %s",
		 circuit_name, date_time (online_start));
	fprintf (imond_log_fp, " %s %5ld %5ld %6.2f",
		 date_time (now), (long) diff,
		 charge_time, charge_val);
	fprintf (imond_log_fp, " %lu %lu %lu %lu",
		 ibytes_overflow, ibytes,
		 obytes_overflow, obytes);
	fprintf (imond_log_fp, " %s %d %6.4f\n",
		 device, charge_int, charge_per_minute);
	imond_close_log ();
    }
}

