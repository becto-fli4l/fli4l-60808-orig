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
 * \brief ISDN specific code
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#include "imond.h"
#include "user.h"
#include "log.h"
#include "led.h"

/** This array structure holds the statistics returned by the IIOCGETCPS ioctl
 */
static struct
{
  unsigned long		ibytes;
  unsigned long		obytes;
} io_bytes[ISDN_MAX_CHANNELS];

/** file decriptor of /dev/isdninfo */
static int		isdninfo_fd;


#ifndef TEST
/** A temporary buffer for the content of /dev/isdninfo */
static char * sgets_bufp;

/**
 * This function reads a line from the buffer sgets_bufp.
 *
 * \param dest The string where the line is stored
 * \param size The maximum size of a line
 *
 * \returns length of the copied string
 */
static int 
sgets (char *dest, int size)
{
    char *  p = sgets_bufp;

    if (p)
    {
	p = strchr (p, '\n');
	if (p)
	{
	    *p='\0';
	    strncpy (dest, sgets_bufp, size - 1);
	    *p='\n';
	    sgets_bufp = p + 1;
	    return (p - sgets_bufp);
	}
	else
	{
	    int len = strlen (sgets_bufp);
	    strncpy (dest, sgets_bufp, size - 1);
	    sgets_bufp += len;
	    return (len);
	}
    }
    else
    {
	*dest = '\0';
	return (0);
    }
}
#endif

/**
 * This function reads the ISDN status from /dev/isdninfo. The infos are
 * written into the global information structures. Online and offline
 * connections are detected. When a connection went offline, a log message is
 * written.
 */
