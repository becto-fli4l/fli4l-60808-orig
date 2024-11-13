/*****************************************************************************
 *  @file package.h
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

#ifndef MKFLI4L_PACKAGE_H_
#define MKFLI4L_PACKAGE_H_

#include "container/hashmap.h"

struct package_t;
struct package_file_t;
struct location_t;

/**
 * @name Package file types.
 * @{
 */
/**
 * Type of file containing configuration (config/package.txt).
 */
#define PACKAGE_FILE_TYPE_CONFIG 0
/**
 * Type of file containing variable types (check/package.exp).
 */
#define PACKAGE_FILE_TYPE_VARTYPE 1
/**
 * Type of file containing variable definitions (check/package.txt).
 */
#define PACKAGE_FILE_TYPE_VARDEF 2
/**
 * Type of file containing an extended check script (check/package.ext).
 */
#define PACKAGE_FILE_TYPE_EXTCHECK 3
/**
 * Type of file containing image copy actions (opt/package.txt).
 */
#define PACKAGE_FILE_TYPE_IMAGE 4
/**
 * @}
 */

/**
 * @name Module initialization and finalization.
 * @{
 */

/**
 * Initializes this module.
 */
void
package_init_module(void);

/**
 * Finalizes this module.
 */
void
package_fini_module(void);

/**
 * @}
 */

/**
 * @name Miscellaneous.
 * @{
 */

/**
 * Maps a package file type to a string.
 *
 * @param type
 *  The file type.
 * @return
 *  A string representing the file type.
 */
char const *
package_type_to_string(unsigned type);

/**
 * @}
 */

/**
 * @name Packages.
 * @{
 */

/**
 * Looks up a named package.
 *
 * @param name
 *  The name of the package.
 * @return
 *  A pointer to the corresponding package_t object or NULL if not found.
 */
struct package_t *
package_get(char const *name);

/**
 * Looks up a named package. Does not log an error if package does not exist.
 *
 * @param name
 *  The name of the package.
 * @return
 *  A pointer to the corresponding package_t object or NULL if not found.
 */
struct package_t *
package_try_get(char const *name);

/**
 * Looks up the one unnamed internal package used for internal definitions.
 *
 * @return
 *  A pointer to the corresponding package_t object.
 */
struct package_t *
package_get_internal(void);

/**
 * Adds a new package.
 *
 * @param name
 *  The name of the package.
 * @return
 *  A pointer to the new package_t object or NULL if the package could not be
 *  created (e.g. because a package with the same name already exists).
 */
struct package_t *
package_add(char const *name);

/**
 * Returns a package by name or adds a new package if it does not exist yet.
 *
 * @param name
 *  The name of the package.
 * @return
 *  A pointer to the new package_t object.
 */
struct package_t *
package_get_or_add(char const *name);

/**
 * Returns the name of the package.
 *
 * @param THIS
 *  The package.
 * @return
 *  The name of the package.
 */
char const *
package_get_name(struct package_t *THIS);

/**
 * Returns the name of some package file.
 *
 * @param THIS
 *  The package.
 * @param type
 *  The type of the file whose name is to be returned.
 * @return
 *  A package_file_t object or NULL if the package does not contain a file of
 *  the type passed.
 */
struct package_file_t *
package_get_file(struct package_t *THIS, unsigned type);

/**
 * Sets the name of some package file. This function cannot change file names
 * already set.
 *
 * @param THIS
 *  The package.
 * @param type
 *  The type of the file whose name is to be set.
 * @param file
 *  The name of the file.
 * @return
 *  The package file belonging to the type passed.
 */
struct package_file_t *
package_set_file(struct package_t *THIS, unsigned type, char const *file);

/**
 * Iterates over all packages and calls a visitor on them. If the visitor
 * returns FALSE at some point, the iteration is aborted and FALSE is returned.
 *
 * @param visitor
 *  The visitor.
 * @param data
 *  The data for the visitor.
 * @return
 *  TRUE if all visitor calls returned TRUE, FALSE otherwise.
 */
BOOL
package_foreach(
    hashmap_visitor_t visitor,
    void *data
);

/**
 * @}
 */

/**
 * @name Package files.
 * @{
 */

/**
 * Returns the package associated with a package file.
 *
 * @param THIS
 *  The package file.
 * @return
 *  The package.
 */
struct package_t *
package_file_get_package(struct package_file_t *THIS);

/**
 * Returns the type associated with a package file.
 *
 * @param THIS
 *  The package file.
 * @return
 *  The type.
 */
unsigned
package_file_get_type(struct package_file_t *THIS);

/**
 * Returns the name associated with a package file.
 *
 * @param THIS
 *  The package file.
 * @return
 *  The name.
 */
char const *
package_file_get_name(struct package_file_t *THIS);

/**
 * Prints a package file.
 *
 * @param THIS
 *  The package file.
 * @return
 *  A dynamically allocated string representing the package file.
 */
char *
package_file_to_string(struct package_file_t *THIS);

/**
 * @}
 */

/**
 * @name Package file locations.
 * @{
 */

/**
 * Creates a location_t object.
 *
 * @param file
 *  The location's file.
 * @param line
 *  The location's line.
 * @param column
 *  The location's column.
 * @return
 *  A location_t object. Use location_destroy() to release associated memory
 *  after use.
 */
struct location_t *
location_create(struct package_file_t *file, int line, int column);

/**
 * Creates a location_t object pointing to a non-existing, virtual location.
 * This is useful if some object needing a location is defined internally.
 *
 * @param type
 *  The type of the package file to use.
 * @return
 *  A location_t object. Use location_destroy() to release associated memory
 *  after use.
 */
struct location_t *
location_create_internal(unsigned type);

/**
 * Compares two locations for equality. Two locations are equal if they
 * reference the same position in the same file.
 *
 * @param THIS
 *  The first location.
 * @param other
 *  The second location.
 * @return
 *  TRUE if both locations are equal, FALSE otherwise.
 */
BOOL
location_equals(struct location_t const *THIS, struct location_t const *other);

/**
 * Transforms a location_t object to a string.
 *
 * @param location
 *  The location object.
 * @return
 *  A dynamically allocated string containing a string representation of the
 *  location object.
 */
char *
location_toString(struct location_t const *location);

/**
 * Clones a location.
 *
 * @param location
 *  The location to clone.
 * @return
 *  The cloned location.
 */
struct location_t *
location_clone(struct location_t const *location);

/**
 * Returns the package file associated with a location.
 *
 * @param location
 *  The location.
 * @return
 *  The associated package file.
 */
struct package_file_t *
location_get_package_file(struct location_t const *location);

/**
 * Destroys a location_t object, releasing all memory associated with it.
 *
 * @param location
 *  The location_t object to destroy.
 */
void
location_destroy(struct location_t *location);

/**
 * @}
 */

#endif
