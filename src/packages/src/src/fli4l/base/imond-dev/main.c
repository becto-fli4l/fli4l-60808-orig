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
 * \brief Main code
 * \version $Revision$
 * \date Creation: 04.04.2000 fm
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */
/**
 * \mainpage
 * \verbinclude README
 */
#include "imond.h"
#include "date.h"
#include "log.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <linux/kd.h>

int password_mode;				/**< password mode	    */

/** minimum interval between two successive rate measurements */
#define MIN_MEASURE_INTERVAL	0.5

/** saves the last time traffic measurements were gathered */
struct timeval		last_time_measured;
/** saves the current time */
struct timeval		current_time;

/** process id of a process that was forked to generate a beep sequence */
static int		beep_pid = 0;

/** current default routing device */
static char            default_device[16];
/** current default circuit */
char            default_circuit[32];

int			pppoe_used;	    /**< flag if pppoe used */
int			n_active_channels;  /**< number of active channels */

/** all global options */
globals_t globals;

/** structure that holds infos for all ISDN channels */
isdn_info_t infos[ISDN_MAX_CHANNELS];

/** structure that holds infos for PPPoE channel */
pppoe_info_t pppoe_infos;

  /** the name of the main fli4l config file */
#define CONF_FILE		"/etc/imond.conf"

/** maximum number of lines in main configuration file */
#define MAX_CONFIG_LINES	32
/** maximum number of configured circuits */
#define MAX_CIRCUITS		32

/** LCR state as 3-char strings */
char * conf_state_string [] = { "inv", "dis", "ena", "lcr" };
/** convert LCR state into a string */
#define CONF_TT_STR(s) s>=-1 && s<3 ? conf_state_string[s+1] : "invalid state"
#define CONF_INVALID  -1
#define CONF_DISABLED 0
#define CONF_ENABLED  1
#define CONF_LCR      2
#define CONF_IS_DISABLED(c) (conf_data[c].state==CONF_DISABLED)
#define CONF_IS_ENABLED(c) (conf_data[c].state==CONF_ENABLED)
#define CONF_IS_LCR(c)     (conf_data[c].state==CONF_LCR)
#define CONF_HAS_DEFAULT_ROUTE(c) (conf_data[c].is_default_route)
#define CONF_IS_VALID(c,tt_idx) (circuits[(c).circuit_idx].time_table[tt_idx]!=-1)

#define CIRCUIT_IS_DISABLED(c,tt_idx) \
	(CONF_IS_DISABLED((int)circuits[c].time_table[tt_idx]))
#define CIRCUIT_IS_LCR(c,tt_idx) \
	(CONF_IS_LCR((int)circuits[c].time_table[tt_idx]))
#define CIRCUIT_CONF_IDX(c,tt_idx) \
	((int)circuits[c].time_table[tt_idx])
 
char * status_str[] = { "Exclusive", "Offline", "Calling", "Online" };

static char			time_table[HOURS_PER_DAY * DAYS_PER_WEEK];
conf_data_t			conf_data[MAX_CONFIG_LINES];
int				n_conf_lines_used;

/** this structure holds all imond circuits */
circuit_t			circuits[MAX_CIRCUITS];
/** number of circuits */
int				n_circuits;

int				route_idx = -1;		/**< -1: automatic */

int				online_counter;		/**< number of channels online */

/** flag if imond is running under Linux kernel 2.4 */
int				running_under_2_4 = 0;

/** saves the name of the command */
char * argv0;

char systemname[10];
/**
 * This checks the version of the running Linux kernel and saves the result in
 * running_under_2_4.
 */
static void
check_kernel_version (void)
{
    struct utsname uts_info;
    if (uname (&uts_info) < 0)
    {
	perror ("uname");
	exit (1);
    }
    if (!strncmp (uts_info.release, "2.4", 3))
    {
	imond_syslog (LOG_INFO, "running under kernel 2.4\n");
	running_under_2_4 = 1;
    }
}

/**
 * This checks under which system imond is running (fli4l/eisfair/other).
 */
static void
check_system (void)
{
    FILE *  fp;
    char    buf[256];
    
    strcpy(systemname, "other");
    systemname[0]='\0';
    fp = fopen ("/etc/issue", "r");
    if (! fp)
    {
	return;
    }

    while (fgets(buf, 255, fp))
    {
    	if (strstr(buf,"eisfair")!=NULL)
    	{
    	    strcpy(systemname, "eisfair");
    	    break;
	} else if (strstr(buf,"fli4l")!=NULL)
	{
	    strcpy(systemname, "fli4l");
	    break;
	}		
    }
    fclose (fp);
    imond_syslog (LOG_INFO, "running under %s", systemname);
}

