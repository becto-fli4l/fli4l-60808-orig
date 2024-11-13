/*
 * ppp2i4l.c - Allows to use the stock and unpatched pppd daemon to communicate
 * over ISDN4Linux ISDN devices.
 *
 * Copyright (c) 2013 Christoph Schulz and the fli4l team
 * Last Revision: $Id$
 *
 * This program makes the following assumptions:
 * (1) The pppd must use asynchronous PPP (N_PPP line discipline).
 * (2) The MTU must not exceed 1500 bytes.
 * (3) The kernel needs to be patched in order to send _all_ PPP traffic
 *     to the pppd (i.e. to us). This is necessary for a successful
 *     decompression of compressed PPP packets coming from the ISDN device.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include <unistd.h>           /* write(), read(), close() */
#include <stdlib.h>           /* malloc(), free() */
#include <stdio.h>            /* sprintf() */
#include <signal.h>           /* struct sigaction, sigaction() */
#include <string.h>           /* strerror() */
#include <errno.h>            /* errno */
#include <fcntl.h>            /* open(), O_RDWR */
#include <sys/ioctl.h>        /* ioctl(), TIOCSETD */
#include <sys/time.h>         /* struct timeval */
#include <net/if.h>           /* struct ifreq, needed by linux/if_ppp.h */
#include <netinet/in.h>       /* ntohs() */
#include <linux/isdn.h>       /* struct isdn_net_ioctl_phone, IIOCNETGPN */
#include <linux/ppp_defs.h>   /* PPP-related definitions as PPP_ALLSTATIONS */
#include <linux/if_ppp.h>     /* more PPP-related definitions as PPPIOCGCHAN */

#define SYSLOG_NAMES
#include <syslog.h>           /* openlog(), syslog() */

/**
 * boolean type
 */
typedef int BOOL;
/**
 * boolean false
 */
#define FALSE 0
/**
 * boolean true
 */
#define TRUE  1

/**
 * The path to device files.
 */
#define DEV_PREFIX "/dev/"
/**
 * Path to the isdninfo device.
 */
#define ISDNINFO_DEV DEV_PREFIX "isdninfo"
/**
 * Path to the isdnctrl device.
 */
#define ISDNCTRL_DEV DEV_PREFIX "isdnctrl"
/**
 * Maximum buffer length for packets received from pppd/the network. At least
 * no HiSax ISDN card driver supports buffers > 4096, see the definitions of
 * HSCX_BUFMAX or MAX_DATA_SIZE in hisax.h. (Many ISDN card drivers have an
 * even lower maximum value.)
 */
#define BUFLEN 4096

/**
 * Describes a PPP header.
 */
struct ppp_header {
    /**
     * The address byte. Always 0xFF (PPP_ALLSTATIONS).
     */
    unsigned char addr;
    /**
     * The control byte. Always 0x03 (PPP_UI).
     */
    unsigned char ctrl;
    /**
     * The PPP protocol, in network byte order (big endian).
     */
    unsigned short proto;
};

/**
 * The packet buffer.
 */
unsigned char packet_buf[BUFLEN];
/**
 * 1 if the application should terminate because a signal has been received,
 * 0 otherwise.
 */
sig_atomic_t do_exit = 0;
/**
 * The debug level.
 */
int debug_level = LOG_WARNING;

/**
 * Handles a signal.
 * @param sig
 *  The signal received.
 */
static void
handle_signal(int sig)
{
    do_exit = 1;
    (void) sig;
}

/**
 * Prints a received packet to the syslog.
 * @param buf
 *  Points to the packet data.
 * @param len
 *  The length of the data in bytes.
 */
static void
dump_packet(unsigned char const *buf, int len)
{
    char line[80];
    int offset = 0;
    while (len > 0) {
        int i;
        int blocklen = len > 8 ? 8 : len;
        char *p = line;

        for (i = 0; i < blocklen; ++i, --len, p += 3) {
            sprintf(p, "%02X ", *buf++);
        }
        syslog(LOG_DEBUG, "%s", line);
        offset += blocklen;
    }
}

