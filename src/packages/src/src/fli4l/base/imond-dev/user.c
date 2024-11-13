/*
 * IMonD - deamon to monitor/control network connections
 *
 * Copyright (c) 2000-2016 - Frank Meyer, fli4l-Team <team@fli4l.de>
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
 *
 * Last Update:  $Id$
 */
/**
 * \file
 * \brief Client command parsing
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdio.h>
#include <shadow.h>
#include <crypt.h>
#include <time.h>
#include <linux/isdn.h>

#include "imond.h"
#include "user.h"
#include "log.h"
#include "date.h"
#include "md5.h"

/** file transfer bufffer size */
#define TRANSFER_BUFFER_SIZE	1024

/** string only including ACK char */
#define ACK_STRING		"\006"
/** string only including NAK char */
#define NAK_STRING		"\025"
/** ACK char */
#define ACK_CHAR		'\006'
/** NAK char */
#define NAK_CHAR		'\025'

/** uptime measure time */
#define UPTIME_MEASURE_TIME	60
/** cpu load measure time */
#define CPU_MEASURE_TIME	5

/** Use this to get an unified calling syntax for all user commands */
#define USERCOMMAND(X)	static int X (char * answer, int argc, char * argv[], int client_idx)

USERCOMMAND(cmd_help);
USERCOMMAND(cmd_adjusttime);
USERCOMMAND(cmd_pass);
USERCOMMAND(cmd_salt);
USERCOMMAND(cmd_md5pass);
USERCOMMAND(cmd_quit);
USERCOMMAND(cmd_ip);
USERCOMMAND(cmd_version);
USERCOMMAND(cmd_isenabled);
USERCOMMAND(cmd_fli4lid);
USERCOMMAND(cmd_highscore);
USERCOMMAND(cmd_hostname);
USERCOMMAND(cmd_dialmode);
USERCOMMAND(cmd_channels);
USERCOMMAND(cmd_timetable);
USERCOMMAND(cmd_circuits);
USERCOMMAND(cmd_date);
USERCOMMAND(cmd_route);
USERCOMMAND(cmd_pppoe);
USERCOMMAND(cmd_uptime);
USERCOMMAND(cmd_cpu);
USERCOMMAND(cmd_telmondlogfile);
USERCOMMAND(cmd_imondlogfile);
USERCOMMAND(cmd_mgettylogfile);
USERCOMMAND(cmd_clientip);
USERCOMMAND(cmd_enable);
USERCOMMAND(cmd_disable);
USERCOMMAND(cmd_reboot);
USERCOMMAND(cmd_halt);
USERCOMMAND(cmd_poweroff);
USERCOMMAND(cmd_dial);
USERCOMMAND(cmd_hangup);
USERCOMMAND(cmd_sync);
USERCOMMAND(cmd_resettelmondlogfile);
USERCOMMAND(cmd_resetimondlogfile);
USERCOMMAND(cmd_driverid);
USERCOMMAND(cmd_status);
USERCOMMAND(cmd_phone);
USERCOMMAND(cmd_onlinetime);
USERCOMMAND(cmd_time);
USERCOMMAND(cmd_chargetime);
USERCOMMAND(cmd_charge);
USERCOMMAND(cmd_usage);
USERCOMMAND(cmd_inout);
USERCOMMAND(cmd_quantity);
USERCOMMAND(cmd_rate);
USERCOMMAND(cmd_circuit);
USERCOMMAND(cmd_device);
USERCOMMAND(cmd_links);
USERCOMMAND(cmd_logdir);
USERCOMMAND(cmd_isallowed);
USERCOMMAND(cmd_support);
USERCOMMAND(cmd_receive);
USERCOMMAND(cmd_send);
USERCOMMAND(cmd_delete);
USERCOMMAND(cmd_huptimeout);
USERCOMMAND(cmd_addlink);
USERCOMMAND(cmd_removelink);
USERCOMMAND(cmd_system);

/**
 * This is a structure to express one user command.
 */
struct command
{
    const char * name; /**< Command name */
    char valid_as;
	    /**< Who can execute the command.
	     * \li PASSWORD_STATE_NONE = everyone,
	     * \li PASSWORD_STATE_NORMAL = user,
	     * \li PASSWORD_STATE_ADMIN = admin */
    int args;	/**< How much arguments the command takes. It is a bit-array:
		 * \li 0. bit set: can be called without arguments,
		 * \li 1. bit set: can be called with 1 argument,
		 * \li etc. */
    int (* function)(char *, int, char **, int); 
	/**< The function to execute when the user sends this command.
	 * Parameters are: answer, argc, argv, client_idx.
	 *
	 * May return #ERR, #OK, #RAW or #QUIT
	 * \see USERCOMMAND, cmd_help */
    const char * paramhelp; /**< A string describing the parameters the
			     * command can get */
    const char * help;	/**< The help line for this command */
};

/** This is the list of all commands a client/user can send to imond
 * 
 * \warning These must be kept in strict alphabetical order or bsearch(3)
 * will not work anymore (This is what the bsearch man page says...)
 * \warning Sorting can be achieved with this command (only when the original
 * syntax stayed intact):
 * \verbatim sed -e :a -e '/",$/N; s/",\n/",##/; ta' | sort | sed 's/##/\n/' \endverbatim
 */
static const struct command commands[] =
{
    { "addlink", PASSWORD_STATE_ADMIN, 1<<1, cmd_addlink, "ci-index",
	"add link to channel"},
    { "adjust-time", PASSWORD_STATE_ADMIN, 1<<1, cmd_adjusttime, "#seconds",
	"adjust time by n seconds"},
    { "channels", PASSWORD_STATE_NORMAL, 1<<0, cmd_channels, "",
	"show number of channels"},
    { "charge", PASSWORD_STATE_NORMAL, 1<<1, cmd_charge, "#channel-id",
	"show charge"},
    { "chargetime", PASSWORD_STATE_NORMAL, 1<<1, cmd_chargetime, "#channel-id",
	"show sum of charge times"},
    { "circuit", PASSWORD_STATE_NORMAL, 1<<1, cmd_circuit, "ci-index",
	"show name of circuit"},
    { "circuits", PASSWORD_STATE_NORMAL, 1<<0, cmd_circuits, "",
	"show number of default route circuits"},
    { "client-ip", PASSWORD_STATE_NORMAL, 1<<0 | 1<<1, cmd_clientip, "[ip-address]",
	"show/set ip address of client"},
    { "cpu", PASSWORD_STATE_NORMAL, 1<<0, cmd_cpu, "",
	"show cpu usage"},
    { "date", PASSWORD_STATE_NORMAL, 1<<0, cmd_date, "",
	"show date"},
    { "delete", PASSWORD_STATE_ADMIN, 1<<2, cmd_delete, "filename pw",
	"delete a file"},
    { "device", PASSWORD_STATE_NORMAL, 1<<1, cmd_device, "ci-index",
	"show device of circuit"},
    { "dial", PASSWORD_STATE_NORMAL, 1<<0 | 1<<1, cmd_dial, "",
	"dial default route connection"},
    { "dialmode", PASSWORD_STATE_NORMAL, 1<<0 | 1<<1, cmd_dialmode, "[auto|manual|off]",
	"get/set dialmode"},
    { "disable", PASSWORD_STATE_NORMAL, 1<<0, cmd_disable, "",
	"hangup, set dialmode off"},
    { "driverid", PASSWORD_STATE_NORMAL, 1<<1, cmd_driverid, "#channel-id",
	"show driver id of channel"},
    { "enable", PASSWORD_STATE_NORMAL, 1<<0, cmd_enable, "",
	"set dialmode to 'auto'"},
    { "fli4l-id", PASSWORD_STATE_NORMAL, 1<<0, cmd_fli4lid, "",
	"get fli4l-id"},
    { "halt", PASSWORD_STATE_NORMAL, 1<<0, cmd_halt, "",
	"halt the router"},
    { "hangup", PASSWORD_STATE_NORMAL, 1<<0 | 1<<1, cmd_hangup, "[#channel-id]",
	"hangup connections"},
    { "help", PASSWORD_STATE_NORMAL, 1<<0 | 1<<1, cmd_help, "[command]",
	"get help to a command"},
    { "highscore", PASSWORD_STATE_NORMAL, 1<<0, cmd_highscore, "",
	"get highscore"},
    { "hostname", PASSWORD_STATE_NORMAL, 1<<0, cmd_hostname, "",
	"get hostname"},
    { "hup-timeout", PASSWORD_STATE_ADMIN, 1<<1 | 1<<2, cmd_huptimeout, "ci-index [value]",
	"show/set hup-timeout of circuit"},
    { "imond-log-file", PASSWORD_STATE_NORMAL, 1<<0, cmd_imondlogfile, "",
	"show logfile of imond"},
    { "inout", PASSWORD_STATE_NORMAL, 1<<1, cmd_inout, "#channel-id",
	"show direction of channel"},
    { "ip", PASSWORD_STATE_NORMAL, 1<<1, cmd_ip, "#channel-id",
	"show ip address of interface using channel"},
    { "is-allowed", PASSWORD_STATE_NORMAL, 1<<1, cmd_isallowed, "dial|dialmode|route|reboot|imond-log|telmond-log|mgetty-log",
	"get permission"},
    { "is-enabled", PASSWORD_STATE_NORMAL, 1<<0, cmd_isenabled, "",
	"return 1 if enabled, else 0"},
    { "links", PASSWORD_STATE_NORMAL, 1<<1, cmd_links, "ci-index",
	"show number of links: 0, 1, or 2"},
    { "log-dir", PASSWORD_STATE_NORMAL, 1<<1, cmd_logdir, "imond|telmond|mgetty",
	"get logdirectory"},
    { "md5pass", PASSWORD_STATE_NONE, 1<<0 | 1<<1, cmd_md5pass, "[password]",
	"set password in a secure way"},
    { "mgetty-log-file", PASSWORD_STATE_NORMAL, 1<<0, cmd_mgettylogfile, "",
	"show logfile of mgetty"},
    { "online-time", PASSWORD_STATE_NORMAL, 1<<1, cmd_onlinetime, "#channel-id",
	"show current online time of channel"},
    { "pass", PASSWORD_STATE_NONE, 1<<0 | 1<<1, cmd_pass, "[password]",
	"ask, if password required or set password"},
    { "phone", PASSWORD_STATE_NORMAL, 1<<1, cmd_phone, "#channel-id",
	"show phone number/circuit name"},
    { "poweroff", PASSWORD_STATE_NORMAL, 1<<0, cmd_poweroff, "",
	"poweroff the router"},
    { "pppoe", PASSWORD_STATE_NORMAL, 1<<0, cmd_pppoe, "",
	"return 1 if pppoe used, otherwise 0"},
    { "quantity", PASSWORD_STATE_NORMAL, 1<<1, cmd_quantity, "#channel-id",
	"show transferred data quantity of channel"},
    { "quit", PASSWORD_STATE_NONE, 1<<0, cmd_quit, "",
	"quit the session"},
    { "rate", PASSWORD_STATE_NORMAL, 1<<1, cmd_rate, "#channel-id",
	"show current transfer rate of channel"},
    { "reboot", PASSWORD_STATE_NORMAL, 1<<0, cmd_reboot, "",
	"reboot the router"},
    { "receive", PASSWORD_STATE_ADMIN, 1<<3, cmd_receive, "filename #bytes pw",
	"receive a file"},
    { "removelink", PASSWORD_STATE_ADMIN, 1<<1, cmd_removelink, "ci-index",
	"remove link from channel"},
    { "reset-imond-log-file", PASSWORD_STATE_ADMIN, 1<<0, cmd_resetimondlogfile, "",
	"reset logfile of imond"},
    { "reset-telmond-log-file", PASSWORD_STATE_ADMIN, 1<<0, cmd_resettelmondlogfile, "",
	"reset logfile of telmond"},
    { "route", PASSWORD_STATE_NORMAL, 1<<0 | 1<<1, cmd_route, "[ci-index]",
	"get/set default route circuit"},
    { "salt", PASSWORD_STATE_NONE, 1<<0, cmd_salt, "",
	"send a random salt to the client"},
    { "send", PASSWORD_STATE_ADMIN, 1<<2, cmd_send, "filename pw",
	"send a file"},
    { "set-status", PASSWORD_STATE_ADMIN, 1<<2, cmd_setstatus, "status",
	"set status of imond"},
    { "status", PASSWORD_STATE_NORMAL, 1<<1, cmd_status, "#channel-id",
	"show status of channel"},
    { "support", PASSWORD_STATE_ADMIN, 1<<1, cmd_support, "password",
	"show config/status of fli4l"},
    { "sync", PASSWORD_STATE_NORMAL, 1<<0, cmd_sync, "",
	"synchronize cache with mounted devices"},
    { "system", PASSWORD_STATE_NORMAL, 1<<0, cmd_system, "",
        "returns under which system imond is running"}, 
    { "telmond-log-file", PASSWORD_STATE_NORMAL, 1<<0, cmd_telmondlogfile, "",
	"show logfile of telmond"},
    { "time", PASSWORD_STATE_NORMAL, 1<<1, cmd_time, "#channel-id",
	"show sum of online times of channel"},
    { "timetable", PASSWORD_STATE_NORMAL, 1<<0 | 1<<1, cmd_timetable, "[ci-index]",
	"show timetable"},
    { "uptime", PASSWORD_STATE_NORMAL, 1<<0, cmd_uptime, "",
	"show router uptime (in seconds)"},
    { "usage", PASSWORD_STATE_NORMAL, 1<<1, cmd_usage, "#channel-id",
	"show usage of channel"},
    { "version", PASSWORD_STATE_NONE, 1<<0, cmd_version, "",
	"show protocol- and imond-version"},
};

