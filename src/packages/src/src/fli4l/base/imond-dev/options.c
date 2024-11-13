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
 * \brief Option parsing
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "imond.h"
#include "log.h"

static int opt_bind_ip (char ** argv);
static int opt_dialmode (char ** argv);

/** This classifies the types of options */
enum opt_type {
	o_noarg = 0,	/**< option takes no argument */
	o_bool,		/**< option is boolean (takes argument "yes"/"no") */
	o_int,		/**< option takes an integer argument */
	o_str,		/**< option takes a string argument */
	o_str_move,	/**< option takes a string argument which should be
			  cleared from the command line after parsing it (to
			  not make passwors visible through /proc) */
	o_func		/**< option is parsed by another function */
};

/** This is a prototype for hook functions for command line options */
typedef int (* opt_func_t)(char ** arg);

/** This struct holds the definition of one command line option */
typedef struct {
    char	*name;		/**< name of the option */
    enum opt_type type;		/**< type of option */
    int		n_arg;		/**< number of arguments the option takes */
    void	*addr;		/**< pointer to variable to change */
    opt_func_t  opt_func;	/**< optional pointer to hook function for
				  type == o_func */
} option_t;

/**
 * This is an array describing all the command line options imond accepts.
 */
option_t options[] = {
    { "-beep", 	o_bool, 	1,	&globals.do_beep},
    { "-enable",o_bool, 	1,	&globals.may_enable},
    { "-dial",	o_bool, 	1,	&globals.may_dial},
    { "-route",	o_bool, 	1,	&globals.may_route},
    { "-reboot",o_bool, 	1,	&globals.may_reboot},
    { "-circ",  o_bool, 	1,	&globals.circ},
    { "-logproto",  o_bool, 	1,	&globals.log_proto},
    { "-port", 	o_int,		1, 	&globals.imond_port}, 
    { "-led", 	o_str, 		1, 	&globals.led_device},
    { "-pass", 	o_str_move, 	1, 	&globals.imond_pass},
    { "-admin-pass", o_str_move, 1, 	&globals.imond_admin_pass},
    { "-imond-log-dir", o_str, 1, 	&globals.imond_log_dir},
    { "-mgetty-log-dir", o_str, 1, 	&globals.mgetty_log_dir},
    { "-telmond-log-dir", o_str, 1, 	&globals.telmond_log_dir},
    { "-log-to-syslog", o_func, 0, 	0, (opt_func_t)imond_open_syslog},
    { "-ip", 	o_func, 	0, 	0, 	opt_bind_ip},
    { "-dialmode", o_func, 	0,	0,	opt_dialmode},
    { NULL }
};

/**
 * This function parses the command line for the "-ip" option. Parameters are
 * interpreted as IP addresses until an argument is detected that is not an IP
 * address.
 *
 * \param argv pointer to array of arguments following the "-ip" option
 *
 * \returns number of IPs passed to the option
 */
int
opt_bind_ip (char ** argv)
{
    struct in_addr my_ip;
    int n_ip = 0;
    char ** p;

    for (p = globals.ips; *p; p++)
	;

    while (*argv && inet_aton(*argv, &my_ip))
    {
	n_ip++;

	if (p - globals.ips < MAX_IP)
	{
	    *(p++) = *argv;
	    *p = NULL;
	}
	else
	{
	    imond_syslog (LOG_INFO, "to many ip-addresses, ignoring %s\n",
			  *argv);
	}
	argv++;
    }

    if (!n_ip)
    {
	imond_syslog (LOG_ERR,"ignoring invalid ip-address: %s\n",
		      *argv);
    }
    return n_ip;
}

/**
 * This function parses the -dialmode option. Based on the value after the
 * opttion, the variable globals.dialmode is set to DIALMODE_OFF (value after
 * option was "off"), DIALMODE_AUTO ("auto") or DIALMODE_MANUAL ("manual").
 * When the string after the option does not match any of these strings, the
 * dialmode is set to DIALMODE_OFF.
 *
 * \param argv pointer to command line after this option
 *
 * \retval 1 always, because this option takes one argument
 */
int
opt_dialmode (char ** argv)
{
    if (! strcmp (*argv, "off"))
    {
	globals.dialmode = DIALMODE_OFF;
    }
    else if (! strcmp (*argv, "auto"))
    {
	globals.dialmode = DIALMODE_AUTO;
    }
    else if (! strcmp (*argv, "manual"))
    {
	globals.dialmode = DIALMODE_MANUAL;
    }
    else
    {
	imond_syslog (LOG_WARNING,
		      "DIALMODE='%s' is unknown. Setting to 'off'...\n",
		      *argv);
	globals.dialmode = DIALMODE_OFF;
    }
    return 1;
}

/**
 * Find the specific option in the array ::options.
 *
 * \param option the option that is searched
 *
 * \returns pointer to found option struct or 0 if option was not found
 */
option_t * find_option (char * option)
{
    option_t * opt;
    for (opt = options; opt->name && strcmp (opt->name, option); opt++)
	;
    if (opt->name)
	return opt;
    else
	return 0;
}

/**
 * This functions parses a specific option from the command line. Based on the
 * option that is handled, a specific action is performed.
 *
 * \param opt pointer to struct of currently processed option
 * \param argv pointer to the part of the commandline after the currently
 * parsed option
 *
 * \return number of arguments that were used for this option
 */
int
process_option (option_t * opt, char ** argv)
{
    switch (opt->type) {
    case o_noarg:
	*(int *)opt->addr = 1;
	break;
    case o_bool:
	*(int *)opt->addr = !strcasecmp (*argv, "yes");
	break;
    case o_int:
	*(int *)opt->addr = atoi (*argv);
	break;
    case o_str_move:
	strncpy (opt->addr, *argv, OPT_SIZE);
	**argv = '\0';
	break;
    case o_str:
	if (**argv)
	{
	    *(char **)opt->addr = *argv;
	    break;
	}
	else
	{
	    *(char **)opt->addr = 0;
	}
    case o_func:
	return opt->opt_func (argv);
    }
    return opt->n_arg;
}

/**
 * This function parses an array of arguments from the command line.
 *
 * \param argc number of arguments that were passed to the command
 * \param argv array of arguments
 *
 * \retval 0 parsing of options was succesful
 * \retval -1 a fatal error occoured while parsing the options
 */
int
parse_args(int argc, char **argv)
{
    char *arg;
    option_t *opt;
    int n;
    int ret = 0;

    while (argc > 0) {
	arg = *argv++;
	--argc;
	opt = find_option(arg);
	if (opt == NULL) {
	    fprintf (stderr, "unrecognized option '%s'", arg);
	    ret = -1;
	}
	if (argc < opt->n_arg) {
	    fprintf(stderr, "too few parameters for option %s", arg);
	    ret = -1;
	}
	n = process_option (opt, argv); 
	argc -= n;
	argv += n;
    }
    if (*globals.imond_pass)
    {
	password_mode |= PASS_REQUIRED;
    }
    if (*globals.imond_admin_pass)
    {
	password_mode |= ADMIN_PASS_REQUIRED;
    }
    if (globals.mgetty_log_dir)
    {
	snprintf (globals.mgetty_log_file, MAX_FNAME,
		  "%s/mfax.log", globals.mgetty_log_dir);
		  
    }
    if (globals.imond_log_dir)
    {
    set_imond_log_dir(globals.imond_log_dir);	
    }
    return ret;
}
