/*****************************************************************************
 *  @file var/slot.h
 *  Functions for processing variable slots (private interface).
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

#ifndef MKFLI4L_VAR_SLOT_H_
#define MKFLI4L_VAR_SLOT_H_

#include <libmkfli4l/defs.h>
#include <vartype.h>
#include <var.h>

/**
 * Represents one or more values belonging to a variable. Depending on the slot
 * level, either "value" contains a valid string or "slots" points to an array
 * of slots of the next level. Only slots of the level equal to the level of
 * the variable contain values.
 */
union varslot_t {
    /**
     * A leaf entry. It contains a single value and is valid if this slot's
     * level equals to the variable's level.
     */
    struct {
        /**
         * The value.
         */
        char *value;
        /**
         * The associated default value.
         */
        char *def_value;
        /**
         * The extra data.
         */
        struct var_slot_extra_t *extra;
        /**
         * The slot priority. Initially VAR_PRIORITY_UNDEFINED.
         */
        enum var_priority_t priority;
        /**
         * TRUE if the slot is being read.
         */
        BOOL read_active;
    } entry;
    /**
     * An array of values. It is valid if this slot's level differs from the
     * variable's level.
     */
    struct {
        /**
         * The number of slots.
         */
        unsigned num_slots;
        /**
         * The slots of the next level (if any).
         */
        union varslot_t *slots;
    } array;
};

/**
 * Initializes a slot of a variable.
 *
 * @param THIS
 *  The slot to initialize.
 * @param level
 *  The level of the slot.
 * @param maxlevel
 *  The level of the associated variable, i.e. the maximum possible slot level.
 */
void
var_slot_init(union varslot_t *THIS, int level, int maxlevel);

/**
 * Destroys a slot of a variable.
 *
 * @param THIS
 *  The slot to destroy.
 * @param level
 *  The level of the slot.
 * @param maxlevel
 *  The level of the associated variable, i.e. the maximum possible slot level.
 */
void
var_slot_destroy(union varslot_t *THIS, int level, int maxlevel);

/**
 * Reads a value from a slot. The following rules apply:
 *
 * - If the slot is disabled, the returned var_value_t object is of kind
 *   VAR_VALUE_KIND_DISABLED. If the variable has been assigned a disabled
 *   value, it is returned in the "value" member, otherwise NULL is stored.
 * - If a value has been set, the resulting var_value_t object is of kind
 *   VAR_VALUE_KIND_NORMAL, and the slot's value is returned in the "value"
 *   member.
 * - If no value has been set, but the underlying variable has been assigned a
 *   default value, the resulting var_value_t object is of kind
 *   VAR_VALUE_KIND_DEFAULT. The default value is returned in the "value"
 *   member.
 * - If no value has been set and no default value exists, the resulting
 *   var_value_t object is of kind VAR_VALUE_KIND_NONE. The "value" member is
 *   set to NULL.
 *
 * Note that this function does not check whether the slot is disabled.
 *
 * @param THIS
 *  The slot.
 * @param inst
 *  The variable instance the slot belongs to.
 * @return
 *  The slot value.
 */
struct var_value_t *
var_slot_get_value(union varslot_t *THIS, struct var_instance_t *inst);

#endif