/** number of commands, calculated from the array ::commands */
#define NUMCOMMANDS (sizeof (commands) / sizeof (struct command) )

/** How many commands should be displayed on one line in the help output */
#define HELP_COMMANDSBYLINE   3
/** How much space can one command consume in the help command overview */
#define HELP_COMMANDWIDTH     "22"

/**
 * This function receives a file from the user.
 *
 * \param client_idx index of client who sends us the file
 * \param filename where the file will be saved
 * \param nbytes size of file
 *
 * \retval OK the transfer was successful
 * \retval ERR an error occoured while transfering the file
 *
 * \par file transfer:
 * \verbatim
    command: receive filename nbytes password
    send ACK or NAK
    if ACK:
	repeat:
	    receive raw data in 1024 byte blocks (or rest)
	    send ACK or NAK
	end-repeat
    end-if
    send OK\r\n or ERR\r\n
    \endverbatim
 */
static int
receive_file (int client_idx, char * filename, long nbytes)
{
    char    buffer[TRANSFER_BUFFER_SIZE];
    FILE *  fp;
    size_t  bytes_to_receive;
    size_t  bytes_read;
    size_t  bytes_sum;

    fp = fopen (filename, "w");

    if (! fp)
    {
	write (client[client_idx].fd, NAK_STRING, 1);
	return (ERR);
    }

    while (nbytes > 0)
    {
	write (client[client_idx].fd, ACK_STRING, 1);

	bytes_sum = 0;

	if (nbytes < TRANSFER_BUFFER_SIZE)
	{
	    bytes_to_receive = nbytes;
	    nbytes = 0;
	}
	else
	{
	    bytes_to_receive = TRANSFER_BUFFER_SIZE;
	    nbytes -= TRANSFER_BUFFER_SIZE;
	}

	do
	{
	    bytes_read = read (client[client_idx].fd, buffer,
			       bytes_to_receive - bytes_sum);

	    if (bytes_read <= 0 ||
		fwrite (buffer, bytes_read, 1, fp) != 1)
	    {
		fclose (fp);
		write (client[client_idx].fd, NAK_STRING, 1);
		return (ERR);
	    }

	    bytes_sum += bytes_read;
	} while (bytes_sum < bytes_to_receive);
    }

    (void) fclose (fp);
    (void) write (client[client_idx].fd, ACK_STRING, 1);
    return (OK);
} /* receive_file (client_idx, filename, nbytes) */


/**
 * This function sends a file to the user.
 *
 * \param client_idx index of client who requested the file
 * \param filename the file that was requested
 *
 * \retval OK the transfer was successful
 * \retval ERR an error occoured while transfering the file
 *
 * \par file transfer:
 * \verbatim
    command: send filename password
    if filename ok:
	send "OK len-in-bytes\r\n"
	repeat:
	    send raw data in 1024 byte blocks (or rest)
	    receive ACK or NAK
	end-repeat
    end-if
    send OK\r\n or ERR\r\n
    \endverbatim
 */
static int
send_file (int client_idx, char * filename)
{
    char	buffer[TRANSFER_BUFFER_SIZE];
    long	nbytes;
    long	bytes_to_send;
    struct stat	st;
    FILE *	fp;
    int		rtc = OK;

    if (stat (filename, &st) != 0)
    {
	return (ERR);
    }

    fp = fopen (filename, "r");

    if (! fp)
    {
	return (ERR);
    }
    
    if (strstr(filename,"/proc/")!=NULL)
    {
    	char buf[256];
    	nbytes=0;
    	
    	while(fgets(buf,255,fp))
    	{
    	    nbytes+=strlen(buf);	
    	}
    	
    	rewind(fp);
    }
    else
    {
        nbytes = st.st_size;
    }

    sprintf (buffer, "OK %ld\r\n", nbytes);
    write (client[client_idx].fd, buffer, strlen (buffer));

    while (nbytes > 0)
    {
	if (nbytes >= TRANSFER_BUFFER_SIZE)
	{
	    bytes_to_send = TRANSFER_BUFFER_SIZE;
	}
	else
	{
	    bytes_to_send = nbytes;
	}

	/* error handling missing here, later */
	(void) fread (buffer, bytes_to_send, 1, fp);
	(void) write (client[client_idx].fd, buffer, bytes_to_send);
	(void) read (client[client_idx].fd, buffer, 1);

	if (*buffer == NAK_CHAR)
	{
	    rtc = ERR;
	    break;
	}

	nbytes -= bytes_to_send;
    }

    (void) fclose (fp);
    return (rtc);
} /* send_file (client_idx, filename, nbytes) */

/**
 * This function returns the name of the telmond log file.
 *
 * \param client_idx index of client who requested the log file
 * \return the name of the log file
 */
static char *
get_telmond_log_file (int client_idx)
{
    static char	log_file[128];

    if (globals.telmond_log_dir)
    {
	sprintf (log_file, "%s/telmond-%s.log", globals.telmond_log_dir,
		 client[client_idx].ip);

	if (access (log_file, 0) != 0)
	{
	    sprintf (log_file, "%s/telmond.log", globals.telmond_log_dir);
	}

	return (log_file);
    }
    return ((char *) NULL);
} /* get_telmond_log_file () */


/**
 * This function is a small wrapper to the md5 algorithm.
 * It can be used on a string and returns the md5sum
 * in hex digits.
 *
 * \param string string to calculate the md5sum from
 *
 * \return md5sum in hex digits of string
 */
static char *
hexmd5(const char *string) {
	static unsigned char md5buffer[16];
	static char tmp[33];
	size_t i;
	
	md5_buffer(string, strlen(string), md5buffer);
	
	for (i = 0; i < 16; i++)
		sprintf (tmp + (2 * i), "%02x", md5buffer[i]);
	      
	return tmp;
}


/**
 * This function checks the given password against the system password of
 * root.
 *
 * \param arg the password that should be checked
 *
 * \retval -1 pasword entry could not be retrieved from system
 * \retval TRUE password is valid
 * \retval FALSE password is not valid
 */
static int
password_valid (char * arg)
{
    char *	    pass;
    struct  spwd *  sp;

    sp = getspnam ("root");

    if (! sp)
    {
	return (-1);
    }

    pass = sp->sp_pwdp;

    if (pass && *pass)
    {
	char    salt[3];

	salt[2] = '\0';
	strncpy (salt, pass, 2);

	if (! strcmp (pass, crypt (arg, salt)))
	{
	    return (TRUE);
	}
    }
    return (FALSE);
} /* password_valid (arg) */

/**
 * This function is a compare wrapper for bsearch. It finds a user command in
 * an array of command structures.
 *
 * \param a pointer to the value bsearch wants to compare
 * \param b pointer to entry in the array bsearch wants to know if its
 * greater or smaller
 *
 * \return if a is smaller/equal/bigger than b->name
 */
static int cmdcmp(const void *a, const void *b)
{
    return strcmp((const char *) a, ((struct command *) b)->name);
}

/**
 * This function sends help about commands to the user.
 *
 * \param answer buffer to write the answer to the client to
 * \param argc number of arguments
 * \param argv array of argument
 * \param client_idx the client who sent the request
 *
 * \retval OK always
 */
USERCOMMAND(cmd_help)
{
    struct command * cmd;

    if (argc == 1)
    {
	int i, j;
	if ((client[client_idx].password_state & PASSWORD_STATE_ADMIN) != 0)
	{
	    strcat (answer, " Administrative Commands:");

	    j = 0;
	    for (i = 0; i < NUMCOMMANDS; i++)
	    {
		if (commands[i].valid_as & PASSWORD_STATE_ADMIN)
		{
		    if (!(j % HELP_COMMANDSBYLINE))
		    {
			strcat (answer, "\r\n   ");
		    }
		    sprintf (answer + strlen (answer),
			    "%-" HELP_COMMANDWIDTH "s ",
			    commands[i].name);
		    j++;
		}
	    }
	    
	    if ((j - 1) % HELP_COMMANDSBYLINE)
	    {
		strcat (answer, "\r\n ");
	    }
	    strcat (answer, "\r\n");
	}
	else
	{
	    *answer = '\0';
	}

	strcat (answer, " User Commands:");

	j = 0;
	for (i = 0; i < NUMCOMMANDS; i++)
	{
	    if (!(commands[i].valid_as & PASSWORD_STATE_ADMIN))
	    {
		if (!(j % HELP_COMMANDSBYLINE))
		{
		    strcat (answer, "\r\n   ");
		}
		sprintf (answer + strlen (answer),
			"%-" HELP_COMMANDWIDTH "s ",
			commands[i].name);
		j++;
	    }
	}
	    
	if ((j - 1) % HELP_COMMANDSBYLINE)
	{
	    strcat (answer, "\r\n ");
	}
	strcat (answer, "\r\n Type \"help command\" to get more help.");
    }
    else if ((cmd = bsearch (argv[1], commands, NUMCOMMANDS,
		    sizeof (struct command), cmdcmp)))
    {
	/* this emulates acuratly the old help format */
	char temp[100];
	sprintf (temp, "%s %s", cmd->name, cmd->paramhelp);
	sprintf (answer, "    %-24s %s", temp, cmd->help);
#if 0
	/* New format, but only one sprintf and no temp var */
	sprintf (answer, "    %-15s %-14s  %s", cmd->name,
		cmd->paramhelp, cmd->help);
#endif
    }
    else
    {
	sprintf (answer, "    unknown command: %s", argv[1]);
    }

    strcat (answer, "\r\nOK 0");
    return RAW;
} /* cmd_help */

