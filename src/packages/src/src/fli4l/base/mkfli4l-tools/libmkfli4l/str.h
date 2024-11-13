/*----------------------------------------------------------------------------
 *  str.h - functions for string processing
 *
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

#ifndef LIBMKFLI4L_STR_H_
#define LIBMKFLI4L_STR_H_

#include <stddef.h>

/**
 * Allocates memory. Calls fatal_exit() when memory allocation fails.
 * @param size
 *  The size of the memory to be allocated in bytes.
 * @param file
 *  The source file where the allocation takes place.
 * @param function
 *  The function where the allocation takes place.
 * @param line
 *  The source line where the allocation takes place.
 * @return
 *  A non-null pointer to the allocated memory block.
 */
extern void *str_safe_malloc(size_t size, char const *file, char const *function, int line);

/**
 * Reallocates memory. Calls fatal_exit() when memory reallocation fails.
 * @param p
 *  The memory block to reallocate.
 * @param size
 *  The new size in bytes.
 * @param file
 *  The source file where the reallocation takes place.
 * @param function
 *  The function where the reallocation takes place.
 * @param line
 *  The source line where the reallocation takes place.
 * @return
 *  A non-null pointer to the reallocated memory block.
 */
extern void *str_safe_realloc(void *p, size_t size, char const *file, char const *function, int line);

/**
 * Calls str_save_malloc(), passing the correct values for source file,
 * function, and line.
 * @param size
 *  The size of the memory to be allocated in bytes.
 * @return
 *  A non-null pointer to the allocated memory block.
 */
#define safe_malloc(size) str_safe_malloc(size, __FILE__, __func__, __LINE__)

/**
 * Calls str_save_realloc(), passing the correct values for source file,
 * function, and line.
 * @param p
 *  The memory block to reallocate.
 * @param size
 *  The size of the memory to be allocated in bytes.
 * @return
 *  A non-null pointer to the allocated memory block.
 */
#define safe_realloc(p, size) str_safe_realloc(p, size, __FILE__, __func__, __LINE__)

/**
 * Converts a string to uppercase.
 * @param str
 *  The string to convert.
 * @return
 *  A pointer to an internal buffer containing the transformed string. The
 *  caller needs to make a copy of the string before calling this function
 *  again.
 * @todo
 *  This function is not reentrant!
 */
extern char *convert_to_upper(char const *str);

/**
 * Returns a dynamically allocated copy of a string.
 * @param s
 *  The string to copy.
 * @return
 *  s == NULL ? NULL : strdup(s)
 */
extern char *strsave(char const *s);

/**
 * Returns a dynamically allocated copy of a substring.
 * @param start
 *  The start of the string to copy.
 * @param end
 *  The end of the string to copy. This points to the first character not to be
 *  copied anymore.
 * @return
 *  A dynamically allocated copy of the substring.
 */
extern char *substrsave(char const *start, char const *end);

/**
 * Appends a string to a dynamic string.
 * @param s
 *  The dynamic string.
 * @param a
 *  The string to append to s.
 * @return
 *  A pointer to a dynamic string containing s concatenated with a.
 * @warning
 *  The dynamic string s must not be used afterwards as its memory may have
 *  been freed.
 */
extern char *strcat_save(char *s, char const *a);

/**
 * Returns a dynamically allocated copy of a string, removing its first and
 * last character.
 * @param s
 *  The quoted string to copy.
 * @return
 *  A pointer to a dynamic string containing s without the first and last
 *  character.
 */
extern char *strsave_quoted(char const *s);

/**
 * Returns a dynamically allocated copy of a string, replacing all sequences
 * of whitespace by a single blank.
 * @param s
 *  The string to copy.
 * @return
 *  A pointer to a dynamic string containing s with reduced whitespace.
 */
extern char *strsave_ws(char const *s);

/**
 * Forms a variable name by replacing the first '%' placeholder by an integer,
 * e.g.:
 *  PF_USR_CHAIN_%_RULE_% --> PF_USR_CHAIN_1_RULE_% (with index == 1)
 * @param s
 *  The name of the variable with at least one '%' placeholder.
 * @param index
 *  The index to replace the first '%' placeholder.
 * @param file
 *  The source file calling this function.
 * @param line
 *  The source line calling this function.
 * @return
 *  A pointer to a dynamic string containing s where the first '%' placeholder
 *  has been replaced by 'index'.
 */
extern char *get_set_var_name_int(char const *s, int index, char const *file, int line);

/**
 * Forms a variable name by replacing the first '%' placeholder by an integer
 * represented by a string, e.g.:
 *  PF_USR_CHAIN_%_RULE_% --> PF_USR_CHAIN_1_RULE_% (with index == "1")
 * @param s
 *  The name of the variable with at least one '%' placeholder.
 * @param index
 *  A string containing the index to replace the first '%' placeholder.
 * @param file
 *  The source file calling this function.
 * @param line
 *  The source line calling this function.
 * @return
 *  A pointer to a dynamic string containing s where the first '%' placeholder
 *  has been replaced by 'index'.
 */
extern char *get_set_var_name_string(char const *s, char const *index, char const *file, int line);

/**
 * Removes everything behind the first '%' placeholder when multiple
 * placeholders exist, e.g.:
 *  PF_USR_CHAIN_%_RULE_% --> PF_USR_CHAIN_%
 * @param s
 *  The variable name to transform.
 * @return
 *  The transformed variable name.
 * @note
 *  If less than two '%' placeholders exist, nothing is stripped.
 */
extern char *strip_multiple_indices(char const *s);

/**
 * Replaces indices in a variable name by '%' placeholders, e.g.:
 *  PF_USR_CHAIN_1_RULE_2 --> PF_USR_CHAIN_%_RULE_%
 * @param s
 *  The name of the variable to transform.
 * @return
 *  The transformed variable name.
 */
extern char *replace_set_var_indices(char const *s);

/**
 * Converts a string into an unsigned long value. Hexadecimal values prefixed
 * by '0x' are supported.
 * @param val
 *  The value to convert.
 * @param file
 *  The source file calling this function.
 * @param line
 *  The source line calling this function.
 * @return
 *  The unsigned long value.
 * @warning
 *  Calls fatal_exit() when conversion fails.
 */
extern unsigned long convert_to_long(char const *val, char const *file, int line);

/**
 * Performs a safe sprintf(), allocating a buffer that is large enough to hold
 * the result.
 * @param fmt
 *  The format string.
 * @param ...
 *  The arguments as specified by the format string.
 * @return
 *  A dynamically allocated string containing the result or NULL if an error
 *  occurred.
 */
extern char *safe_sprintf(char const *fmt, ...)
#ifdef __GNUC__
__attribute__ ((format (printf, 1, 2)))
#endif
;

#endif