/**
 * Writes a packet to some target. Handles EINTR properly. If a packet cannot
 * be written completely, an error message is written to the syslog.
 * @param tgt
 *  The file descriptor to write to.
 * @param buf
 *  Points to the packet data to write.
 * @param len
 *  Number of bytes to write.
 */
static void
do_write(int tgt, unsigned char const *buf, int len)
{
    while (len > 0) {
        int wr = write(tgt, buf, len);
        if ((wr < 0 && errno != EINTR) || wr == 0) {
            syslog(LOG_ERR,
                    "Writing %d bytes to %d failed: %s", len, tgt,
                    wr == 0 ? "Nothing written" : strerror(errno));
            return;
        }
        len -= wr;
        buf += wr;
    }
}

/**
 * Copies pppd data to the ISDN device.
 * @param pppd_fd
 *  Data from the pppd is read from this tty using the N_PPP line discipline.
 * @param ippp_fd
 *  The ISDN device where to write packets to the remote site.
 */
static void
do_handle_pppd_input(int pppd_fd, int ippp_fd)
{
    /* due to N_PPP line discipline, the pppd packets do not have the HDLC
     * address and control bytes, so we have to prepend them for ipppd */
    int len = read(pppd_fd, packet_buf + 2, sizeof packet_buf - 2);
    if (len > 0) {
        struct ppp_header *hdr = (struct ppp_header *) packet_buf;
        syslog(LOG_INFO,
                "Local PPP 0x%04X packet: %d", ntohs(hdr->proto), len);
        dump_packet(packet_buf + 2, len);

        hdr->addr = PPP_ALLSTATIONS;
        hdr->ctrl = PPP_UI;
        do_write(ippp_fd, packet_buf, len + 2);
    }
}

/**
 * Copies non-network data from the ISDN device to pppd.
 * @param ippp_fd
 *  The ISDN device where to read packets from the remote site.
 * @param pppd_fd
 *  Data to the pppd is written to this tty using the N_PPP line discipline.
 */
static void
do_handle_ippp_input(int ippp_fd, int pppd_fd)
{
    int len = read(ippp_fd, packet_buf, sizeof packet_buf);
    if (len > 2) {
        struct ppp_header *hdr = (struct ppp_header *) packet_buf;
        syslog(LOG_INFO,
                "Remote PPP 0x%04X packet: %d", ntohs(hdr->proto), len);
        dump_packet(packet_buf, len);

        /* due to N_PPP line discipline, the pppd packets do not require the
         * HDLC address and control bytes, so we have to strip them */
        do_write(pppd_fd, packet_buf + 2, len - 2);
    }
}

/**
 * Copies data from the ISDN device to pppd and data from pppd to the ISDN
 * device.
 *
 * @param pppd_fd
 *  Data from/to the pppd is read from/written to this tty using the N_PPP line
 *  discipline. Don't use N_SYNC_PPP, it is broken! See the (lengthy) comment
 *  in main() about this.
 * @param ippp_fd
 *  The ISDN device where to read packets from and write packets to the remote
 *  site.
 * @param isdninfo_fd
 *  A file descriptor to the isdninfo device. This is used to determine whether
 *  the ISDN connection has been established. This is important as writing to
 *  the ISDN device without a connection does not block but simply returns
 *  zero. So we have to avoid writing anything to the ISDN device if not
 *  connected.
 * @param isdnctrl_fd
 *  A file descriptor to the isdnctrl device. This is used to hangup the ISDN
 *  line before exiting.
 * @param ifname
 *  The name of the network interface. This is needed for determining whether
 *  the ISDN connection has been established or not.
 * @param timeout
 *  Number of seconds to wait until the ISDN line must be up. If the value is
 *  zero or negative, no timeout exists.
 */