/**
 * This function adjusts the system time by argv[1] seconds. External commands
 * can get the current time from a time server and tell imond to set it, so
 * imond can correct its messurements.
 *
 * \see cmd_help for parameters
 *
 * \retval ERR if setting the time fails
 * \retval OK everything is fine
 */
USERCOMMAND(cmd_adjusttime)
{
    time_t  seconds;
    int delta = atoi (argv[1]);

    seconds = time ((time_t *) NULL) + delta;

    if (stime (&seconds) == 0)
    {
	int channel_idx;
	for (channel_idx = 0; channel_idx < ISDN_MAX_CHANNELS;
	     channel_idx++)
	{
	    if (infos[channel_idx].is_active &&
		infos[channel_idx].online_start != 0)
	    {
		infos[channel_idx].online_start += delta;
	    }
	}

	if (pppoe_infos.online_start != 0)  /* pppoe online?    */
	{
	    pppoe_infos.online_start += delta;
	}

	last_time_measured.tv_sec += delta;
    }
    else
    {
	return ERR;
    }

    return OK;
}

/**
 * This function gets a password from the client or reports the current
 * password state.
 *
 * When the client sends a password, it is checked against the user and admin
 * password. If it matches, the client state is set accordingly. The new state
 * is reported to the client.
 *
 * When the client requests the password state or has successfully entered a
 * password, a value ORed together from the following values is returned:
 * 
 * \li ADMIN_MODE user is in admin mode
 * \li PASS_REQUIRED a user password is required to use imond
 * \li ADMIN_PASS_REQUIRED the admin password is required to use admin commands
 *
 * \todo Remove this function. Replace with cmd_md5pass.
 *
 * \see cmd_help for parameters
 *
 * \retval ERR password incorrect
 * \retval OK password correct, new client state in \a answer
 */
USERCOMMAND(cmd_pass)
{
    int error_occured = FALSE;

    if (argc == 2)
    {
	error_occured = TRUE;

	if (globals.imond_admin_pass && ! strcmp (argv[1], globals.imond_admin_pass))
	{
	    client[client_idx].password_state |=   PASSWORD_STATE_ADMIN |
					    PASSWORD_STATE_NORMAL;
	    error_occured = FALSE;
	}

	if (globals.imond_pass && ! strcmp (argv[1], globals.imond_pass))
	    /* NOT else if! */
	{
	    client[client_idx].password_state |= PASSWORD_STATE_NORMAL;

	    if (! globals.imond_admin_pass)	/* no admin pass necessary  */
	    {
		client[client_idx].password_state |= PASSWORD_STATE_ADMIN;
	    }
	    error_occured = FALSE;
	}
    }

    if (error_occured)
    {
	return ERR;
    }
    
    if (client[client_idx].password_state & PASSWORD_STATE_ADMIN)
    {
	sprintf (answer, "%d", password_mode | ADMIN_MODE);
    }
    else
    {
	sprintf (answer, "%d", password_mode);
    }
    return OK;
} /* cmd_pass */

/**
 * This function sends a random challenge to the client. It does not use
 * cryptographic strong random numbers, but that should be OK for one use...
 *
 * \see cmd_help for parameters
 *
 * \todo Rename? I think I picked the wrong english word (tobig)
 * \todo Return salt of root password to enable secure passwords for file transfer
 *
 * \retval OK always, the challenge is in \a answer
 */
USERCOMMAND(cmd_salt)
{
    FILE * random;
    char buf[1024];
    char salt[33];
    static unsigned char md5buffer[16];
    size_t i;

    random = fopen("/dev/urandom", "r");
    fread (buf, 1024, 1, random);
    fclose (random);
   
    md5_buffer(buf, 1024, md5buffer);

    for (i = 0; i < 16; i++)
	sprintf (salt + (2 * i), "%02x", md5buffer[i]);
    
    strcpy (answer, salt);
    strcpy (client[client_idx].salt, salt);
    return OK;
}

/**
 * This function gets a password from the client in a secure way or reports the
 * current password state.
 *
 * The client must first get a challenge, than concatunate the passwort the
 * user entered and the challenge together and send the md5 digest of this
 * string back to the server. This ensures the password is not transfered in
 * plain text, but is not save against brute force attacks! If you want real
 * security, tunnel imond communication with SSL/TLS or SSH.
 * 
 * When the client sends a password, it is checked against the user and admin
 * password. If it matches, the client state is set accordingly. The new state
 * is reported to the client.
 *
 * When the client requests the password state or has successfully entered a
 * password, a value ORed together from the following values is returned:
 * 
 * \li ADMIN_MODE user is in admin mode
 * \li PASS_REQUIRED a user password is required to use imond
 * \li ADMIN_PASS_REQUIRED the admin password is required to use admin commands
 * 
 * \see cmd_help for parameters
 *
 * \retval ERR password incorrect
 * \retval OK password correct, new client state in \a answer
 */
USERCOMMAND(cmd_md5pass)
{
    int	error_occured = FALSE;
    char tmp[255];
    char * md5passwd;
    
    if (argc == 2)
    {
	error_occured = TRUE;

	if (strlen(client[client_idx].salt) == 0)
	{
	    strcpy (answer, "there was no challenge!");
	    return ERR;
	}

	if (globals.imond_admin_pass)
	{
	    sprintf (tmp, "%s%s", globals.imond_admin_pass,
		    client[client_idx].salt);
	    md5passwd = hexmd5(tmp);
	    if (! strcmp (argv[1], md5passwd))
	    {
		client[client_idx].password_state |= PASSWORD_STATE_ADMIN |
						PASSWORD_STATE_NORMAL;
		error_occured = FALSE;
	    }
	}

	if (globals.imond_pass)
	{
	    sprintf (tmp, "%s%s", globals.imond_pass,
		    client[client_idx].salt);
	    md5passwd = hexmd5(tmp);
	    if (! strcmp (argv[1], md5passwd))
	    {
		client[client_idx].password_state |= PASSWORD_STATE_NORMAL;

		if (! globals.imond_admin_pass)	/* no admin pass necessary  */
		{
		    client[client_idx].password_state |= PASSWORD_STATE_ADMIN;
		}
		error_occured = FALSE;
	    }
	}
    }

    if (error_occured)
    {
	return ERR;
    }
    else if (client[client_idx].password_state & PASSWORD_STATE_ADMIN)
    {
	sprintf (answer, "%d", password_mode | ADMIN_MODE);
	return OK;
    }
    else
    {
	sprintf (answer, "%d", password_mode);
	return OK;
    }
} /* cmd_md5pass */

/**
 * This function closes the client connection gracefully. Well, not really this
 * function, it is handed down to the calling function that the client
 * requested to close the connection.
 *
 * \see cmd_help for parameters
 *
 * \retval QUIT signal the caller the user wants to quit
 */
USERCOMMAND(cmd_quit)
{
    return QUIT;
} /* cmd_quit */

/**
 * This function returns the IP address of the queried channel.
 *
 * \see cmd_help for parameters
 *
 * \retval OK if IP was successfully retrieved, the IP is in \a answer
 * \retval ERR user requested a not existing channel
 */
USERCOMMAND(cmd_ip)
{
    if (! strcmp(argv[1], "pppoe"))
    {
	if (* pppoe_infos.ip)
	{
	    strcpy (answer, pppoe_infos.ip);
	}

	else if (pppoe_infos.online_start != 0)
	{
	    char    ip_file[64];
	    char    ip_buf[32];
	    FILE *  ip_fp;
	    int	    conf_idx;

	    ip_buf[0] = '\0';

	    conf_idx = pppoe_infos.conf_idx;

	    if (conf_idx >= 0)
	    {
		int circuit_idx = conf_data[conf_idx].circuit_idx;

		if (circuit_idx >= 0)
		{
		    sprintf (ip_file, "/var/run/%s.ip",
			     circuits[circuit_idx].device_1);

		    ip_fp = fopen (ip_file, "r");

		    if (ip_fp)
		    {
			if (fgets (ip_buf, 32, ip_fp))
			{
			    char *  p = strchr (ip_buf, '\r');

			    if (! p)
			    {
				p = strchr (ip_buf, '\n');
			    }

			    if (p)	    /* copy only if file complete   */
			    {
				*p = '\0';

				strncpy (pppoe_infos.ip, ip_buf, 16);
			    }
			}

			fclose (ip_fp);
		    }
		}
	    }

	    strcpy (answer, ip_buf);
	}
    }
    else
    {
	int idx = atoi(argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    if (* infos[channel_idx].ip)
	    {
		strcpy (answer, infos[channel_idx].ip);
	    }
	    else if (infos[channel_idx].online_start != 0)
	    {
		char    ip_file[64];
		char    ip_buf[32];
		FILE *  ip_fp;
		int	conf_idx;

		ip_buf[0] = '\0';

		conf_idx = infos[channel_idx].conf_idx;

		if (conf_idx >= 0)
		{
		    int circuit_idx = conf_data[conf_idx].circuit_idx;

		    if (circuit_idx >= 0)
		    {
			sprintf (ip_file, "/var/run/%s.ip",
				 circuits[circuit_idx].device_1);

			ip_fp = fopen (ip_file, "r");

			if (ip_fp)
			{
			    if (fgets (ip_buf, 32, ip_fp))
			    {
				char *  p = strchr (ip_buf, '\r');

				if (! p)
				{
				    p = strchr (ip_buf, '\n');
				}

				if (p)	/* copy only if file complete   */
				{
				    *p = '\0';

				    strncpy (infos[channel_idx].ip,
					    ip_buf, 16);
				}
			    }

			    fclose (ip_fp);
			}
		    }
		}

		strcpy (answer, ip_buf);
	    }
	}
	else
	{
	    return ERR;
	}
    }

    return OK;
} /* cmd_ip */

/**
 * This function sends the protocol version and the imond version to the
 * client. The protocol version is an integer number which can be used by the
 * client to determine which commands are supported. The imond version is a
 * purely informational value and should not be used to make any assumptions
 * about the server. It is intended to be reported to the user.
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_version)
{
    sprintf (answer, "%d %s", PROTOCOL_VERSION, IMOND_VERSION);
    return OK;
}

/**
 * This function tells the client if dialing is enabled.
 *
 * \a answer will contain one of the following values:
 * \li 1 if dialmode is DIALMODE_AUTO or DIALMODE_MANUAL
 * \li 0 if dialmode is DIALMODE_OFF
 * 
 * \retval OK always
 */
