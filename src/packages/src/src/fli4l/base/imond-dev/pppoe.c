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
 * \brief PPPoE specific functions
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#include "imond.h"
#include "user.h"
#include "log.h"
#include "led.h"

/** structure to hold PPPoE status from ioctl call */
static struct ifpppstatsreq pppoe_req;
static int pppoe_socket = -1;  /**< socket of ppp0 to messure rates */

/** This macro selects the right ibytes value depending on the Linux kernel version */
#define PPPOE_STATS_IBYTES (running_under_2_4 ? pppoe_req.stats.p.ppp_ibytes : pppoe_req.stats.p.ppp_ioctects)
/** This macro selects the right obytes value depending on the Linux kernel version */
#define PPPOE_STATS_OBYTES (running_under_2_4 ? pppoe_req.stats.p.ppp_obytes : pppoe_req.stats.p.ppp_ooctects)

static int pppoe_current_circuit;

/**
 * This function measures the current PPPoE rates. The gathered data is written
 * to pppoe_infos.
 *
 * \param seconds_passed number of seconds since last call of this function
 */
void
measure_pppoe_rates (float seconds_passed)
{
    
    if (! pppoe_used || pppoe_socket < 0)
    {
	return;
    }
    

    if (pppoe_infos.online_start > 0)
    {    
	if (ioctl (pppoe_socket, SIOCGPPPSTATS, &pppoe_req) == 0)
	{
	    unsigned long ibytes, obytes;
	    
	    if (!PPPOE_STATS_IBYTES && !PPPOE_STATS_OBYTES)
	    {
		/* channel not active, hangup? */
		imond_syslog (LOG_INFO, 
			      "measure_pppoe_rates : channel hangup?\n");
		return;
	    }
	    ibytes = PPPOE_STATS_IBYTES - pppoe_infos.ibytes;
	    obytes = PPPOE_STATS_OBYTES - pppoe_infos.obytes;
	    if (PPPOE_STATS_IBYTES < pppoe_infos.ibytes) {
		pppoe_infos.ibytes_overflows++;
		imond_syslog (LOG_INFO, "pppoe_ibytes overflow: %lx < %lx,"
			      " pppoe_ibytes_overlow=%ld\n", 
			      (unsigned long)PPPOE_STATS_IBYTES,
			      pppoe_infos.ibytes,
			      pppoe_infos.ibytes_overflows);
	    }
	    if (PPPOE_STATS_OBYTES < pppoe_infos.obytes) {
		pppoe_infos.obytes_overflows++;
		imond_syslog (LOG_INFO, "pppoe_obytes overflow: %lx < %lx,"
			      " pppoe_obytes_overlow=%ld\n", 
			      (unsigned long)PPPOE_STATS_OBYTES,
			      pppoe_infos.obytes,
			      pppoe_infos.obytes_overflows);
	    }

	    pppoe_infos.ibytes_per_second = (int)
		((float) ibytes / seconds_passed);
	    
	    pppoe_infos.obytes_per_second = (int)
		((float) obytes / seconds_passed);

	    pppoe_infos.ibytes = PPPOE_STATS_IBYTES;
	    pppoe_infos.obytes = PPPOE_STATS_OBYTES;
	}
    }
    else					/* we are offline   */
    {
	pppoe_infos.ibytes_per_second = 0;
	pppoe_infos.obytes_per_second = 0;
    }

    return;
} /* measure_pppoe_rates */

/**
 * This functions lets another process change the status of the PPPoE
 * connection. It only starts when \a argv[1] is "pppoe:" and \a argv[2] is
 * either "up" or "down" and takes messures to signal the user the new state
 * (LED, beep, status string). When the connection is going down, a log entry
 * is also written.
 *
 * \todo Maybe add access restriction, this should only be possible from
 * 127.0.0.1
 *
 * \see cmd_help for parameters
 *
 * \retval OK everything was fine
 * \retval ERR arguments were wrong or connection was already up or down.
 */