/**
 * This function sends a beep to tty1 to (hopefully) play it on the internal
 * PC speaker
 *
 * \param freq frequency of tone in ??? (FIXME)
 * \param duration tone duration in msec
 */
void 
play_beep (int freq, int duration)
{
  int fd = open("/dev/tty1",0);
  if (fd)
    {
      ioctl (fd, KDMKTONE, (duration << 16) + 1193180 / freq);
      close (fd);
    }
}

/**
 * This function composes a beep sequence of two beep tones. This is used to
 * signal the user if the connection is going online or offline.
 *
 * We fork here to return immediately and continue our work-
 *
 * \param is_going_up flag to select which tone sequence is played
 */
void
beep (int is_going_up)
{
    int	pid;

    if (!globals.do_beep || beep_pid)
	return;

    if ((pid = fork ()) != 0)				/* it's the parent  */
    {
	beep_pid = pid;
	return;
    }
    
    if (is_going_up)
    {
        play_beep (660, 200);
        usleep (300000);
        play_beep (800, 200);
    }
    else
    {
        play_beep (880, 200);
        usleep (300000);
        play_beep (660, 200);
    }
    exit (0);
} /* beep (is_going_up) */

/**
 * This function tries to find \a phone in the comma-separated list \a phone_str.
 *
 * \param phone Phone number (string) to find
 * \param phone_str comma-separated list of phone numbers which are tested
 *
 * \retval TRUE if one of the numbers in \a phone_str is the same as \a phone
 * \retval FALSE no match found
 */
int
find_phone (char * phone_str, char * phone)
{
    char *  pp;

    do
    {
	pp = strchr (phone_str, ',');	    /* numbers separated by comma   */

	if (pp)
	{
	    *pp = '\0';
	}

	if (! strcmp (phone_str, phone))    /* phone is pattern to find	    */
	{
	    if (pp)
	    {
		*pp = ',';
	    }
	    return (TRUE);
	}

	if (pp)
	{
	    *pp = ',';
	    phone_str = pp + 1;
	}
    } while (pp);

    return (FALSE);
} /* find_phone (phone_str, phone) */

/**
 * This function calculates how many hours have passed since midnight of the
 * last Sunday.
 *
 * \returns hours since Su 0.00
 */
int
get_hour_for_circuits(void)
{
	struct tm *	tm;
	time_t	  	seconds;
	int 		wday;

	seconds = time ((time_t *) 0);
	tm	= localtime (&seconds);

	if (is_holiday (tm->tm_mday, tm->tm_mon + 1, tm->tm_year +1900))
	{
	  wday = SUNDAY;
	}
	else
	{
	  wday = tm->tm_wday;
	}
	
	return wday * HOURS_PER_DAY + tm->tm_hour;
} // get_hour_for_circuits


/**
 * This functions finds the conf_data entry which is currently active and has
 * the phone number \a phone.
 *
 * \param phone The phone number which is searched in conf_data
 * \param is_outgoing_connection TRUE to search for outgoing numbers, FALSE for
 * incoming
 *
 * \retval -1 no active circuit could be found
 */
int
find_conf_data (char * phone, int is_outgoing_connection)
{
    int		i;

    int hour = get_hour_for_circuits ();
    
    for (i = 0; i < n_circuits; i++)
    {
	int conf_idx = circuits[i].time_table[hour];
	char *p = is_outgoing_connection ?  
	    conf_data[conf_idx].outgoing_phone : 
	    conf_data[conf_idx].incoming_phone;

	if (! circuits[i].is_pppoe &&
	    ! (is_outgoing_connection && 
	       CONF_IS_DISABLED(conf_idx) && 
	       CONF_HAS_DEFAULT_ROUTE(i)) && 
	    find_phone (p, phone) == TRUE)
	{
	    /* check whether we actually picked the right circuit */
	    if (!circuits[i].online &&
		circuit_device_online (circuits[i].device_1))
	    {
		circuits[i].online = 1;
		return (i);
	    }
	}
    }
    imond_syslog (LOG_ERR, "can't find circuit for %s connection an phone number %s at hour %d\n",
		  is_outgoing_connection ? "outgoing" : "incoming",
		  phone, hour);
    return (-1);
} /* find_conf_data (phone, is_outgoing_connection) */


/**
 * This function finds the conf_data entry for the PPPoE device and the current
 * time.
 *
 * \retval -1 if no active PPPoE circuit could be found for that time.
 */
