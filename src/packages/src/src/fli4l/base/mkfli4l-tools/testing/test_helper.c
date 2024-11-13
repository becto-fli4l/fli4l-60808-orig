/*****************************************************************************
 *  @file test_helper.c
 *  Helper functions for testing.
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

#include <testing/tests.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <package.h>

/**
 * The test package to use.
 */
static struct package_t *test_package = NULL;

struct location_t *
test_location_create(unsigned type, int line)
{
    struct package_file_t *file;
    if (!test_package) {
        test_package = package_add("myfile");
        package_set_file(test_package, PACKAGE_FILE_TYPE_CONFIG,
            "config/myfile.txt");
        package_set_file(test_package, PACKAGE_FILE_TYPE_VARTYPE,
            "check/myfile.exp");
        package_set_file(test_package, PACKAGE_FILE_TYPE_VARDEF,
            "check/myfile.txt");
        package_set_file(test_package, PACKAGE_FILE_TYPE_EXTCHECK,
            "check/myfile.ext");
        package_set_file(test_package, PACKAGE_FILE_TYPE_IMAGE,
            "opt/myfile.txt");
    }
    file = package_get_file(test_package, type);
    return location_create(file, line, 0);
}