USERCOMMAND(cmd_isenabled)
{
    sprintf (answer, "%d", globals.dialmode ? 1 : 0);
    return OK;
}

/**
 * This functions retrieves the fli4l-id of the router. It is read from
 * /var/run/fli4l-id.
 *
 * The fli4l-id is written to \a answer if it could be read.
 *
 * \todo Maybe add a plug-in sytem for these commands?
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_fli4lid)
{
    FILE *  fp;
    int	    fli4l_id = 0;

    fp = fopen ("/var/run/fli4l-id", "r");

    if (fp)
    {
    	(void) fscanf (fp, "%d", &fli4l_id);
    	fclose (fp);
    }

    sprintf (answer, "%d", fli4l_id);
    return OK;
}

/**
 * This function retrieves the highscore entry of the router. It is read from
 * /var/run/highscore.
 *
 * The highscore entry is written to \a answer if it could be read.
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_highscore)
{
    FILE *  fp;
    int	    highscore = 0;

    fp = fopen ("/var/run/highscore", "r");

    if (fp)
    {
	(void) fscanf (fp, "%d", &highscore);
	fclose (fp);
    }

    sprintf (answer, "%d", highscore);
    return OK;
}

/**
 * This function retrieves the hostname of the current host.
 * 
 * The hostname is written to \a answer if it could be read.
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_hostname)
{
    struct utsname  utsbuf;

    if (uname (&utsbuf) == 0)
    {
	sprintf (answer, "%s", utsbuf.nodename);
    }
    return OK;
}

/**
 * This function gets or sets the dialmode.
 * 
 * If called without arguments, writes the curent dialmode to \a answer.
 *
 * If called with one argument, set the dialmode to that dialmode if the user
 * is allowed to do it.
 *
 * \see cmd_help for parameters
 *
 * \retval OK when everything went fine
 * \retval ERR if user is not allowed to set dialmode or new dialmode is not
 *             "off", "auto" or "manual".
 */
USERCOMMAND(cmd_dialmode)
{
    if (argc == 1)
    {
	char * dialmode_string;

	switch (globals.dialmode)
	{
	    case DIALMODE_OFF:	dialmode_string = "off";    break;
	    case DIALMODE_AUTO:	dialmode_string = "auto";   break;
	    default:		dialmode_string = "manual"; break;
	}

	strcpy (answer, dialmode_string);
    }
    else
    {
	int may_enable = ((client[client_idx].password_state &
		    PASSWORD_STATE_ADMIN) != 0) || globals.may_enable;

	if (may_enable)
	{
	    int rtc;
	    if (! strcmp (argv[1], "off"))
	    {
		rtc = set_dialmode (DIALMODE_OFF);
	    }
	    else if (! strcmp (argv[1], "auto"))
	    {
		rtc = set_dialmode (DIALMODE_AUTO);
	    }
	    else if (! strcmp (argv[1], "manual"))
	    {
		rtc = set_dialmode (DIALMODE_MANUAL);
	    }
	    else
	    {
		return ERR;
	    }

	    sprintf (answer, "%d", rtc);
	}
	else
	{
	    return ERR;
	}
    }
    return OK;
}

/**
 * This functions retrieves the number of active ISDN channels. The number is
 * stored in the buffer \a answer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_channels)
{
#ifdef TEST
    strcpy (answer, "2");
#else
    sprintf (answer, "%d", n_active_channels);
#endif
    return OK;
}

/**
 * This function gets a time table for a specifed config. If no config is
 * specified, the LCR time table is retrieved. The time table is stored in the
 * buffer \a answer.
 *
 * \see cmd_help for parameters
 *
 * \retval RAW always, since time table is multi-line
 */
USERCOMMAND(cmd_timetable)
{
    if (argc == 1)
    {
	strcpy (answer, get_timetable (-1));
    }
    else
    {
	int conf_idx = atoi (argv[1]) - 1;

	if (conf_idx < n_conf_lines_used)
	{
	    strcpy (answer, get_timetable (conf_idx));
	}
	else
	{
	    *answer = '\0';
	}
    }

    strcat (answer, "OK");
    return RAW;
}

/**
 * This function retrieves the number of configured (ISDN) circuits. The number
 * is copied to the buffer \a answer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_circuits)
{
    sprintf (answer, "%d", n_circuits);
    return OK;
}

/**
 * This function retrieves the current date of the host. It saves the current
 * time into the buffer \a answer. The format is "weekday DD/MM/YYYY HH:MM:SS".
 *
 * \see cmd_help for parameters, get_weekday for possible values of "weekday"
 * in the string copied to \a answer.
 *
 * \retval OK always
 */
USERCOMMAND(cmd_date)
{
    struct tm *	tm;
    time_t	seconds;
    seconds = time ((time_t *) 0);

    tm = localtime (&seconds);

    sprintf (answer, "%s %02d/%02d/%4d %02d:%02d:%02d",
	     get_weekday(tm->tm_wday), tm->tm_mday, tm->tm_mon + 1,
	     1900 + tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);
    return OK;
}

/**
 * This function gets/sets the current default route.
 *
 * When called without arguments (\a argc == 1), write the index of the
 * currently active route to \a answer and return OK. The index 0 means that
 * LCR is active, higher numbers mean that somebody has selected that specific
 * circuit.
 *
 * When called with one argument (\a argc == 2), it is checked if the current
 * \a client_idx is allowed to issue this command and the argument is valid.
 * When everything is correct, set the route to the new one.
 *
 * \see cmd_help for parameters
 *
 * \retval OK everything went fine
 * \retval ERR something went wrong, error message in \a answer
 */
USERCOMMAND(cmd_route)
{
    if (argc == 1)
    {
	sprintf (answer, "%d", route_idx + 1);	    /* -1 -> 0 : automatic */
	return OK;
    }
    else
    {
	int may_route = ((client[client_idx].password_state &
		    PASSWORD_STATE_ADMIN) != 0) || globals.may_route;

	if (may_route)
	{
	    int circuit_idx = atoi (argv[1]) - 1;
	    int err = OK;

	    if (circuit_idx >= -1 && circuit_idx < n_circuits)
	    {				    /* -1: automatic rtg    */
		if (circuit_idx < 0)
		{
		    route_idx = -1;
#ifdef DEBUG_ROUTES
		    imond_syslog (LOG_INFO, "route lcr\n");
#endif
		}
		else
		{
		    /* OK and ERR are our local playground */
		    err = (set_route (circuit_idx)) ? OK : ERR;
#ifdef DEBUG_ROUTES
		    if (err != ERR)
		    {
			imond_syslog (LOG_INFO, 
				      "route circuit %d (\"%s\")\n", 
				      circuit_idx, 
				      circuits[circuit_idx].device_1);
		    }
#endif
		}

		if (err != ERR)
		{
		    
#ifdef DEBUG_ROUTES
		    imond_syslog (LOG_INFO, "checking routes\n");
		    show_routing (0, 
				  "before check_routing (SET_IF_CHANGED)");
#endif
		    check_routing (SET_IF_CHANGED);
#ifdef DEBUG_ROUTES
		    show_routing (1, 
				  "after check_routing (SET_IF_CHANGED)");
#endif
		    return OK;
		}
		else
		{
		    strcpy (answer, "circuit disabled at this point in time");
		}
	    }
	    else
	    {
		strcpy (answer, "invalid circuit number");
	    }
	}
	else
	{
	    strcpy (answer, "you are not allowed to use this command");
	}
	return ERR;
    }
}

/**
 * This functions returns whether PPPoE is used. Is writes 1 to \a anwer when
 * it is used and 0 if not.
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_pppoe)
{
    sprintf (answer, "%d", pppoe_used ? 1 : 0);
    return OK;
}

/**
 * This function retrieves the current uptime. It reads the current uptime form
 * /proc/uptime and writes it to the buffer \a answer. If less then
 * UPTIME_MEASURE_TIME has passed since the last call of this function,
 * calculate uptime from last value.
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_uptime)
{
    FILE *	    uptime_fp;
    static long     last_uptime_tv_sec;
    static long     last_uptime = 0;
    int 	    TimeDiff;

    TimeDiff=current_time.tv_sec - last_uptime_tv_sec;

    if (TimeDiff > UPTIME_MEASURE_TIME)
    {
	uptime_fp = fopen ("/proc/uptime", "r");

	if (uptime_fp)
	{
	    if (fscanf (uptime_fp, "%ld", &last_uptime) == 1)
	    {
		sprintf(answer, "%ld", last_uptime);
	    }
	    fclose(uptime_fp);
	}

	last_uptime_tv_sec = current_time.tv_sec;
    }
    else				/* not enough time passed   */
    {					/* return last value	    */
	if (current_time.tv_sec < last_uptime_tv_sec)
	{				/* sometimes???		    */
	    last_uptime_tv_sec = current_time.tv_sec;
	}

	sprintf (answer, "%ld", last_uptime + TimeDiff);
    }
    return OK;
}

USERCOMMAND(cmd_system)
{
    sprintf (answer, "%s", systemname);
    return OK;
	
}


/**
 * This function retrieves the current CPU usage. It is calculated from the
 * first line of /proc/stat and written to the buffer \a answer. If less then
 * CPU_MEASURE_TIME has passed since the last call of this function, the last
 * value is returned.
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_cpu)
{
    FILE *	    cpu_fp;
    static long	    percentage;
    static long long int last_busy = -1;
    static long long int last_total = -1;
    static long long int last_cpu_tv_sec;
    long long int        user;
    long long int        nice;
    long long int        system;
    long long int        idle;
    long long int        busy;
    long long int        total;

    if (current_time.tv_sec - last_cpu_tv_sec > CPU_MEASURE_TIME)
    {
	cpu_fp = fopen ("/proc/stat", "r");

	if (cpu_fp)
	{
	    if (fscanf (cpu_fp, "cpu %llu %llu %llu %llu", &user, &nice,
			&system, &idle) == 4)
	    {
		busy = user + nice + system;
		total = busy + idle;

		if (last_total > 0 && (total - last_total) > 0)
		{
		    percentage = (100L * (busy - last_busy)) /
				 (total - last_total);
		    sprintf (answer, "%ld", percentage);
		}
		else
		{
		    strcpy (answer, "0");
		}

		last_busy = busy;
		last_total = total;
	    }
	    fclose (cpu_fp);
	}

	last_cpu_tv_sec = current_time.tv_sec;
    }
    else				/* not enough time passed   */
    {					/* return last value	    */
	if (current_time.tv_sec < last_cpu_tv_sec)  /* sometimes??? */
	{
	    last_cpu_tv_sec = current_time.tv_sec;
	}

	sprintf (answer, "%ld", percentage);
    }

    return OK;
}

/**
 * This function sends the telmond log file to the current client. All lines
 * are prepended with a space and ended with the sequence \\r\\n.
 *
 * \see cmd_help for parameters
 *
 * \retval OK when there was no fatal error
 * \retval QUIT when log file could not be written to the client
 */
