/*----------------------------------------------------------------------------
 *  telmond.c   - monitor incoming telephone calls
 *
 *  should be started as daemon by fli4l-boot script /etc/rc
 *
 *  usage:  telmond [-port <portno>]
 *          [-imond-port <portno>]
 *          [-log-to-syslog]
 *          [-syslog-fifo <fifo-msg-file>]
 *          [-capi-ctrl <ctrl-number>]*
 *          [-d]
 *          [log-directory]
 *
 *  example:    telmond /var/log
 *
 *  Copyright (c) 2000-2001 - Frank Meyer <frank@fli4l.de>
 *  Copyright (c) 2001-2016 - fli4l-Team <team@fli4l.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Creation:       10.05.2000  fm
 *  Last Update:    $Id$
 *----------------------------------------------------------------------------
 *
 *  if you want to parse the output of syslogd instead of reading /proc/kmsg,
 *
 *  - create a fifo file:
 *      mkfifo /var/log/kernel-info
 *
 *  - add following line into /etc/syslogd.conf:
 *
 *      *.info    |/var/log/kernel-info
 *
 *  - and call telmond as following:
 *
 *      telmond -syslog-fifo /var/log/kernel-info
 *
 *----------------------------------------------------------------------------
 *  format of /etc/telmond.conf:
 *
 *  log msn ip1 [ip2 ....]
 *  exec msn caller command
 *
 *  caller can be a phone number (with area code!) or '*'
 *  msn is a msn without area code
 *
 *  Example:
 *
 *  log   1234567 192.168.6.1
 *  log   1234568 192.168.6.2
 *  exec  1234569 * sleep 5; imonc dial
 *  exec  1234560 0987654321 start-coffee-machine
 *----------------------------------------------------------------------------
 */

#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <ctype.h>
#include <linux/isdn.h>

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>              /* decl of inet_addr()      */
#include <sys/socket.h>
#include <time.h>               /* decl of struct timeval   */
#include <sys/wait.h>               /* declaration of wait()    */

#include <flicapi/flicapi.h>
#include <flicapi/ets300102.h>

#define KLOG_MSG_FILE           "/proc/kmsg"
#define CONFIG_FILE         "/etc/telmond.conf"

#define KLOG_MSG_PREFIX         "<6>isdn_net: call from "
#define SYSLOG_MSG_PREFIX       "kernel: isdn_net: call from "

char *  syslog_msg_fifo;
int log_to_syslog = FALSE;

#define MAX_MSN             16
#define MAX_IP_ADDRESSES_PER_MSN    16
#define MAX_IP_ADDRESSES        32

#define MINIMUM_CALL_TIME_DIFFERENCE    4   /* diff between 2 calls in sec  */

static struct
{
    char    msn[32];
    char    ip[MAX_IP_ADDRESSES_PER_MSN][32];
} msn_map[MAX_MSN];

static int msn_used;

static struct
{
    char    msn[32];
    char    caller[32];
    char    exec_cmd[128];
} exec_list[MAX_MSN];

static int exec_used;

static struct
{
    char    ip[32];
    char    date_time_stamp[32];
    char    caller[32];
    char    my_phone[32];
} ip_map[MAX_IP_ADDRESSES];

static int ip_used;

static char *   log_directory;

static int  msg_fd = -1;

static char global_date_time_stamp[32];
static char global_caller[32];
static char global_my_phone[32];

static char *   date_time (time_t);
static void     open_msg_file(void);
static int  read_line (int, char *, int);
static void add_ip_map (char const *);
static void set_ip_map (char const *, char const *, char const *, char const *);
static int  search_ip_map (char const *);
static void read_conf_file (void);
static void do_log (char const *, char const *, char const *);
static void read_msg_file (void);
int     main (int, char **);

/**
 * Set to TRUE if program termination has been requested.
 */
static sig_atomic_t exit_app = FALSE;

/**
 * Handles termination signals (SIGINT and SIGTERM) by setting exit_app to
 * TRUE.
 *
 * @param sig
 *  (ignored) The signal received.
 */
