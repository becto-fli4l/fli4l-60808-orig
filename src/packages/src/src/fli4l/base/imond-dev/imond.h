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
 * \brief Global declarations
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#ifndef IMOND_H
#define IMOND_H

#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/ppp_defs.h>

#ifndef aligned_u64
#  define aligned_u64 unsigned long long __attribute__((aligned(8)))
#endif

#include <linux/types.h>
#include <linux/if_ppp.h>
#include <linux/isdn.h>

#include <string.h>
#include <errno.h>

#include <time.h>
/* 
 * commonly used constants 
 */

#define TRUE			1
#define FALSE			0

#define PROTOCOL_VERSION	12		/* protocol version	    */
#define IMOND_VERSION		"2.1.9"		/* imond version	    */

#define DIALMODE_OFF		0
#define DIALMODE_AUTO		1
#define DIALMODE_MANUAL		2

/*
 * command line options
 */
int parse_args (int argc, char ** argv);

/* 
 * connection handling 
 */

struct client_t
{
    int		    fd;
    char	    ip[16];
    unsigned char   password_state;
    char	    salt[33];
    int 	    (*conn_handler)(struct client_t * client); 
    char * 	    handler_name; 
};

extern int password_mode;
extern int dialmode;
extern struct client_t client[];

#define	PASS_REQUIRED		1		/* flag if normal pass req. */
#define ADMIN_PASS_REQUIRED	2		/* flag if admin pass req.  */
#define ADMIN_MODE              4		/* flag if in admin mode    */

#define PASSWORD_STATE_NONE	0
#define PASSWORD_STATE_NORMAL	1
#define PASSWORD_STATE_ADMIN	2

#define MAX_FNAME		256
#define MAX_CLIENTS		64
/** imond listens on maxium this number of IPs */
#define MAX_IP			8

/** polling interval for check_routing() */
#define POLLING_INTERVAL	10

void conn_init (void);
int conn_register_fd (int new_fd, const char * ip, int password_state, 
		      int (*conn_handler)(struct client_t * client),
		      char * handler_name);
int conn_listen (char **ips, int port);
int conn_select (void);
void conn_handle_fd (void);

/*
 * user interface
 */
int	my_system (char * cmd);
int	map_channel (int info_idx);
int	find_channel_idx_by_circuit_idx (int circuit_idx);
int	find_pppoe_conf_data (int circuit_idx);
int	find_conf_data (char * phone, int is_outgoing_connection);
char *	get_timetable (int circuit_idx);
int	find_circuit (void);
char *	get_circuit_name_by_conf_idx (int conf_idx);

int	set_route (int circuit_idx);
int	set_dialmode (int new_dialmode);
int	do_dial (char * channel_id, int dial_flag);
int	addlink (int circuit_idx);
int	removelink (int circuit_idx);

int cmd_setstatus (char * answer, int argc, char * argv[], int client_idx);

void beep (int is_going_up);

/* possible commands for check_routing */
#define SET_IMMEDIATELY		1
#define SET_AGAIN		2
#define SET_IF_CHANGED		3

void check_routing (int);

/* 
 * Struct for all config variables set by environment or command
 * line 
 */
#define OPT_SIZE 32
typedef struct
{
    char 	imond_pass[OPT_SIZE+1];
    char 	imond_admin_pass[OPT_SIZE+1];
    char	mgetty_log_file[MAX_FNAME];
    char *	led_device;	    /* led: /dev/comX */
    char *	telmond_log_dir;
    char *	imond_log_dir;
    char *	mgetty_log_dir;
    char *	ips[MAX_IP+1];
    int		imond_port;
    int		dialmode;
    int		do_beep;
    int		may_enable;
    int		may_dial;
    int		may_route;
    int		may_reboot;
    int		circ;
    int		log_proto;
    int		pppoe_used;
} globals_t;
extern globals_t globals;

/*
 * circuit config, info structures
 */
typedef struct
{
     char    state;		    /* 0 - disabled
 				       1 - enabled
 				       2 - lcr */
    char    is_default_route;	    /* TRUE or FALSE			    */
    char    outgoing_phone[128];    /* outgoing phone number		    */
    char    incoming_phone[128];    /* incoming phone numbers (sep by comma)*/
    int	    circuit_idx;	    /* index of circuit			    */
    float   charge_per_minute;	    /* charge per min			    */
    int	    charge_int;		    /* charge interval in seconds, e.g. 60  */
} conf_data_t;