USERCOMMAND(cmd_telmondlogfile)
{
    char * log_file = get_telmond_log_file (client_idx);

    if (log_file)
    {
	FILE *	tmp_fp = fopen (log_file, "r");
	char	tmp_buf[512];
	int     len;

	if (tmp_fp)
	{
	    tmp_buf[0] = ' ';

	    while (fgets (tmp_buf + 1, 511, tmp_fp))
	    {
		len = strlen (tmp_buf);
		*(tmp_buf + len - 1) = '\r';
		*(tmp_buf + len)     = '\n';
		*(tmp_buf + len + 1) = '\0';

		len++;

		if (write (client[client_idx].fd, tmp_buf, len) != len)
		{
		    close (client[client_idx].fd);
		    client[client_idx].fd = -1;
		    break;
		    fclose (tmp_fp);
		    return QUIT;
		}
	    }

	    fclose (tmp_fp);
	}
    }

    strcpy (answer, "0");	    /* always return OK	    */
    return OK;
}

/**
 * This function sends the imond log file to the current client. All lines
 * are prepended with a space and ended with the sequence \\r\\n.
 *
 * \see cmd_help for parameters
 *
 * \retval OK when there was no fatal error
 * \retval QUIT when log file could not be written to the client
 */
USERCOMMAND(cmd_imondlogfile)
{
    const char * imond_log_file = get_imond_log_file ();
    if (imond_log_file)
    {
	FILE *	tmp_fp = fopen (imond_log_file, "r");
	char	tmp_buf[512];
	int     len;

	if (tmp_fp)
	{
	    tmp_buf[0] = ' ';

	    while (fgets (tmp_buf + 1, 511, tmp_fp))
	    {
		len = strlen (tmp_buf);
		*(tmp_buf + len - 1) = '\r';
		*(tmp_buf + len)     = '\n';
		*(tmp_buf + len + 1) = '\0';

		len++;

		if (write (client[client_idx].fd, tmp_buf, len) != len)
		{
		    close (client[client_idx].fd);
		    client[client_idx].fd = -1;
		    break;
		    fclose (tmp_fp);
		    return QUIT;
		}
	    }
	    fclose (tmp_fp);
	}
    }

    strcpy (answer, "0");	    /* always return OK	    */
    return OK;
}

/**
 * This function sends the mgetty log file to the current client. All lines
 * are prepended with a space and ended with the sequence \\r\\n.
 *
 * \see cmd_help for parameters
 *
 * \retval OK when there was no fatal error
 * \retval QUIT when log file could not be written to the client
 */
USERCOMMAND(cmd_mgettylogfile)
{
    if (*globals.mgetty_log_file)
    {
	FILE *	tmp_fp = fopen (globals.mgetty_log_file, "r");
	char	tmp_buf[512];
	int     len;

	if (tmp_fp)
	{
	    tmp_buf[0] = ' ';

	    while (fgets (tmp_buf + 1, 511, tmp_fp))
	    {
		len = strlen (tmp_buf);

		if (*(tmp_buf + len - 1) == '\n')
		{
		    *(tmp_buf + len - 1) = '\r';
		    *(tmp_buf + len)     = '\n';
		    *(tmp_buf + len + 1) = '\0';
		}
		else 
		{
		    *(tmp_buf + len)     = '\r';
		    *(tmp_buf + len + 1) = '\n';
		    *(tmp_buf + len + 2) = '\0';
		    len++;
		}

		len++;

		if (write (client[client_idx].fd, tmp_buf, len) != len)
		{
		    close (client[client_idx].fd);
		    client[client_idx].fd = -1;
		    break;
		    fclose (tmp_fp);
		    return QUIT;
		}
	    }
	    fclose (tmp_fp);
	}
    }

    strcpy (answer, "0");	    /* always return OK	    */
    return OK;
}

/**
 * This function gets or sets the client IP.
 *
 * If the function is called with an argument (\a argv == 2), that argument is
 * taken as the new IP address. Note that only a client connected from
 * localhost (127.0.0.1) may change the IP.
 *
 * \see cmd_help for parameters
 *
 * \retval OK everything went right, IP is in \a answer
 * \retval ERR the calling client may not change its IP
 */
USERCOMMAND(cmd_clientip)
{
    if (argc == 2)
    {
	if (! strcmp (client[client_idx].ip, "127.0.0.1"))
	{
	    strncpy (client[client_idx].ip, argv[1], 15);
	    client[client_idx].ip[15] = '\0';
	}
	else
	{
	    strcpy (answer, "you may not change your IP");
	    return ERR;
	}
    }
    strcpy (answer, client[client_idx].ip);
    return OK;
}

/**
 * This functions enables the connection. It does this by setting the dialmode
 * to DIALMODE_AUTO. Before this, it is checked if the user is allowed to to
 * this.
 *
 * \see cmd_help for parameters
 *
 * \retval OK dialmode changed, return value is in \a answer
 * \retval ERR the client is not allowed to change the dialmode
 */
USERCOMMAND(cmd_enable)
{
    int may_enable = globals.may_enable ||
	((client[client_idx].password_state & PASSWORD_STATE_ADMIN) != 0);

    if (may_enable)
    {
	int rtc = set_dialmode (DIALMODE_AUTO);
	sprintf (answer, "%d", rtc);
	return OK;
    } else {
	strcpy (answer, "you are not allowed to enable the connection");
	return ERR;
    }
}

/**
 * This functions disables the connection, i.e. it sets the dialmode to
 * DIALMODE_OFF. Before this, it is checked if the user is allowed to to this.
 *
 * \see cmd_help for parameters
 *
 * \retval OK dialmode changed, return value is in \a answer
 * \retval ERR the client is not allowed to change the dialmode
 */
USERCOMMAND(cmd_disable)
{
    int may_disable = globals.may_enable ||
	((client[client_idx].password_state & PASSWORD_STATE_ADMIN) != 0);

    if (may_disable)
    {
	int rtc = set_dialmode (DIALMODE_OFF);
	sprintf (answer, "%d", rtc);
	return OK;
    } else {
	strcpy (answer, "you are not allowed to disable the connection");
	return ERR;
    }
}

/**
 * This function reboots the computer. Before rebooting, it is checked if the
 * user is allowed to reboot the computer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK reboot triggered, return value is in \a answer
 * \retval ERR the client is not allowed to reboot
 */
USERCOMMAND(cmd_reboot)
{
    int may_reboot = globals.may_reboot ||
	((client[client_idx].password_state & PASSWORD_STATE_ADMIN) != 0);

    if (may_reboot)
    {
	int rtc;

	(void) set_dialmode (DIALMODE_OFF);	/* dialmode off	     */
	(void) do_dial ((char *) NULL, FALSE);	/* hangup conections */

	rtc = my_system ("/sbin/reboot -d 3 &");
	sprintf (answer, "%d", rtc);
	return OK;
    } else {
	strcpy (answer, "you are not allowed to reboot the machine");
	return ERR;
    }
}

/**
 * This function halts the computer. Before halting, it is checked if the
 * user is allowed to halt the computer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK halt triggered, return value is in \a answer
 * \retval ERR the client is not allowed to halt
 */
USERCOMMAND(cmd_halt)
{
    int may_halt = globals.may_reboot ||
	((client[client_idx].password_state & PASSWORD_STATE_ADMIN) != 0);

    if (may_halt)
    {
	int rtc;

	(void) set_dialmode (DIALMODE_OFF);	/* dialmode off	     */
	(void) do_dial ((char *) NULL, FALSE);	/* hangup conections */

	rtc = my_system ("(sleep 3; /sbin/halt) &");
	sprintf (answer, "%d", rtc);
	return OK;
    } else {
	strcpy (answer, "you are not allowed to halt the machine");
	return ERR;
    }
}

/**
 * This function halts the computer and switches its power off. Before
 * poweroffing, it is checked if the user is allowed to poweroff the computer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK poweroff triggered, return value is in \a answer
 * \retval ERR the client is not allowed to poweroff
 */
USERCOMMAND(cmd_poweroff)
{
    int may_poweroff = globals.may_reboot ||
	((client[client_idx].password_state & PASSWORD_STATE_ADMIN) != 0);

    if (may_poweroff)
    {
	int rtc;

	(void) set_dialmode (DIALMODE_OFF);	/* dialmode off	     */
	(void) do_dial ((char *) NULL, FALSE);	/* hangup conections */

	rtc = my_system ("(sleep 3; /sbin/poweroff) &");
	sprintf (answer, "%d", rtc);
	return OK;
    } else {
	strcpy (answer, "you are not allowed to poweroff the machine");
	return ERR;
    }
}

/**
 * This function triggers dialing. Before dialing, it is checked if the user is
 * allowed to dial.
 *
 * If called without arguments (\a argc == 1), the default route is dialed. If
 * called with one argument (\a argv == 2), the specifed route is dialed.
 *
 * \see cmd_help for parameters
 *
 * \retval OK dialing triggered, return value is in \a answer
 * \retval ERR the client is not allowed to dial
 */
USERCOMMAND(cmd_dial)
{
    int may_dial = globals.may_dial ||
	((client[client_idx].password_state & PASSWORD_STATE_ADMIN) != 0);

    if (may_dial)
    {
	int rtc;
	if (argc == 1)
	{
	    rtc = do_dial ((char *) NULL, TRUE);
	}
	else
	{
	    rtc = do_dial (argv[1], TRUE);
	}
	sprintf (answer, "%d", rtc);
	return OK;
    } else {
	strcpy (answer, "you are not allowed to dial the connection");
	return ERR;
    }
}

/**
 * This function hangs up a connection. Before hanging up, it is checked if the
 * user is allowed to hang up.
 *
 * If called without arguments (\a argc == 1), the default route is hung up. If
 * called with one argument (\a argv == 2), the specifed route is hung up.
 *
 * \see cmd_help for parameters
 *
 * \retval OK dialing triggered, return value is in \a answer
 * \retval ERR the client is not allowed to dial
 */
USERCOMMAND(cmd_hangup)
{
    int may_hangup = globals.may_dial ||
	((client[client_idx].password_state & PASSWORD_STATE_ADMIN) != 0);

    if (may_hangup)
    {
	int rtc;
	if (argc == 1)
	{
	    rtc = do_dial ((char *) NULL, FALSE);
	}
	else
	{
	    rtc = do_dial (argv[1], FALSE);
	}
	sprintf (answer, "%d", rtc);
	return OK;
    } else {
	strcpy (answer, "you are not allowed to hangup the connection");
	return ERR;
    }
}

/**
 * This function flushes all caches to discs. See the man page sync(3). A child
 * process is spawned to prevent lockups.
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_sync)
{
    if (fork () == 0)
    {
	/* Execute sync call as child process to prevent lockups */
	sync ();
	exit (0);
    }
    return OK;
}

/**
 * This function empties the telmond log file. Since every IP can have its own
 * telmond log file, this function acts on the clients log file if it is
 * present, on the global log file otherwise.
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_resettelmondlogfile)
{
    char * log_file = get_telmond_log_file (client_idx);

    if (log_file && access (log_file, 0) == 0)
    {
	FILE * tmp_fp = fopen (log_file, "w");

	if (tmp_fp)
	{
	    fclose (tmp_fp);
	}
    }
    strcpy (answer, "0");	    /* always return OK	    */
    return OK;
}