static void handle_termination(int sig)
{
    (void) sig;
    exit_app = TRUE;
}

/*----------------------------------------------------------------------------
 * date_time ()                 - return date and time
 *----------------------------------------------------------------------------
 */
static char *
date_time (time_t seconds)
{
    static char buf[32];
    struct tm * tm;

    tm = localtime (&seconds);

    sprintf (buf, "%04d/%02d/%02d %02d:%02d:%02d",
         1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
         tm->tm_hour, tm->tm_min, tm->tm_sec);
    return (buf);
} /* date_time (seconds) */


static int
read_line (int fd, char * buf, int maxlen)
{
    char *  p = buf;
    int     i;
    int     rtc;

    for (p = buf, i = 1; i < maxlen; i++, p++)
    {
        rtc = read (fd, p, 1);

        if (rtc != 1)
        {
            *p = '\0';
            return rtc;
        }

        if (*p == '\n')
        {
            break;
        }
    }

    p++;
    *p = '\0';
    return p-buf;
}


static void
add_ip_map (char const * ip_address)
{
    int idx;

    for (idx = 0; idx < ip_used; idx++)
    {
        if (! strcmp (ip_map[idx].ip, ip_address))
        {
            return;
        }
    }

    if (idx < MAX_IP_ADDRESSES)
    {
        strncpy (ip_map[idx].ip, ip_address, 31);

        if (log_directory)
        {
            char    log_file[256];
            FILE *  log_fp;

            sprintf (log_file, "%s/telmond-%s.log", log_directory, ip_address);

            log_fp = fopen (log_file, "a");

            if (log_fp)
            {
                fclose (log_fp);
                sync ();    /* if we log on floppy, flush linux cache   */
            }
        }

        ip_used++;
    }

    return;
} /* add_ip_map (ip_address) */


static void
set_ip_map (char const * ip_address, char const * date_time_stamp,
        char const * caller, char const * my_phone)
{
    int idx;

    for (idx = 0; idx < ip_used; idx++)
    {
        if (! strcmp (ip_map[idx].ip, ip_address))
        {
            strcpy (ip_map[idx].date_time_stamp, date_time_stamp);
            strncpy (ip_map[idx].caller, caller, 31);
            strncpy (ip_map[idx].my_phone, my_phone, 31);
            return;
        }
    }

    return;
} /* set_ip_map (ip_address, date_time_stamp, caller, my_phone) */


static int
search_ip_map (char const * ip_address)
{
    int idx;

    for (idx = 0; idx < ip_used; idx++)
    {
        if (! strcmp (ip_map[idx].ip, ip_address))
        {
            return (idx);
        }
    }

    return (-1);
} /* search_ip_map (ip_address) */


