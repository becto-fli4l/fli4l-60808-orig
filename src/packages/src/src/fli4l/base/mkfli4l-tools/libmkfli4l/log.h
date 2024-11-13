/*----------------------------------------------------------------------------
 *  log.h - functions for logging
 *
 *  Copyright (c) 2002 - Frank Meyer
 *  Copyright (c) 2002-2016 - fli4l-Team <team@fli4l.de>
 *
 *  This file is part of fli4l.
 *
 *  fli4l is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  fli4l is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with fli4l.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#ifndef LIBMKFLI4L_LOG_H_
#define LIBMKFLI4L_LOG_H_

#include <stdio.h>
#include "libmkfli4l/defs.h"

/**
 * Log level for informative messages.
 */
#define LOG_INFO           0x0001
/**
 * Log level for verbose messages.
 */
#define LOG_VERBOSE        0x0002
/**
 * Log level for messages about archive generation.
 */
#define LOG_ZIPLIST        0x0004
/**
 * Log level for messages about skipped files while generating archive.
 */
#define LOG_ZIPLIST_SKIP   0x0008
/**
 * Log level for messages about parse tree generation.
 */
#define LOG_TREE_BUILD     0x0010
/**
 * Log level for messages about execution of extended checks.
 */
#define LOG_TREE_EXEC      0x0020
/**
 * Log level for messages about the variable checking process.
 */
#define LOG_VAR            0x0040
/**
 * Log level for messages about resolving library dependencies.
 */
#define LOG_LIB_DEP        0x0080
/**
 * Log level for messages about regular expressions used while generating
 * archive.
 */
#define LOG_ZIPLIST_REGEXP 0x0100
/**
 * Log level for messages about the scanning process.
 */
#define LOG_SCAN           0x0200
/**
 * Log level for messages about matching regular expressions.
 */
#define LOG_REGEXP         0x0400
/**
 * Log level for messages about parsing regular expressions.
 */
#define LOG_EXP            0x0800
/**
 * Log level for messages about resolving kernel module dependencies.
 */
#define LOG_DEP            0x1000
/**
 * Log level for messages about creating library symlinks.
 */
#define LOG_LIB_SYMLINK    0x2000
/**
 * Log level for messages about reading UIDs and GIDs.
 */
#define LOG_UIDGID         0x4000
/**
 * Log level for messages about creating archives.
 */
#define LOG_ARCHIVE        0x8000

/**
 * Set to TRUE to disable the line breaking algorithm. Initially set to FALSE.
 */
extern BOOL no_reformat;

/**
 * Opens the log file.
 * @param logfile
 *  The log file to open.
 * @warning
 *  The log file is opened the first time a logging message is to be written
 *  to (lazy initialization). If opening fails, abort() is called.
 */
extern void open_logfile(char const *logfile);

/**
 * Closes the log file previously opened by open_logfile(),
 */
extern void close_logfile(void);

/**
 * Changes the log level.
 * @param level
 *  The new combined log level. This is a bitmask of log levels defined above.
 */
extern void set_log_level(int level);

/**
 * Increments the indentation by one level.
 */
extern void inc_log_indent_level(void);

/**
 * Decrements the indentation by one level.
 */
extern void dec_log_indent_level(void);

/**
 * Returns a printable representation of a character. Control characters are
 * mapped to corresponding escape sequences, e.g.:\n
 *  &lt;BEL&gt; --> \\a\n
 *  &lt;TAB&gt; --> \\t\n
 *  &lt;SOH&gt; --> \\001
 * @param c
 *  The character.
 * @return
 *  A printable representation of 'c'.
 */
extern char const *log_get_printable(char c);

/**
 * Prints an informative message to standard error and logs it to the log file.
 * If the log level of the message is not enabled, nothing is printed or
 * logged.
 * @param level
 *  The log level of the message.
 * @param fmt
 *  The format string, followed by required arguments (if any).
 */
extern void log_info(int level, char const *fmt, ...)
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
;

/**
 * Prints a warning message to standard error and logs it to the log file,
 * regardless of the log level set.
 * @param fmt
 *  The format string, followed by required arguments (if any).
 */
extern void log_warn(char const *fmt, ...)
#ifdef __GNUC__
__attribute__ ((format (printf, 1, 2)))
#endif
;

/**
 * Prints an error message to standard error and logs it to the log file,
 * regardless of the log level set.
 * @param fmt
 *  The format string, followed by required arguments (if any).
 */
extern void log_error(char const *fmt, ...)
#ifdef __GNUC__
__attribute__ ((format (printf, 1, 2)))
#endif
;

/**
 * Prints an error message to standard error and logs it to the log file,
 * regardless of the log level set. Then the program exits.
 * @param fmt
 *  The format string, followed by required arguments (if any).
 */
extern void fatal_exit(char const *fmt, ...)
#ifdef __GNUC__
__attribute__ ((format (printf, 1, 2))) __attribute__ ((noreturn))
#endif
;

/**
 * Prints a scanner message to standard error and logs it to the log file.
 * @param file
 *  Not used.
 * @param fmt
 *  The format string, followed by required arguments (if any).
 */
extern void log_lex_fprintf(FILE *file, char const *fmt, ...)
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
;

/**
 * Prints a parser message to standard error and logs it to the log file.
 * @param file
 *  Not used.
 * @param fmt
 *  The format string, followed by required arguments (if any).
 */
extern void log_yacc_fprintf(FILE *file, char const *fmt, ...)
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
;

#endif