int
find_pppoe_conf_data (int idx)
{
    int		i;
    int hour = get_hour_for_circuits ();

    if (idx != -1) {
      if (circuits[idx].is_pppoe)
	return CIRCUIT_CONF_IDX(idx, hour);
      else
	return -1;
    }

    for (i = 0; i < n_circuits; i++)
    {
      imond_syslog (LOG_INFO, "n_circuits: %d, i: %d, is_pppoe: %d, diasbled: %d\n",
		    n_circuits, i, circuits[i].is_pppoe, CIRCUIT_IS_DISABLED(i, hour));
	if (circuits[i].is_pppoe && ! CIRCUIT_IS_DISABLED(i, hour))
	{
	  return CIRCUIT_CONF_IDX(i, hour);
	}
    }
    imond_syslog (LOG_ERR, "can't find pppoe circuit for hour %d\n",
		  hour);
    return (-1);
} /* find_pppoe_conf_data () */


/**
 * find circuit index of current default route circuit
 *
 */
int
find_circuit (void)
{
    int	i;
    int hour = get_hour_for_circuits ();

    if (route_idx >= 0)
    {
	return (route_idx);
    }


    for (i = 0; i < n_circuits; i++)
    {
	if (CIRCUIT_IS_LCR(i, hour))
	{
	    return i;
	}
    }

    return (-1);
} /* find_circuit () */

/**
 * This function searches for the current channel index of the specified
 * circuit.
 *
 * \param circuit_idx number of circuit for which the channel index should be
 * retrieved.
 */
int
find_channel_idx_by_circuit_idx (int circuit_idx)
{
    int channel_idx;
    int	conf_idx;
    int	n_active_channels_found = 0;			    /* optimization */

    for (channel_idx = 0; channel_idx < ISDN_MAX_CHANNELS; channel_idx++)
    {
	if (infos[channel_idx].is_active)
	{
	    if (infos[channel_idx].online_start != 0)
	    {					/* valid ch only if online  */
		conf_idx = (channel_idx >= 0 ? infos[channel_idx].conf_idx : -1);

		if (conf_idx >= 0)
		{
		    if (conf_data[conf_idx].circuit_idx == circuit_idx)
		    {
			return (channel_idx);
		    }
		}
	    }

	    n_active_channels_found++;

	    if (n_active_channels_found >= n_active_channels)
	    {			    /* no active channels following, break  */
		break;
	    }
	}
    }
    return (-1);
} /* find_channel_idx_by_circuit_idx (circuit_idx) */

/**
 * map info index to channel index
 *
 * \warning info_idx begins with 1, not 0!
 *
 */
int
map_channel (int info_idx)
{
    int	channel_idx;
    int	n_active_channels_found	= 0;

    for (channel_idx = 0; channel_idx < ISDN_MAX_CHANNELS; channel_idx++)
    {
#ifdef TEST
	if (n_active_channels_found < 2)
#else
	if (infos[channel_idx].is_active == TRUE)
#endif
	{
	    n_active_channels_found++;

	    if (n_active_channels_found == info_idx)
	    {
		return (channel_idx);
	    }
	}
    }

    return (-1);
} /* map_channel (info_idx) */


/**
 * This functions trys to find the name of a circuit out of the arrays
 * conf_data and circuits.
 *
 * \retval "unknown" if name of circuit could not be found
 * \retval other name of the circuit that was looked up
 */
char *
get_circuit_name_by_conf_idx (int conf_idx)
{
    if (conf_idx >= 0 && conf_idx < n_conf_lines_used)
    {
	int  circuit_idx = conf_data[conf_idx].circuit_idx;

	if (circuit_idx >= 0)
	{
	    return circuits[circuit_idx].name;
	}
    }

    return "unknown";
} /* get_circuit_name_by_conf_idx (conf_idx) */


/**
 * This function returns a string containing a user readable representation
 * of the timetable of a given circuit
 *
 * \param circuit_idx This is the circuit for which the timetable will be
 *                    built.
 *
 * \return string with timetable
 */
