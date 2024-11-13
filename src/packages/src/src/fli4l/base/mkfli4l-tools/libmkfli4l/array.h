/*----------------------------------------------------------------------------
 *  array.h - functions for working with dynamically sized arrays
 *
 *  Copyright (c) 2006-2016 - fli4l-Team <team@fli4l.de>
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

#ifndef LIBMKFLI4L_ARRAY_H_
#define LIBMKFLI4L_ARRAY_H_

/**
 * Opaque array type.
 */
typedef struct array array_t;

/**
 * Type of an array iterator.
 *
 * @warning
 *  Do not use its members directly, use instead init_array_iterator(),
 *  get_next_elem(), and dup_array_iterator() as well as the macros
 *  DECLARE_ARRAY_ITER, ARRAY_ITER_LOOP, and ARRAY_ITER.
 */
typedef struct array_iterator {
    int      index;
    array_t *array;
} array_iterator_t;

/**
 * Initializes a new dynamic array. Such an array contains of a contiguous
 * block of pointers to the array elements. This indirection ensures that
 * array reallocation does not invalidate the pointers to the array elements.
 *
 * @param initial_number
 *  The number of entries for which memory is initially allocated.
 *  Additionally, each time the memory allocated is exhausted, the array is
 *  enlarged by this number of elements.
 * @return
 *  A non-null pointer to the array.
 * @warning
 *  If there is not enough memory available, fatal_exit() is called!
 */
array_t *
init_array(int initial_number);

/**
 * Frees a dynamic array previously created by init_array(). Afterwards, the
 * array may not be passed to any array function.
 *
 * @param array
 *  A non-null pointer to the array, previously returned by init_array().
 */
void
free_array(array_t *array);

/**
 * Removes all entries from the array.
 *
 * @param array
 *  The array to clear.
 */
void
reset_array(array_t *array);

/**
 * Returns the start of the array's underlying memory block, i.e. a pointer to
 * the pointer to the first entry. This is e.g. useful for sorting the array
 * via qsort().
 *
 * @param array
 *  The array.
 */
void **
get_array_start(array_t *array);

/**
 * Returns the end of the array's underlying memory block, i.e. a pointer to
 * the pointer behind the pointer to the array's last entry. This is e.g.
 * useful for sorting the array via qsort().
 *
 * @param array
 *  The array.
 */
void **
get_array_end(array_t *array);

/**
 * Returns a pointer to the first element in the array.
 *
 * @param array
 *  The array.
 * @return
 *  A pointer to the first element in the array or NULL if the array is empty.
 */
void *
get_first_elem(array_t *array);

/**
 * Returns a pointer to the last element in the array.
 *
 * @param array
 *  The array.
 * @return
 *  A pointer to the last element in the array or NULL if the array is empty.
 */
void *
get_last_elem(array_t *array);

/**
 * Appends a new element to the end of the array.
 *
 * @param array
 *  The array.
 * @param elem
 *  The new element to append.
 * @warning
 *  If reallocation is necessary and there is not enough memory available,
 *  fatal_exit() is called!
 */
void
append_new_elem(array_t *array, void *elem);

/**
 * Initializes an array iterator to point to the first element in the array.
 *
 * @param iterator
 *  The iterator to be initialized.
 * @param array
 *  The array containing the elements to iterate over.
 */
void
init_array_iterator(array_iterator_t *iterator, array_t *array);

/**
 * Returns the element the iterator points to and advances the iterator to
 * the next element.
 *
 * @param iterator
 *  The iterator
 * @return
 *  A pointer to the element the iterator points to or NULL if the iterator
 *  is outside the array bounds.
 */
void *
get_next_elem(array_iterator_t *iterator);

/**
 * Duplicates an iterator. Afterwards, dst points to the same element as src
 * does.
 *
 * @param src
 *  The source iterator.
 * @param dst
 *  The destination iterator.
 */
void
dup_array_iterator(array_iterator_t *src, array_iterator_t *dst);

/**
 * Declares an array iterator.
 *
 * @param iter
 *  The name of the iterator variable to be declared.
 * @param var
 *  The name of the variable pointing to the array element while iterating.
 * @param type
 *  The type of the array elements to iterate over.
 */
#define DECLARE_ARRAY_ITER(iter, var, type)     \
    array_iterator_t iter;                      \
    type *var

/**
 * Starts a loop over an array.
 *
 * @param iter
 *  The iterator variable to use.
 * @param array
 *  The array containing the elements to iterate over.
 * @param var
 *  The name of the variable pointing to the array element while iterating.
 */
#define ARRAY_ITER_LOOP(iter, array, var)       \
    init_array_iterator(&iter, array);          \
    while ((var = get_next_elem(&iter)) != 0)

/**
 * Declares an array iterator and starts a loop over an array.
 *
 * @param iter
 *  The iterator variable to use.
 * @param array
 *  The array containing the elements to iterate over.
 * @param var
 *  The name of the variable pointing to the array element while iterating.
 * @param type
 *  The type of the array elements to iterate over.
 */
#define ARRAY_ITER(iter, array, var, type)      \
    DECLARE_ARRAY_ITER(iter, var, type);        \
    ARRAY_ITER_LOOP(iter, array, var)

#endif /*  LIBMKFLI4L_ARRAY_H_ */
