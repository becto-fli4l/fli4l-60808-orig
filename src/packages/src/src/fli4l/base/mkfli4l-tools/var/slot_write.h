/*****************************************************************************
 *  @file var/slot_write.h
 *  Functions for writing into variable slots (private interface).
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
 *  Last Update: $Id$
 *****************************************************************************
 */

#ifndef MKFLI4L_VAR_SLOT_READ_H_
#define MKFLI4L_VAR_SLOT_READ_H_

#include <libmkfli4l/defs.h>
#include <vartype.h>
#include <var.h>
#include "slot.h"

/**
 * Sets the value of a variable slot.
 *
 * @param THIS
 *  The variable instance addressing the scalar slot. Excessive or missing
 *  indices are flagged as an error.
 * @param value
 *  The value.
 * @param extra
 *  Extra slot data. May be NULL. Ownership is passed to the slot.
 * @param mode
 *  The access mode, expressed by some VAR_ACCESS_* constant.
 * @param priority
 *  The priority of the value being set. This is some arbitrary value that is
 *  stored together with the variable slots and that, together with the access
 *  mode, controls whether a slot can be changed or not.
 * @param check
 *  If TRUE, the value is checked against the variable type. FALSE is used when
 *  reading variable values from the configuration as the type system is not
 *  fully initialized at this point; checking these variable values is done at
 *  a later point.
 * @param ignore_base_priority
 *  If TRUE, the access priority is not checked against the variable's base
 *  priority. This is used while accessing array bound variables.
 */
BOOL
var_instance_do_set_value(
    struct var_instance_t *THIS,
    char const *value,
    struct var_slot_extra_t *extra,
    enum var_access_mode_t mode,
    enum var_priority_t priority,
    BOOL check,
    BOOL ignore_base_priority
);

#endif
