/*-----------------------------------------------------------------------------
 *  isdn_rate.c - show isdn rates and other fli4l stuff on LCD
 *
 *  usage: isdn_rate [-port imond-port]
 *                   [-telmond-port telmond-port]
 *                   [-type hitachi|matrix-orbital|tty]
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  History:
 *
 *  Juergen Bauer <jb@idowa.de>         creation                        05.08.2000
 *  Frank Meyer   <FrankMeyer@gmx.li>   imond client interface          20.11.2000
 *  Ulf Lanz      <u.lanz@odn.de>       variable output formats         10.12.2000
 *  Frank Meyer   <FrankMeyer@gmx.li>   matrix-orbital support          22.12.2000
 *  Frank Meyer   <FrankMeyer@gmx.li>   some new data types             02.02.2001
 *  Frank Meyer   <FrankMeyer@gmx.li>   telmond client interface        21.02.2001
 *  Frank Meyer   <FrankMeyer@gmx.li>   shorter bars                    04.03.2001
 *  Roland Hopfer <ronni@uni.de>        new data type onlinetime        30.05.2001
 *  Roland Hopfer <ronni@uni.de>        new data type quantity & CPU    03.06.2001
 *                                      and some output modifications
 *  Roland Hopfer <ronni@uni.de>        change output by online/offline 16.06.2001
 *  Roland Hopfer <ronni@uni.de>        new data type fixed texts       17.06.2001
 *  Roland Hopfer <ronni@uni.de>        new data type variable texts    01.07.2001
 *  Robert Resch  <rresch@gmx.de>       fixed Problem with new lcd.o    31.08.2001
 *  Roland Hopfer <ronni@uni.de>        fixed 2GB barrier at quantity   25.11.2001
 *                                      increased display types to 32
 *  Roland Hopfer <ronni@uni.de>        new data type uptime            02.12.2001
 *  Gernot Miksch <ibgm@gmx.de>         new data types LCD_DSL_SPEED_IN
 *                                      and -_OUT for scaling the bars  08.05.2004
 *  Gernot Miksch <ibgm@gmx.de>         Initialize filled bargraph-
 *                                      segments with character 0x04
 *                                      instead of 0xff                 09.12.2004
 *  Gernot Miksch <ibgm@gmx.de>         repair of displaying user-text
 *                                      lcd-config file as parameter
 *                                      ip-address as parameter
 *                                      possible output to tty-device   31.12.2005
 *  Gernot Miksch <ibgm@gmx.de>         repair of loading bargraph      2006-08-31
 *                                      
 *-----------------------------------------------------------------------------
 *  Credits:
 *
 *    original source proclcd by
 *      Nils Faerber (http://www.home.unix-ag.org/nils/lcd.html)
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <linux/errno.h>
#include <sys/stat.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>                            /* decl of inet_addr()      */
#include <sys/socket.h>

#define TRUE                        1
#define FALSE                       0

#define MAX_CHAN                    2             /* max. # of isdn channels  */
#define DELAY_SECONDS               1             /* output delay             */

#define LCD_RESET                   "\033R"       /* ESC R       Reset        */
#define LCD_CURSOR_OFF              "\033K"       /* ESC K       Cursor Off   */
#define LCD_CLEAR                   "\033c"       /* ESC c       Clear        */
#define LCD_HOME                    "\033h"       /* ESC h       Home         */
#define LCD_GOTO                    "\033g%c%c"   /* ESC g X Y   Goto         */
#define LCD_LDCHAR                  "\033%c%.8s"   /* ESC n ....  Load Char    */

#define TTY_RESET                   "\033[2J\033[H" /* ESC [c      Reset        */
#define TTY_CURSOR_OFF              ""            /*             Cursor Off   */
#define TTY_CLEAR                   "\033[2J"     /* ESC [2J     Clear        */
#define TTY_HOME                    "\033[H"      /* ESC [H      Home         */
#define TTY_GOTO                    "\033[%d;%dH" /* ESC [y;xH   Goto         */

#define MATRIX_ORBITAL_SCROLL_OFF   "\376R"       /* 0xFE R      Auto Scroll Off */
#define MATRIX_ORBITAL_CLR_HOME     "\376X"       /* 0xFE X      Clr & Home   */
#define MATRIX_ORBITAL_LIGHT        "\376B%c"     /* 0xFE B 0x00 Backlight On */
#define MATRIX_ORBITAL_CONTRAST     "\376P%c"     /* 0xFE P 0xC8 Contrast     */
#define MATRIX_ORBITAL_GOTO         "\376G%c%c"   /* 0xFE G X Y  Goto X Y     */
#define MATRIX_ORBIAL_LDCHAR        "\376N%s"     /* 0xFE N .... Load Char    */

const char      bar[][8] =  {{16, 16, 16, 16, 16, 16, 16, 16},
                            {24, 24, 24, 24, 24, 24, 24, 24},
                            {28, 28, 28, 28, 28, 28, 28, 28},
                            {30, 30, 30, 30, 30, 30, 30, 30},
                            {31, 31, 31, 31, 31, 31, 31, 31}};
/*const char      max[][8] =  {{16, 16, 16, 16, 16, 16, 16, 16},
                            { 8,  8,  8,  8,  8,  8,  8,  8},
                            { 4,  4,  4,  4,  4,  4,  4,  4},
                            { 2,  2,  2,  2,  2,  2,  2,  2},
                            { 1,  1,  1,  1,  1,  1,  1,  1}};
*/
static int      fd;
static int      telmond_fd;
static char     configfile[512]="/var/run/lcd.conf";              /* Name of config-file  */


/*----------------------------------------------------------------------------
 *  Types of LCD displays:
 *----------------------------------------------------------------------------
 */

#define HITACHI             1
#define MATRIX_ORBITAL      2
#define TTY                 3
static int                  lcd_type = TTY;

/*----------------------------------------------------------------------------
 *  Possible data types
 *  don't forget to increment MAX_FORMAT_TYPES below if you add a new
 *  data type!
 *----------------------------------------------------------------------------
 */

#define LOCAL_LONG_DATE     0
#define LOCAL_SHORT_DATE    1
#define LOCAL_TIME          2

#define REMOTE_LONG_DATE    3
#define REMOTE_SHORT_DATE   4
#define REMOTE_TIME         5

#define STATUS_0            6
#define STATUS_1            7
#define STATUS_P            8

#define CIRCUIT_0           9
#define CIRCUIT_1           10
#define CIRCUIT_P           11

#define IRATE_BAR           12
#define ORATE_BAR           13
#define IRATE_BAR_P         14
#define ORATE_BAR_P         15

#define IRATE_BUF           16
#define ORATE_BUF           17
#define IRATE_BUF_P         18
#define ORATE_BUF_P         19

#define CHARGE_BUF_0        20
#define CHARGE_BUF_1        21
#define CHARGE_BUF_P        22

#define IP_ADDRESS_0        23
#define IP_ADDRESS_1        24
#define IP_ADDRESS_P        25

#define LOAD_1              26
#define LOAD_2              27

#define PHONE               28

#define ONLINE_TIME_0       29
#define ONLINE_TIME_1       30
#define ONLINE_TIME_P       31

#define QUANTITY_IN_0       32
#define QUANTITY_IN_1       33
#define QUANTITY_IN_P       34

#define QUANTITY_OUT_0      35
#define QUANTITY_OUT_1      36
#define QUANTITY_OUT_P      37

#define CPU                 38

#define TEXT                39
#define VAR_TEXT_1          40
#define VAR_TEXT_2          41
#define VAR_TEXT_3          42
#define VAR_TEXT_4          43