char *
get_timetable (int circuit_idx)
{
    static char	timetable_str[4096];
    char *	tp;
    char *	p;
    int		i;
    int		j;

    if (circuit_idx < 0 || circuit_idx >= MAX_CIRCUITS)
    {
	circuit_idx = -1;
	tp = time_table;
    }
    else
    {
	tp = circuits[circuit_idx].time_table;
    }

    strcpy (timetable_str, "   ");

    for (j = 0; j < HOURS_PER_DAY; j++)
    {
	sprintf (timetable_str + strlen (timetable_str), " %2d", j);
    }

    p = timetable_str + strlen (timetable_str);

    *p++ = '\r';
    *p++ = '\n';
    *p++ = ' ';

    for (j = 0; j < 74; j++)
    {
	*p++ = '-';
    }

    *p++ = '\r';
    *p++ = '\n';
    *p++ = '\0';

    for (i = 0; i < DAYS_PER_WEEK; i++)
    {
	sprintf (timetable_str + strlen (timetable_str), " %s",
		 get_weekday(i));

	for (j = 0; j < HOURS_PER_DAY; j++)
	{
	    int conf = tp[i * HOURS_PER_DAY + j];
	    if (circuit_idx < 0)
	    {
		if (conf < 0)
		    strcat (timetable_str, "  M");
		else
		    sprintf (timetable_str + strlen (timetable_str), " %2d",
			     conf + 1);
	    }
	    else
	    {
		if (CONF_IS_DISABLED(conf) && CONF_HAS_DEFAULT_ROUTE(conf))
		    strcat (timetable_str, "  D");
		else
		    sprintf (timetable_str + strlen (timetable_str), " %2d",
			     conf + 1);
	    }
	}
	strcat (timetable_str, "\r\n");
    }

    strcat (timetable_str,
	    " \r\n No.  Name                     LCR     DRoute  Device  Ch/Min   ChInt\r\n");

    for (i = 0; i < n_conf_lines_used; i++)
    {
	if ( (circuit_idx < 0 && conf_data[i].state == CONF_LCR) 
	     || 
	     (conf_data[i].circuit_idx == circuit_idx && 
	      !(CONF_IS_DISABLED(i) && CONF_HAS_DEFAULT_ROUTE(i))) 
	    )
	{
	    int c_idx = conf_data[i].circuit_idx;
	    
	    sprintf (timetable_str + strlen (timetable_str),
		     " %2d   %-20s     %-7s %-7s %-7s %6.4f    %3d\n",
		     i + 1, circuits[c_idx].name,
		     CONF_TT_STR(conf_data[i].state),
		     conf_data[i].is_default_route ? "yes" : "no",
		     circuits[c_idx].device_1,
		     conf_data[i].charge_per_minute,
		     conf_data[i].charge_int);
	}
    }

    return (timetable_str);
} /* get_timetable (conf_idx) */


/**
 * This function spawns a subprocess. It is a wrapper for the system(3) call.
 *
 * \param cmd The command to execute
 * 
 * \retval 0 the command was executed
 * \retval !0 there was an error executing the command
 */
int
my_system (char * cmd)
{
    int	rtc;

#ifdef TEST
    printf ("imond: %s\n", cmd);
    rtc = 0;
#else

#ifdef DEBUG_ROUTES
    imond_syslog (LOG_INFO, "my_system (\"%s\")\n", cmd);
#endif
    rtc = (int) ((unsigned int) system (cmd) >> 8);

#endif /* TEST */

    return (rtc);
} /* my_system (cmd) */

/**
 * This function sets the route to a specified circuit, if that circuit is
 * enabled.
 *
 * \param circuit_idx The circuit to set the route to
 *
 * \retval TRUE setting the route was successful (circuit was enabled)
 * \retval FALSE setting the route was not successful (circuit was not
 *               enabled)
 */
int
set_route (int circuit_idx)
{
    int hour = get_hour_for_circuits ();

    if (! CIRCUIT_IS_DISABLED(circuit_idx, hour))
    {
	route_idx = circuit_idx;
	return TRUE;
    }
    return FALSE;
}

/**
 * This function checks the current routing and takes appropriate actions. This
 * includes the setting and removing of the default route if the current
 * setting does not fit our expected state.
 *
 * \param cmd Action to take, my be SET_IMMEDIATELY to set the default route
 * immediately to the active device or SET_AGAIN to reset a default route that
 * may have been deleted.
 */
