/*****************************************************************************
 *  @file vartype/regex.h
 *  Functions for processing regular expressions associated with variable
 *  types (private interface).
 *
 *  Copyright (c) 2012-2016 The fli4l team
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

#ifndef MKFLI4L_VARTYPE_REGEX_H_
#define MKFLI4L_VARTYPE_REGEX_H_

#include <libmkfli4l/defs.h>
#include <vartype.h>
#include <regex.h>

/**
 * Describes a variable type's state that is built during compilation.
 */
struct vartype_compiled_state_t {
    /**
     * The compiled regular expression.
     */
    regex_t regex;
    /**
     * The complete error message.
     */
    char *complete_error_msg;
    /**
     * The complete comment.
     */
    char *complete_comment;
};

/**
 * Compiles a variable type if not already done so. This includes compilation
 * of the associated regular expression and the assembly of the complete error
 * message. The expression is expanded and type extensions are processed before
 * being compiled.
 *
 * This function is useful for temporary anonymous types as these are typically
 * created after vartype_finalize_type_system() has been called (e.g. in an
 * extended check script).
 *
 * @param THIS
 *  The variable type.
 * @return
 *  TRUE if compilation succeeded, FALSE otherwise. A possible error is a cycle
 *  detected while expanding the regular expression.
 */
BOOL
vartype_compile(struct vartype_t *THIS);

/**
 * Resets the compiled state associated with a variable type, if any.
 *
 * @param THIS
 *  The variable type.
 */
void
vartype_free_compiled_state(struct vartype_t *THIS);

#endif