#define UPTIME              44

#define MAX_FORMAT_TYPES    45          /* max. number of possible types    */

static unsigned char        format_type_used[MAX_FORMAT_TYPES];

/*----------------------------------------------------------------------------
 *  Lengths of data types:
 *----------------------------------------------------------------------------
 */

#define MAX_LONG_DATE_LEN   10
#define MAX_SHORT_DATE_LEN   8
#define MAX_TIME_LEN         8
#define MAX_STATUS_LEN       7
#define MAX_CIRCUIT_LEN     16
#define MAX_RATE_BAR_LEN     8
#define MAX_RATE_BLEN        5
#define MAX_RATE_P_BLEN      5
#define MAX_LOAD_BLEN        5
#define MAX_CHARGE_BLEN      6
#define MAX_IP_ADDR_LEN     15
#define MAX_PHONE_LEN       16
#define MAX_ONLINE_TIME_LEN  8
#define MAX_QUANTITY_LEN    10
#define MAX_CPU_LEN          3
#define MAX_TEXT_LEN        80
#define MAX_UPTIME_LEN      11
#define MAX_LINE_LEN        MAX_TEXT_LEN+20

typedef struct
{
    int     type;
    int     on;
    int     xPos;
    int     yPos;
    char    text[MAX_TEXT_LEN + 1];
    int     textlen;
} Types;

#define MAXTYPES            32          /* max # of display defintion types */
static Types    tp_array[MAXTYPES];
static Types    tp_array_online[MAXTYPES];
static Types    tp_array_offline[MAXTYPES];
static int      no_of_types;
static int      no_of_types_online;
static int      no_of_types_offline;

/*----------------------------------------------------------------------------
 *  Buffers for output:
 *----------------------------------------------------------------------------
 */

static char     local_long_date[MAX_LONG_DATE_LEN + 1];     /* local date   */
static char     local_short_date[MAX_SHORT_DATE_LEN + 1];   /* local date   */
static char     local_time[MAX_TIME_LEN + 1];               /* local time   */
static char     remote_long_date[MAX_LONG_DATE_LEN + 1];    /* remote date  */
static char     remote_short_date[MAX_SHORT_DATE_LEN + 1];  /* remote date  */
static char     remote_time[MAX_TIME_LEN + 1];              /* remote time  */

static char     status[MAX_CHAN][MAX_STATUS_LEN + 1];       /* status       */
static char     status_p[MAX_STATUS_LEN + 1];               /* status pppoe */

static char     circuit[MAX_CHAN][MAX_CIRCUIT_LEN + 1];     /* circ isdn    */
static char     circuit_p[MAX_CIRCUIT_LEN + 1];             /* circ pppoe   */

static char     ip_address[MAX_CHAN][MAX_IP_ADDR_LEN + 1];  /* ip addr  */
static char     ip_address_p[MAX_IP_ADDR_LEN + 1];          /* ip addr  */

static char     irate_bar[MAX_RATE_BAR_LEN + 1];            /* input bar    */
static char     orate_bar[MAX_RATE_BAR_LEN + 1];            /* output bar   */

static char     irate_bar_p[MAX_RATE_BAR_LEN + 1];          /* input bar    */
static char     orate_bar_p[MAX_RATE_BAR_LEN + 1];          /* output bar   */

static char     irate_buf[MAX_RATE_BLEN + 1];               /* input rate   */
static char     irate_buf_p[MAX_RATE_P_BLEN + 1];           /* pppoe i rate */

static char     orate_buf[MAX_RATE_BLEN + 1];               /* output rate  */
static char     orate_buf_p[MAX_RATE_BLEN + 1];             /* pppoe o rate */

static char     charge_buf[MAX_CHAN][MAX_CHARGE_BLEN + 1];  /* charge isdn  */
static char     charge_buf_p[MAX_CHARGE_BLEN + 1];          /* charge pppoe */

static char     phone[MAX_PHONE_LEN + 1];                   /* phone number */

static char     load_1[MAX_LOAD_BLEN + 1];                  /* 1st load val */
static char     load_2[MAX_LOAD_BLEN + 1];                  /* 2nd load val */

static char online_time[MAX_CHAN][MAX_ONLINE_TIME_LEN + 1]; /* online time isdn */
static char online_time_p[MAX_ONLINE_TIME_LEN + 1];         /* online time pppoe */

static unsigned long q_in = 0;
static unsigned long q_out = 0;
static float qi = 0;
static float qo = 0;

static char quantity_in[MAX_CHAN][MAX_QUANTITY_LEN + 1];  /* quantity in isdn */
static char quantity_in_p[MAX_QUANTITY_LEN + 1];          /* quantity in pppoe */

static char quantity_out[MAX_CHAN][MAX_QUANTITY_LEN + 1];
static char quantity_out_p[MAX_QUANTITY_LEN + 1];

static char cpu[MAX_CPU_LEN + 1];                   /* CPU usage */

static float    irate;                              /* input rate isdn      */
static float    orate;                              /* output rate isdn     */
static float    irate_p;                            /* input rate pppoe     */
static float    orate_p;                            /* output rate pppoe    */
static float    irate_div;                          /* divider to normalize input-rate for bar-display*/
static float    orate_div;                          /* same for output */

static int      ostat = 0;

static char uptime[MAX_UPTIME_LEN + 1];          /* router uptime */
static long utime = 0;

/*----------------------------------------------------------------------------
 *  loadbar (float rate, int is_isdn, int is_input) - show loadbar
 *----------------------------------------------------------------------------
 */
static void loadbar (float rate, int is_isdn, int is_input)
{
    unsigned char *     bp;
    float               bar;
    float               res     = 0;
    int                 i;

    if (is_isdn)
    {
        if (is_input)
        {
            bp = (unsigned char *) irate_bar;
        }
        else
        {
            bp = (unsigned char *) orate_bar;
        }

        rate /= 2.;                                 /* normalize isdn speed */
    }
    else
    {
        if (is_input)
        {
            bp = (unsigned char *) irate_bar_p;
            rate /= irate_div; //13;                 /* normalize dsl speed  */
        }
        else
        {
            bp = (unsigned char *) orate_bar_p;
            rate /= orate_div; //2;                  /* normalize dsl speed  */
        }
    }

    if (rate >= MAX_RATE_BAR_LEN)
    {
        bar = MAX_RATE_BAR_LEN;
    }
    else
    {
        bar = rate;
    }

    for (i = 0; i < (bar - 1); i++)
    {
      if (lcd_type == TTY)
      {
        *(bp + i) = '#';
      }
      else
      {
        *(bp + i) = 0x05;
      }
    }

    res = bar - i;
    res *= 4;
    *(bp + i) = (unsigned char) res;

    if (lcd_type == TTY)
    {
     *(bp + i) = (unsigned char) '0' + res;
    }

    for (i = bar + 1; i < MAX_RATE_BAR_LEN; i++)
    {
        *(bp + i) = ' ';
    }

    if (rate >= MAX_RATE_BAR_LEN)
    {
        *(bp + MAX_RATE_BAR_LEN - 1) = '+';
    }

    return;
} /* loadbar (float rate, int is_input) */


