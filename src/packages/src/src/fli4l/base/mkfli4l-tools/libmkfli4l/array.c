/*----------------------------------------------------------------------------
 *  array.c - functions for working with dynamically sized arrays
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

#include <stdlib.h>
#include <string.h>
#include "libmkfli4l/array.h"
#include "libmkfli4l/log.h"
#include "libmkfli4l/str.h"

/**
 * Represents a dynamic array.
 */
struct array {
    /**
     * The number of elements memory has been allocated for.
     */
    int elem_number;
    /**
     * The index of the last element or -1 if the array is empty.
     */
    int last;
    /**
     * Pointer to the memory allocated.
     */
    void **array;
};

array_t *
init_array(int initial_number)
{
    array_t *const a = (array_t *) malloc(sizeof(array_t));

    if (a) {
        a->elem_number = initial_number;
        a->last = -1;
        a->array = (void **) calloc(initial_number, sizeof(void *));
    }

    if (!a || !a->array)
        fatal_exit ("unable to allocate array\n");

    return a;
}

void
free_array(array_t *array)
{
    memset(array->array, 0, array->elem_number * sizeof(void *));
    free(array->array);
    memset(array, 0, sizeof(array_t));
    free(array);
}

void
reset_array(array_t *array)
{
    array->last = -1;
}

void **
get_array_start(array_t *array)
{
    return array->array;
}

void **
get_array_end(array_t *array)
{
    return array->array + (array->last + 1);
}

void *
get_first_elem(array_t *array)
{
    return array->last != -1 ? *array->array : NULL;
}

void *
get_last_elem(array_t *array)
{
    return array->last != -1 ? *(array->array + array->last) : NULL;
}

void
append_new_elem(array_t *array, void *elem)
{
    ++array->last;
    if (array->last == array->elem_number) {
        int const inc = array->elem_number;
        array->array = (void **) realloc(array->array, (array->elem_number + inc) * sizeof(void *));
        if (!array->array) {
            fatal_exit ("unable to resize array\n");
        }

        memset(array->array + array->elem_number, 0, inc * sizeof(void *));
        array->elem_number += inc;
    }

    *(array->array + array->last) = elem;
}

void
init_array_iterator(array_iterator_t *iterator, array_t *array)
{
    iterator->index = 0;
    iterator->array = array;
}

void *
get_next_elem(array_iterator_t *iterator)
{
    if (iterator->index > iterator->array->last)
        return NULL;
    else
        return *(iterator->array->array + iterator->index++);
}

void
dup_array_iterator(array_iterator_t *src, array_iterator_t *dst)
{
    *dst = *src;
}