static void
read_conf_file (void)
{
    char    buf[1024];
    char *  p;
    char *  pp;
    int     log_idx = 0;
    int     exec_idx = 0;
    FILE *  fp;

    fp = fopen (CONFIG_FILE, "r");

    if (fp)
    {
        while (fgets (buf, 1024, fp))
        {
            p = strchr (buf, '#');

            if (p)
            {
                *p = '\0';
            }

            for (p = buf; *p == ' ' || * p == '\t'; p++)
            {
                ;                   /* skip whitespaces */
            }

            if (*p == '\r' || *p == '\n')
            {
                continue;               /* skip empty lines */
            }

            for (pp = p; *pp; pp++)     /* transform tabs to blanks */
            {
                if (*pp == '\t')
                {
                    *pp = ' ';
                }
            }

            if (! strncmp (p, "log", 3) && *(p + 3) == ' ')
            {
                for (p = p + 4; *p == ' '; p++)
                {
                    ;
                }

                if (sscanf (p,
                    "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
                    msn_map[log_idx].msn,
                    msn_map[log_idx].ip[0],
                    msn_map[log_idx].ip[1],
                    msn_map[log_idx].ip[2],
                    msn_map[log_idx].ip[3],
                    msn_map[log_idx].ip[4],
                    msn_map[log_idx].ip[5],
                    msn_map[log_idx].ip[6],
                    msn_map[log_idx].ip[7],
                    msn_map[log_idx].ip[8],
                    msn_map[log_idx].ip[9],
                    msn_map[log_idx].ip[10],
                    msn_map[log_idx].ip[11],
                    msn_map[log_idx].ip[12],
                    msn_map[log_idx].ip[13],
                    msn_map[log_idx].ip[14],
                    msn_map[log_idx].ip[15]) >= 2)
                {
                    int j;

                    for (j = 0; *msn_map[log_idx].ip[j]; j++)
                    {
                        add_ip_map (msn_map[log_idx].ip[j]);
                    }

                    log_idx++;
                }
                else
                {
                    if (log_to_syslog)
                    {
                        syslog (LOG_WARNING, "wrong log command: %s", buf);
                    }
                    else
                    {
                        fprintf (stderr,
                            "telmond: warning: wrong log command: %s", buf);
                    }
                }
            }
            else if (! strncmp (p, "exec", 4) && *(p + 4) == ' ')
            {
                for (p = p + 5; *p == ' '; p++)
                {
                    ;
                }

                pp = strchr (p, ' ');

                if (pp)
                {
                    *pp = '\0';

                    strncpy (exec_list[exec_idx].msn, p, 31);

                    for (p = pp + 1; *p == ' '; p++)
                    {
                        ;
                    }

                    pp = strchr (p, ' ');

                    if (pp)
                    {
                        *pp = '\0';

                        strncpy (exec_list[exec_idx].caller, p, 31);

                        for (p = pp + 1; *p == ' '; p++)
                        {
                            ;
                        }

                        pp = strchr (p, '\r');

                        if (! pp)
                        {
                            pp = strchr (p, '\n');
                        }

                        if (pp)
                        {
                            *pp = '\0';
                            strncpy (exec_list[exec_idx].exec_cmd, p, 127);
                            exec_idx++;
                        }
                        else
                        {
                            if (log_to_syslog)
                            {
                                syslog (LOG_WARNING,
                                    "missing newline in line: %s", buf);
                            }
                            else
                            {
                                fprintf (stderr,
                                "telmond: warning: missing newline in line: %s\n",
                                buf);
                            }
                        }
                    }
                    else
                    {
                        if (log_to_syslog)
                        {
                            syslog (LOG_WARNING,
                                "missing caller telno: %s", buf);
                        }
                        else
                        {
                            fprintf (stderr,
                            "telmond: warning: missing caller telno: %s\n",
                            buf);
                        }
                    }
                }
                else
                {
                    if (log_to_syslog)
                    {
                        syslog (LOG_WARNING, "wrong exec command: %s", buf);
                    }
                    else
                    {
                        fprintf (stderr,
                             "telmond: warning: wrong exec command: %s", buf);
                    }
                }
            }
            else
            {
                if (log_to_syslog)
                {
                    syslog (LOG_WARNING, "unknown command: %s", buf);
                }
                else
                {
                    fprintf (stderr,
                         "telmond: warning: unknown command: %s", buf);
                }
            }
        }

        msn_used = log_idx;
        exec_used = exec_idx;
        fclose (fp);
    }
    return;
}


static void
do_log (char const * date_time_stamp, char const * caller, char const * my_phone)
{
    char    log_file[256];
    FILE *  log_fp;
    int     i;
    int     j;

    for (i = 0; i < msn_used; i++)
    {
        if (! strcmp (msn_map[i].msn, my_phone))
        {
            for (j = 0; *msn_map[i].ip[j]; j++)
            {
                sprintf (log_file, "%s/telmond-%s.log", log_directory,
                     msn_map[i].ip[j]);

                log_fp = fopen (log_file, "a");

                if (log_fp)
                {
                    fprintf (log_fp, "%s %s %s\n", date_time_stamp, caller,
                         my_phone);
                    fclose (log_fp);
                    sync ();    /* if we log on floppy, flush linux cache   */
                }

                set_ip_map (msn_map[i].ip[j], date_time_stamp,
                        caller, my_phone);
            }

            return;
        }
    }

    sprintf (log_file, "%s/telmond.log", log_directory);

    log_fp = fopen (log_file, "a");

    if (log_fp)
    {
        fprintf (log_fp, "%s %s %s\n", date_time_stamp, caller, my_phone);
        fclose (log_fp);
        sync ();        /* if we log on floppy, flush linux cache   */
    }

    return;
} /* do_log (date_time_stamp, caller, my_phone) */