static void
read_info (void)
{
#ifndef TEST
    char buffer [2048];
#endif

#ifndef TEST
    char *  ptr_idmap;
    char *  ptr_chmap;
    char *  ptr_usage;
    char *  ptr_flags;
    char *  ptr_phone;
    int     len;

    static char		idmap_line[4096];
    static char		chmap_line[4096];
    static char		drmap_line[4096];
    static char		usage_line[4096];
    static char		flags_line[4096];
    static char		phone_line[4096];
#endif

    int	    channel_hung_up;
    int     usage;
    int     chanum;
    int     flags;
    int     channel_idx;

    n_active_channels = 0;

    printf ("read_info\n");
#ifdef TEST

    if (channel_idx == 0)
    {
	usage |= 7;
    }

#else

    if (read (isdninfo_fd, buffer, 2047) < 0)
    {
	fprintf (stderr, "    read (isdninfo_fd, buffer, 2047) failed\n");
	return;
    }

    buffer[2047] = '\0';
    sgets_bufp = buffer;
 
    if (!sgets (idmap_line, 4095))
    {
	fprintf (stderr, "    sgets (idmap_line, 4095) failed\n");
	return;
    }

    if (!sgets (chmap_line, 4095))
    {
	fprintf (stderr, "    sgets (chmap_line, 4095) failed\n");
	return;
    }

    if (!sgets (drmap_line, 4095))
    {
	fprintf (stderr, "    sgets (drmap_line, 4095) failed\n");
	return;
    }

    if (!sgets (usage_line, 4095))
    {
	fprintf (stderr, "    sgets (usage_line, 4095) failed\n");
	return;
    }

    if (!sgets (flags_line, 4095))
    {
	fprintf (stderr, "    sgets (flags_line, 4095) failed\n");
	return;
    }

    if (!sgets (phone_line, 4095))
    {
	fprintf (stderr, "    sgets (phone_line, 4095) failed\n");
	return;
    }

    ptr_idmap = idmap_line + 7;
    ptr_chmap = chmap_line + 7;
    ptr_usage = usage_line + 7;
    ptr_flags = flags_line + 7;
    ptr_phone = phone_line + 7;

#endif

    for (channel_idx = 0; channel_idx < ISDN_MAX_CHANNELS; channel_idx++)
    {
#ifdef TEST

	chanum = channel_idx;

#else
	sscanf (ptr_idmap, "%s %n", infos[channel_idx].drvid, &len);
	ptr_idmap += len;

	sscanf (ptr_usage, "%d %n", &usage, &len);
	ptr_usage += len;

	sscanf (ptr_phone, "%s %n", infos[channel_idx].phone, &len);
	ptr_phone += len;

	sscanf (ptr_chmap, "%d %n", &chanum, &len);
	ptr_chmap += len;

	if (chanum == 0)
	{
	    sscanf (ptr_flags, "%d %n", &flags, &len);
	    ptr_flags += len;
	}

	/* A channel-number of -1 indicates an nonexistent channel */

	if (chanum == -1)
	{
	    infos[channel_idx].is_active = FALSE;
	    continue;
	}

#endif

	infos[channel_idx].is_active = TRUE;
	n_active_channels++;

	channel_hung_up = FALSE;

	/* if usage & 7 is zero, there is no connection */

	if (!(usage & ISDN_USAGE_MASK))
	{
	    int status = (usage & ISDN_USAGE_EXCLUSIVE) ? stat_exclusive : stat_offline;
	    strcpy (infos[channel_idx].status, get_status_str (status));

	    infos[channel_idx].phone[0] = '\0';
	    infos[channel_idx].usage[0] = '\0';
	    infos[channel_idx].inout[0] = '\0';

	    if (infos[channel_idx].online_start != 0)
	    {
		channel_hung_up = TRUE;
	    }
	}
	else
	{
	    int online = (flags & (1 << chanum));

	    if (online)
	    {
		if (infos[channel_idx].online_start == 0)    /* new connection?  */
		{
		    online_counter++;

		    if (led_fd < 0 && globals.led_device)
		    {
			led_fd = open (globals.led_device, 0);
		    }

		    beep (TRUE);

		    strcpy (infos[channel_idx].status, 
			    get_status_str (stat_online));

		    infos[channel_idx].online_start    = time ((time_t *) NULL);

		    if (usage & ISDN_USAGE_OUTGOING)
		    {
			infos[channel_idx].conf_idx =
			    find_conf_data (infos[channel_idx].phone, TRUE);
			strcpy (infos[channel_idx].inout, "Outgoing");
			infos[channel_idx].is_outgoing = TRUE;
		    }
		    else
		    {
			infos[channel_idx].conf_idx =
					find_conf_data (infos[channel_idx].phone, FALSE);
			strcpy (infos[channel_idx].inout, "Incoming");
			infos[channel_idx].is_outgoing = FALSE;
		    }
		}

	    }
	    else
	    {
		strcpy (infos[channel_idx].status, get_status_str (stat_calling));

		infos[channel_idx].inout[0] = '\0';

		if (infos[channel_idx].online_start != 0)
		{
		    channel_hung_up = TRUE;
		}
	    }

	    switch (usage & ISDN_USAGE_MASK)
	    {
		case ISDN_USAGE_RAW:
		    strcpy (infos[channel_idx].usage, "Raw");
		    break;
		case ISDN_USAGE_MODEM:
		    strcpy (infos[channel_idx].usage, "Modem");
		    break;
		case ISDN_USAGE_NET:
		    strcpy (infos[channel_idx].usage, "Net");
		    break;
		case ISDN_USAGE_VOICE:
		    strcpy (infos[channel_idx].usage, "Voice");
		    break;
		case ISDN_USAGE_FAX:
		    strcpy (infos[channel_idx].usage, "Fax");
		    break;
		default:
		    strcpy (infos[channel_idx].usage, "-----");
		    break;
	    }
	}

	if (channel_hung_up)
	{
	    int	    conf_idx    =
		(channel_idx >= 0 ? infos[channel_idx].conf_idx : -1);
	    int	    circuit_idx =
		(conf_idx >= 0 ? conf_data[conf_idx].circuit_idx : -1);
	    int	    charge_int  =
		(conf_idx >= 0 ? conf_data[conf_idx].charge_int : 1);
	    int	    charge_per_minute =
		(conf_idx >= 0 ? conf_data[conf_idx].charge_per_minute : 0);
	    time_t  now		= time ((time_t *) 0);
	    time_t  diff	= now - infos[channel_idx].online_start;
	    time_t  charge_diff = 0;
	    float   charge	= 0.0;
	    
	    if (online_counter > 0)
	    {
		online_counter--;

		if (online_counter == 0 && led_fd >= 0)
		{
		    close (led_fd);
		    led_fd = -1;
		}

		beep (FALSE);
	    }

	    if (circuit_idx >= 0)
	    {
		circuits[circuit_idx].link_added = 0;
		circuits[circuit_idx].online = 0;
	    }

	    if (infos[channel_idx].is_outgoing)
	    {
		charge = (diff * charge_per_minute) / 60;
		charge_diff = diff;
		if (charge_int > 1)
		{
		    charge_diff = (charge_diff / charge_int) * 
			charge_int + charge_int;
		}
	    }
	    infos[channel_idx].charge_time += charge_diff;
	    infos[channel_idx].charge += charge;
	    
	    imond_write_record (circuit_idx >= 0 ?
				circuits[circuit_idx].name : "<unknown>",
				infos[channel_idx].online_start,
				charge_diff, charge,
				infos[channel_idx].ibytes_overflows, 
				infos[channel_idx].ibytes, 0,
				infos[channel_idx].obytes_overflows, 
				infos[channel_idx].obytes, 0, 
				circuit_idx >= 0 ?
				circuits[circuit_idx].device_1 : "<unknown>",
				charge_int, charge_per_minute);
				
	    infos[channel_idx].ibytes		    = 0;
	    infos[channel_idx].obytes		    = 0;
	    infos[channel_idx].ibytes_overflows	    = 0;
	    infos[channel_idx].obytes_overflows	    = 0;
	    infos[channel_idx].ibytes_per_second    = 0;
	    infos[channel_idx].obytes_per_second    = 0;
	    infos[channel_idx].online_time	    += diff;
	    infos[channel_idx].online_start	    = 0;
	    infos[channel_idx].ip[0]		    = '\0';
	}
    }

    return;
} /* read_info () */

