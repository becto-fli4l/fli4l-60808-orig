/*****************************************************************************
 *  @file var/inst.h
 *  Functions for processing variable instances (private interface).
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

#ifndef MKFLI4L_VAR_INST_H_
#define MKFLI4L_VAR_INST_H_

#include <libmkfli4l/defs.h>
#include <vartype.h>
#include <var.h>
#include <regex.h>
#include "slot.h"

/**
 * Describes a request for deducing an instantiation.
 */
struct var_instantiation_request_t {
    /**
     * The original variable name.
     */
    char const *name;
    /**
     * Receives the resulting instance if successful.
     */
    struct var_instance_t *instance;
};

/**
 * Invalidates a variable instance. This happens when the underlying variable
 * is destroyed.
 *
 * @param THIS
 *  The variable instance.
 */
void
var_instance_invalidate(struct var_instance_t *THIS);

#endif
