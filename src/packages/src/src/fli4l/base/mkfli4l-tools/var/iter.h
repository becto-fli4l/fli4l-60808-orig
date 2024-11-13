/*****************************************************************************
 *  @file var/iter.h
 *  Functions for processing variable iterators (private interface).
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

#ifndef MKFLI4L_VAR_ITER_H_
#define MKFLI4L_VAR_ITER_H_

#include <libmkfli4l/defs.h>
#include <vartype.h>
#include <var.h>

/**
 * Describes a slot iterator.
 */
struct varslot_iterator_t {
    /**
     * Points to the parent slot iterator, i.e. to the slot iterator with a
     * level one less than the level of this iterator. If NULL, this is the
     * root slot iterator pointing to the variable's top-level slot.
     */
    struct varslot_iterator_t *parent;
    /**
     * The variable associated with the iterator.
     */
    struct var_t *var;
    /**
     * The slot this slot iterator points to.
     */
    union varslot_t *slot;
    /**
     * The level of the associated slot.
     */
    int level;
    /**
     * The current index. Only valid if the slot is an array slot.
     */
    int i;
};

/**
 * Describes a value iterator. A value iterator manages multiple slot
 * iterators, one for each level from root to leaf.
 */
struct var_value_iterator_t {
    /**
     * The instance associated with the iterator.
     */
    struct var_instance_t *inst;
    /**
     * The slot iterator pointing to the last slot entry or NULL if this is an
     * END iterator.
     */
    struct varslot_iterator_t *iter;
    /**
     * The value read the last time.
     */
    struct var_value_t *last;
    /**
     * The level of the top-level slot.
     */
    int level;
    /**
     * Take all values into consideration or only the non-NULL ones?
     */
    BOOL all;
};

#endif