static void
record_call (char const *caller, char const *my_phone)
{
    static time_t   last_now;
    time_t      now;
    char *      date_time_stamp;
    int         i;

    now = time ((time_t *) 0);
    date_time_stamp = date_time (now);

    if (log_to_syslog)
    {
        syslog (LOG_INFO, "incoming call: %s %s -> %s", date_time_stamp, caller, my_phone);
    }
    else
    {
        printf ("telmond: incoming call: %s %s -> %s\n", date_time_stamp, caller, my_phone);
    }

    if (now - last_now < MINIMUM_CALL_TIME_DIFFERENCE &&
        ! strcmp (global_caller, caller) &&
        ! strcmp (global_my_phone, my_phone))
    {
        return;
    }

    last_now = now;

    if (log_directory)
    {
        do_log (date_time_stamp, caller, my_phone);
    }

    strcpy (global_date_time_stamp, date_time_stamp);
    strncpy (global_caller, caller, 31);
    strncpy (global_my_phone, my_phone, 31);

    for (i = 0; i < exec_used; i++)
    {
        if (! strcmp (exec_list[i].msn, my_phone))
        {
            if (! strcmp (exec_list[i].caller, "*") ||
                ! strcmp (exec_list[i].caller, caller))
            {
                char    exec_cmd[512];
                char *  s = exec_list[i].exec_cmd;
                char *  t = exec_cmd;

                while (*s)
                {
                    if (*s == '%')
                    {
                        s++;

                        switch ((int) *s)
                        {
                        case 'd':           /* date     */
                            strncpy (t, global_date_time_stamp, 10);
                            t += 10;
                            break;
                        case 't':           /* time     */
                            strncpy (t, global_date_time_stamp + 11, 8);
                            t += 8;
                            break;
                        case 'p':           /* phone no.    */
                            strcpy (t, global_caller);
                            t += strlen (global_caller);
                            break;
                        case 'm':           /* msn      */
                            strcpy (t, my_phone);
                            t += strlen (my_phone);
                            break;
                        case '%':
                            *t++ = '%';
                            break;
                        default:
                            *t++ = '%';
                            *t++ = *s;
                            break;
                        }

                        s++;
                    }
                    else
                    {
                        *t++ = *s++;
                    }
                }

                *t = '\0';

                if (log_to_syslog)
                {
                    syslog (LOG_INFO, "executing '%s'", exec_cmd);
                }
                else
                {
                    printf ("telmond: executing '%s'\n", exec_cmd);
                }

                (void) system (exec_cmd);
            }
        }
    }
}

static void
read_msg_file (void)
{
    char        buf[512];
    char        caller[512];
    char        my_phone[512];
    char *      p;
    char *      pp;
    char *      ppp;
    int         rtc;

    /*----------------------------------------------------------------
     * fm, 06.01.2003: Perhaps found a kernel bug: select() tells
     * telmond that there is data in the pipe, but there is nothing if
     * an overflow occurs caused by excessive log messages (e.g. large
     * firewall log after a port-scan).
     *--------------------------------------------------------------*/
    alarm (2);      /* check for read timeout   */
    rtc=read_line (msg_fd, buf, 511);
    alarm (0);

    if (rtc > 0)
    {
        if (syslog_msg_fifo)
        {
            int i;
            p = buf;

            for (i = 0; i < 5; i++) /* skip 5 blanks in syslog output   */
            {
                p = strchr (p, ' ');

                if (! p)
                {
                    return;
                }

                while (*p == ' ')
                {
                    p++;
                }
            }

            if (strncmp (p, SYSLOG_MSG_PREFIX,
                 sizeof (SYSLOG_MSG_PREFIX) - 1) != 0)
            {
                return;
            }

            p += sizeof (SYSLOG_MSG_PREFIX) - 1;
        }
        else
        {
            if (strncmp (buf, KLOG_MSG_PREFIX, sizeof (KLOG_MSG_PREFIX) - 1)
                != 0)
            {
                return;
            }

            p = buf + sizeof (KLOG_MSG_PREFIX) - 1;
        }

        caller[0] = '0';

        if (sscanf (p, "%s -> %s", caller + 1, my_phone) != 2)
        {
            return;
        }

        pp = strchr (caller, ',');

        if (! pp)
        {
            return;
        }

        ppp = strchr (pp + 1, ',');

        if (! ppp)
        {
            return;
        }

        *ppp = '\0';

        if (atoi (pp + 1) != 1)
        {
            return;
        }

        *pp = '\0';

        record_call(caller, my_phone);
    }
    else if (rtc == 0) {
        close(msg_fd);
        open_msg_file();
    }
    else {
        if (errno != EINTR)
        {
            if (log_to_syslog)
            {
                syslog (LOG_WARNING, "unexpected error %s", strerror(errno));
            }
            else
            {
                fprintf (stderr, "telmond: unexpected error %s\n", strerror(errno));
                fflush (stderr);
            }
        }
    }

    return;
}

