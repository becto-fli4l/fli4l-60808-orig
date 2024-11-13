/*****************************************************************************
 *  @file var/slot_read.h
 *  Functions for reading from variable slots (private interface).
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
 * Returns a pointer to some variable slot for reading. Note that explicit
 * array bounds (_N variables) are not checked. This is done by higher-level
 * accessor functions only (like var_instance_read_value()).
 *
 * @param THIS
 *  The variable instance addressing the slot. Excessive indices are flagged as
 *  an error, missing indices are allowed to address an array slot.
 * @param warn
 *  If TRUE, warn about indices which are out of bounds.
 * @return
 *  The variable slot or NULL if an error occurred (e.g. if too many indices
 *  were passed).
 */
union varslot_t *
var_instance_get_slot_for_reading(
    struct var_instance_t *THIS,
    BOOL warn
);

/**
 * Returns the value of a variable slot. In contrast to var_slot_get_value(),
 * this function correctly handles disabled slots and checks the indices
 * against the array bounds. Consequently, the conditions described in
 * var_slot_get_value() are extended by the following ones:
 *
 * - If the slot is disabled, the returned var_value_t object is of kind
 *   VAR_VALUE_KIND_DISABLED. If the variable has been assigned a disabled
 *   value, it is returned in the "value" member, otherwise NULL is stored.
 * - If the slot does not exist because of excessive indices or if the number
 *   of indices exceeds the variable's level, NULL is returned.
 *
 * @param THIS
 *  The variable instance addressing the scalar slot.
 * @param slot
 *  The slot to access.
 * @param warn
 *  If TRUE, produce warning messages when access fails.
 * @return
 *  The value of the slot or NULL. See above for details. Use
 *  var_value_destroy() to destroy it if you don't need it anymore.
 */
struct var_value_t *
var_instance_read_value(
    struct var_instance_t *THIS,
    union varslot_t *slot,
    BOOL warn
);

#endif