/**
 * This function empties the imond log file.
 *
 * \see cmd_help for parameters
 *
 * \retval OK always
 */
USERCOMMAND(cmd_resetimondlogfile)
{
    imond_reset_log ();
    strcpy (answer, "0");	    /* always return OK	    */
    return OK;
}

/**
 * This function retrieves the driver id of the specified channel. The driver
 * id is read from the isdninfo device for ISDN and reported as "PPPoE" for
 * PPPoE always.
 *
 * \todo Remove this? Is this information used? Does anybody care?
 *
 * \see cmd_help for parameters
 *
 * \retval OK could retrieve driver id, get it from \a answer
 * \retval ERR could not retrieve driver id
 */
USERCOMMAND(cmd_driverid)
{
    if (! strcmp (argv[1], "pppoe"))
    {
	if (pppoe_used)
	{
	    strcpy (answer, "PPPoE");
	}
	else
	{
	    strcpy (answer, "no pppoe available");
	    return ERR;
	}
    }
    else
    {
	int idx = atoi (argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    strcpy (answer, infos[channel_idx].drvid);
	}
	else
	{
	    return ERR;
	}
    }
    return OK;
}

/**
 * This functions retrieves the status of the specified channel. The status is copied to \a answer and can be one of "Exclusive", "Offline", "Calling" and "Online".
 *
 * \see cmd_help for parameters
 *
 * \retval OK status could be retrieved
 * \retval ERR if channel could not be found
 */
USERCOMMAND(cmd_status)
{
    if (! strcmp (argv[1], "pppoe"))
    {
	if (pppoe_used)
	{
	    strcpy (answer, pppoe_infos.status);
	}
	else
	{
	    strcpy (answer, "no pppoe available");
	    return ERR;
	}
    }
    else
    {
	int idx = atoi (argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    strcpy (answer, infos[channel_idx].status);
	}
	else
	{
	    return ERR;
	}
    }

    return OK;
}

/**
 * This function retrieves the phone number / name of the connected circuit of
 * the specified channel.
 *
 * \see cmd_help for parameters
 *
 * \retval OK information could be retireved and was saved in \a answer
 * \retval ERR something went wrong
 */
USERCOMMAND(cmd_phone)
{
    if (! strcmp (argv[1], "pppoe"))
    {
	if (pppoe_used)
	{
	    int	conf_idx    = pppoe_infos.conf_idx;
	    int	circuit_idx = (conf_idx >= 0 ? conf_data[conf_idx].circuit_idx : -1);

	    if (circuit_idx >= 0)
	    {
		strcpy (answer, circuits[circuit_idx].name);
	    }
	    else
	    {
		strcpy (answer, "<unknown>");
	    }
	}
	else
	{
	    strcpy (answer, "no pppoe available");
	    return ERR;
	}
    }
    else
    {
	int idx = atoi (argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    char *  phone_info = infos[channel_idx].phone;

	    if (infos[channel_idx].online_start != 0 &&
		    infos[channel_idx].conf_idx >= 0)
	    {		/* we are online and found a conf_data line */
		int conf_idx = infos[channel_idx].conf_idx;

		strncpy (phone_info,
			 get_circuit_name_by_conf_idx (conf_idx), 31);
		*(phone_info + 31) = '\0';
	    }

	    strcpy (answer, phone_info);
	}
	else
	{
	    return ERR;
	}
    }
    return OK;
}

/**
 * This function retrieves the time the specifed channel has been online this
 * time. The online time is written to \a answer in the format "HH:MM:SS".
 *
 * \see cmd_help for parameters
 *
 * \retval OK if online time could be retrieved
 * \retval ERR if specifed channel did not exist
 */
USERCOMMAND(cmd_onlinetime)
{
    if (! strcmp (argv[1], "pppoe"))
    {
	if (pppoe_used)
	{
	    int	    seconds;
	    int	    minutes;
	    int	    hours;

	    seconds = (pppoe_infos.online_start > 0) ? 
		time ((time_t *) NULL) - pppoe_infos.online_start :
		0;

	    hours = seconds / 3600;
	    seconds -= (hours * 3600);

	    minutes = seconds / 60;
	    seconds -= (minutes * 60);

	    sprintf (answer, "%02d:%02d:%02d",
		     hours, minutes, seconds);
	}
	else
	{
	    strcpy (answer, "no pppoe available");
	    return ERR;
	}
    }
    else
    {
	int idx = atoi (argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    int	seconds;
	    int	minutes;
	    int	hours;

	    if (infos[channel_idx].online_start > 0)
	    {
		seconds = time ((time_t *) NULL) - infos[channel_idx].online_start;
	    }
	    else
	    {
		seconds = 0;
	    }

	    hours = seconds / 3600;
	    seconds -= (hours * 3600);

	    minutes = seconds / 60;
	    seconds -= (minutes * 60);

	    sprintf (answer, "%02d:%02d:%02d",
		     hours, minutes, seconds);
	}
	else
	{
	    return ERR;
	}

    }
    return OK;
}

/**
 * This function retrieves the time the specifed channel has been online (sum
 * of all connections). The online time is written to \a answer in the format
 * "HH:MM:SS".
 *
 * \see cmd_help for parameters
 *
 * \retval OK if online time could be retrieved
 * \retval ERR if specifed channel did not exist
 */
USERCOMMAND(cmd_time)
{
    if (! strcmp (argv[1], "pppoe"))
    {
	if (pppoe_used)
	{
	    int	seconds = (int) pppoe_infos.online_time;
	    int	minutes;
	    int	hours;

	    if (pppoe_infos.online_start > 0)
	    {
		seconds += time ((time_t *) NULL) - pppoe_infos.online_start;
	    }

	    hours = seconds / 3600;
	    seconds -= (hours * 3600);

	    minutes = seconds / 60;
	    seconds -= (minutes * 60);

	    sprintf (answer, "%02d:%02d:%02d",
		     hours, minutes, seconds);
	}
	else
	{
	    strcpy (answer, "no pppoe available");
	    return ERR;
	}

    }
    else
    {
	int idx = atoi (argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    int	seconds = (int) infos[channel_idx].online_time;
	    int	minutes;
	    int	hours;

	    if (infos[channel_idx].online_start > 0)
	    {
		seconds += time ((time_t *) NULL) - infos[channel_idx].online_start;
	    }

	    hours = seconds / 3600;
	    seconds -= (hours * 3600);

	    minutes = seconds / 60;
	    seconds -= (minutes * 60);

	    sprintf (answer, "%02d:%02d:%02d",
		     hours, minutes, seconds);
	}
	else
	{
	    return ERR;
	}

    }
    return OK;
}

/**
 * This function retrieves the time that will be actually charged when the
 * current channel is hung up right now. The time is written to \a answer in
 * the format "HH:MM:SS".
 *
 * \see cmd_help for parameters
 *
 * \retval OK if time could be retrieved
 * \retval ERR if specifed channel did not exist
 */
USERCOMMAND(cmd_chargetime)
{
    if (! strcmp (argv[1], "pppoe"))
    {
	if (pppoe_used)
	{
	    int	seconds = (int) pppoe_infos.charge_time;
	    int	minutes;
	    int	hours;

	    if (pppoe_infos.online_start > 0)
	    {
		int conf_idx	= pppoe_infos.conf_idx;
		int charge_int	= (conf_idx >= 0 ? conf_data[conf_idx].charge_int : 1);
		int diff	= time ((time_t *) NULL) -
				    pppoe_infos.online_start;

		if (charge_int > 1)
		{
		    diff = (diff / charge_int) * charge_int +
			    charge_int;
		}

		seconds += diff;
	    }

	    hours = seconds / 3600;
	    seconds -= (hours * 3600);

	    minutes = seconds / 60;
	    seconds -= (minutes * 60);

	    sprintf (answer, "%02d:%02d:%02d",
		     hours, minutes, seconds);
	}
	else
	{
	    strcpy (answer, "no pppoe available");
	    return ERR;
	}
    }
    else
    {
	int idx = atoi (argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    int	    seconds = (int) infos[channel_idx].charge_time;
	    int	    minutes;
	    int	    hours;

	    if (infos[channel_idx].is_outgoing)
	    {
		if (infos[channel_idx].online_start > 0)
		{
		    int conf_idx    = infos[channel_idx].conf_idx;
		    int charge_int  = (conf_idx >= 0 ? conf_data[conf_idx].charge_int : 1);
		    int diff	    = time ((time_t *) NULL) -
			infos[channel_idx].online_start;

		    if (charge_int > 1)
		    {
			diff = (diff / charge_int) * charge_int +
				charge_int;
		    }

		    seconds += diff;
		}
	    }

	    hours = seconds / 3600;
	    seconds -= (hours * 3600);

	    minutes = seconds / 60;
	    seconds -= (minutes * 60);

	    sprintf (answer, "%02d:%02d:%02d",
		     hours, minutes, seconds);
	}
	else
	{
	    return ERR;
	}

    }
    return OK;
}

/**
 * This function retrieves what will be actually charged when the current
 * channel is hung up right now. The charge is writte to \a answer with two
 * digits behind the decimal point.
 *
 * \see cmd_help for parameters
 *
 * \retval OK if charge could be retrieved
 * \retval ERR if specifed channel did not exist
 */
USERCOMMAND(cmd_charge)
{
    if (! strcmp (argv[1], "pppoe"))
    {
	if (pppoe_used)
	{
	    float   charge  = pppoe_infos.charge;

	    if (pppoe_infos.online_start > 0)
	    {
		int conf_idx	= pppoe_infos.conf_idx;
		int charge_int	= (conf_idx >= 0 ? conf_data[conf_idx].charge_int : 1);
		int diff	= time ((time_t *) NULL) -
				    pppoe_infos.online_start;

		if (charge_int > 1)
		{
		    diff = (diff / charge_int) * charge_int +
			    charge_int;
		}

		if (conf_idx >= 0)
		{
		    charge += (diff * conf_data[conf_idx].charge_per_minute) / 60;
		}
	    }

	    sprintf (answer, "%0.2f", charge);
	}
	else
	{
	    strcpy (answer, "no pppoe available");
	    return ERR;
	}
    }
    else
    {
	int idx = atoi (argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    float   charge  = infos[channel_idx].charge;

	    if (infos[channel_idx].is_outgoing)
	    {
		if (infos[channel_idx].online_start > 0)
		{
		    int conf_idx    = infos[channel_idx].conf_idx;
		    int charge_int  = (conf_idx >= 0 ? conf_data[conf_idx].charge_int : 1);
		    int diff	    = time ((time_t *) NULL) -
			infos[channel_idx].online_start;

		    if (charge_int > 1)
		    {
			diff = (diff / charge_int) * charge_int +
				charge_int;
		    }

		    if (conf_idx >= 0)
		    {
			charge += (diff * conf_data[conf_idx].charge_per_minute) / 60;
		    }
		}
	    }

	    sprintf (answer, "%0.2f", charge);
	}
	else
	{
	    return ERR;
	}

    }
    return OK;
}