static void my_alarm (int sig)
{
#if 1
    long dx;

    (void) signal (SIGALRM, my_alarm);

    dx = time ((time_t *) NULL);
    fprintf (stderr, "telmond: %s: got signal %d\n", date_time (dx), sig);
    fflush (stderr);
#else
    (void) signal (SIGALRM, my_alarm);
#endif
    return;
}

void open_msg_file (void)
{
    char * filename;

    if (syslog_msg_fifo)
    {
        filename = syslog_msg_fifo;
    }
    else
    {
        filename = KLOG_MSG_FILE;
    }

    msg_fd = open (filename, O_RDONLY | O_NONBLOCK);

    if (msg_fd < 0)
    {
        if (log_to_syslog)
        {
            syslog (LOG_ERR, "cannot open %s (%s)", filename, strerror(errno));
        }

        fprintf (stderr, "cannot open %s (%s)\n", filename, strerror(errno));
        exit (1);
    }
}

/**
 * Extracts a phone number, adding the correct number of prefix (zero) digits.
 *
 * @param numberType
 *  The number type.
 * @param digits
 *  A pointer to the array of digits as provided by the ISDN/CAPI layer. Note
 *  that this array is not zero-terminated.
 * @param numDigits
 *  The number of digits in the array.
 * @return
 *  A pointer to a dynamically allocated zero-terminated string containing the
 *  desired phone number.
 */
static char *
get_number(int numberType, char const *digits, int numDigits)
{
    char const *prefix = "";
    int prefixLen;
    char *number;

    if (numDigits == 0 || isdigit(digits[0])) {
        switch (numberType) {
        case ETS300102_NUMBER_TYPE_INTERNATIONAL :
            prefix = "+";
            break;
        case ETS300102_NUMBER_TYPE_NATIONAL :
            prefix = "0";
            break;
        default :
            break;
        }
    }
    /* else: not a (simple) analogue/ISDN number, e.g. SIP */

    prefixLen = strlen(prefix);
    number = (char *) malloc(prefixLen + numDigits + 1);
    strcpy(number, prefix);
    memcpy(number + prefixLen, digits, numDigits);
    number[prefixLen + numDigits] = '\0';
    return number;
}

/**
 * Handles a CONNECT_IND message by extracting and logging the phone numbers of
 * the calling and the called party and by responding with a CONNECT_RESP
 * message rejecting the call. Such a message is received when an incoming call
 * is detected by the ISDN/CAPI layer.
 *
 * @param cmsg
 *  The CONNECT_IND message.
 * @return
 *  Always zero.
 */