void
check_routing (int cmd)
{
    static int	default_route_deleted = TRUE;
    char	buf[256];
    FILE *	fp;
    char *	device;
    char *	circuit;
    int		route_changed;
    int		circuit_idx;

    buf[0] = '\0';

    if (route_idx >= 0)				/* manual routing ...   */
    {
	circuit_idx = route_idx;
    }
    else					/* automatic LC routing */
    {
	circuit_idx = find_circuit ();

#ifdef TEST
	printf ("imond: check_routing(): circuit_idx = %d\n", circuit_idx);
#endif
    }

    if (circuits[circuit_idx].check_routing)
    {
	circuits[circuit_idx].check_routing (circuit_idx);
    }
    else {

    if (circuit_idx >= 0)
    {
	device	= circuits[circuit_idx].device_1;
	circuit	= circuits[circuit_idx].name;

#ifdef TEST
	printf ("imond: check_routing(): device = %s,"
		" last default device = %s\n", device, default_device);
#endif

	if (strcmp (default_device, device) != 0)
	{
	    route_changed = TRUE;
	}
	else
	{
	    route_changed = FALSE;
	}

	if (cmd == SET_IMMEDIATELY || route_changed == TRUE)
	{
	    unlink ("/etc/last-default-route-interface");
	    fp = fopen ("/etc/default-route-interface", "w");

	    if (fp)
	    {
		fprintf (fp, "%s\n", device);
		fclose (fp);
	    }
	    else
	    {
		perror ("/etc/default-route-interface");
		exit (1);
	    }

	    if (*default_device)
	    {
		imond_syslog (LOG_INFO,
			      "removing default route to circuit '%s'\n",
			      default_circuit);

		sprintf (buf,
		    "/usr/local/bin/fli4lctrl hangup %s;"
		    " /usr/local/bin/delete-all-routes %s; ",
		    default_device, default_device);
	    }

	    strcpy (default_device, device);
	    strcpy (default_circuit, circuit);
	}

	if (cmd == SET_IMMEDIATELY)
	{
	    if (globals.dialmode == DIALMODE_AUTO || strcmp (default_device, "pppoe"))
	    {
		imond_syslog (LOG_INFO, 
			      "setting default route to circuit '%s'\n",
			      default_circuit);

		sprintf (buf + strlen (buf),
			 "/usr/local/bin/add-default-route %s 2>/dev/null",
			 default_device);
		default_route_deleted = FALSE;
	    }
	}
	else if (cmd == SET_AGAIN)		/* hangup of pppoe device   */
	{
	    if (globals.dialmode == DIALMODE_AUTO || strcmp (default_device, "pppoe"))
	    {					/* auto or new isdn default */
		imond_syslog (LOG_INFO, 
			      "setting default route again to circuit '%s'\n",
			      default_circuit);

		sprintf (buf + strlen (buf),
			 "sleep 3; /usr/local/bin/add-default-route %s 2>/dev/null",
			 default_device);
		default_route_deleted = FALSE;
	    }
	    else if (! strcmp (default_device, "pppoe"))
	    {			/* non-auto and default circuit is pppoe    */
		sprintf (buf + strlen (buf),
			 "/usr/local/bin/delete-all-routes %s",
			 default_device);
		default_route_deleted = TRUE;
		*default_device = '\0';
		rename ("/etc/default-route-interface", 
			"/etc/last-default-route-interface");
	    }
	}
	else if (route_changed == TRUE)
	{
	    if (globals.dialmode == DIALMODE_AUTO || strcmp (default_device, "pppoe"))
	    {
		imond_syslog (LOG_INFO, 
			      "changing default route to circuit '%s'\n",
			      default_circuit);

		sprintf (buf + strlen (buf),
			 "(sleep 3; /usr/local/bin/add-default-route %s 2>/dev/null) &",
			 default_device);
		default_route_deleted = FALSE;
	    }
	}
    }
    else
    {
	if (! default_route_deleted)
	{
	    imond_syslog (LOG_INFO, "deleting default route\n");

	    sprintf (buf,
		 "/sbin/route del default 2>/dev/null; /usr/local/bin/fli4lctrl hangup %s",
		 default_device);
	    default_route_deleted = TRUE;
	    *default_device = '\0';
	    rename ("/etc/default-route-interface", 
		    "/etc/last-default-route-interface");

	}
    }
    }
    if (* buf)
    {
	(void) my_system (buf);
    }

    return;
} /* check_routing (cmd) */


/**
 * This function sets the dialmode for all circuits. This is used to switch
 * automatic dialing on or off at all possible times.
 *
 * \param new_dialmode the new dialmode, one of DIALMODE_OFF,
 *                     DIALMODE_MANUAL and DIALMODE_AUTO
 *
 * \retval 0 everything succeeded
 * \retval -1 an error occurred (system(3) failed)
 */
int
set_dialmode (int new_dialmode)
{
    char    buf[128];
    char *  dialmode_string;
    int	    circuit_idx;
    int	    srtc;
    int	    rtc = 0;

    switch (new_dialmode)
    {
	case DIALMODE_OFF:	dialmode_string = "off";    break;
	case DIALMODE_AUTO:	dialmode_string = "auto";   break;
	default:		dialmode_string = "manual"; break;
    }

    for (circuit_idx = 0; circuit_idx < n_circuits; circuit_idx++)
    {
	sprintf (buf, "/usr/local/bin/fli4lctrl dialmode %s %s",
		 circuits[circuit_idx].device_1, dialmode_string);

	srtc = my_system (buf);

	if (srtc != 0)
	{
	    rtc = -1;
	}
    }

    globals.dialmode = new_dialmode;

    return (rtc);
} /* set_dialmode (new_dialmode) */


/**
 * This function triggers dialing or tries to hang up a connection.
 *
 * \param channel_id The channel to dial/hang up, when 0 dial/hang up the
 *                   default route interface
 * \param dial_flag if TRUE, dial. if FALSE, hang up
 *
 * \retval 0 everything worked well
 * \retval -1 an error occurred (system(3) failed)
 */
