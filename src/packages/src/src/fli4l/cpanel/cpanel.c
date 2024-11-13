/*****************************************************************************\
*   opt_cpanel3.6.x                                              20.11.2005   *
*                                                                             *
*   cpanel - Kontrollprogramm für die Serielle Fernsteuerung                  *
*   Copyright (C) 2000 bei Mirko Dölle, DG2FER <dg2fer@qsl.net>               *
*     modified by Georg Kainzbauer <georgkainzbauer@gmx.net>                  *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software Foundation,   *
*   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.            *
\*****************************************************************************/

#define VERINFO "opt_cpanel3.6.x"
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>

FILE *Infile;
char ScriptResult[6];
int fd = 0, LEDStatus, ScriptLEDStatus, ScriptCounter = 0, ScriptTime, SwitchTime;

typedef enum
  {
    ROT = 0, GRUEN, GELB, GRUENGELB
  } Farbe;

void help();
void PanelControl(int fd, int currstat);
void script(int fd, int currstat);
int work(int fd, int currstat);
int SetPanelLED(Farbe LEDs, int fd, int currstat);
void SetRed(int code);

int main(int argc, char** argv)
  {
    /* int oldstat = 0; */
    int currstat = 0;

    /* Parameterprüfung */
    if(argc != 4)
      {
        help();
      }

    /* Das Device öffnen */
    if((fd = open(argv[1], O_RDWR | O_NDELAY)) < 0)
      {
        printf("Fehler: Device \"%s\" kann nicht geöffnet werden.\n", argv[1]);
        return 2;
      } 

    /* Signale INT und TERM abfangen, erst Panel auf rot setzen dann exit */
    signal (SIGINT, SetRed);
    signal (SIGTERM, SetRed);

    /* Panel initialisieren: Grün */
    currstat = TIOCM_DTR;
    ioctl(fd, TIOCMSET, &currstat);

    /* Argumente einlesen */
    SwitchTime = atoi(argv[2]);
    ScriptTime = atoi(argv[3]);

    /* Panelsteuerung aufrufen */
    PanelControl(fd, currstat);

    return 0;
  }

void help()
  {
    #ifdef VERINFO
      puts(VERINFO);
    #endif
    printf("\ncpanel ermöglicht die Abfrage von 4 Tasten am seriellen Port.\n");
    printf("Beim Drücken der Tasten werden die Skripte /usr/local/cpanel/funktion1-14\n");
    printf("aufgerufen. Diese Skripte können nach belieben geändert werden.\n\n");
    printf("Aufruf:");
    printf("  cpanel <device> <switchtime> <scripttime>\n");
    exit(0);
  }

/* Panel-Ansteuerung */
void PanelControl(int fd, int currstat)
  {
    int laststat = currstat;
    int changed = 1;
    int i = 0;

    while(1)
      {
        usleep(100000);                       /* 100ms Delay */
        ioctl(fd, TIOCMGET, &currstat);       /* Schnittstellenstatus lesen */
        currstat &= ~(TIOCM_DTR | TIOCM_RTS); /* LED-Status abschneiden */

        /* Scriptabfrage aufrufen */
        script(fd, currstat);

        if(currstat != laststat)
          {
            laststat = currstat ;
            i = 0;
            changed = 0;
          }
        else
          {
            if(i < SwitchTime)
              {
                i++;
              }
            else if(changed == 0)
              {
                /* 2 * Delay: Taste ist gedrückt, Aktion auslösen.  */
                /* DTR und RTS (Anzeige-LED) bleiben hierbei        */
                /* unbeachtet, werden also hier gelöscht.           */
                changed = 1;
                switch(currstat)
                  {
                    case TIOCM_RNG:
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion1 ");
                      break;
                    case TIOCM_CTS:
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion2 ");
                      break;
                    case (TIOCM_CTS | TIOCM_RNG):
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion3 ");
                      break;
                    case TIOCM_DSR:
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion4 ");
                      break;
                    case (TIOCM_DSR | TIOCM_RNG):
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion5 ");
                      break;
                    case (TIOCM_DSR | TIOCM_CTS):
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion6 ");
                      break;
                    case (TIOCM_DSR | TIOCM_CTS | TIOCM_RNG):
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion7 ");
                      break;
                    case TIOCM_CAR:
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion8 ");
                      break;
                    case (TIOCM_CAR | TIOCM_RNG):
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion9 ");
                      break;
                    case (TIOCM_CAR | TIOCM_CTS):
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion10 ");
                      break;
                    case (TIOCM_CAR | TIOCM_CTS | TIOCM_RNG):
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion11 ");
                      break;
                    case (TIOCM_CAR | TIOCM_DSR):
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion12 ");
                      break;
                    case (TIOCM_CAR | TIOCM_DSR | TIOCM_RNG):
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion13 ");
                      break;
                    case (TIOCM_CAR | TIOCM_DSR | TIOCM_CTS):
                      work(fd, currstat);
                      system("/usr/local/cpanel/funktion14 ");
                      break;
                    default:
                      SetPanelLED(GRUEN, fd, currstat);
                  }
              }
          }
      }
  }