static int
handleConnectIND(_cmsg *cmsg)
{
    unsigned rc;

    unsigned lenCalled;
    struct ETS300102_CalledPartyNumber *calledNumberStruc;
    unsigned lenCalling;
    struct ETS300102_CallingPartyNumber *callingNumberStruc;
    char *calledNumber;
    char *callingNumber;

    calledNumberStruc =
        (struct ETS300102_CalledPartyNumber *) flicapi_get_capi_struct(
            CONNECT_IND_CALLEDPARTYNUMBER(cmsg),
            &lenCalled
        );
    calledNumber = lenCalled >= offsetof(struct ETS300102_CalledPartyNumber, digits)
        ? get_number(calledNumberStruc->numberType, calledNumberStruc->digits,
            lenCalled - offsetof(struct ETS300102_CalledPartyNumber, digits))
        : NULL;

    callingNumberStruc =
        (struct ETS300102_CallingPartyNumber *) flicapi_get_capi_struct(
            CONNECT_IND_CALLINGPARTYNUMBER(cmsg),
            &lenCalling
        );
    callingNumber = lenCalling >= offsetof(struct ETS300102_CallingPartyNumber, digits)
        ? get_number(callingNumberStruc->numberType, callingNumberStruc->digits,
            lenCalling - offsetof(struct ETS300102_CallingPartyNumber, digits))
        : NULL;

    flicapi_log(
        "CONNECT_IND: Incoming call %s --> %s on controller %u\n",
        callingNumber && *callingNumber ? callingNumber : "UNKNOWN",
        calledNumber && *calledNumber ? calledNumber : "UNKNOWN",
        CONNECT_IND_PLCI(cmsg) & 0x7F
    );
    record_call(
        callingNumber && *callingNumber ? callingNumber : "UNKNOWN",
        calledNumber && *calledNumber ? calledNumber : "UNKNOWN"
    );

    free(callingNumber);
    free(calledNumber);
    free(callingNumberStruc);
    free(calledNumberStruc);

    CONNECT_RESP_HEADER(cmsg, cmsg->ApplId, cmsg->Messagenumber,
            CONNECT_IND_PLCI(cmsg));
    CONNECT_RESP_REJECT(cmsg) = 1;
    if ((rc = CAPI_PUT_CMSG(cmsg)) != 0) {
        flicapi_log("CONNECT_IND: Failed to send CONNECT_RESP message: %s.\n",
                capi_info2str(rc));
    }

    return 0;
}

/**
 * Handles a DISCONNECT_IND message by logging it and by responding with a
 * DISCONNECT_RESP message. Such a message is received after an incoming call
 * (CONNECT_IND) is rejected by sending CONNECT_RESP.
 *
 * @param cmsg
 *  The DISCONNECT_IND message.
 * @return
 *  Always zero.
 */
static int
handleDisconnectIND(_cmsg *cmsg)
{
    unsigned rc;
    rc = DISCONNECT_IND_REASON(cmsg);
    flicapi_log("DISCONNECT_IND on controller %u: %s\n",
            DISCONNECT_IND_PLCI(cmsg) & 0x7F,
            rc == 0 ? "Normal clearing" : capi_info2str(rc));
    DISCONNECT_RESP_HEADER(cmsg, cmsg->ApplId, cmsg->Messagenumber,
            DISCONNECT_IND_PLCI(cmsg));
    if ((rc = CAPI_PUT_CMSG(cmsg)) != 0) {
        flicapi_log("Failed to send DISCONNECT_RESP message: %s\n",
                capi_info2str(rc));
    }

    return 0;
}

/**
 * The handlers used by telmond.
 */
static flicapi_handler_t telmond_handlers[] = {
    { CAPI_CONNECT, CAPI_IND, &handleConnectIND, },
    { CAPI_DISCONNECT, CAPI_IND, &handleDisconnectIND, },
};