static void
loop(
    int pppd_fd, int ippp_fd, int isdninfo_fd, int isdnctrl_fd,
    char const *ifname, int timeout
)
{
    isdn_net_ioctl_phone phone;
    int link_active = 0; /* we assume that no ISDN connection exists */
    int seconds = 0;

    /* determine highest file descriptor for select() below */
    int maxfd = pppd_fd;
    if (ippp_fd > maxfd) {
        maxfd = ippp_fd;
    }

    /* initialize structure for IIOCNETGPN ioctl below */
    memset(&phone, 0, sizeof phone);
    strncpy(phone.name, ifname, sizeof phone.name - 1);

    while (1) {
        fd_set readable;
        struct timeval tv;
        int r;

        FD_ZERO(&readable);
        FD_SET(ippp_fd, &readable);
        if (link_active) {
            FD_SET(pppd_fd, &readable);
        }

        /* don't wait indefinitely as there is a possible race: a signal
         * handler could just have set do_exit to 1 after the
         * while(!do_exit) test above */
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        r = select((link_active ? maxfd : ippp_fd) + 1,
                        &readable, NULL, NULL, &tv);
        if (r < 0) {
            if (errno != EINTR) {
                syslog(LOG_ERR, "select() failed: %s", strerror(errno));
                break;
            }
        }

        if (do_exit) {
            break;
        }

        if (r > 0) {
            /* handle pppd --> ipppd */
            if (FD_ISSET(pppd_fd, &readable)) {
                do_handle_pppd_input(pppd_fd, ippp_fd);
            }
            /* handle ipppd --> pppd and ISDN link state changes  */
            if (FD_ISSET(ippp_fd, &readable)) {
                int new_link_active;
                if (link_active) {
                    do_handle_ippp_input(ippp_fd, pppd_fd);
                }
                new_link_active = ioctl(isdninfo_fd, IIOCNETGPN, &phone) == 0;
                if (new_link_active < link_active) {
                    syslog(LOG_NOTICE, "ISDN connection is down");
                    link_active = new_link_active;
                    break;
                }
                else if (link_active < new_link_active) {
                    syslog(LOG_NOTICE, "ISDN connection is up");
                    link_active = new_link_active;
                }
            }
        }

        if (!link_active && timeout > 0) {
            ++seconds;
            if (seconds == timeout) {
                syslog(LOG_WARNING,
                        "ISDN connection not up within %d seconds, giving up",
                        timeout);
                break;
            }
        }
    }

    /* hangup connection if necessary */
    if (link_active) {
        if (ioctl(isdnctrl_fd, IIOCNETHUP, ifname) < 0) {
            syslog(LOG_ERR,
                "Failed to hangup %s: %s", ifname, strerror(errno));
        }
    }
}

/**
 * Initializes the syslog.
 * @param facility_name
 *  The facility name. If NULL, the default "daemon" is used.
 * @param log_level
 *  The log level. 0 corresponds to <= LOG_WARNING, 1 to <= LOG_NOTICE etc.
 */
static void
initsyslog(char const *facility_name, int log_level)
{
    int facility = -1;

    if (facility_name) {
        CODE *code = facilitynames;
        while (code->c_name) {
            if (strcmp(code->c_name, facility_name) == 0) {
                facility = code->c_val;
                break;
            }
            ++code;
        }
    }

    if (facility < 0) {
        facility = LOG_DAEMON;
    }

    openlog("ppp2i4l", LOG_PID, facility);

    /* log level 0 corresponds to LOG_WARNING and higher */
    log_level += LOG_WARNING;
    setlogmask(LOG_UPTO(log_level));
}

/**
 * Prints the program usage to stderr.
 */
static void
usage(void)
{
    fprintf(stderr,
            "Usage: ppp2i4l [-s] [-d <loglevel>] [-l <facility>] [t <timeout>] "
            "<isdn-device> <isdn-network-interface>\n");
}