void PrintType (int typ_no, char *text)
{
    switch (typ_no)
    {
        case LOCAL_LONG_DATE:
            fputs (local_long_date, stdout);                
            break;
        case LOCAL_SHORT_DATE:
            fputs (local_short_date, stdout);               
            break;
        case LOCAL_TIME:
            fputs (local_time, stdout);                 
            break;

        case REMOTE_LONG_DATE:
            fputs (remote_long_date, stdout);               
            break;
        case REMOTE_SHORT_DATE:
            fputs (remote_short_date, stdout);          
            break;
        case REMOTE_TIME:
            fputs (remote_time, stdout);                    
            break;

        case STATUS_0:
            printf ("%-8s", status[0]);                 
            break;
        case STATUS_1:
            printf ("%-8s", status[1]);                 
            break;
        case STATUS_P:
            printf ("%-8s", status_p);                  
            break;

        case CIRCUIT_0:
            printf ("%-16s", circuit[0]);                   
            break;
        case CIRCUIT_1:
            printf ("%-16s", circuit[1]);                   
            break;
        case CIRCUIT_P:
            printf ("%-16s", circuit_p);                    
            break;

        case IRATE_BAR:
            fputs (irate_bar, stdout);                  
            break;
        case ORATE_BAR:
            fputs (orate_bar, stdout);                  
            break;
        case IRATE_BAR_P:
            fputs (irate_bar_p, stdout);                    
            break;
        case ORATE_BAR_P:
            fputs (orate_bar_p, stdout);                    
            break;
        case IRATE_BUF:
            fputs (irate_buf, stdout);                  
            break;
        case ORATE_BUF:
            fputs (orate_buf, stdout);                  
            break;
        case IRATE_BUF_P:
            fputs (irate_buf_p, stdout);                    
            break;
        case ORATE_BUF_P:
            fputs (orate_buf_p, stdout);                    
            break;

        case CHARGE_BUF_0:
            fputs (charge_buf[0], stdout);              
            break;
        case CHARGE_BUF_1:
            fputs (charge_buf[1], stdout);              
            break;
        case CHARGE_BUF_P:
            fputs (charge_buf_p, stdout);                   
            break;

        case IP_ADDRESS_0:
            fputs (ip_address[0], stdout);              
            break;
        case IP_ADDRESS_1:
            fputs (ip_address[1], stdout);              
            break;
        case IP_ADDRESS_P:
            fputs (ip_address_p, stdout);                   
            break;

        case PHONE:
            fputs (phone, stdout);
            break;
        case LOAD_1:
            fputs (load_1, stdout);                     
            break;
        case LOAD_2:
            fputs (load_2, stdout);                     
            break;

        case ONLINE_TIME_0:
            fputs (online_time[0], stdout);
            break;
        case ONLINE_TIME_1:
            fputs (online_time[1], stdout);
            break;
        case ONLINE_TIME_P:
            fputs (online_time_p, stdout);
            break;

        case QUANTITY_IN_0:
            fputs (quantity_in[0], stdout);
            break;
        case QUANTITY_IN_1:
            fputs (quantity_in[1], stdout);
            break;
        case QUANTITY_IN_P:
            fputs (quantity_in_p, stdout);
            break;

        case QUANTITY_OUT_0:
            fputs (quantity_out[0], stdout);
            break;
        case QUANTITY_OUT_1:
            fputs (quantity_out[1], stdout);
            break;
        case QUANTITY_OUT_P:
            fputs (quantity_out_p, stdout);
            break;

        case CPU:
            printf ("%3s%%", cpu);
            break;

        case TEXT:
            fputs (text, stdout);
            break;

        case VAR_TEXT_1:
            fputs (text, stdout);
            break;
        case VAR_TEXT_2:
            fputs (text, stdout);
            break;
        case VAR_TEXT_3:
            fputs (text, stdout);
            break;
        case VAR_TEXT_4:
            fputs (text, stdout);
            break;

        case UPTIME:
            fputs (uptime, stdout);
            break;
                            
        default : fputs ("nix", stdout);
    }
}


static void lcd_init (void)
{
    switch (lcd_type)
    {
      case MATRIX_ORBITAL:
      {
        fputs (MATRIX_ORBITAL_SCROLL_OFF, stdout);
        fputs (MATRIX_ORBITAL_CLR_HOME, stdout);
        fprintf (stdout, MATRIX_ORBITAL_LIGHT, '\0');
        fprintf (stdout, MATRIX_ORBITAL_CONTRAST, 0xC8);
        break;
      }
      case HITACHI:
      {
        fputs (LCD_RESET, stdout);
        fputs (LCD_CURSOR_OFF, stdout);
        fputs (LCD_CLEAR,stdout);
        fputs (LCD_HOME,stdout);
        break;
      }
      default:
      {
        fputs (TTY_RESET, stdout);
      }
    }

    fflush (stdout);
    return;
} /* lcd_init (void) */

void GotoXY (char x, char y)
{
    switch (lcd_type)
    {
      case MATRIX_ORBITAL:
      {
        fprintf (stdout, MATRIX_ORBITAL_GOTO, x + 1, y + 1);
        break;
      }
      case HITACHI:
      {
        fprintf (stdout, LCD_GOTO, x, y);
        break;
      }
      default:
      {
        fprintf (stdout, TTY_GOTO, y+1, x+1);
      }
    }
    fflush (stdout);
    return;
} /* GotoXY (char x, char y) */


static void lcd_clear (void)
{
    switch (lcd_type)
    {
      case MATRIX_ORBITAL:
      {
        fputs (MATRIX_ORBITAL_CLR_HOME, stdout);
        break;
      }
      case HITACHI:
      {
        GotoXY (0,254); /* Address both Display's */
        fputs (LCD_HOME, stdout);
        fputs (LCD_CLEAR, stdout);
        break;
      }
      default:
      {
        fputs (TTY_HOME, stdout);
        fputs (TTY_CLEAR, stdout);
      }
    }
    fflush (stdout);
    return;
} /* lcd_clear (void) */


/*----------------------------------------------------------------------------
 * The Function GetLCDSettings reads a configfile called lcd.conf.
 * First it looks for the number of specified types to display.
 * Then it takes the values of the type to display an after that it
 * reads the position where to display.
 * All the values are stored in a array of structures called tp_array.
 *----------------------------------------------------------------------------
 */