int
main (int argc, char ** argv)
{
    char            buf[256];
    static struct sockaddr_in   my_listen_addr;
    static size_t       my_listen_size;
    fd_set          fdset;
    FILE *          pid_fp;
    int             select_rtc;
    int             sock_fd;
    int             reuse_addr;
    int             port = 5001;
    int             rtc;
    struct sigaction sig;
    BOOL            do_fork = TRUE;
    BOOL            no_capi = FALSE;
    BOOL            capi_used = FALSE;
    unsigned        capi_id = 0;
    unsigned        capi_ctrl_num = 0;
    unsigned       *capi_ctrl = NULL;

    (void) signal (SIGHUP, SIG_IGN);
    (void) signal (SIGPIPE, SIG_IGN);

    (void) signal (SIGALRM, my_alarm);

    memset(&sig, 0, sizeof sig);
    sig.sa_handler = &handle_termination;
    sig.sa_flags   = SA_RESTART;
    sigaction(SIGINT, &sig, NULL);

    memset(&sig, 0, sizeof sig);
    sig.sa_handler = &handle_termination;
    sig.sa_flags   = SA_RESTART;
    sigaction(SIGTERM, &sig, NULL);

    while (argc >= 3 && *(argv[1]) == '-')
    {
        if (! strcmp (argv[1], "-port"))
        {
            port = atoi (argv[2]);

            argc -= 2;
            argv += 2;
        }
        else if (! strcmp (argv[1], "-log-to-syslog"))
        {
            log_to_syslog = TRUE;
            openlog ("telmond", LOG_PID, LOG_DAEMON);

            argc --;
            argv ++;
        }
        else if (! strcmp (argv[1], "-syslog-fifo"))
        {
            syslog_msg_fifo = argv[2];

            argc -= 2;
            argv += 2;
        }
        else if (! strcmp (argv[1], "-no-capi"))
        {
            no_capi = TRUE;

            argc --;
            argv ++;
        }
        else if (! strcmp (argv[1], "-capi-ctrl"))
        {
            int index = atoi (argv[2]);
            if (index > 0) {
                capi_ctrl = (unsigned *)
                    realloc(capi_ctrl, (capi_ctrl_num + 2) * sizeof(unsigned));
                capi_ctrl[capi_ctrl_num] = (unsigned) index;
                capi_ctrl[capi_ctrl_num + 1] = 0;
                ++capi_ctrl_num;
            }
            else {
                fprintf(stderr, "telmond: Invalid CAPI controller index %d, ignoring it\n", index);
            }

            argc -= 2;
            argv += 2;
        }
        else if (! strcmp (argv[1], "-d"))
        {
            do_fork = FALSE;

            argc --;
            argv ++;
        }
        else
        {
            break;
        }
    }

    if (argc == 2)
    {
        log_directory = argv[1];
    }

    if (do_fork) {
        rtc = fork ();
        if (rtc < 0)
        {
            perror ("fork");
            exit (1);
        }

        if (rtc > 0)                    /* it's the parent  */
        {
            return (0);
        }
    }

    read_conf_file ();
    open_msg_file();

    if (!no_capi) {
        rtc = flicapi_init("telmond", &capi_id, capi_ctrl, log_to_syslog);
        if (rtc == 0) {
            capi_used = TRUE;
        }
        else {
            if (log_to_syslog) {
                syslog(LOG_WARNING, "Initializing CAPI layer failed (return code %d), falling back to reading kernel messages", rtc);
            }
            fprintf(stderr, "telmond: Initializing CAPI layer failed (return code %d), falling back to reading kernel messages\n", rtc);
        }
    }

    if ((sock_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)   /* open socket  */
    {
        if (log_to_syslog)
        {
            syslog (LOG_ERR, "socket: %m");
        }

        perror ("socket");
        exit (1);
    }

    /* set SO_REUSEADDR flag on socket */
    reuse_addr = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof reuse_addr);

    my_listen_size = sizeof (my_listen_addr);

    (void) memset ((char *) &my_listen_addr, 0, my_listen_size);

    my_listen_addr.sin_family      = AF_INET;
    my_listen_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    my_listen_addr.sin_port        = htons (port);

    if (bind (sock_fd, (struct sockaddr *) &my_listen_addr, my_listen_size) < 0)
    {
        (void) close (sock_fd);
        perror ("bind");
        fflush (stderr);
        exit (1);
    }

    pid_fp = fopen ("/var/run/telmond.pid", "w");

    if (pid_fp)
    {
        fprintf (pid_fp, "%d\n", getpid ());
        fclose (pid_fp);
    }

    (void) listen (sock_fd, 5);

    while (!exit_app)
    {
        struct timeval tv;
        FD_ZERO (&fdset);
        if (!capi_used) {
            FD_SET (msg_fd, &fdset);
        }
        FD_SET (sock_fd, &fdset);

        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;

        select_rtc = select (32, &fdset, (fd_set *) 0, (fd_set *) 0, &tv);

        if (select_rtc > 0)
        {
            if (FD_ISSET (msg_fd, &fdset))
            {
                read_msg_file ();
            }

            if (FD_ISSET (sock_fd, &fdset))
            {
                int new_fd;

                new_fd = accept (sock_fd, (struct sockaddr *) &my_listen_addr,
                         (socklen_t *) &my_listen_size);

                if (new_fd >= 0)
                {
                    unsigned char   a[4];
                    char            client_addr[16];
                    char *      date_time_stamp;
                    char *      my_phone;
                    char *      caller;
                    int         idx;

                    (void) memcpy (a, &(my_listen_addr.sin_addr.s_addr), 4);

                    (void) sprintf (client_addr, "%d.%d.%d.%d",
                            a[0], a[1], a[2], a[3]);

                    idx = search_ip_map (client_addr);

                    if (idx >= 0)
                    {
                        date_time_stamp = ip_map[idx].date_time_stamp;
                        caller      = ip_map[idx].caller;
                        my_phone        = ip_map[idx].my_phone;
                    }
                    else
                    {
                        date_time_stamp = global_date_time_stamp;
                        caller      = global_caller;
                        my_phone    = global_my_phone;
                    }

                    if (*my_phone)
                    {
                        sprintf (buf, "%s %s %s\r\n",
                             date_time_stamp, caller, my_phone);

                        alarm (2);
                        (void) write (new_fd, buf, strlen (buf));
                        alarm (0);
                    }

                    (void) close (new_fd);
                }
            }
        }
        else if (select_rtc < 0 && !exit_app)
        {
            if (log_to_syslog)
            {
                syslog (LOG_ERR, "select rtc = %d, errno = %d",
                    select_rtc, errno);
            }

            fprintf (stderr, "telmond: select rtc = %d, errno = %d\n",
                 select_rtc, errno);
            fflush (stderr);
            sleep (1);
        }

        if (capi_used) {
            rtc = flicapi_process_messages(capi_id, telmond_handlers);
            if (rtc != 0) {
                flicapi_fini(capi_id);
                capi_used = FALSE;
            }
        }
    }

    if (capi_used) {
        flicapi_fini(capi_id);
    }

    return (0);
}