/**
 * This function measures the current ISDN rates for a specific channel. This
 * function also controls the automatic channel-bundling.
 *
 * <TABLE>
 * <TR><TH>bandwidth_rate</TH><TH>bandwidth_time</TH><TH>result</TH></TR>
 * <TR><TD>	0	 </TD><TD>	0	</TD><TD>add link immediately</TD></TR>
 * <TR><TD>	5	 </TD><TD>	30	</TD><TD>add/remove link after 30 seconds</TD></TR>
 * <TR><TD>	-1	 </TD><TD>	-1	</TD><TD>add/remove never</TD></TR>
 * </TABLE>
 *
 * \param channel_idx index o channel to messure rates for
 * \param seconds_passed how many seconds have passed since the last call of
 *                       this function
 */
static void
measure_isdn_rates_for_channel (int channel_idx, float seconds_passed)
{
#ifdef TEST
    static unsigned long tmp;
    tmp++;
    if (tmp % 10 == 0) {
	infos[channel_idx].ibytes_overflows++;
	printf("generating ibytes overflow\n");
    }
#endif

    if (infos[channel_idx].online_start > 0)
    {
	int conf_idx    = (channel_idx >= 0 ? infos[channel_idx].conf_idx : -1);
	int circuit_idx = (conf_idx >= 0 ? conf_data[conf_idx].circuit_idx : -1);

	if (ioctl (isdninfo_fd, IIOCGETCPS, io_bytes) == 0)
	{
	    unsigned long ibytes, obytes;

	    if (!io_bytes[channel_idx].ibytes 
		&& !io_bytes[channel_idx].obytes)
	    {
		/* ISDN channel not activ, hangup? */
		imond_syslog (LOG_INFO, 
			      "measure_isdn_rates : channel hangup?\n");
		return;
	    }

	    ibytes = io_bytes[channel_idx].ibytes - infos[channel_idx].ibytes;
	    obytes = io_bytes[channel_idx].obytes - infos[channel_idx].obytes;

	    if ( io_bytes[channel_idx].ibytes < infos[channel_idx].ibytes)
	    {
		imond_syslog (LOG_INFO, "isdn_ibytes overflow: %lx < %lx,"
			      " isdn_ibytes_overlow=%ld\n", 
			      io_bytes[channel_idx].ibytes, infos[channel_idx].ibytes,
			      infos[channel_idx].ibytes_overflows+1);
		infos[channel_idx].ibytes_overflows++;
	    }
	    if ( io_bytes[channel_idx].obytes < infos[channel_idx].obytes)
	    {
		imond_syslog (LOG_INFO, "isdn_obytes overflow: %lx < %lx,"
			      " isdn_obytes_overlow=%ld\n", 
			      io_bytes[channel_idx].obytes, infos[channel_idx].obytes,
			      infos[channel_idx].obytes_overflows+1);
		infos[channel_idx].obytes_overflows++;
	    }

	    infos[channel_idx].ibytes_per_second = (int)
		((float) ibytes / seconds_passed);
	    
	    infos[channel_idx].obytes_per_second = (int)
		((float) obytes / seconds_passed);
	    
	    if (channel_idx == 0 &&				/* hack */
		*(circuits[circuit_idx].device_2) &&
		circuits[circuit_idx].bandwidth_rate >= 0 &&
		circuits[circuit_idx].bandwidth_time >= 0)
	    {
		if (infos[channel_idx].ibytes_per_second +
		    infos[channel_idx].obytes_per_second >=
		    circuits[circuit_idx].bandwidth_rate)
		{
		    infos[channel_idx].time_bandwidth_lower = 0;
		    
		    if (infos[channel_idx].time_bandwidth_higher == 0)
		    {
			infos[channel_idx].time_bandwidth_higher =
			    current_time.tv_sec;
		    }
		    else if (circuits[circuit_idx].link_added == 0 &&
			     current_time.tv_sec -
			     infos[channel_idx].time_bandwidth_higher
			     >= circuits[circuit_idx].bandwidth_time)
		    {
			addlink (circuit_idx);
		    }
		}
		else
		{
		    infos[channel_idx].time_bandwidth_higher = 0;
		    
		    if (infos[channel_idx].time_bandwidth_lower == 0)
		    {
			infos[channel_idx].time_bandwidth_lower =
			    current_time.tv_sec;
		    }
		    else if (circuits[circuit_idx].link_added > 0 &&
			     current_time.tv_sec -
			     infos[channel_idx].time_bandwidth_lower
			     >= circuits[circuit_idx].bandwidth_time)
		    {
			removelink (circuit_idx);
		    }
		}

	    }
	    infos[channel_idx].ibytes = io_bytes[channel_idx].ibytes;
	    infos[channel_idx].obytes = io_bytes[channel_idx].obytes;
	}
    }
    else					/* we are offline   */
    {
	infos[channel_idx].ibytes_per_second = 0;
	infos[channel_idx].obytes_per_second = 0;
    }

    return;
} /* measure_isdn_rates_for_channel (channel_idx, seconds_passed) */

