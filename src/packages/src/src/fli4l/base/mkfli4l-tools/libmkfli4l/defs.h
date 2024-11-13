/*----------------------------------------------------------------------------
 *  defs.h - general definitions
 *
 *  Copyright (c) 2000-2001 - Frank Meyer
 *  Copyright (c) 2001-2016 - fli4l-Team <team@fli4l.de>
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
 *  Creation:    2000-05-03  fm
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#ifndef LIBMKFLI4L_DEFS_H_
#define LIBMKFLI4L_DEFS_H_

#include <stddef.h>
#include <assert.h>

/**
 * A synthetic BOOL type.
 */
typedef int BOOL;
/**
 * Boolean TRUE value. Don't use it when comparing boolean values, only for
 * setting variables! E.g. "var = TRUE;" is correct, "if(var == TRUE)" not; use
 * "if(var)" or "if(var != FALSE)" instead.
 */
#define TRUE                1
/**
 * Boolean FALSE value.
 */
#define FALSE               0

/**
 * Error code denoting success.
 */
#define OK                  0
/**
 * Error code denoting a generic, unspecified error. Don't use it when checking
 * whether an error occurred, insted use something like "if(rc != OK)"!
 */
#define ERR                 (-1)

/**
 * Generic buffer size.
 * @todo
 *  A fixed buffer size is inherently insecure!
 */
#define BUF_SIZE            1024

/**
 * Returns the minimum of two integer values.
 * @param x
 *  The first value.
 * @param y
 *  The second value.
 * @return
 *  The minimum of x and y.
 */
#define MIN(x,y) ((x)<(y)?(x):(y))

/**
 * Returns the maximum of two integer values.
 * @param x
 *  The first value.
 * @param y
 *  The second value.
 * @return
 *  The maximum of x and y.
 */
#define MAX(x,y) ((x)>(y)?(x):(y))

/**
 * Expresses that a parameter is not used within a function body.
 * @param x
 *  The parameter.
 */
#define UNUSED(x) (void)(x)

/**
 * @def my_assert(cond)
 * Aborts the program if a condition is not true. This check is only enabled
 * if COVERAGE is not greater than zero.
 *
 * @param cond
 *  The condition to check.
 */

#if defined(DEBUG) && COVERAGE == 0
#define my_assert(cond) assert(cond)
#define my_assert_var(var) my_assert(var)
#else
#define my_assert(cond)
#define my_assert_var(var) UNUSED(var)
#endif

#endif