#if 0
/*----------------------------------------------------------------------------
 *  send_call (host_name, port, call)       - send call information to imond
 *----------------------------------------------------------------------------
 */
static int
send_call (char * host_name, int port, char * call)
{
    struct sockaddr_in  addr;
    struct hostent *    host_p;
    char        buf[256];
    int         len;
    int         fd;
    int         opt = 1;

    (void) memset ((char *) &addr, 0, sizeof (addr));

    if ((addr.sin_addr.s_addr = inet_addr ((char *) host_name)) == INADDR_NONE)
    {
        host_p = gethostbyname (host_name);

        if (! host_p)
        {
            fprintf (stderr, "%s: host not found\n", host_name);
            return (-1);
        }

        (void) memcpy ((char *) (&addr.sin_addr), host_p->h_addr,
                host_p->h_length);
    }

    addr.sin_family  = AF_INET;
    addr.sin_port    = htons ((unsigned short) port);

    if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {                       /* open socket          */
        perror ("socket");
        return (-1);
    }

    (void) setsockopt (fd, IPPROTO_TCP, TCP_NODELAY,
               (char *) &opt, sizeof (opt));

    if (connect (fd, (struct sockaddr *) &addr, sizeof (addr)) != 0)
    {
        (void) close (fd);
        perror (host_name);
        return (-1);
    }

    sprintf (buf, "%s\r\n", call);

    len = strlen (buf);
    write (fd, buf, len);

    (void) read (fd, buf, 256);

    close (fd);
    return (0);
} /* send_call (char * host_name, int port, char * port) */

#endif
