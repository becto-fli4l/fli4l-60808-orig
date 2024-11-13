/*****************************************************************************
 *  @file package/core.c
 *  Functions for processing packages.
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

#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <container/hashmap.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"

#define MODULE "package::core"

/**
 * The name of the internal package.
 */
#define PACKAGE_INTERNAL_NAME "<internal>"

struct hashmap_t *package_map = NULL;
/**
 * The one internal package used for internal definitions.
 */
static struct package_t *internal_package = NULL;

char const *
package_type_to_string(unsigned type)
{
    switch (type) {
    case PACKAGE_FILE_TYPE_CONFIG :
        return "configuration file";
    case PACKAGE_FILE_TYPE_VARTYPE :
        return "variable type file";
    case PACKAGE_FILE_TYPE_VARDEF :
        return "variable definition file";
    case PACKAGE_FILE_TYPE_EXTCHECK :
        return "extended check script";
    case PACKAGE_FILE_TYPE_IMAGE :
        return "image file";
    default :
        my_assert(FALSE);
        return NULL;
    }
}

/**
 * Creates a package.
 *
 * @param name
 *  The name of the package.
 * @return
 *  A pointer to the new package_t object.
 */
static struct package_t *
package_create(char const *name)
{
    unsigned i;

    struct package_t *const package
        = (struct package_t *) safe_malloc(sizeof(struct package_t));
    package->name = strsave(name);
    for (i = 0; i < PACKAGE_FILE_TYPE_NUM; ++i) {
        package->files[i] = NULL;
    }
    return package;
}

/**
 * Destroys a package.
 *
 * @param THIS
 *  The package to destroy.
 */
static void
package_destroy(struct package_t *THIS)
{
    unsigned i;
    for (i = 0; i < PACKAGE_FILE_TYPE_NUM; ++i) {
        if (THIS->files[i]) {
            package_file_destroy(THIS->files[i]);
        }
    }
    free(THIS->name);
    free(THIS);
}

struct package_t *
package_try_get(char const *name)
{
    return (struct package_t *) hashmap_get(package_map, &name);
}

struct package_t *
package_get(char const *name)
{
    struct package_t *package = package_try_get(name);
    if (package) {
        return package;
    }
    else {
        log_error(MODULE ": Package '%s' does not exist.\n", name);
        return NULL;
    }
}

struct package_t *
package_get_internal(void)
{
    return internal_package;
}

struct package_t *
package_add(char const *name)
{
    struct package_t *package;

    my_assert(name != NULL);

    package = package_try_get(name);
    if (!package) {
        package = package_create(name);
        hashmap_put(package_map, package);
        return package;
    }
    else {
        log_error(
            MODULE ": Redefinition of package '%s' is not allowed.\n",
            name
        );
        return NULL;
    }
}

struct package_t *
package_get_or_add(char const *name)
{
    struct package_t *package = package_try_get(name);
    if (package) {
        return package;
    }
    else {
        return package_add(name);
    }
}

char const *
package_get_name(struct package_t *THIS)
{
    return THIS->name;
}

struct package_file_t *
package_get_file(struct package_t *THIS, unsigned type)
{
    my_assert(type < PACKAGE_FILE_TYPE_NUM);
    return THIS->files[type];
}

struct package_file_t *
package_set_file(struct package_t *THIS, unsigned type, char const *file)
{
    my_assert(type < PACKAGE_FILE_TYPE_NUM);
    my_assert(!THIS->files[type]);
    return THIS->files[type] = package_file_create(THIS, type, file);
}

BOOL
package_foreach(hashmap_visitor_t visitor, void *data)
{
    return hashmap_foreach(package_map, visitor, data);
}

/**
 * Destroys a package.
 *
 * @param entry
 *  The package to destroy. It is of type void* in order to be able to use
 *  this function as a hash map object destructor.
 */
static void
package_destroy_action(void *entry)
{
    struct package_t *const THIS = (struct package_t *) entry;
    package_destroy(THIS);
}

void
package_core_init(void)
{
    package_map = hashmap_create(
        offsetof(struct package_t, name),
        1024,
        hash_ci_string,
        compare_ci_string_keys,
        package_destroy_action
    );

    internal_package = package_create(PACKAGE_INTERNAL_NAME);
    package_set_file(internal_package, PACKAGE_FILE_TYPE_CONFIG,
        PACKAGE_INTERNAL_NAME);
    package_set_file(internal_package, PACKAGE_FILE_TYPE_VARTYPE,
        PACKAGE_INTERNAL_NAME);
    package_set_file(internal_package, PACKAGE_FILE_TYPE_VARDEF,
        PACKAGE_INTERNAL_NAME);
    package_set_file(internal_package, PACKAGE_FILE_TYPE_EXTCHECK,
        PACKAGE_INTERNAL_NAME);
    package_set_file(internal_package, PACKAGE_FILE_TYPE_IMAGE,
        PACKAGE_INTERNAL_NAME);
}

void
package_core_fini(void)
{
    package_destroy(internal_package);
    hashmap_destroy(package_map);
    package_map = NULL;
}
