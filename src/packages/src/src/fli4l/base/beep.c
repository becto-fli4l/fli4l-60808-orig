/*----------------------------------------------------------------------------
 *  beep.c - send beep to console
 *
 *  Usage: beep [FREQUENCY DURATION] [... ...] (parameters must not be zero) 
 *
 *  Copyright (c) 2000-2001 - Frank Meyer <frank@fli4l.de>
 *  Copyright (c) 2001-2016 - fli4l-Team <team@fli4l.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Creation:	    05.08.2004  hs
 *  Last Update:    15.12.2004  hh
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

int main (int argc, char ** argv)
{
    int i,dauer,ton;

    if ( argc==1 )
    {
      argc=3;
      argv[1]="750";
      argv[2]="120";
    }

    if ( (argc > 2) && ( ((argc-1) % 2) == 0) )                      // mehr als 2 Parameter und in Paaren?
    {
      int fd = open("/dev/tty1",0);
      if (fd)
      {

        for (i = 1; i < argc ; i = i + 2 )                           // alle Parameter abfragen
        {
          ton=atoi(argv[i]);                                         // char in int umwandeln
          dauer=atoi(argv[i+1]);                                     // char in int umwandeln

          if ( (ton != 0) && (dauer != 0) )                          // Weder Dauer noch Tonhöhe ist 0?
          {
            ioctl(fd,KDMKTONE, (dauer << 16) + 1193180 / ton);       // Ton ausgeben
            usleep (dauer * 1010);
          }
          else
          {
            // Hinweis ausgeben
            fprintf (stderr, "Usage: %s [FREQUENCY DURATION] [... ...] (parameters must not be zero)\n", argv[0]);
            close(fd);
            exit(1);
          }

        }

        close(fd);
      }
    }
    else
    {
      // Hinweis ausgeben
      fprintf (stderr, "Usage: %s [FREQUENCY DURATION] [... ...] (parameters must not be zero)\n", argv[0]);
      exit(1);
    }
    exit(0);
}
