/*!
 * $Id$
 *
 * @file    log.c
 * @brief   implementation of log functions for hwsuppd deamon
 *
 * @author  Carsten Spie&szlig; fli4l@carsten-spiess.de
 * @date    17.08.2013
*/

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

#include "hwsuppd.h"

static void log_vaprintf (int level, const char *fmt, va_list ap)
{
    char buf[8192];

    if (level == LOG_DEBUG && g_data.fDebug < 1)
    {
        return;
    }

    vsnprintf(buf, sizeof(buf), fmt, ap);

    if (g_data.fDaemonized)
    {
        syslog (LOG_LOCAL7|level, "%s", buf);
    }
    else
    {
        char *strLevel = "(!?)";
        FILE *out = (level == LOG_DEBUG || level == LOG_ERR) ? stderr : stdout;
        switch (level)
        {
            case LOG_DEBUG:
                strLevel = "debug";
                break;
            case LOG_ERR:
                strLevel = "error";
                break;
            case LOG_WARNING:
                strLevel = "warn";
                break;
            case LOG_NOTICE:
                strLevel = "notice";
                break;
            case LOG_INFO:
                strLevel = "info";
                break;
        }
        fprintf(out, "[%6s] %s\n",strLevel, buf);
    }
}

void log_printf(int level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    log_vaprintf (level, fmt, ap);

    va_end(ap);
}

/*_oOo*/