typedef struct
{
    char    name[32];		    /* name of circuit			    */
    char    type[32];		    /* type of circuit			    */
    void    (*check_routing)(int circuit_idx);
    int	    is_pppoe;		    /* flag: pppoe circuit 		    */
    char    device_1[16];	    /* "ipppX" or "isdnX"		    */
    char    device_2[16];	    /* "ipppX" or "isdnX" (ch-bundling)	    */
    int     online;                 /* flag if circuit online               */
    int	    hup_timeout;	    /* hangup timeout in seconds	    */
    int	    bandwidth_time;	    /* time for bandwidth channel bundling  */
    int	    bandwidth_rate;	    /* rate for bandwidth channel bundling  */
    int	    time_2;		    /* time down			    */
    int	    rate_2;		    /* rate down			    */
    int	    link_added;		    /* flag if link added (for ch-bundling) */
    char    time_table[168];        /* timetable			    */

} circuit_t;

typedef struct
{
    int		    is_active;			/**< flag: TRUE if active     */
    int		    conf_idx;			/**< idx in array conf_data   */
    int		    is_outgoing;		/**< flag: TRUE if outgoing   */
    unsigned long   ibytes;			/**< # input bytes	      */
    unsigned long   obytes;			/**< # output bytes	      */
    unsigned long   ibytes_overflows;		/**< # of overflows in ibytes */
    unsigned long   obytes_overflows;		/**< # of overflows in obytes */
    int		    time_bandwidth_higher;	/**< time since rate higher   */
    int		    time_bandwidth_lower;	/**< time since rate lower    */
    int		    ibytes_per_second;		/**< input bytes per second   */
    int		    obytes_per_second;		/**< output bytes per second  */
    time_t	    online_time;		/**< online time in seconds   */
    time_t	    charge_time;		/**< charge time in seconds   */
    time_t	    online_start;		/**< time start we are online */
    float	    charge;			/**< charge		      */
    char	    ip[16];			/**< only filled if wanted    */
    char	    drvid[32];
    char	    status[32];
    char	    phone[32];
    char	    usage[32];
    char	    inout[32];
} isdn_info_t;

typedef struct
{
    int		    conf_idx;			/* idx in array conf_data   */
    time_t	    online_time;		/* online time in seconds   */
    time_t	    charge_time;		/* charge time in seconds   */
    time_t	    online_start;		/* time start we are online */
    unsigned long   ibytes_start;		/* # input bytes	    */
    unsigned long   obytes_start;		/* # output bytes	    */
    unsigned long   ibytes;                     /* # input bytes            */
    unsigned long   obytes;                     /* # output bytes           */
    unsigned long   ibytes_overflows;           /* # of overflows in ibytes */
    unsigned long   obytes_overflows;           /* # of overflows in obytes */
    int		    ibytes_per_second;		/* input bytes per second   */
    int		    obytes_per_second;		/* output bytes per second  */
    float	    charge;			/* charge		    */
    char	    ip[16];			/* only filled if wanted    */
    char	    status[32];
} pppoe_info_t;

#define INIT_PPPOE_INFO ((pppoe_info_t){-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0., })

typedef enum {
    stat_exclusive=0,
    stat_offline,
    stat_calling,
    stat_online
} status_t;

extern char * 		status_str[];
#define get_status_str(status)  status_str[status]

extern conf_data_t  	conf_data[];
extern int		n_conf_lines_used;

extern circuit_t 	circuits[];
extern int 		n_circuits;
extern int		route_idx;	    /* -1: automatic	    */

extern pppoe_info_t 	pppoe_infos;
extern int		pppoe_used;	    /* flag if pppoe used	    */

extern isdn_info_t 	infos[];
extern int 		n_active_channels;  /* number of active channels    */

extern struct timeval	last_time_measured;
extern struct timeval	current_time;

extern int		running_under_2_4;
extern char             systemname[10];
extern int		online_counter;

extern char 		default_circuit[32];
/* 
 * circuit types
 */
void isdn_init (void);
void measure_isdn_rates (float seconds_passed);
int  circuit_device_online (char * name);

void pppoe_init (void);
void measure_pppoe_rates (float seconds_passed);
void pppoe_check_routing (int idx);

#endif /* IMOND_H */
