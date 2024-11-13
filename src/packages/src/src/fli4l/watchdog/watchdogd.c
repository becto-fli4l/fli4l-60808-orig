/*
 * Simple Watchdog-Daemon with signal-Handling and syslog messaging
 * You can configure the trigger-interval, the device-name and
 * the reset-timeout.
 * Not every watchdog-driver allows the programming of the reset-timeout
 * so please be carefull about this option
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 *
 * I don't accept liability nor provide warranty for this Software.
 * This material is provided "AS-IS" and at no charge.
 *
 * Version History
 *
 * 0.1     rresch    Initial Release
 * 0.2     rresch    Fixed several Bugs
 * 0.3     rresch    Add debug-logging
 *
 *
 * rresch  (c) 2006 Robert Resch <rresch@gmx.de>
 *
 */

#include <stdlib.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <syslog.h>
#define version "0.3"

volatile int shutdown_flag=0;
volatile int wd_trigger_enable=1;

/* Set Watchdog-Clean-Exit-Flag */
static void handlerterm(int signum)
{
        shutdown_flag = 1;
}

/* Toggle Watchdog-Disable Flag */
static void handlerhup(int signum)
{
        wd_trigger_enable=!wd_trigger_enable;
}

int main(int argc, const char *argv[])
{
  /* Signal Handler for SIGTERM */
  struct sigaction sa1;
  /* Signal Handler for SIGHUP */
  struct sigaction sa2;
  /* Structure for holding some information about the watchdog we found */
  struct watchdog_info ident;
  /* File-Descriptor for Watchdog */
  int fd;
  /* Temporary used File-Descriptor for configuring wd1100 Watchdog */
  int fd1;
  /* Default-Sleeptime */
  int sleeptime=1;
  /* Reset-time 0=don't change */
  int resettime=0;
  /* Try configuring wd1100 special files in /proc/... */
  int wd1100hack=0;
  /* Set if wd1100 is auto-detected */
  int wd1100detected=0;
  /* set to one if watchdog-ident was ok */
  int wdidentok=0;
  /* Flag signalling that the help-message should be printed */
  int help=0;
  /* Counter for universal usage */
  int i;
  /* Buffer used for converting a integer to a ascii value */
  char buf[16];
  /* base-name of binary */
  char *progname;
  /* Device for watchdog */
  char *wd_devicename="/dev/watchdog";
  /* this 2 Variables are only used for Unicode-Compatibility */
  const char *zero="0\n";
  const char *one="1\n";
  /* return-value */
  int retval=0;
  /* log each trigger */
  int wd_debug_log=0;

  /* extract programm-binary-name from full-path in argv[0] */
  progname=(char *) argv[0];
  for (i=0;argv[0][i];i++)
  {
    if (argv[0][i] == '/')
    {
      progname=(char *) argv[0]+i+1;
    }
  }

  /* parse commandline for parameters */
  for (i=1;i<argc;i++)
  {
    /* Parameter trigger-interval */
    if (!strncmp("-t",argv[i],2) && i+1<argc )
    {
      i++;
      if (atoi(argv[i])>0)
      {
        sleeptime=atoi(argv[i]);
      }
      else
      {
        printf("Invalid trigger-interval\n");
        help=1;
      }
    }
    /* Parameter reset-timeout */
    if (!strncmp("-r",argv[i],2) && i+1<argc )
    {
      i++;
      if (atoi(argv[i])>0)
      {
        resettime=atoi(argv[i]);
      }
      else
      {
        printf("Invalid reset-timout\n");
        help=1;
      }
    }
    /* Parameter Device */
    if (!strncmp("-d",argv[i],2) && i+1<argc )
    {
      i++;
      wd_devicename=(char *) argv[i];
    }
    /* Manually activate wd1100 Hack */
    /* This watchdog doesn't implement all IOCTL but is configured by /proc... Filesystem */
    if (!strncmp("-s",argv[i],2))
    {
      wd1100hack=1;
    }
    if (!strncmp("-D",argv[i],2))
    {
      wd_debug_log=1;
    }
    /* Help Message */
    if (!strncmp("--help",argv[i],6) || !strncmp("--ver",argv[i],5))
    {
      help = 1;
    }
  }

  /* Reset-Timeout must be greater than trigger interval */
  if (resettime)
  {
    if (resettime <= sleeptime)
    {
      printf("reset-timeout may not be less than or equal to trigger-interval\n");
      help=1;
    }
  }

  /* Print help and exit */
  if (help)
  {
    printf("Watchdogd Version %s (c) Robert Resch <rresch@gmx.de>\n",version);
    printf("Usage: %s [options]\n",progname);
    printf("Options:\n");
    printf("  -t <n>     trigger the watchdog every <n> seconds\n");
    printf("             default: 1\n");
    printf("  -d <dev>   use <dev> as watchdog-device\n");
    printf("             default: /dev/watchdog\n");
    printf("  -r <n>     set watchdog-reset timeout to <n> seconds\n");
    printf("             default: use watchdog-driver default\n");
    printf("  -s         configure watchdog-driver via /proc/sys/dev/wd/*\n");
    printf("  -D         Debug-log every Watchdog-Timer Reset\n");
    printf("  --help\n  --version  this text\n");
    exit(0);
  }

  /* prepare signal handlers */
  sa1.sa_handler = handlerterm;
  sa2.sa_handler = handlerhup;
  sigemptyset(&sa1.sa_mask);
  sigemptyset(&sa2.sa_mask);
  sa1.sa_flags = SA_RESTART;
  sa2.sa_flags = SA_RESTART;
  
  /* open Syslog */
  openlog(progname, LOG_PID|LOG_CONS , LOG_DAEMON);

  /* install signal handlers and start watchdog if successul */
  if (sigaction(SIGTERM, &sa1, NULL) != -1 && sigaction(SIGHUP, &sa2, NULL) != -1)
  {
    /* we can only trigger a existing watchdog */
    if ((fd=open(wd_devicename,O_WRONLY)) != -1)
    {
      /* write start-message to syslog */
      syslog(LOG_INFO,"using watchdog-device: %s", wd_devicename);
      /* IOCTL to get ident Information and other info from watchdog-device */
      if (ioctl(fd, WDIOC_GETSUPPORT, &ident) != -1)
      {
        syslog(LOG_INFO,"found watchdog: %s", ident.identity);
        wdidentok=1;
        /* If watchdog has this identity then we need to activate the wd1100 hack */
        if (!strncmp("NSC SC1x00 WD",ident.identity,13))
        {
          syslog(LOG_INFO,"enabling special support for watchdog-driver");
          wd1100hack=1;
          wd1100detected=1;
        }
      }
      else
      {
        syslog(LOG_WARNING,"could not detect watchdog");
      }

      /* Only set timeout if >0 */
      if (resettime)
      {
        i=0;
        if (wdidentok)
        {
          if (ident.options&WDIOF_SETTIMEOUT)
          {
            ioctl(fd, WDIOC_SETTIMEOUT, &resettime);
            i=1;
          }
        }
        if (wd1100hack)
        {
          if ((fd1=open("/proc/sys/dev/wd/timeout",O_WRONLY)) != -1)
          {
            sprintf(buf,"%d\n",resettime);
            write (fd1,buf,strlen(buf));
            close(fd1);
            i=1;
          }
        }
        if (i)
        {
          syslog(LOG_INFO,"watchdog timeout set to %d second(s)", resettime);
        }
        else
        {
          syslog(LOG_WARNING,"could not set watchdog timeout");
        }
      }
      /* Set graceful Flag to 0 -> abnormal termination of this program triggers reset */
      if (wd1100hack)
      {
        if ((fd1=open("/proc/sys/dev/wd/graceful",O_WRONLY)) != -1)
        {
          write (fd1,zero,strlen(zero));
          close(fd1);
        }
      }
      syslog(LOG_INFO,"triggering watchdog every %d second(s)",sleeptime);

      /* daemon() -> detach from console */
      if (daemon(1,1) == -1)
      {
        syslog(LOG_ERR, "cannot detach from console");
        shutdown_flag=1;
   	retval=3;
      }

      /* Main Watchdog-Loop */
      while (!shutdown_flag)
      {
        /* watchdog-triggering disabled? */
        if (wd_trigger_enable)
        {
          /* Log trigger-event if requested */
          if (wd_debug_log>0)
          {
            if (wd_debug_log==1) syslog(LOG_INFO,"triggering watchdog device");
            if (wd_debug_log==2)
            {
              syslog(LOG_INFO,"triggering watchdog device ");
              wd_debug_log=0;
            }
              wd_debug_log++;
          }
          /* Reset watchdog-timer */
          write(fd,"\0",1);
          fsync(fd);
          for (i=0;i<sleeptime&&!shutdown_flag&&wd_trigger_enable;i++) sleep(1);
        }
        else
        {
          syslog(LOG_WARNING,"disabled watchdog triggering - expect hw-reset");
          /* do nothing for one second */
          /* and wait for reset or another SIGHUP to re-enable */
          while (!wd_trigger_enable && !shutdown_flag) sleep(1);
          syslog(LOG_WARNING,"re-enabled watchdog triggering");
        }
      }
      /* End of Main Watchdog-Loop */
      
      /* try a clean shutdown */
      if (wd1100detected)
      {
        syslog(LOG_WARNING,"disabling watchdog");
      }
      else
      {
        syslog(LOG_WARNING,"disabling watchdog - some hardware doesn't allow to be disabled!");
      }
      /* Set graceful-flag to '1' -> disable Watchdog after close() */
      if (wd1100hack)
      {
        if ((fd1=open("/proc/sys/dev/wd/graceful",O_WRONLY)) != -1)
        {
          write (fd1,one,strlen(one));
          close(fd1);
        }
      }
      /* send ioctl to safely disable watchdog card */
      ioctl(fd, WDIOC_SETOPTIONS, WDIOS_DISABLECARD);
      /* write 'magic' to watchdog-device - most watchdog-drivers need it to disable */
      /* needs to be the last Character before close() */
      write(fd,"V",1);
      close(fd);
      syslog(LOG_INFO,"exiting");
    }
    else
    {
      syslog(LOG_ERR,"cannot open Watchdog-device: %s - exiting\n",wd_devicename);
      retval=1;
    }
  }
  else
  {
    syslog(LOG_ERR,"cannot install signal Handlers - exiting");
    retval=2;
  }
  closelog();
  return (retval);
}
