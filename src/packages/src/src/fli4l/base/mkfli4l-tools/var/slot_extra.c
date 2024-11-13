/*****************************************************************************
 *  @file var/slot_extra.c
 *  Functions for processing a variable's extra slot data.
 *
 *  Copyright (c) 2015 - fli4l-Team <team@fli4l.de>
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

#include <var.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <stdlib.h>

#define MODULE "var::slot_extra"

struct var_slot_extra_t *
var_slot_extra_create(void *data, var_slot_extra_free_t free)
{
    struct var_slot_extra_t *result = (struct var_slot_extra_t *)
        safe_malloc(sizeof(struct var_slot_extra_t));

    result->data = data;
    result->free = free;
    return result;
}

void
var_slot_extra_destroy(struct var_slot_extra_t *THIS)
{
    if (THIS->free) {
        THIS->free(THIS->data);
    }
    free(THIS);
}