/**
 * Returns a control socket for manipulating network interfaces.
 * Taken from iproute2 source code.
 */
static int
get_ctl_fd(void)
{
    int s_errno;
    int fd;

    fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd >= 0) {
        return fd;
    }
    s_errno = errno;
    fd = socket(PF_PACKET, SOCK_DGRAM, 0);
    if (fd >= 0) {
        return fd;
    }
    fd = socket(PF_INET6, SOCK_DGRAM, 0);
    if (fd >= 0) {
        return fd;
    }
    errno = s_errno;
    return -1;
}

/**
 * Enables network interface.
 * Taken from iproute2 source code.
 */
static BOOL
do_chflags(const char *ifname, __u32 flags, __u32 mask)
{
    struct ifreq ifr;
    int fd;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    fd = get_ctl_fd();
    if (fd < 0) {
        return FALSE;
    }
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        close(fd);
        return FALSE;
    }
    if (((ifr.ifr_flags ^ flags) & mask) != 0) {
        ifr.ifr_flags &= ~mask;
        ifr.ifr_flags |= mask & flags;
        if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
            close(fd);
            return FALSE;
        }
    }
    close(fd);
    return TRUE;
}

/**
 * The main entry point.
 * @param argc
 *  The number of arguments.
 * @param argv
 *  The arguments.
 */