/**
 * This function adds a channel to a currently running ISDN connection.
 *
 * \param circuit_idx The circuit which is currently using the channel
 */
int
addlink (int circuit_idx)
{
    int	rtc;

    if (circuits[circuit_idx].link_added == 0)
    {
	char buf[256];

	sprintf (buf, "/usr/local/bin/fli4lctrl addlink %s >/dev/console 2>&1",
		 circuits[circuit_idx].device_1);

	rtc = my_system (buf);

	if (rtc == 0)
	{
	    circuits[circuit_idx].link_added++;
	}
    }
    else
    {
	rtc = -1;
    }
    return (rtc);
} /* addlink (int circuit_idx) */


/**
 * This function removes a channel from a currently running ISDN connection.
 *
 * \param circuit_idx The circuit which is currently using the connection
 */
int
removelink (int circuit_idx)
{
    int	rtc;

    if (circuits[circuit_idx].link_added > 0)
    {
	char buf[256];

	sprintf (buf,
		 "/usr/local/bin/fli4lctrl removelink %s >/dev/console 2>&1",
		 circuits[circuit_idx].device_1);

	rtc = my_system (buf);

	if (rtc == 0)
	{
	    circuits[circuit_idx].link_added--;
	}
    }
    else
    {
	rtc = -1;
    }
    return (rtc);
} /* removelink (int circuit_idx) */

/**
 * This function messures the current transfer rates for all ISDN channels.
 *
 * \param seconds_passed how many seconds have passed since the last call of
 *                       this function.
 */
void measure_isdn_rates (float seconds_passed)
{
    int channel_idx;
    for (channel_idx = 0; channel_idx < ISDN_MAX_CHANNELS;
	 channel_idx++)
    {
	if (infos[channel_idx].is_active)
	{
	    measure_isdn_rates_for_channel (channel_idx, seconds_passed);
	}
    }
}

/**
 * This function checks whether an ISDN device is actually online.
 *
 * \param name name of the device to test
 *
 * \retval 1 if device is online
 * \retval 0 if device is offline
 */
int 
circuit_device_online (char * name)
{
    isdn_net_ioctl_phone phone;
    int rc;
    
    memset(&phone, 0, sizeof phone);
    strncpy(phone.name, name, sizeof phone.name);
    rc = ioctl(isdninfo_fd, IIOCNETGPN, &phone);
    if (rc == 0) 
    {
	return 1;
    }
    if (errno != ENOTCONN) 
    {
	imond_syslog (LOG_INFO, 
		      "circuit_device_online (): unknown error: %s\n",
		      strerror (errno));
    }
    return 0;
}

/**
 * This function initializes all ISDN-related variables.
 */
void isdn_init (void)
{
    int channel_idx;
    
#ifdef TEST
    isdninfo_fd = fileno (stdin);
#else
    isdninfo_fd = open ("/dev/isdninfo", O_RDONLY);
#endif
    if (isdninfo_fd >= 0)
    {
	conn_register_fd (isdninfo_fd, "none", 0, 	
			  (int (*)(struct client_t * client))read_info,
			  "read_isdn_info");
    }
    for (channel_idx = 0; channel_idx < ISDN_MAX_CHANNELS; channel_idx++)
    {
	infos[channel_idx].ibytes_overflows = 0;
	infos[channel_idx].obytes_overflows = 0;
    }

}