void GetLCDSettings(void)
{
    FILE * config_file;
    char        line[MAX_LINE_LEN+1];
    char        format[32];
    int         searching;
    int         i,j;
    int         type_no;
    int         type_x;
    int         type_y;

    config_file = fopen (configfile,"r");

    if (! config_file)
    {
        if (strlen(configfile)<502)
        {
          perror (strcat(configfile," not found"));
        }
        else
        {
          strcat(&configfile[10],"....");
          perror (strcat(&configfile[10]," not found"));
        }
        exit (1);
    }

    /*********************************************/
    /*    types to display online and offline    */
    /*********************************************/
    
    irate_div=13.;
    orate_div=2.;

    /* first scan for number of types to display */
    no_of_types = 0;
    searching = TRUE;

    while (searching)
    {
        if ( ! fgets(line, MAX_LINE_LEN, config_file) )
        {
            searching = FALSE;
        }

        if (sscanf( line, "N_LCD_TYPES='%d'", &no_of_types))
        {
            searching = FALSE;
        }
    }

    if (no_of_types > 0)
    {
        if (no_of_types > MAXTYPES)
        {
            no_of_types = MAXTYPES;
        }
    
        if (no_of_types < 1)
        {
            perror ("N_LCD_TYPES set to value lower than 1");
            //exit (1);
        }
    
    
        for (i = 1; i <= no_of_types; i++)
        {
            /* scanning for required types */
            fseek ( config_file, 0, SEEK_SET);      // go to beginning of file
            sprintf (format,"LCD_TYPE_%d='%%d'",i);
            searching = TRUE;
    
            while (searching)
            {
                if (! fgets (line, MAX_LINE_LEN, config_file))
                {
                    searching = FALSE;
                }
    
                if (sscanf (line, format, &type_no))
                {
                    searching = FALSE;
                    tp_array[i].type = type_no;
                    tp_array[i].on = TRUE;
    
                    if (type_no < MAX_FORMAT_TYPES) // for optimization of
                    {                               // communication with router
                        format_type_used[type_no] = TRUE;
                    }
               }
            }
    
            /* scanning for required type positions */
            fseek( config_file, 0, SEEK_SET);       // go to beginning of file
            sprintf(format,"LCD_POS_%d='%%d,%%d'",i);
            searching = TRUE;
    
    
            while (searching)
            {
                if ( ! fgets (line, MAX_LINE_LEN, config_file))
                {
                    searching = FALSE;
                }
    
                if (sscanf (line, format, &type_x, &type_y))
                {
                    searching = FALSE;
                    tp_array[i].xPos = type_x;
                    tp_array[i].yPos = type_y;
                }
            }

            /* scanning for text */
            fseek( config_file, 0, SEEK_SET);       // go to beginning of file
            sprintf(format,"LCD_TEXT_%d='",i);
            searching = TRUE;
    
            while (searching)
            {
                if ( ! fgets (line, MAX_LINE_LEN, config_file))
                {
                    searching = FALSE;
                }
   
                if (strstr(line, format))
                {
                    searching = FALSE;
                    char* s=strchr(line,0x27);
                    char* e=strchr(s+1,0x27);
                    *e='\0';
                    strcpy (tp_array[i].text, s+1);
                }
            }

            /* scanning for speed */
            fseek( config_file, 0, SEEK_SET);       // go to beginning of file
            searching = TRUE;
    
            while (searching)
            {
                if ( ! fgets (line, MAX_LINE_LEN, config_file))
                {
                    searching = FALSE;
                }
   
                if (sscanf (line, "LCD_DSL_SPEED_IN='%d'", &j))
                {
                    searching = FALSE;
                    irate_div=(float)j/(8*MAX_RATE_BAR_LEN);      // calculate divider 
                }
            }

            fseek( config_file, 0, SEEK_SET);       // go to beginning of file
            searching = TRUE;
    
            while (searching)
            {
                if ( ! fgets (line, MAX_LINE_LEN, config_file))
                {
                    searching = FALSE;
                }
   
                if (sscanf (line, "LCD_DSL_SPEED_OUT='%d'", &j))
                {
                    searching = FALSE;
                    orate_div=(float)j/(8*MAX_RATE_BAR_LEN);
                }
            }
        }
    }
    /*********************************************/
    /*     types to display only when online     */
    /*********************************************/

    fseek( config_file, 0, SEEK_SET);   // go to beginning of file
    
    /* first scan for number of types to display */
    no_of_types_online = 0;
    searching = TRUE;

    while (searching)
    {
        if ( ! fgets(line, MAX_LINE_LEN, config_file) )
        {
            searching = FALSE;
        }

        if (sscanf( line, "N_LCD_TYPES_ONLINE='%d'", &no_of_types_online))
        {
            searching = FALSE;
        }
    }

    if (no_of_types_online > 0)
    {
        if (no_of_types_online > MAXTYPES)
        {
            no_of_types_online = MAXTYPES;
        }
    
        if (no_of_types_online < 1)
        {
             perror ("N_LCD_TYPES_ONLINE set to value lower than 1");
             //exit (1);
        }
    
    
        for (i = 1; i <= no_of_types_online; i++)
        {
            /* scanning for required types */
            fseek ( config_file, 0, SEEK_SET);      // go to beginning of file
            sprintf (format,"LCD_TYPE_ONLINE_%d='%%d'",i);
            searching = TRUE;
    
            while (searching)
            {
                if (! fgets (line, MAX_LINE_LEN, config_file))
                {
                    searching = FALSE;
                }
    
                if (sscanf (line, format, &type_no))
                {
                    searching = FALSE;
                    tp_array_online[i].type = type_no;
                    tp_array_online[i].on = TRUE;
    
                    if (type_no < MAX_FORMAT_TYPES) // for optimization of
                    {                               // communication with router
                        format_type_used[type_no] = TRUE;
                    }
                 }
            }
    
            /* scanning for required type positions */
            fseek( config_file, 0, SEEK_SET);       // go to beginning of file
            sprintf(format,"LCD_POS_ONLINE_%d='%%d,%%d'",i);
            searching = TRUE;
    
    
            while (searching)
            {
                if ( ! fgets (line, MAX_LINE_LEN, config_file))
                {
                    searching = FALSE;
                }
    
                if (sscanf (line, format, &type_x, &type_y))
                {
                    searching = FALSE;
                    tp_array_online[i].xPos = type_x;
                    tp_array_online[i].yPos = type_y;
                }
            }

            /* scanning for text */
            fseek( config_file, 0, SEEK_SET);       // go to beginning of file
            sprintf(format,"LCD_TEXT_ONLINE_%d='",i);
            searching = TRUE;
    
            while (searching)
            {
                if ( ! fgets (line, MAX_LINE_LEN, config_file))
                {
                    searching = FALSE;
                }
   
                if (strstr(line, format))
                {
                    searching = FALSE;
                    char* s=strchr(line,0x27);
                    char* e=strchr(s+1,0x27);
                    *e='\0';
                    strcpy (tp_array_online[i].text, s+1);
                }
            }
        }
    }
    
    /*********************************************/
    /*     types to display only when offline    */
    /*********************************************/

    fseek( config_file, 0, SEEK_SET);   // go to beginning of file
    
    /* first scan for number of types to display */
    no_of_types_offline = 0;
    searching = TRUE;

    while (searching)
    {
        if ( ! fgets(line, MAX_LINE_LEN, config_file) )
        {
            searching = FALSE;
        }

        if (sscanf( line, "N_LCD_TYPES_OFFLINE='%d'", &no_of_types_offline))
        {
            searching = FALSE;
        }
    }

    if (no_of_types_offline > 0)
    {
        if (no_of_types_offline > MAXTYPES)
        {
            no_of_types_offline = MAXTYPES;
        }
    
        if (no_of_types_offline < 1)
        {
            perror ("N_LCD_TYPES_OFFLINE set to value lower than 1");
            //exit (1);
        }
    
    
        for (i = 1; i <= no_of_types_offline; i++)
        {
            /* scanning for required types */
            fseek ( config_file, 0, SEEK_SET);      // go to beginning of file
            sprintf (format,"LCD_TYPE_OFFLINE_%d='%%d'",i);
            searching = TRUE;
    
            while (searching)
            {
                if (! fgets (line, MAX_LINE_LEN, config_file))
                {
                    searching = FALSE;
                }
    
                if (sscanf (line, format, &type_no))
                {
                    searching = FALSE;
                    tp_array_offline[i].type = type_no;
                    tp_array_offline[i].on = TRUE;
    
                    if (type_no < MAX_FORMAT_TYPES) // for optimization of
                    {                               // communication with router
                        format_type_used[type_no] = TRUE;
                    }
               }
            }
    
            /* scanning for required type positions */
            fseek( config_file, 0, SEEK_SET);       // go to beginning of file
            sprintf(format,"LCD_POS_OFFLINE_%d='%%d,%%d'",i);
            searching = TRUE;
    
    
            while (searching)
            {
                if ( ! fgets (line, MAX_LINE_LEN, config_file))
                {
                    searching = FALSE;
                }
    
                if (sscanf (line, format, &type_x, &type_y))
                {
                    searching = FALSE;
                    tp_array_offline[i].xPos = type_x;
                    tp_array_offline[i].yPos = type_y;
                }
            }

            /* scanning for text */
            fseek( config_file, 0, SEEK_SET);       // go to beginning of file
            sprintf(format,"LCD_TEXT_OFFLINE_%d='",i);
            searching = TRUE;
    
            while (searching)
            {
                if ( ! fgets (line, MAX_LINE_LEN, config_file))
                {
                    searching = FALSE;
                }
   
                if (strstr(line, format))
                {
                    searching = FALSE;
                    char* s=strchr(line,0x27);
                    char* e=strchr(s+1,0x27);
                    *e='\0';
                    strcpy (tp_array_offline[i].text, s+1);
                }
            }
        }
    }
    
    fclose (config_file);
}