int 
cmd_setstatus (char * answer, int argc, char * argv[], int client_idx)
{
    int rtc = ERR;
    const char name_tag[]="name:";
    const int tag_size = sizeof(name_tag)-1;

    if (! strcmp (argv[1], "pppoe:"))		/* e.g. "pppoe: up" */
    {
	if (! strcmp (argv[2], "up"))
	{
	    if (pppoe_infos.online_start == 0)	    /* new connection? */
	    {
		online_counter++;
		led_on ();
		beep (TRUE);

		strcpy (pppoe_infos.status, get_status_str(stat_online));

		pppoe_infos.online_start = time ((time_t *) NULL);

		pppoe_infos.conf_idx = find_pppoe_conf_data (pppoe_current_circuit);

		rtc = OK;
	    }
	}
	else if (! strcmp (argv[2], "down"))
	{
	    strcpy (pppoe_infos.status, get_status_str (stat_offline));

	    if (pppoe_infos.online_start != 0)		/* known connection?*/
	    {
		int	conf_idx    = pppoe_infos.conf_idx;
		int	circuit_idx =
		    (conf_idx >= 0 ? conf_data[conf_idx].circuit_idx : -1);
		int	charge_int  =
		    (conf_idx >= 0 ? conf_data[conf_idx].charge_int : 1);
		int	charge_per_minute = (conf_idx >= 0 ?
			conf_data[conf_idx].charge_per_minute : 0);
		time_t	now	    = time ((time_t *) 0);
		time_t	diff	    = now - pppoe_infos.online_start;
		time_t  charge_diff;
		float   charge;
		
		if (online_counter > 0)
		{
		    online_counter--;

		    if (online_counter == 0)
		    {
		        led_off ();
		    }

		    beep (FALSE);
		}

		charge = (diff * charge_per_minute) / 60;
		charge_diff = diff;
		if (charge_int > 1)
		{
		    charge_diff = (charge_diff / charge_int) * charge_int 
			+ charge_int;
		}
 
		pppoe_infos.charge_time += charge_diff;
		pppoe_infos.charge += charge;

		imond_write_record (circuit_idx >= 0 ?
					circuits[circuit_idx].name :
					"<unknown>",
				    pppoe_infos.online_start,
				    charge_diff, charge,
				    pppoe_infos.ibytes_overflows, 
				    pppoe_infos.ibytes,
				    pppoe_infos.ibytes_start,
				    pppoe_infos.obytes_overflows, 
				    pppoe_infos.obytes,
				    pppoe_infos.obytes_start,
				    circuit_idx >= 0 ?
					circuits[circuit_idx].device_1 :
					"<unknown>",
				    charge_int, charge_per_minute);

		/* pppoe doesn't reset byte counters if hangup */
		pppoe_infos.ibytes_start	= pppoe_infos.ibytes;
		pppoe_infos.obytes_start	= pppoe_infos.obytes;
		pppoe_infos.ibytes		= 0;
		pppoe_infos.obytes		= 0;
		pppoe_infos.ibytes_overflows	= 0;
		pppoe_infos.obytes_overflows	= 0;
		pppoe_infos.ibytes_per_second	= 0;
		pppoe_infos.obytes_per_second	= 0;
		pppoe_infos.online_time		+= diff;
		pppoe_infos.online_start	= 0;
		pppoe_infos.ip[0]		= '\0';

		rtc = OK;
	    }

	    check_routing (SET_AGAIN);		    /* must set route again */
	}
	else if (! strncmp (argv[2], name_tag, tag_size))
	{
	  int	conf_idx    = pppoe_infos.conf_idx;
	  int	circuit_idx =
	    (conf_idx >= 0 ? conf_data[conf_idx].circuit_idx : -1);
	  imond_syslog (LOG_INFO, "changing name from %s to %s\n", 
			circuits[circuit_idx].name, argv[2]+tag_size);
	  if (strlen(argv[2])-tag_size < sizeof(circuits[circuit_idx].name)) 
	    {
	      if (!strcmp(circuits[circuit_idx].name, default_circuit))
		strcpy(default_circuit, argv[2]+tag_size);
	      strcpy(circuits[circuit_idx].name, argv[2]+tag_size);
	      imond_syslog (LOG_INFO, "new name now %s\n",  
			    circuits[circuit_idx].name);
	      pppoe_infos.ibytes_start	= 0;
	      pppoe_infos.obytes_start	= 0;
	      pppoe_infos.ibytes		= 0;
	      pppoe_infos.obytes		= 0;
	      pppoe_infos.ibytes_overflows	= 0;
	      pppoe_infos.obytes_overflows	= 0;
	      pppoe_infos.ibytes_per_second	= 0;
	      pppoe_infos.obytes_per_second	= 0;
	      pppoe_infos.online_time		= 0;
	      pppoe_infos.online_start	= 0;
	      pppoe_infos.ip[0]		= '\0';
	    }
	}
    }

    return (rtc);
}