/**
 * This function retrieves the usage of the specified channel. The usage is
 * saved in \a answer and will be "Net" most of the time.
 *
 * ISDN can have some other values here.
 *
 * \see cmd_help for parameters
 *
 * \retval OK if usage could be retrieved
 * \retval ERR if specified channel did not exist
 */
USERCOMMAND(cmd_usage)
{
    if (! strcmp (argv[1], "pppoe"))
    {
	if (pppoe_used)
	{
	    strcpy (answer, "Net");
	}
	else
	{
	    strcpy (answer, "no pppoe available");
	    return ERR;
	}
    }
    else
    {
	int idx = atoi (argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    strcpy (answer, infos[channel_idx].usage);
	}
	else
	{
	    return ERR;
	}

    }
    return OK;
}

/**
 * This function retrieves the connection direction of the specified channel.
 * If the connnection is initiated by the computer this program is running on,
 * "Outgoing" is copied to \a answer, if the connection was coming from another
 * party, "Incoming" is copied to \a answer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK if direction could be retrieved
 * \retval ERR if specified channel did not exist
 */
USERCOMMAND(cmd_inout)
{
    if (! strcmp (argv[1], "pppoe"))
    {
	if (pppoe_used)
	{
	    strcpy (answer, "Outgoing");
	}
	else
	{
	    strcpy (answer, "no pppoe available");
	    return ERR;
	}
    }
    else
    {
	int idx = atoi (argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    strcpy (answer, infos[channel_idx].inout);
	}
	else
	{
	    return ERR;
	}
    }
    return OK;
}

/**
 * This function retrieves the name of the specified circuit. The circuit name
 * is copied to \a answer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK if circuit name could be retrieved
 * \retval ERR if specified circuit did not exist
 */
USERCOMMAND(cmd_circuit)
{
    int idx = atoi (argv[1]);
    int circuit_idx = idx - 1;

    if (circuit_idx >= 0 && circuit_idx < n_circuits)
    {
	strcpy (answer, circuits[circuit_idx].name);
    }
    else
    {
	return ERR;
    }

    return OK;
}

/**
 * This function retrieves the device associated with a specified circuit. The
 * device name is placed in \a answer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK if device of circuit could be retrieved
 * \retval ERR if specified circuit did not exist
 */
USERCOMMAND(cmd_device)
{
    int circuit_idx = atoi (argv[1]) - 1;

    if (circuit_idx >= 0 && circuit_idx < n_circuits)
    {
	strcpy (answer, circuits[circuit_idx].device_1);
    }
    else
    {
	return ERR;
    }

    return OK;
}

/**
 * This function retrieves the number of links a connected circuit uses. This
 * is mostly interesting with ISDN since channel bundling is commonly used
 * there. The number of links is placed in \a answer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK if number of links could be retrieved
 * \retval ERR if specified circuit did not exist
 */
USERCOMMAND(cmd_links)
{
    int circuit_idx = atoi (argv[1]) - 1;

    if (circuit_idx < 0)		/* use automatic circuit    */
    {
	circuit_idx = find_circuit ();		/* get circuit_idx  */
    }

    if (circuit_idx >= 0 && circuit_idx < n_circuits)
    {
	int channel_idx = find_channel_idx_by_circuit_idx (circuit_idx);

	if (channel_idx >= 0 && infos[channel_idx].online_start != 0)
	{
	    if (*(circuits[circuit_idx].device_2))
	    {
		if (circuits[circuit_idx].link_added == 0)
		{
		    strcpy (answer, "1");    /* 1 ch used    */
		}
		else
		{
		    strcpy (answer, "2");    /* 2 chs used   */
		}
	    }
	    else
	    {					/* only 1 channel   */
		strcpy (answer, "0");		/* available	    */
	    }
	}
	else					/* offline	    */
	{					/* only 1 channel   */
	    strcpy (answer, "0");		/* available	    */
	}
    }
    else
    {
	return ERR;
    }

    return OK;
}

/**
 * This function retrieves the log directory of telmond or mgetty. The first
 * argument determines which logfile to retrieve ("telmond" or "mgetty"). The
 * log directory is saved in \a answer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK log directory could be retrieved
 * \retval ERR if argument was invalid
 */
USERCOMMAND(cmd_logdir)
{
    char * directory = (char *) NULL;

    if (! strcmp (argv[1], "telmond"))
    {
	directory = globals.telmond_log_dir;
    }
    else if (! strcmp (argv[1], "mgetty"))
    {
	directory = globals.mgetty_log_dir;
    }
    else if (! strcmp (argv[1], "imond"))
    {
    directory = globals.imond_log_dir;	
    }

    if (! directory)
    {
	return ERR;
    }

    strcpy (answer, directory);
    return OK;
}

/**
 * This function checks if the current \a client_idx may use a specified
 * function. If the user is allowed to use it, "1" is written to \a answer, "0"
 * if he is not allowed to use it. The function to check is stored in \a
 * argv[1], possible values are:
 *
 * \li "imond-log" imond log file is availible
 * \li "telmond-log" telmond log file is availible
 * \li "mgetty-log" mgetty log file is availible
 * \li "dial" / "hangup" current \a client_idx may dial/hang up the connection
 * \li "dialmode" current \a client_idx may change the dialmode
 * \li "route" current \a client_idx may change the default route
 * \li "reboot" / "halt" current \a client_idx may reboot / halt the computer
 *
 * \see cmd_help for parameters
 *
 * \retval OK the value in \a argv[1] was valid
 * \retval ERR the argument was invalid
 */
USERCOMMAND(cmd_isallowed)
{
    int may_do = FALSE;

    if (! strcmp (argv[1], "imond-log"))
    {
	may_do = get_imond_log_file () != NULL;
    }
    else if (! strcmp (argv[1], "telmond-log"))
    {
	may_do = ((int) globals.telmond_log_dir);
    }
    else if (! strcmp (argv[1], "mgetty-log"))
    {
	may_do = (*globals.mgetty_log_file);
    }
    else if (! strcmp (argv[1], "dial") || ! strcmp (argv[1], "hangup"))
    {
	may_do = ((client[client_idx].password_state &
		    PASSWORD_STATE_ADMIN) != 0) || globals.may_dial;
    }
    else if (! strcmp (argv[1], "dialmode"))
    {
	may_do = ((client[client_idx].password_state &
		    PASSWORD_STATE_ADMIN) != 0) || globals.may_enable;
    }
    else if (! strcmp (argv[1], "route"))
    {
	may_do = ((client[client_idx].password_state &
		    PASSWORD_STATE_ADMIN) != 0) || globals.may_route;
    }
    else if (! strcmp (argv[1], "reboot") || ! strcmp (argv[1], "halt"))
    {
	may_do = ((client[client_idx].password_state &
		    PASSWORD_STATE_ADMIN) != 0) || globals.may_reboot;
    }
    else
    {
	return ERR;
    }

    sprintf (answer, "%d", (may_do) ? 1 : 0);
    return OK;
}

/**
 * This function retrieves support informations. The informations are generated
 * by the shell script /usr/local/bin/support.sh and placed into the file
 * /tmp/support.txt. imond reads this file and sends it to the client.
 *
 * The client must supply the root password.
 *
 * \see cmd_help for parameters
 *
 * \retval OK password was valid, support infos were transfered to client or
 * error message is in \a answer.
 * \retval ERR password was not valid
 */
USERCOMMAND(cmd_support)
{
    int	pw_valid = password_valid (argv[1]);

    if (pw_valid)
    {
	char	tmp_buf[512];
	FILE *	fp;
	char *	tmp_p;

	system ("/usr/local/bin/support.sh");

	fp = fopen ("/tmp/support.txt", "r");

	if (! fp)
	{
	    strcpy (answer, "cannot get support file");
	    return OK;
	}

	tmp_buf[0] = ' ';		    /* one leading space    */
	tmp_p = tmp_buf + 1;

	while (fgets (tmp_p, 500, fp))
	{
	    int len = strlen (tmp_buf);
	    *(tmp_buf + len - 1) = '\r';
	    *(tmp_buf + len)     = '\n';
	    *(tmp_buf + len + 1) = '\0';

	    len++;

	    (void) write (client[client_idx].fd, tmp_buf, len);
	}

	fclose (fp);
	unlink ("/tmp/support.txt");
	return OK;
    }
    else if (pw_valid == -1)
    {
	strcpy (answer, "cannot get passwd entry for root");
    }
    else
    {
	strcpy (answer, "password incorrect");
    }
    return ERR;
}

/**
 * This function parses the command to initiate file receives. The client must
 * supply the filename (\a argv[1]), number of bytes to transfer (\a argv[2])
 * and the valid root password (\a argv[3]). When the password is correct, the
 * function receive_file is called to do the actual transfer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK file receive succeeded
 * \retval ERR password was not valid
 */
USERCOMMAND(cmd_receive)
{
    long    nbytes;
    int	    pw_valid;

    /* argv[1] is filename, argv[2] number of bytes, argv[3] root password */
    nbytes = atol (argv[2]);
    if (nbytes < 1)
    {
	write (client[client_idx].fd, NAK_STRING, 1);
	return ERR;
    }

    pw_valid = password_valid (argv[3]);

    if (pw_valid)
    {
	return (receive_file (client_idx, argv[1], nbytes));
    }
    else if (pw_valid == -1)
    {
	write (client[client_idx].fd, NAK_STRING, 1);
	strcpy (answer, "cannot get passwd entry for root");
    }
    else
    {
	write (client[client_idx].fd, NAK_STRING, 1);
	strcpy (answer, "password invalid");
    }
    return ERR;
}

/**
 * This function parses the command to initiate file sends. The client must
 * supply the filename (\a argv[1]) and the valid root password (\a argv[2]).
 * When the password is correct, the function send_file is called to do the
 * actual transfer.
 *
 * \see cmd_help for parameters
 *
 * \retval OK file send succeeded
 * \retval ERR password was not valid
 */
USERCOMMAND(cmd_send)
{
    int	    pw_valid;

    /* argv[1] is filename, argv[2] is root password */
    pw_valid = password_valid (argv[2]);

    if (pw_valid)
    {
	return (send_file (client_idx, argv[1]));
    }
    else if (pw_valid == -1)
    {
	strcpy (answer, "cannot get passwd entry for root");
    }
    else
    {
	strcpy (answer, "password invalid");
    }
    return ERR;
}

/**
 * This function removes the specified file. The client must supply the
 * filename (\a argv[1]) and the valid root password (\a argv[2]).  When the
 * password is correct, unlink(2) is called to remove the file.
 *
 * \see cmd_help for parameters
 *
 * \retval OK file delete succeeded
 * \retval ERR password was not valid or file could not be deleted
 */

USERCOMMAND(cmd_delete)
{
    int	    pw_valid;

    /* argv[1] is filename, argv[2] is root password */
    pw_valid = password_valid (argv[2]);

    if (pw_valid)
    {
	return (unlink (argv[1]));
    }
    else if (pw_valid == -1)
    {
	strcpy (answer, "cannot get passwd entry for root");
    }
    else
    {
	strcpy (answer, "password invalid");
    }
    return ERR;
}