/*----------------------------------------------------------------------------
 *  flush_display (void)                    - output data on LCD
 *----------------------------------------------------------------------------
 */
 
static void flush_display (void)
{
    int         i,j;
    char  *     line;

    line = 0;
    //lcd_clear ();

    if (! strcmp (status[0], "Online") || 
        ! strcmp (status[1], "Online") ||
        ! strcmp (status_p, "Online"))
    {
        if (ostat == 0)
        {
            lcd_clear ();
            ostat = 1;
        }
        for (i = 1; i <= no_of_types_online; i++)
        {
            if (tp_array_online[i].type >= 40 &&
                tp_array_online[i].type <= 43)
            {
                GotoXY (tp_array_online[i].xPos, tp_array_online[i].yPos);
                for (j = 1; j <= tp_array_online[i].textlen; j++)
                {
                    printf (" ");
                }
                tp_array_online[i].textlen = strlen(tp_array_online[i].text);
            }
            GotoXY (tp_array_online[i].xPos, tp_array_online[i].yPos);
            PrintType (tp_array_online[i].type, tp_array_online[i].text);
            fflush (stdout);
        }
    }
    else
    {
      if (ostat == 1)
      {
        lcd_clear ();
        ostat = 0;
      }
    }
    
    if (ostat == 0)
    {
        if (ostat == 1)
        {
            lcd_clear ();
            ostat = 0;
        }
        for (i = 1; i <= no_of_types_offline; i++)
        {
            if (tp_array_offline[i].type >= 40 &&
                tp_array_offline[i].type <= 43)
            {
                GotoXY (tp_array_offline[i].xPos, tp_array_offline[i].yPos);
                for (j = 1; j <= tp_array_offline[i].textlen; j++)
                {
                    printf (" ");
                }
                tp_array_offline[i].textlen = strlen(tp_array_offline[i].text);
            }
            GotoXY (tp_array_offline[i].xPos, tp_array_offline[i].yPos);
            PrintType (tp_array_offline[i].type, tp_array_offline[i].text);
            fflush (stdout);
        }
    }
    
    for (i = 1; i <= no_of_types; i++)
    {
        if (tp_array[i].type >= 40 &&
            tp_array[i].type <= 43)
        {
                GotoXY (tp_array[i].xPos, tp_array[i].yPos);
                for (j = 1; j <= tp_array[i].textlen; j++)
                {
                    printf (" ");
                }
                tp_array[i].textlen = strlen(tp_array[i].text);
        }
        GotoXY (tp_array[i].xPos, tp_array[i].yPos);
        PrintType (tp_array[i].type, tp_array[i].text);
        fflush (stdout);
    }

    return;
} /* flush_display (void) */


static void flush_display1 (void)
{
    int         i;
    char  *     line;

    line = 0;

    for (i = 1; i <= no_of_types; i++)
    {
        if (tp_array[i].type == LOCAL_TIME)
        {
          GotoXY (tp_array[i].xPos+5, tp_array[i].yPos);
          printf(" ");
          fflush (stdout);
        }
    }

    if (! strcmp (status[0], "Online") ||
        ! strcmp (status[1], "Online") ||
        ! strcmp (status_p, "Online"))
    {
        for (i = 1; i <= no_of_types_online; i++)
        {
            if (tp_array_online[i].type == LOCAL_TIME)
            {
                GotoXY (tp_array_online[i].xPos+5, tp_array_online[i].yPos);
                printf(" ");
                fflush (stdout);
            }
        }
    }
    
    if (! strcmp (status[0], "Offline") &&
        ! strcmp (status[1], "Offline") &&
        ! strcmp (status_p, "Offline"))
    {
        for (i = 1; i <= no_of_types_offline; i++)
        {
            if (tp_array_offline[i].type == LOCAL_TIME)
            {
                GotoXY (tp_array_offline[i].xPos+5, tp_array_offline[i].yPos);
                printf(" ");
                fflush (stdout);
            }
        }
    }

    return;
} /* flush_display1 (void) */



/*----------------------------------------------------------------------------
 *  service_connect (host_name, port)       - connect to tcp-service
 *----------------------------------------------------------------------------
 */