int
do_dial (char * channel_id, int dial_flag)
{
    char    buf[128];
    int	    circuit_idx;
    int	    srtc;
    int	    rtc = 0;

    /* build command */
    strcpy (buf, "/usr/local/bin/fli4lctrl ");
    strcat (buf, (dial_flag) ? "dial " : "hangup ");

    if (channel_id)	/* hangup/dial specific channel  */
    {
	if (! strcmp (channel_id, "pppoe"))
	{
	    strcat (buf, "pppoe");

	    srtc = my_system (buf);

	    if (srtc != 0)
	    {
		rtc = -1;
	    }

	    return (rtc);			/* return here !	    */
	}
	else
	{
	    int	    channel_idx = map_channel (atoi (channel_id));
	    int	    conf_idx    =
		(channel_idx >= 0 ? infos[channel_idx].conf_idx : -1);

	    circuit_idx =
		(conf_idx >= 0 ? conf_data[conf_idx].circuit_idx : -1);
	}
    }
    else
    {
	circuit_idx = find_circuit ();
    }

    if (circuit_idx >= 0)
    {
	strcat (buf, circuits[circuit_idx].device_1);

	srtc = my_system (buf);

	if (srtc != 0)
	{
	    rtc = -1;
	}
    }

    return (rtc);
} /* do_dial (channel_id, dial_flag) */

#ifdef DEBUG_ROUTES
/**
 * This functions calls an external program for debug reasons. The program
 * is called with an optional '-wait' command line argument, the next
 * argument is the string supplied in the info parameter of this function.
 * Finally a list of devices of all circuits is appended. The program is send
 * to the background to not interfere with imond.
 *
 * \param wait if TRUE, call the external program with '-wait'
 * \param info additional info, this is appended to the command line of the
 *             external command.
 */
void show_routing (int wait, char * info)
{
    char buf [1024];
    int i;
    strcpy (buf, "/usr/local/bin/route_check.sh ");
    if (wait)
    {
	strcat (buf, "-wait");
    }
    sprintf (buf+strlen (buf), " '%s' ", info);
    for (i=0; i<n_circuits; i++)
    {
	sprintf (buf+strlen (buf), " %s ", circuits[i].device_1);
    }
    if (wait)
    {
	strcat (buf, " &");
    }
    my_system (buf);
}
#endif

/**
 * This function fills the time table with initial values.
 *
 * \param tt pointer to the time table variable in memory
 */
static void
init_time_table (void * tt)
{
    memset (tt, -1, DAYS_PER_WEEK*HOURS_PER_DAY);
}

/**
 * This function checks a time table structure for invalid entries and reports
 * them to the syslog.
 *
 * \param circuit_idx Circuit to check for errors
 */
static void
assert_time_table (int circuit_idx)
{
    int i;
    char * tt = circuits[circuit_idx].time_table;

    for (i = DAYS_PER_WEEK*HOURS_PER_DAY; i; i--, tt++)
    {
	if (*tt == CONF_INVALID)
	{
	    imond_syslog (LOG_ERR, "invalid entry for circuit %s, hour %d\n", 
			  circuits[circuit_idx].name, tt-time_table);
	}
    }
}

/**
 * This function fills a specific range of the global LCR time table with with
 * a specified configuration index.
 *
 * \param circuit_idx number of a circuit
 * \param conf_idx number of a config file line
 * \param start_time start time (hour of week)
 * \param end_time end time (hour of week)
 */
static void
fill_time_table (int circuit_idx, int conf_idx, int start_time, int end_time)
{
    int	i;
    int state = conf_data[conf_idx].state;
    char *tp = circuits[circuit_idx].time_table;

    for (i = start_time; i < end_time; i++)
    {
	tp[i] = conf_idx;
	if (state == CONF_LCR)
	{
	    time_table[i] = conf_idx;
	}
    }

    return;
}


/**
 * This functions reads the configuration file imond.conf and fills the
 * configuration structures with the data.
 */
