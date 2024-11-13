/*****************************************************************************
 *  @file hashfunc.h
 *  Functions for hashing.
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

#ifndef MKFLI4L_CONTAINER_HASHFUNC_H_
#define MKFLI4L_CONTAINER_HASHFUNC_H_

/**
 * Type of a hash function.
 *
 * @param key
 *  Points to the key for which to compute a hash.
 * @return
 *  Some arbitrary hash value which directly depends on the key, i.e. equal
 *  keys must always result in identical hashes.
 */
typedef unsigned (*hash_t)(void const *key);

/**
 * Default hash function for case-sensitive strings.
 *
 * @param key
 *  Pointer to a char const* value.
 * @return
 *  A hash for the string key points to.
 */
unsigned
hash_string(void const *key);

/**
 * Default hash function for case-insensitive strings.
 *
 * @param key
 *  Pointer to a char const* value.
 * @return
 *  A hash for the string key points to.
 */
unsigned
hash_ci_string(void const *key);

/**
 * Default hash function for short integers (signed or unsigned).
 *
 * @param key
 *  Pointer to a signed short or unsigned short value.
 * @return
 *  The value cast to unsigned int.
 */
unsigned
hash_short(void const *key);

/**
 * Default hash function for integers (signed or unsigned).
 *
 * @param key
 *  Pointer to a signed int or unsigned int value.
 * @return
 *  The value cast to unsigned int.
 */
unsigned
hash_int(void const *key);

/**
 * Default hash function for long integers (signed or unsigned).
 *
 * @param key
 *  Pointer to a signed long or unsigned long value.
 * @return
 *  The value cast (and potentially shortened) to unsigned int.
 */
unsigned
hash_long(void const *key);

#endif