static int service_connect (char * host_name, int port)
{
    struct sockaddr_in  addr;
    struct hostent *    host_p;
    int                 fd;
    int                 opt = 1;

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
    {                                           /* open socket              */
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

    return (fd);
} /* service_connect (char * host_name, int port) */


/*----------------------------------------------------------------------------
 *  send_command (int fd, char * str)       - send command to imond
 *----------------------------------------------------------------------------
 */
static void send_command (int fd, char * str)
{
    char    buf[256];
    int     len = strlen (str);

    sprintf (buf, "%s\r\n", str);
    write (fd, buf, len + 2);

    return;
} /* send_command (int fd, char * str) */


/*----------------------------------------------------------------------------
 *  get_answer (int fd)                     - get answer from imond
 *----------------------------------------------------------------------------
 */
static char* get_answer (int fd)
{
    static char buf[8192];
    int         len;

    len = read (fd, buf, 8192);

    if (len <= 0)
    {
        return ((char *) NULL);
    }

    while (len > 1 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
    {
        buf[len - 1] = '\0';
        len--;
    }

    if (! strncmp (buf, "OK ", 3))                      /* OK xxxx          */
    {
        return (buf + 3);
    }
    else if (len > 2 && ! strcmp (buf + len - 2, "OK"))
    {
        *(buf + len - 2) = '\0';
        return (buf);
    }
    else if (len == 2 && ! strcmp (buf + len - 2, "OK"))
    {
        return (buf);
    }

    return ((char *) NULL);                             /* ERR xxxx         */
} /* get_answer (int fd) */


/*----------------------------------------------------------------------------
 *  get_numerical_value (char * cmd, int arg)   - send cmd, get numval
 *----------------------------------------------------------------------------
 */
static int get_numerical_value (char * cmd, int arg)
{
    char    buf[64];
    char *  answer;
    int     rtc;

    if (arg >= 0)
    {
        sprintf (buf, "%s %d", cmd, arg);
        send_command (fd, buf);
    }
    else
    {
        send_command (fd, cmd);
    }

    answer = get_answer (fd);

    if (answer)
    {
        rtc = atoi (answer);
    }
    else
    {
        rtc = -1;
    }
    return (rtc);
} /* get_numerical_value (char * cmd, int arg) */


/*----------------------------------------------------------------------------
 *  get_value (char * cmd, int arg)         - send command, get value
 *----------------------------------------------------------------------------
 */
static char* get_value (char * cmd, int arg)
{
    char    buf[64];
    char *  answer;

    if (arg >= 0)
    {
        sprintf (buf, "%s %d", cmd, arg);
        send_command (fd, buf);
    }
    else
    {
        send_command (fd, cmd);
    }

    answer = get_answer (fd);

    if (answer)
    {
        return (answer);
    }

    return ("");
} /* get_value (char * cmd, int arg) */


/*----------------------------------------------------------------------------
 *  fill_date_time (void)                   - get date/time, fill buffers
 *----------------------------------------------------------------------------
 */
static void fill_date_time (void)
{
    struct tm * tm;
    time_t      now;

    now = time ((time_t *) NULL);

    tm = localtime (&now);

//    sprintf (local_date, "%04d/%02d/%02d",
//           1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday);

//  Auf deutsches Datum-Format geaendert Ulf Lanz  10.12.00
    sprintf (local_long_date, "%02d.%02d.%04d",
             tm->tm_mday, tm->tm_mon + 1, 1900 + tm->tm_year);
    sprintf (local_short_date, "%02d.%02d.%02d",
             tm->tm_mday, tm->tm_mon + 1, (1900 + tm->tm_year) %100);
    sprintf (local_time, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);

    return;
} /* fill_date_time (void) */


/*----------------------------------------------------------------------------
 *  usage (char * pgm_name)                 - print usage
 *----------------------------------------------------------------------------
 */
void usage (char * pgm_name)
{
    fprintf (stderr,"usage:\n%s [-ip router-ip] [-port imond-port] [-telmond-port telmond-port]\n          [-type hitachi|matrix-orbital|tty] [-config configfilename]\n",pgm_name);
    exit (1);
} /* usage (char * pgm_name) */

/*----------------------------------------------------------------------------
 *  FillText (int type, char * text)        - copy var text to struct
 *----------------------------------------------------------------------------
 */
void FillText (int type, char * text)
{
    int i;

    for (i = 1; i <= no_of_types; i++)
    {
        if (tp_array[i].type == type)
        {
            strcpy (tp_array[i].text, text);
        }
    }
    for (i = 1; i <= no_of_types_online; i++)
    {

        if (tp_array_online[i].type == type)
        {
            strcpy (tp_array_online[i].text, text);
        }
    }
    for (i = 1; i <= no_of_types_offline; i++)
    {
        if (tp_array_offline[i].type == type)
        {
            strcpy (tp_array_offline[i].text, text);
        }
    }
    
    return;
} /* FillText (int type, char * text) */

        
/*----------------------------------------------------------------------------
 *  main (void)                             - main function
 *----------------------------------------------------------------------------
 */
int main (int argc, char * argv[])
{
    int     ibytes;                                 /* input bytes          */
    int     obytes;                                 /* output bytes         */
    char    status_fmt[16];                         /* status output format */
    char    circuit_fmt[16];                        /* circ output format   */
    char    buf[32];                                /* temp buffer          */
    char    ip_addr[]={"127.000.000.001"};          /* IP-adress            */
    char *  pgm_name;                               /* name of this program */
    char *  bp;                                     /* temp pointer         */
    char *  answer;                                 /* ptr to imond answer  */
    int     n_channels;                             /* # of channels used   */
    int     pppoe_used = FALSE;                     /* flag: has pppoe      */
    int     fd_load;                                /* temp fd (loadavg)    */
    int     size;                                   /* temp variable        */
    int     loop_counter = 0;                       /* loop counter         */
    int     port = 5000;                            /* tcp/ip port of imond */
    int     telmond_port = 5001;                    /* port of telmond      */
    int     i;                                      /* temp variable        */
    struct tm * tm;
    time_t      now;
    int     sec;
    int         count = 0;



    pgm_name = argv[0];
    while (argc >= 3 && *(argv[1]) == '-')
    {
        if (! strcmp (argv[1], "-port"))
        {
            port = atoi (argv[2]);

            argc -= 2;
            argv += 2;
        }
        else if (! strcmp (argv[1], "-telmond-port"))
        {
            telmond_port = atoi (argv[2]);

            argc -= 2;
            argv += 2;
        }
        else if (! strcmp (argv[1], "-type"))
        {
            if (! strcmp (argv[2], "matrix-orbital"))
            {
                lcd_type = MATRIX_ORBITAL;
            }
            else if (! strcmp (argv[2], "hitachi"))
            {
                lcd_type = HITACHI;
            }
            else if (! strcmp (argv[2], "tty"))
            {
                lcd_type = TTY;
            }
            else
            {
                usage (pgm_name);
            }

            argc -= 2;
            argv += 2;
        }
        else if (! strcmp (argv[1], "-ip"))
        {
            if (strlen(argv[2])<=strlen(ip_addr))
            {
                strcpy(ip_addr,argv[2]);
            }
            argc -= 2;
            argv += 2;
        }
        else if (! strcmp (argv[1], "-config"))
        {
            if (strlen(argv[2])<512)
            {
              strcpy(configfile,argv[2]);
            }
            argc -= 2;
            argv += 2;
        }
        else
        {
            break;
        }
    }

    if (argc != 1)
    {
        usage (pgm_name);
    }
//printf ("Params: IP: %s\nImondPort: %d\nTelmondPort: %d\nType: %d\nConfig: %s\n",ip_addr,port,telmond_port,lcd_type,configfile);
    if ((fd = service_connect (ip_addr, port)) < 0)
    {
        exit (1);
    }
//printf ("Service connected\n");
    GetLCDSettings();
//printf ("LCD-Settings got\n");
    lcd_init ();
//printf ("LCD initialized\n");
    if (lcd_type == HITACHI)
    {
        GotoXY (0,254); /* Address both Display's */
        for (i=0;i<5;i++)
        {
          printf (LCD_LDCHAR, i+1,&bar[i][0]);
        }
        fflush (stdout);
    }

    n_channels = get_numerical_value ("channels", -1);
    pppoe_used = get_numerical_value ("pppoe", -1);
    sprintf (status_fmt, "%%-%ds", MAX_STATUS_LEN);
    sprintf (circuit_fmt, "%%-%ds", MAX_CIRCUIT_LEN);

    for (;;)
    {
        if (format_type_used[PHONE] && telmond_fd != -1)
        {
            char    telbuf[128];

            telmond_fd = service_connect (ip_addr, telmond_port);

            if (telmond_fd >= 0)
            {
                int l = read (telmond_fd, telbuf, 127);

                if (l > 0)
                {
                    char *  p1;

                    telbuf[l] = '\0';

                    p1 = strrchr (telbuf, ' ');
                    if (p1)
                    {
                        *p1 = '\0';
                        p1 = strrchr (telbuf, ' ');

                        if (p1)
                        {
                            strncpy (phone, p1 + 1, MAX_PHONE_LEN);
                        }
                    }
                }

                close (telmond_fd);
            }
        }

        fill_date_time ();

        if (format_type_used[REMOTE_LONG_DATE] ||
            format_type_used[REMOTE_SHORT_DATE] ||
            format_type_used[REMOTE_TIME])
        {
            answer = get_value ("date", -1);
            sscanf (answer, "%s %s", remote_long_date, remote_time);
            strncpy (remote_short_date, remote_long_date, 6);
            strcpy (remote_short_date + 6, remote_long_date + 8);
        }

        if (format_type_used[CPU])
        {
            answer = get_value ("cpu", -1);

            if (answer)
            {
                strcpy (cpu, answer);
            }
            else
            {
                cpu[0] = '\0';
            }
        }

        if (format_type_used[UPTIME])
        {
            answer = get_value ("uptime", -1);
            sscanf(answer, "%ld", &utime);

            if (answer)
            {
                long    h;
                int     m,s;

                h = utime/3600;
                m = ((utime-(h*3600))/60);
                s = utime-(h*3600)-(m*60);
                sprintf(uptime, "%4ld:%2.2d:%2.2d", h, m, s);
            }
            else
            {
                uptime[0] = '\0';
            }
        }
        
        irate = 0.0;
        orate = 0.0;

        for (i = 0; i < MAX_CHAN; i++)
        {
            if (i < n_channels)
            {
                answer = get_value ("status", i + 1);

                if (strlen (answer) > MAX_STATUS_LEN)
                {
                    *(answer + MAX_STATUS_LEN) = '\0';
                }

                strcpy (status[i], answer);
            }
            else
            {
                strcpy (status[i], "Offline");
            }

            if (! strcmp (status[i], "Online"))
            {
                if (format_type_used[CIRCUIT_0] ||
                    format_type_used[CIRCUIT_1])
                {
                    answer = get_value ("phone", i + 1);

                    if (strlen (answer) > MAX_CIRCUIT_LEN)
                    {
                        *(answer + MAX_CIRCUIT_LEN) = '\0';
                    }

                    sprintf (circuit[i], circuit_fmt, answer);
                }

                if (format_type_used[IRATE_BAR] ||
                    format_type_used[ORATE_BAR] ||
                    format_type_used[IRATE_BUF] ||
                    format_type_used[ORATE_BUF])
                {
                    answer = get_value ("rate", i + 1);

                    if (sscanf (answer, "%d %d", &ibytes, &obytes) == 2)
                    {
                        irate += ((float) ibytes) / 1024;
                        orate += ((float) obytes) / 1024;
                    }
                }

                if (format_type_used[IP_ADDRESS_0] ||
                    format_type_used[IP_ADDRESS_1])
                {
                    answer = get_value ("ip", i + 1);

                    if (answer)
                    {
                        strcpy (ip_address[i], answer);
                    }
                    else
                    {
                        ip_address[i][0] = '\0';
                    }
                }

                if (format_type_used[CHARGE_BUF_0] ||
                    format_type_used[CHARGE_BUF_1])
                {
                    answer = get_value ("charge", i + 1);

                    if (answer)
                    {
                        strcpy (charge_buf[i], answer);
                    }
                    else
                    {
                        charge_buf[i][0] = '\0';
                    }
                }

                if (format_type_used[ONLINE_TIME_0] ||
                    format_type_used[ONLINE_TIME_1])
                {
                    answer = get_value ("online-time", i + 1);

                    if (answer)
                    {
                        strcpy (online_time[i], answer);
                    }
                    else
                    {
                        online_time[i][0] = '\0';
                    }
                }

                if (format_type_used[QUANTITY_IN_0] ||
                    format_type_used[QUANTITY_IN_1] ||
                    format_type_used[QUANTITY_OUT_0] ||
                    format_type_used[QUANTITY_OUT_1])
                {
                    answer = get_value ("quantity", i + 1);

                    if (answer)
                    {
      long q_in_high, q_out_high;
                        sscanf(answer, "%lu %lu %lu %lu", &q_in_high, &q_in, 
             &q_out_high, &q_out);

      if (q_in_high)
      {
          q_in >>= 10;
          q_in_high <<= 22;
          q_in |= q_in_high;
          if (q_in > 1048576)
          {
        qi = q_in / 1048576.0;
        sprintf (quantity_in[i], "%6.1fGB", qi);
          }
          else
          {
        qi = q_in / 1024.0;
        sprintf (quantity_in[i], "%6.1fMB", qi);
          }
      }
      else
      {
          if (q_in > 1048576)
          {
        qi = q_in / 1048576.0;
        sprintf (quantity_in[i], "%6.1fMB", qi);
          }
          else
          {
        qi = q_in / 1024.0;
        sprintf (quantity_in[i], "%6.1fKB", qi);
          }
      }

      if (q_out_high)
      {
          q_out >>= 10;
          q_out_high <<= 22;
          q_out |= q_out_high;
          if (q_out > 1048576)
          {
        qo = q_out / 1048576.0;
        sprintf (quantity_out[i], "%6.1fGB", qo);
          }
          else
          {
        qo = q_out / 1024.0;
        sprintf (quantity_out[i], "%6.1fMB", qo);
          }
      }
      else
      {
          if (q_out > 1048576)
          {
        qo = q_out / 1048576.0;
        sprintf (quantity_out[i], "%6.1fMB", qo);
          }
          else
          {
        qo = q_out / 1024.0;
        sprintf (quantity_out[i], "%6.1fKB", qo);
          }
      }
                    }
                    else
                    {
                        quantity_in[i][0] = '\0';
                        quantity_out[i][0] = '\0';
                    }
                }
            }
            else
            {
                circuit[i][0]           = '\0';
                ip_address[i][0]        = '\0';
                charge_buf[i][0]        = '\0';
                online_time[i][0]       = '\0';
                quantity_in[i][0]       = '\0';
                quantity_out[i][0]      = '\0';
                q_in = 0;
                q_out = 0;
                qi = 0;
                qo = 0;
            }
        }

        if (pppoe_used)
        {
            answer = get_value ("status pppoe", -1);

            if (strlen (answer) > MAX_STATUS_LEN)
            {
                *(answer + MAX_STATUS_LEN) = '\0';
            }

            strcpy (status_p, answer);

            if (! strcmp (status_p, "Online"))
            {
                if (format_type_used[CIRCUIT_P])
                {
                    answer = get_value ("phone pppoe", -1);

                    if (strlen (answer) > MAX_CIRCUIT_LEN)
                    {
                        *(answer + MAX_CIRCUIT_LEN) = '\0';
                    }

                    sprintf (circuit_p, circuit_fmt, answer);
                }

                if (format_type_used[IRATE_BAR_P] ||
                    format_type_used[ORATE_BAR_P] ||
                    format_type_used[IRATE_BUF_P] ||
                    format_type_used[ORATE_BUF_P])
                {
                    answer = get_value ("rate pppoe", -1);

                    if (sscanf (answer, "%d %d", &ibytes, &obytes) == 2)
                    {
/*
*                       Original mit fester Umrechnung /1024
*                       irate_p = ((float) ibytes) / 1024;
*                       orate_p = ((float) obytes) / 1024;
*                       sprintf (irate_buf_p, "%5.1f", irate_p);
*                       sprintf (orate_buf_p, "%5.1f", orate_p);
*                       Neu mit Anzeige der Einheiten und 
*                       automatischer Umschaltung zwischen 
*                       kB/s und MB/s 
*/
                       if (ibytes > 1048576)
                       {
                          irate_p = ((float) ibytes) / 1048576;
                          sprintf (irate_buf_p, "%5.1fMB/s", irate_p);
                        }
                       else
                       {
                          irate_p = ((float) ibytes) / 1024;
                          sprintf (irate_buf_p, "%5.1fkB/s", irate_p);
                        }
                       if (obytes > 1048576)
                       {
                          orate_p = ((float) obytes) / 1048576;
                          sprintf (orate_buf_p, "%5.1fMB/s", orate_p);
                        }
                       else
                       {
                          orate_p = ((float) obytes) / 1024;
                          sprintf (orate_buf_p, "%5.1fkB/s", orate_p);
                        }
                    }
                    else
                    {
                        irate_p = 0;
                        orate_p = 0;
                    }
                }


                if (format_type_used[IP_ADDRESS_P])
                {
                    answer = get_value ("ip pppoe", -1);

                    if (answer)
                    {
                        strcpy (ip_address_p, answer);
                    }
                    else
                    {
                        ip_address_p[0] = '\0';
                    }
                }

                if (format_type_used[CHARGE_BUF_P])
                {
                    answer = get_value ("charge pppoe", -1);

                    if (answer)
                    {
                        strcpy (charge_buf_p, answer);
                    }
                    else
                    {
                        charge_buf_p[0] = '\0';
                    }
                }

                if (format_type_used[ONLINE_TIME_P])
                {
                    answer = get_value ("online-time pppoe", -1);

                    if (answer)
                    {
                        strcpy (online_time_p, answer);
                    }
                    else
                    {
                        online_time_p[0] = '\0';
                    }
                }

                if (format_type_used[QUANTITY_IN_P] ||
                    format_type_used[QUANTITY_OUT_P])
                {
                    answer = get_value ("quantity pppoe", -1);

                    if (answer)
                    {
      long q_in_high, q_out_high;
                        sscanf(answer, "%lu %lu %lu %lu", &q_in_high, &q_in, 
             &q_out_high, &q_out);

      if (q_in_high)
      {
          q_in >>= 10;
          q_in_high <<= 22;
          q_in |= q_in_high;
          if (q_in > 1048576)
          {
        qi = q_in / 1048576.0;
        sprintf (quantity_in[i], "%6.1fGB", qi);
          }
          else
          {
        qi = q_in / 1024.0;
        sprintf (quantity_in[i], "%6.1fMB", qi);
          }
      }
      else
      {
          if (q_in > 1048576)
          {
        qi = q_in / 1048576.0;
        sprintf (quantity_in[i], "%6.1fMB", qi);
          }
          else
          {
        qi = q_in / 1024.0;
        sprintf (quantity_in[i], "%6.1fKB", qi);
          }
      }

      if (q_out_high)
      {
          q_out >>= 10;
          q_out_high <<= 22;
          q_out |= q_out_high;
          if (q_out > 1048576)
          {
        qo = q_out / 1048576.0;
        sprintf (quantity_out[i], "%6.1fGB", qo);
          }
          else
          {
        qo = q_out / 1024.0;
        sprintf (quantity_out[i], "%6.1fMB", qo);
          }
      }
      else
      {
          if (q_out > 1048576)
          {
        qo = q_out / 1048576.0;
        sprintf (quantity_out[i], "%6.1fMB", qo);
          }
          else
          {
        qo = q_out / 1024.0;
        sprintf (quantity_out[i], "%6.1fKB", qo);
          }
      }
                    }
                    else
                    {
                        quantity_in_p[0] = '\0';
                        quantity_out_p[0] = '\0';
                    }
                }
            }
            else
            {
                irate_p             = 0;
                orate_p             = 0;
                circuit_p[0]            = '\0';
                ip_address_p[0]         = '\0';
                charge_buf[i][0]        = '\0';
                online_time_p[0]        = '\0';
                quantity_in_p[0]        = '\0';
                quantity_out_p[0]       = '\0';
                q_in = 0;
                q_out = 0;
                qi = 0;
                qo = 0;
            }
        }

        if (format_type_used[LOAD_1] || format_type_used[LOAD_2])
        {
            fd_load = open ("/proc/loadavg", O_RDONLY);
            size = read (fd_load, buf, 31);
            close (fd_load);

            buf[size - 1]='\0';

            if ((bp = strtok (buf, " ")) != NULL)
            {
                strncpy (load_1, bp, MAX_LOAD_BLEN);
            }
            else
            {
                load_1[0] = '\0';
            }

            if ((bp = strtok (NULL, " ")) != NULL)
            {
                strncpy (load_2, bp, MAX_LOAD_BLEN);
            }
            else
            {
                load_2[0] = '\0';
            }
        }

        if (loop_counter == 20)  lcd_clear ();  /*clear display every 20 sec*/
        if (loop_counter == 40)  lcd_clear ();

        if (lcd_type == HITACHI && loop_counter == 60)  /* fm: my lcd hangs */
        {                                               /* after a while, so*/
            lcd_init ();                                /* reset it every   */
            loop_counter = 0;                           /* minute ...       */
        }

        if (format_type_used[VAR_TEXT_1] ||
            format_type_used[VAR_TEXT_2] ||
            format_type_used[VAR_TEXT_3] ||
            format_type_used[VAR_TEXT_4])
        {
            FILE    *text_file;
            char    line[21];
            
            if (format_type_used[VAR_TEXT_1])
            {           
                text_file = fopen ("/etc/lcd_text1.txt", "r");

                if (! text_file)
                {
                    perror ("/etc/lcd_text1.txt not found");
                    exit (1);
                }
                
                if (! fgets (line, 20, text_file))
                {
                    FillText (VAR_TEXT_1, "");
                }
                else
                {
                    FillText (VAR_TEXT_1, line);
                }
                fclose(text_file);
            }

            if (format_type_used[VAR_TEXT_2])
            {           
                text_file = fopen ("/etc/lcd_text2.txt", "r");

                if (! text_file)
                {
                    perror ("/etc/lcd_text2.txt not found");
                    exit (1);
                }
                
                if (! fgets (line, 20, text_file))
                {
                    FillText (VAR_TEXT_2, "");
                }
                else
                {
                    FillText (VAR_TEXT_2, line);
                }
                fclose(text_file);
            }

            if (format_type_used[VAR_TEXT_3])
            {           
                text_file = fopen ("/etc/lcd_text3.txt", "r");

                if (! text_file)
                {
                    perror ("/etc/lcd_text3.txt not found");
                    exit (1);
                }
                
                if (! fgets (line, 20, text_file))
                {
                    FillText (VAR_TEXT_3, "");
                }
                else
                {
                    FillText (VAR_TEXT_3, line);
                }
                fclose(text_file);
            }

            if (format_type_used[VAR_TEXT_4])
            {           
                text_file = fopen ("/etc/lcd_text4.txt", "r");

                if (! text_file)
                {
                    perror ("/etc/lcd_text4.txt not found");
                    exit (1);
                }
                
                if (! fgets (line, 20, text_file))
                {
                    FillText (VAR_TEXT_4, "");
                }
                else
                {
                    FillText (VAR_TEXT_4, line);
                }
                fclose(text_file);
            }
        }
        
        if (format_type_used[IRATE_BUF])
        {
            sprintf (irate_buf, "%5.2f", irate);
        }

        if (format_type_used[ORATE_BUF])
        {
            sprintf (orate_buf, "%5.2f", orate);
        }

        if (format_type_used[IRATE_BAR] || format_type_used[ORATE_BAR])
        {
            loadbar (irate, TRUE, TRUE);
            loadbar (orate, TRUE, FALSE);
        }

        if (format_type_used[IRATE_BAR_P] || format_type_used[ORATE_BAR_P])
        {
            loadbar (irate_p, FALSE, TRUE);
            loadbar (orate_p, FALSE, FALSE);
        }

        flush_display ();
        loop_counter++;
        usleep (500000);  /* wait ~ 1/2 second */
        flush_display1 ();
        now = time ((time_t *) NULL);
        tm = localtime (&now);
        sec=tm->tm_sec;
        usleep (350000); /* wait 350 msecs */
        count = 0;
        while (tm->tm_sec == sec)  /* sync with second change */
        {
           usleep(10000); /* wait 10 msec for saving cpu-time */
           now = time ((time_t *) NULL);
           tm = localtime (&now);
           if (count <= 20)
           {
                count += 1;
           }
           else
           {
                count = 0;
                break;
           }
        }       
    }
    return (0);                                             /* not reached  */
} /* main (void) */
