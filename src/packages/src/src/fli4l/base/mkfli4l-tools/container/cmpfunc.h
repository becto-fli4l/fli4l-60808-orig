/*****************************************************************************
 *  @file cmpfunc.h
 *  Functions for comparing keys.
 *
 *  Copyright (c) 2012-2016 The fli4l team
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
 *****************************************************************************
 */

#ifndef MKFLI4L_CONTAINER_CMPFUNC_H_
#define MKFLI4L_CONTAINER_CMPFUNC_H_

#include <libmkfli4l/defs.h>

/**
 * Type of a function comparing keys. Such a function is necessary as comparing
 * via operator == is not always appropriate (think e.g. about strings).
 *
 * @param key1
 *  Points to the first key.
 * @param key2
 *  Points to the second key.
 * @return
 *  TRUE if the keys are equal, false otherwise.
 */
typedef BOOL (*compare_t)(void const *key1, void const *key2);

/**
 * Compares two string keys in a case-sensitive way.
 *
 * @param key1
 *  Points to the first char const* value.
 * @param key2
 *  Points to the second char const* value.
 * @return
 *  TRUE if strcmp(*key1,*key2) == 0, FALSE otherwise.
 */
BOOL
compare_string_keys(void const *key1, void const *key2);

/**
 * Compares two string keys in a case-insensitive way.
 *
 * @param key1
 *  Points to the first char const* value.
 * @param key2
 *  Points to the second char const* value.
 * @return
 *  TRUE if strcasecmp(*key1,*key2) == 0, FALSE otherwise.
 */
BOOL
compare_ci_string_keys(void const *key1, void const *key2);

/**
 * Compares two (signed or unsigned) short integer keys.
 *
 * @param key1
 *  Points to the first signed short or unsigned short value.
 * @param key2
 *  Points to the second signed short or unsigned short value.
 * @return
 *  TRUE if *key1 == *key2, FALSE otherwise.
 */
BOOL
compare_short_keys(void const *key1, void const *key2);

/**
 * Compares two (signed or unsigned) integer keys.
 *
 * @param key1
 *  Points to the first signed int or unsigned int value.
 * @param key2
 *  Points to the second signed int or unsigned int value.
 * @return
 *  TRUE if *key1 == *key2, FALSE otherwise.
 */
BOOL
compare_int_keys(void const *key1, void const *key2);

/**
 * Compares two (signed or unsigned) long integer keys.
 *
 * @param key1
 *  Points to the first signed long or unsigned long value.
 * @param key2
 *  Points to the second signed long or unsigned long value.
 * @return
 *  TRUE if *key1 == *key2, FALSE otherwise.
 */
BOOL
compare_long_keys(void const *key1, void const *key2);

#endif