/**
 * This functions retrieves or sets the hang up timeout of the specified
 * circuit.
 *
 * If the client specified a new hang up timeout in \a argv[2], it is
 * tried to set the new timeout. The only way to see if this has failed is to
 * compare the returned value with the old value.
 *
 * The current / new hang up timeout is stored in \a answer when the circuit
 * existed.
 *
 * \see cmd_help for parameters
 *
 * \retval OK current / new hang up timeout is in \a answer
 * \retval ERR hang up timeout of circuit can not be changed or specified
 * circuit does not exist.
 */
USERCOMMAND(cmd_huptimeout)
{
    int circuit_idx = atoi (argv[1]) - 1;
    int value = (argc > 2) ? atoi (argv[2]) : -1;

    if (circuit_idx >= 0 && circuit_idx < n_circuits)
    {
	if (value >= 0)		    /* 2nd argument ...		    */
	{
	    if (strcmp (circuits[circuit_idx].device_1, "pppoe") != 0)
	    {			    /* no timeout setting on pppoe  */
		char tmp_buf[128];
		int rtc;

		sprintf (tmp_buf, "/sbin/isdnctrl huptimeout %s %d",
			 circuits[circuit_idx].device_1, value);

		rtc = my_system (tmp_buf);

		if (rtc == 0 && *(circuits[circuit_idx].device_2))
		{
		    sprintf (tmp_buf, "/sbin/isdnctrl huptimeout %s %d",
			     circuits[circuit_idx].device_2, value);

		    rtc = my_system (tmp_buf);
		}

		if (rtc == 0)
		{
		    circuits[circuit_idx].hup_timeout = value;
		}

		sprintf (answer, "%d", circuits[circuit_idx].hup_timeout);
	    }
	    else
	    {
		return ERR;
	    }
	}
	else			    /* no 2nd arg, send current val */
	{
	    sprintf (answer, "%d", circuits[circuit_idx].hup_timeout);
	}
    }
    else
    {
	return ERR;
    }

    return OK;
}

/**
 * This function adds a link to a connected circuit. This obviously only works
 * on circuits that support channel bundling (ISDN). The new number of channels
 * is stored in \a answer. So this is 0 if the channel did not support channel
 * bundling and 2 if the bundling was succesful.
 *
 * \see cmd_help for parameters
 *
 * \retval ERR circuit does not exist
 * \retval OK otherwise
 */
USERCOMMAND(cmd_addlink)
{
    int circuit_idx = atoi (argv[1]) - 1;

    if (circuit_idx < 0)			/* use automatic circuit    */
    {
	circuit_idx = find_circuit ();			/* get circuit_idx  */
#ifdef TEST
	printf ("found circuit %d (%s) for channel-bundling\n",
		circuit_idx, circuit_idx >= 0 ?
		circuits[circuit_idx].name : "NOTHING");
#endif
    }

    if (circuit_idx >= 0 && circuit_idx < n_circuits)
    {
	int rtc = 0;		/* 0: no channel bundling   */
	int channel_idx = find_channel_idx_by_circuit_idx (circuit_idx);

	if (channel_idx >= 0 && infos[channel_idx].online_start != 0)
	{					/* only if online   */
	    if (*(circuits[circuit_idx].device_2) &&
		circuits[circuit_idx].link_added == 0)
	    {
		rtc = addlink (circuit_idx);

		/* return remaining links or 0: no channel bundling */
		rtc = (rtc == 0) ? 2 : 0;
	    }
	}
	sprintf (answer, "%d", rtc);
    }
    else
    {
	return ERR;
    }

    return OK;
}

/**
 * This function removes a link from a connected bundled circuit. This obviously only works
 * on circuits that support channel bundling (ISDN). The new number of channels
 * is stored in \a answer. So this is 0 if the channel did not support channel
 * bundling and 1 if the removing of the link was successful.
 *
 * \see cmd_help for parameters
 *
 * \retval ERR circuit does not exist
 * \retval OK otherwise
 */
USERCOMMAND(cmd_removelink)
{
    int circuit_idx = atoi (argv[1]) - 1;

    if (circuit_idx < 0)			/* use automatic circuit    */
    {
	circuit_idx = find_circuit ();			/* get circuit_idx  */
#ifdef TEST
	printf ("found circuit %d (%s) for channel-bundling\n",
		circuit_idx, circuit_idx >= 0 ?
		circuits[circuit_idx].name : "NOTHING");
#endif
    }

    if (circuit_idx >= 0 && circuit_idx < n_circuits)
    {
	int rtc = 0;		/* 0: no channel bundling   */
	int channel_idx = find_channel_idx_by_circuit_idx (circuit_idx);

	if (channel_idx >= 0 && infos[channel_idx].online_start != 0)
	{					/* only if online   */
	    if (*(circuits[circuit_idx].device_2) &&
		     circuits[circuit_idx].link_added == 1)
	    {
		rtc = removelink (circuit_idx);

		/* return remaining links or 0: no channel bundling */
		rtc = (rtc == 0) ? 1 : 0;
	    }
	}
	sprintf (answer, "%d", rtc);
    }
    else
    {
	return ERR;
    }

    return OK;
}

/**
 * This function returns the quantity of transferred data on the specified
 * channel. The quantity is stored in \a answer in the format "\<IN_OVERFLOWS\>
 * \<IN\> \<OUT_OVERFLOWS\> \<OUT\>", where he actual transferred data can be
 * calculating by multiplying the \<IN/OUT_OVERFLOWS\> with 4GB (2^32) and adding
 * \<IN/OUT\>. This is done to avoid usage of 64-bit-integers in the server.
 *
 * \see cmd_help for parameters
 *
 * \retval OK statistics could be retrieved
 * \retval ERR channel does not exist
 */
USERCOMMAND(cmd_quantity)
{
    unsigned long ibytes = 0, obytes = 0;
    unsigned long ibytes_overflows = 0, obytes_overflows = 0;

    if (! strcmp (argv[1], "pppoe"))				/* PPPoE    */
    {
	if (pppoe_used)
	{
	    if (pppoe_infos.online_start)
	    {
		ibytes = pppoe_infos.ibytes - pppoe_infos.ibytes_start;
		ibytes_overflows = pppoe_infos.ibytes_overflows;
		if (pppoe_infos.ibytes <= pppoe_infos.ibytes_start)
		{
		    ibytes_overflows--;
		}
		
		obytes = pppoe_infos.obytes - pppoe_infos.obytes_start;
		obytes_overflows = pppoe_infos.obytes_overflows;
		if (pppoe_infos.obytes <= pppoe_infos.obytes_start)
		{
		    obytes_overflows--;
		}
	    }
	}
    	else
	{
	    strcpy (answer, "no pppoe available");
	    return ERR;
	}
    }
    else
    {
	int idx = atoi (argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    ibytes = infos[channel_idx].ibytes - 0;
	    ibytes_overflows = infos[channel_idx].ibytes_overflows;
	    if (infos[channel_idx].ibytes <= 0)
	    {
		ibytes_overflows--;
	    }
	    
	    obytes = infos[channel_idx].obytes - 0;
	    obytes_overflows = infos[channel_idx].obytes_overflows;
	    if (infos[channel_idx].obytes <= 0)
	    {
		obytes_overflows--;
	    }
	}
	else
	{
	    return ERR;
	}
    }
    sprintf (answer, "%lu %lu %lu %lu",
	     ibytes_overflows, ibytes, obytes_overflows, obytes);

    return OK;
} /* cmd_quantity */

/**
 * This function retrieves te current transfer rate of the specified channel.
 * The rate is stored in \a answer in the format "<IN> <OUT>". The numbers are
 * in Bytes/s.
 *
 * \see cmd_help for parameters
 *
 * \retval OK rate could be retrieved
 * \retval ERR channel does not exist
 */
USERCOMMAND(cmd_rate)
{
    if (! strcmp(argv[1], "pppoe"))
    {
	if (pppoe_used)
	{
	    sprintf (answer, "%d %d",
		     pppoe_infos.ibytes_per_second,
		     pppoe_infos.obytes_per_second);
	}
	else
	{
	    strcpy (answer, "no pppoe available");
	    return ERR;
	}
    }
    else
    {
	int idx = atoi (argv[1]);
	int channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    sprintf (answer, "%d %d",
		     infos[channel_idx].ibytes_per_second,
		     infos[channel_idx].obytes_per_second);
	}
	else
	{
	    return ERR;
	}

    }
    return OK;
} /* cmd_rate */

/**
 * This function parses an user command, calles the associated function and
 * generates the output.
 *
 * \param client_idx index of the client who sent the command
 * \param buf buffer containing the command sent by the user
 *
 * \return A pointer to a static buffer containing the answer to send to the
 * client or NULL if the connection shall be closed.
 */
char *
parse_command (int client_idx, char * buf)
{
    static char	answer[4096];
    char *  answerp = answer + 5;
    char *  argv[MAX_ARGUMENTS];
    int     argc = 0;
    char *  p = buf;
    struct command * cmd;
    int	    rtc;
    static char errstr[] = "ERR ";
    static char okstr[] = "OK ";
    static char endl[] = "\r\n";
    
    while (*p == ' ')
    {
	p++;
    }
    argv[argc] = p;
    argc++;
    
    while (*p != '\n' && *p != '\r')
    {
	if (*p == ' ' || *p == '\t')
	{
	    *p = '\0';
	    p++;
	    while (*p == ' ' || *p == '\t')
	    {
		p++;
	    }
	    if (*p != '\n' && *p != '\r' && argc != MAX_ARGUMENTS)
	    {
		argv[argc] = p;
		argc++;
	    }
	}
	p++;
    }
    *p = '\0';

    *answerp = '\0';

    /* Find function for command */
    if ((cmd = bsearch (argv[0], commands, NUMCOMMANDS, 
		    sizeof (struct command), cmdcmp)))
    {
	/* Check rights of current user */
	if ((cmd->valid_as == PASSWORD_STATE_NONE) ||
	    (client[client_idx].password_state & cmd->valid_as) != 0)
	{
	    /* Check number of arguments */
	    if (1<<(argc - 1) & cmd->args)
	    {
		rtc = (cmd->function)(answerp, argc, argv, client_idx);
	    }
	    else
	    {
		rtc = ERR;
		strcpy (answerp, "wrong number of arguments");
	    }   
	}
	else
	{
	    rtc = ERR;
	    if (cmd->valid_as & PASSWORD_STATE_ADMIN)
	    {
		strcat (answerp, "administrator ");
	    }
	    strcat (answerp, "password required");
	}
    }
    else
    {
	rtc = ERR;
	strcpy(answerp, "command not understood");
    }

    switch (rtc)
    {
    case OK:
	answerp -= strlen (okstr);
	strncpy (answerp, okstr, strlen (okstr));
	break;
    case ERR:
	answerp -= strlen (errstr);
	strncpy (answerp, errstr, strlen(errstr));
	break;
    case QUIT:
	answerp = (char *) NULL;
	break;
    case RAW:
	/* no changes to the reply, just show that we actually 
	   care about RAW as a return value */
	break;
    }
    if (rtc != QUIT)
	strcat(answerp, endl);
    
    return (answerp);
} /* parse_command */

/* vi: set softtabstop=4 shiftwidth=4: */