int
main(int argc, char *argv[])
{
    /*
     * The N_PPP line discipline for communication with the pppd. Don't use
     * N_SYNC_PPP (and, consequently, do not include "sync" in the pppd's
     * configuration file), as it may result in multiple PPP frames being
     * returned by a single read() call, which leads to errors. Example:
     *
     * ppp2i4l[27836]: PPP C021 packet from 0:
     * ppp2i4l[27836]: 0000 FF 03 C0 21 01 01 00 14
     * ppp2i4l[27836]: 0008 01 04 05 F4 02 06 00 00
     * ppp2i4l[27836]: 0010 00 00 05 06 92 FE 54 2E
     * ppp2i4l[27836]: 0018 FF 03 C0 21 02 01 00 14
     * ppp2i4l[27836]: 0020 01 04 05 F4 02 06 00 00
     * ppp2i4l[27836]: 0028 00 00 05 06 5F 12 79 72
     *
     * This "packet" consists of two different PPP/HDLC frames merged together
     * into one. The remote pppd (obviously) ignores the second one:
     *
     * rcvd [LCP ConfReq id=0x1 <mru 1524> <asyncmap 0x0> <magic 0x92fe542e>]
     * ff 03 c0 21 02 01 00 14 01 04 05 f4 02 06 00 00 00 00 05 06 5f 12 79 72
     *
     * The pppoe documentation contains the following information:
     *
     * "There is an option to pppd and pppoe which enables synchronous PPP. In
     * this case, no byte-stuffing is performed. However, correct operation in
     * this mode relies on pppoe reading exactly one frame at a time from
     * standard input. (There are no frame boundary markers, so pppoe assumes
     * that it gets a complete frame for each read system call.) While this
     * seems to work on fast machines, it is not recommended, because delays in
     * scheduling in pppoe can cause serious problems."
     *
     * (http://www.linuxshowcase.org/2000/2000papers/papers/skoll/skoll_html/)
     *
     * Note that using the N_HDLC line discipline does not help, as at the
     * point of reading the data on the PTY's master terminal (that's we), the
     * data has already been merged by the slave terminal (ppp_synctty is the
     * culprit here, implementing the N_SYNC_PPP line discipline).
     */
    int disc = N_PPP;
    int chindex;
    int ippp_fd;
    int isdninfo_fd;
    int isdnctrl_fd;
    int pppd_fd;
    int rc = 0;
    unsigned long flags;
    struct sigaction sigact;

    char *devname = NULL;
    char *ifname = NULL;
    char *facility_name = NULL;
    int log_level = 0;
    BOOL client_mode = TRUE;
    int timeout = 0;

    /* initialize signal handlers */
    memset(&sigact, 0, sizeof sigact);
    sigact.sa_handler = &handle_signal;
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGHUP, &sigact, NULL);

    /*
     * process option arguments:
     * -d <level>      sets the log level to use:
     *                   0 = LOG_WARNING and higher (default)
     *                   1 = LOG_NOTICE and higher
     *                       (logs when program and connection start/stop)
     *                   2 = LOG_INFO and higher
     *                       (logs when a packet comes or goes over the wire)
     *                   3 = LOG_DEBUG and higher
     *                       (dumps each packet comes or goes over the wire)
     * -l <facility>   sets the syslog facility to use for logging
     * -s              server mode: do not dial out
     * -t <timeout>    number of seconds to wait for ISDN line to be up
     */
    ++rc;
    --argc;
    ++argv;
    while (argc > 0 && argv[0][0] == '-') {
        switch (argv[0][1]) {
        case 'd' :
            --argc;
            ++argv;
            if (argc == 0 || sscanf(argv[0], "%d", &log_level) != 1
                    || log_level < 0) {
                fprintf(stderr, "Error: -d option requires a log level >= 0\n");
                usage();
                goto out;
            }
            break;
        case 'l' :
            --argc;
            ++argv;
            if (argc == 0) {
                fprintf(stderr, "Error: -l option requires a log facility\n");
                usage();
                goto out;
            }
            else {
                facility_name = argv[0];
            }
            break;
        case 's' :
            client_mode = FALSE;
            break;
        case 't' :
            --argc;
            ++argv;
            if (argc == 0 || sscanf(argv[0], "%d", &timeout) != 1
                    || timeout < 0) {
                fprintf(stderr, "Error: -t option requires a timeout >= 0\n");
                usage();
                goto out;
            }
            break;
        default :
            fprintf(stderr, "Error: Unknown option: %s\n", argv[0]);
            usage();
            goto out;
        }

        --argc;
        ++argv;
    }

    /* process more arguments (ISDN device / ISDN network interface) */
    if (argc > 0) {
        devname = argv[0];
    }
    else {
        fprintf(stderr, "Error: ISDN device is missing\n");
        usage();
        goto out;
    }

    --argc;
    ++argv;
    if (argc > 0) {
        ifname = argv[0];
    }
    else {
        fprintf(stderr, "Error: ISDN network interface is missing\n");
        usage();
        goto out;
    }

    --argc;
    ++argv;
    if (argc > 0) {
        fprintf(stderr, "Error: Invalid argument: %s\n", argv[0]);
        goto out;
    }

    /* initialize syslog */
    ++rc;
    initsyslog(facility_name, log_level);

    /* initialize terminal by PPP-ifying the PTY master received from pppd */
    ++rc;
    if (ioctl(0, TIOCSETD, &disc) < 0) {
        syslog(LOG_ERR,
            "Unable to set line discipline to N_PPP: %s", strerror(errno));
        goto out_log;
    }
    ++rc;
    if (ioctl(0, PPPIOCGCHAN, &chindex) < 0) {
        syslog(LOG_ERR, "Could not get channel number: %s", strerror(errno));
        goto out_log;
    }
    ++rc;
    pppd_fd = open(DEV_PREFIX "ppp", O_RDWR);
    if (pppd_fd < 0) {
        syslog(LOG_ERR,
            "Could not open " DEV_PREFIX "ppp: %s", strerror(errno));
        goto out_log;
    }
    ++rc;
    if (ioctl(pppd_fd, PPPIOCATTCHAN, &chindex) < 0) {
        syslog(LOG_ERR,
            "Couldn't attach to channel %d: %s", chindex, strerror(errno));
        goto out_pppd;
    }

    /* open ISDN device and enable network processing */
    ++rc;
    ippp_fd = open(devname, O_RDWR, 0);
    if (ippp_fd < 0) {
        syslog(LOG_ERR, "Failed to open %s: %s", devname, strerror(errno));
        goto out_pppd;
    }
    ++rc;
    if (ioctl(ippp_fd, PPPIOCGFLAGS, &flags) < 0) {
        syslog(LOG_ERR,
            "ioctl(PPPIOCGFLAGS) on %d: %s", ippp_fd, strerror(errno));
        goto out_ippp;
    }
    ++rc;
    flags |= SC_ENABLE_IP;
    if (ioctl(ippp_fd, PPPIOCSFLAGS, &flags) < 0) {
        syslog(LOG_ERR,
            "ioctl(PPPIOCSFLAGS) on %d: %s", ippp_fd, strerror(errno));
        goto out_ippp;
    }

    /* enable ISDN PPP debug mode if log level is at least NOTICE */
    if (log_level > 0) {
        unsigned long debug = 1;
        if (ioctl(ippp_fd, PPPIOCGDEBUG, &debug) < 0) {
            syslog(LOG_WARNING,
                "ioctl(PPPIOCGDEBUG) on %d: %s", ippp_fd, strerror(errno));
            /* ignore error */
        }
        else {
            debug |= 0x01;
            if (ioctl(ippp_fd, PPPIOCSDEBUG, &debug) < 0) {
                syslog(LOG_WARNING,
                    "ioctl(PPPIOCSDEBUG) on %d: %s", ippp_fd, strerror(errno));
                /* ignore error */
            }
        }
    }

    /* enable network interface */
    if (!do_chflags(ifname, IFF_UP, IFF_UP)) {
        syslog(LOG_ERR,
            "Failed to enable network interface %s: %s", ifname,
            strerror(errno));
        goto out_ippp;
    }

    /* open isdninfo device for ISDN link state detection */
    ++rc;
    isdninfo_fd = open(ISDNINFO_DEV, O_RDWR);
    if (isdninfo_fd < 0) {
        syslog(LOG_ERR,
            "Failed to open %s: %s", ISDNINFO_DEV, strerror(errno));
        goto out_netif;
    }

    /* open isdnctrl device */
    ++rc;
    isdnctrl_fd = open(ISDNCTRL_DEV, O_RDWR);
    if (isdnctrl_fd < 0) {
        syslog(LOG_ERR,
            "Failed to open %s: %s", ISDNCTRL_DEV, strerror(errno));
        goto out_isdninfo;
    }

    if (client_mode) {
        /*
         * dial ourselves: We cannot let the pppd call "isdnctrl dial" as a
         * connect script because "isdnctrl dial" relies on us having opened
         * the ippp device. As the connect script is executed before the pty
         * script (that's we), it is possible that we have not yet opened the
         * ippp device at the time isdnctrl asks the kernel to dial, which
         * leads to an error.
         */
        if (ioctl(isdnctrl_fd, IIOCNETDIL, ifname) < 0) {
            syslog(LOG_ERR,
                "Failed to dial %s: %s", ifname, strerror(errno));
            goto out_isdnctrl;
        }
    }

    /* run main loop */
    syslog(LOG_NOTICE, "starting (device %s, network interface %s)",
            devname, ifname);
    loop(pppd_fd, ippp_fd, isdninfo_fd, isdnctrl_fd, ifname, timeout);
    syslog(LOG_NOTICE, "exiting (device %s, network interface %s)",
            devname, ifname);

    /* cleanup and exit */
    rc = 0;

out_isdnctrl:
    close(isdnctrl_fd);
out_isdninfo:
    close(isdninfo_fd);
out_netif:
    if (!do_chflags(ifname, 0, IFF_UP)) {
        syslog(LOG_WARNING,
            "Failed to disable network interface %s: %s", ifname,
            strerror(errno));
    }
out_ippp:
    close(ippp_fd);
out_pppd:
    close(pppd_fd);
out_log:
    closelog();
out:
    return rc;
}