/* Aufruf und auswerten des Scripts */
void script(int fd, int currstat)
  {
    if(ScriptCounter < ScriptTime)
      {
        ScriptCounter++;
      }
    else
      {
        ScriptCounter = 0;
        system("/usr/local/cpanel/script");
        Infile = fopen("/var/run/cpanel.status","rb+");
        fscanf(Infile, "%s", ScriptResult);
        fclose(Infile);
        if(strcmp("on",ScriptResult)==0) ScriptLEDStatus=1;
        if(strcmp("off",ScriptResult)==0) ScriptLEDStatus=2;
        if(strcmp("blink",ScriptResult)==0) ScriptLEDStatus=3;
        switch (ScriptLEDStatus)
          {
            case 1:
              SetPanelLED(GRUEN | GELB, fd, currstat);
              break;
            case 2:
              SetPanelLED(GRUEN, fd, currstat);
              break;
            case 3:
              if((LEDStatus==2) || (LEDStatus==4))
                {
                  SetPanelLED(GRUEN | GELB, fd, currstat);
                }
              else
                {
                  SetPanelLED(GRUEN, fd, currstat);
                }
              break;
            default:
              printf("FEHLER - Statusfile konnte nicht ausgewertet werden");
              SetPanelLED(GRUEN, fd, currstat);
              break;
          }
      }
  }

/* LED-Ausgabe: In Bearbeitung */
int work(int fd, int currstat)
  {
    int i;
    int temp;
    temp = LEDStatus;

    for (i=0; i<3; i++)
      {
        usleep(300000);
        if((temp==3) || (temp==5))
          {
            SetPanelLED(ROT | GELB, fd, currstat);
          }
        else
          {
            SetPanelLED(ROT, fd, currstat);
          }
        usleep(300000);
        if((temp==3) || (temp==5))
          {
            SetPanelLED(GRUEN | GELB, fd, currstat);
          }
        else
          {
            SetPanelLED(GRUEN, fd, currstat);
          }
      }
    LEDStatus = temp;
    return currstat;
  }

/* LED-Ausgabe: Ausgabe der LED-Zustände */
int SetPanelLED(Farbe LEDs, int fd, int currstat)
  {
    if((LEDs & GELB) != 0)
      {
        LEDStatus = 1;
        currstat |= TIOCM_RTS;
      }
    else
      {
        LEDStatus = 0;
        currstat &= ~TIOCM_RTS;
      }
    if((LEDs & GRUEN) != 0)
      {
        LEDStatus += 2;
        currstat |= TIOCM_DTR;
      }
    else
      {
        LEDStatus += 4;
        currstat &= ~TIOCM_DTR;
    }
    ioctl(fd, TIOCMSET, &currstat);
    return currstat;
  }

/* Exit-Sequenz: Panel rot setzen und exit */
void SetRed(int code)
  {
    int status = 0;
    ioctl(fd, TIOCMSET, &status);
    exit(0);
  }