static void
read_config (void)
{
    FILE *  fp;
    char    buf[256];
    int	    line;
    char    phone_buf[64];
    char    start_week_day[16];
    char    end_week_day[16];
    char    state[16];
    char    device[16];
    char *  p;
    char *  device_1;
    char *  device_2;
    char    default_route[16];
    char    circuit[32];
    char    type[32];
    int	    bandwidth_time;
    int	    bandwidth_rate;
    int	    state_val;
    int	    is_default_route;
    int	    day;
    int	    start_day;
    int	    end_day;
    int	    start_hour;
    int	    end_hour;
    int	    start_time;
    int	    end_time;
    int	    hup_timeout;
    int	    i;

    init_time_table (time_table);

    fp = fopen (CONF_FILE, "r");
    imond_syslog (LOG_WARNING, "%s: %s\n", "reading", CONF_FILE);
    if (! fp)
    {
	imond_syslog (LOG_WARNING, "%s: %s\n", CONF_FILE, strerror (errno));
	return;
    }

    for (line = 0, n_conf_lines_used = 0;
	 n_conf_lines_used < MAX_CONFIG_LINES && fgets (buf, 128, fp);
	 line++)
    {
	if (*buf == '#')
	{
	    continue;
	}
	type[0] = '\0';
	if (sscanf (buf, "%2s-%2s %2d-%2d %s %d:%d %d %s %s %s %s %f %d %s",
		    start_week_day,
		    end_week_day,
		    &start_hour,
		    &end_hour,
		    device,
		    &bandwidth_rate,
		    &bandwidth_time,
		    &hup_timeout,
		    state,
		    default_route,
		    phone_buf,
		    circuit,
		    &conf_data[n_conf_lines_used].charge_per_minute,
		    &conf_data[n_conf_lines_used].charge_int,
		    type) < 14)
	{
	    imond_syslog (LOG_ERR, "%s: format error in line %d\n",
			  CONF_FILE, line + 1);
	    fprintf (stderr, "imond: %s: format error in line %d\n",
		     CONF_FILE, line + 1);
	    fclose (fp);
	    exit (1);
	}

	p = strchr (phone_buf, ':');		/* outgoings:incomings  */

	if (p)
	{
	    *p++ = '\0';
	    strncpy (conf_data[n_conf_lines_used].incoming_phone, p, 127);
	}

	strncpy (conf_data[n_conf_lines_used].outgoing_phone, phone_buf, 127);

	device_1 = device;
	device_2 = strchr (device, '/');

	if (device_2)
	{
	    *device_2++ = '\0';
	}

	for (i = 0; i < n_circuits; i++)
	{
	    if (! strcmp (circuits[i].device_1, device_1) && 
		! strcmp (circuits[i].name, circuit))
	    {
		break;
	    }
	}

	conf_data[n_conf_lines_used].circuit_idx = i;

	if (i == n_circuits)
	{
	    assert_time_table(n_circuits);

	    circuits[i].check_routing = 0;
	    circuits[i].online = 0;

	    strcpy (circuits[i].name, circuit);
	    strcpy (circuits[i].type, type);
	    if (! strcmp(type, "pppoe"))
	    {
		circuits[i].check_routing = pppoe_check_routing;
	    }
	    if (! strcmp (device, "pppoe"))
	    {
		strcpy (pppoe_infos.status, get_status_str (stat_offline));
		circuits[i].is_pppoe = TRUE;
		pppoe_used = TRUE;
	    }
	    else
	    {
		circuits[i].is_pppoe = FALSE;
	    }

	    init_time_table (circuits[i].time_table);
	    n_circuits++;
	}
	else
	{
	    if (strcmp (circuits[i].name, circuit) != 0)
	    {
		imond_syslog (LOG_WARNING,
			      "device %s has different circuit names: "
			      "%s and %s\n",
			      device, circuits[i].name, circuit);
	    }
	}

	strcpy (circuits[i].device_1, device_1);

	if (device_2)
	{
	    strcpy (circuits[i].device_2, device_2);
	}
	else
	{
	    *(circuits[i].device_2) = '\0';
	}

	circuits[i].bandwidth_time  = bandwidth_time;
	circuits[i].bandwidth_rate  = bandwidth_rate;
	circuits[i].hup_timeout	    = hup_timeout;

	if (! strcmp (state, "disabled"))
	{
	    state_val = CONF_DISABLED;
	}
	else if (! strcmp (state, "yes"))
	{
	    state_val = CONF_LCR;
	}
	else
	{
	    state_val = CONF_ENABLED;
	}

	if (! strcmp (default_route, "yes"))
	{
	    is_default_route = TRUE;
	}
	else
	{
	    is_default_route = FALSE;
	}

	conf_data[n_conf_lines_used].state		= state_val;
	conf_data[n_conf_lines_used].is_default_route	= is_default_route;

	// FIXME: more error checking
	start_day   = convert_week_day_to_day (start_week_day);
	end_day	    = convert_week_day_to_day (end_week_day);

	day = start_day;

	for (;;)
	{
	    if (start_hour > end_hour)
	    {
		start_time  = day * HOURS_PER_DAY + start_hour;
		end_time    = day * HOURS_PER_DAY + HOURS_PER_DAY;

		fill_time_table (i, n_conf_lines_used, start_time, end_time);

		start_time  = day * HOURS_PER_DAY + 0;
		end_time    = day * HOURS_PER_DAY + end_hour;

		fill_time_table (i, n_conf_lines_used, start_time, end_time);
	    }
	    else
	    {
		start_time  = day * HOURS_PER_DAY + start_hour;
		end_time    = day * HOURS_PER_DAY + end_hour;

		fill_time_table (i, n_conf_lines_used, start_time, end_time);
	    }

	    if (day == end_day)
	    {
		break;
	    }

	    day++;

	    if (day == DAYS_PER_WEEK)
	    {
		day = 0;
	    }
	}

	n_conf_lines_used++;
    }
    fputs (get_timetable (-1), stdout);
    for (i=0; i<n_circuits; i++)
	fputs (get_timetable (i), stdout);
    fclose (fp);
    return;
} /* read_config () */

