/*****************************************************************************
 *  @file var/value.h
 *  Functions for processing values (private interface).
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

#ifndef MKFLI4L_VAR_VALUE_H_
#define MKFLI4L_VAR_VALUE_H_

#include <libmkfli4l/defs.h>
#include <vartype.h>
#include <var.h>

/**
 * Creates a var_value_t object. Use var_value_destroy() to destroy it.
 *
 * @param value
 *  The underlying string value. It is assumed to be either NULL or to point to
 *  a string that is managed otherwise.
 * @param kind
 *  The value kind.
 * @param extra
 *  Points to a var_slot_extra_t object. May be NULL.
 * @param priority
 *  The value priority inherited from its slot.
 */
struct var_value_t *
var_value_create(
    char const *value,
    enum var_value_kind_t kind,
    struct var_slot_extra_t *extra,
    enum var_priority_t priority
);

#endif