/**
 * This function initalizes all structures used for PPPoE.
 */
void pppoe_init (void)
{
    char * interface = "ppp0";
    int i;

    if (!pppoe_used)
	return;

    /* Initialising of the traffic data with 0 */
    pppoe_infos.ibytes_overflows = 0;
    pppoe_infos.obytes_overflows = 0;

    for (i=0; i < n_circuits; i++) {
      if (circuits[i].is_pppoe) {
	pppoe_current_circuit = i;
	break;
      }
    }
    pppoe_socket = socket (AF_INET, SOCK_DGRAM, 0);
    
    if (pppoe_socket < 0)
    {
	perror ("imond: couldn't create IP socket");
    }
    else
    {
	struct ifreq	    ifr;
	
	strncpy (ifr.ifr_ifrn.ifrn_name, interface,
		 sizeof (ifr.ifr_ifrn.ifrn_name));
	
	if (ioctl (pppoe_socket, SIOCGIFFLAGS, (void *) &ifr) < 0)
	{
	    close (pppoe_socket);
	    pppoe_socket = -1;
	    imond_syslog (LOG_ERR,
			  "imond: nonexistent PPPoE interface: '%s'\n",
			  interface);
	}
	else
	{
	    pppoe_req.stats_ptr = (void *) &pppoe_req.stats;
	    strncpy (pppoe_req.ifr__name, interface,
		     sizeof(pppoe_req.ifr__name));
	}
    }
    
    if (pppoe_socket >= 0 &&
	ioctl (pppoe_socket, SIOCGPPPSTATS, &pppoe_req) == 0)
    {
	pppoe_infos.ibytes_start = PPPOE_STATS_IBYTES;
	pppoe_infos.obytes_start = PPPOE_STATS_OBYTES;
    }
}

void
pppoe_check_routing (int idx)
{
  char buf[128];
  if (pppoe_current_circuit == idx)
    return;

  imond_syslog (LOG_INFO, "pppoe_check_routing: current circuit: %s (%d), new circuit: %s (%d)\n",
		pppoe_current_circuit < 0 ? "None" : circuits[pppoe_current_circuit].name, pppoe_current_circuit, 
		idx < 0 ? "None" : circuits[idx].name, idx);
		
  // circuit change, stop current circuit, start a new one if necessary
  if (pppoe_current_circuit >= 0) {
    snprintf (buf, sizeof(buf)-1, "pppoe-circuit-ctrl.sh stop %s %s", 
	      circuits[pppoe_current_circuit].name, idx>=0 ? ";" : "");
    pppoe_infos = INIT_PPPOE_INFO;
    pppoe_infos.ip[0] = '\0';
    strcpy (pppoe_infos.status, get_status_str(stat_offline));
  }
  if (idx >= 0) {
    snprintf (buf+strlen(buf), sizeof(buf)-1, 
	      "pppoe-circuit-ctrl.sh start %s  ", 
	      circuits[idx].name);
    pppoe_current_circuit = idx;
    pppoe_infos.conf_idx = find_pppoe_conf_data (idx);
    strcpy (pppoe_infos.status, get_status_str(stat_offline));
  }
  my_system(buf);
}