/**
 * This functions catches ending signals and ends imond.
 *
 * \param sig The catched signal
 */
static void
catch_sig (int sig)
{
    imond_syslog (LOG_INFO, "%s: got signal %d, exiting now\n", argv0, sig);
    exit (sig);
} /* catch_sig (sig) */

/**
 * This function sets default values for some structures.
 */
void init_values (void)
{
    globals = (globals_t) { "", "", "",
			    0, 0, 0, 0, 
			    { 0 }, 
			    5000, DIALMODE_OFF, 0, 0, 0, 0, 0};
    
    /* imond is always bound on 127.0.0.1 (aka localhost) */
    globals.ips[0] = "127.0.0.1";
    conn_init ();
}

/**
 * This is the main function. This is were all the fun things start ;)
 *
 * \param argc The system-supplied number of arguments
 * \param argv The system-supplied array of arguments
 *
 * \return Never. So not interesting.
 */
int
main (int argc, char ** argv)
{
    FILE *			pid_fp;
    time_t			cur_seconds;
    time_t			last_seconds = 0;
    int				select_rtc;
    int                         killpid;

    float			seconds_passed;

#ifndef TEST
    int				rtc;
#endif

    (void) signal (SIGHUP, SIG_IGN);
    (void) signal (SIGPIPE, SIG_IGN);

    init_values ();
    if (parse_args (argc-1, argv+1) < 0) {
	return (-1);
    }
    
#ifndef TEST
    rtc = fork ();

    if (rtc < 0)
    {
	perror ("fork");
	exit (1);
    }

    if (rtc > 0)				    // it's the parent  
    {
	return (0);
    }
#endif

    signal (SIGINT, catch_sig);
    signal (SIGTERM, catch_sig);

    argv0 = argv[0];

    read_config ();
    check_kernel_version ();
    check_system ();
    isdn_init ();
    pppoe_init ();

    if ((conn_listen (globals.ips, globals.imond_port)) < 0) 
    {
	exit(1);
    }

    check_routing (SET_IMMEDIATELY);

    pid_fp = fopen ("/var/run/imond.pid", "w");

    if (pid_fp)
    {
	fprintf (pid_fp, "%d\n", getpid ());
	fclose (pid_fp);
    }

    /* main loop */
    for (;;)
    {
	select_rtc = conn_select ();
	
	/* kill all zombies from beep & sync  */
	do
	{
	    killpid = waitpid (-1, (int *) NULL, WNOHANG);
	
	    /* we killed the beep-process, make next beep possible */
	    if (killpid > 0 && killpid == beep_pid)
	    {
		beep_pid = 0;
	    }
	} while (killpid > 0);

	/* Update traffic data */
	if (gettimeofday (&current_time, NULL) == 0)
	{
	    seconds_passed =
		(float) (current_time.tv_sec  - last_time_measured.tv_sec) +
		(float) (current_time.tv_usec - last_time_measured.tv_usec) /
		1000000.;
	    if (seconds_passed >= MIN_MEASURE_INTERVAL)
	    {
#ifdef TEST
		printf ("seconds_passed=%f\n", (double) seconds_passed);
#endif
		measure_pppoe_rates (seconds_passed);
		measure_isdn_rates (seconds_passed);

		last_time_measured = current_time;
	    }
	}

	if (select_rtc > 0)
	{
	    conn_handle_fd ();
	}
	/*------------------------------------------------------------
	 * imonc makes heavy I/O, so the select timeout will not
	 * be activated if imonc is connected. But we cannot
	 * call check_routing() too often, because it slows
	 * down the machine. Here we do it 10 seconds after the
	 * last call.
	 *------------------------------------------------------------
	 */
	
	cur_seconds = time ((time_t *) NULL);
	
	if (cur_seconds >= last_seconds + POLLING_INTERVAL)
	{
	    check_routing (SET_IF_CHANGED);
	    last_seconds = cur_seconds;
	}
    }

    /* NOTREACHED */
    return (0);
} /* main (int argc, char **argv) */

/* vi: set softtabstop=4 shiftwidth=4: */
