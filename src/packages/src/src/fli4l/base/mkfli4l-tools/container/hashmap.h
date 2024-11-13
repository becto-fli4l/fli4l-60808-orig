/*****************************************************************************
 *  @file hashmap.h
 *  Functions for creating and managing hash maps.
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

#ifndef MKFLI4L_CONTAINER_HASHMAP_H_
#define MKFLI4L_CONTAINER_HASHMAP_H_

#include <libmkfli4l/defs.h>
#include <libmkfli4l/array.h>
#include <container/hashfunc.h>
#include <container/cmpfunc.h>

struct hashmap_t;

/**
 * Type of a function destroying hash map entries. If an entry is allocated
 * via malloc() but does not contain any other pointers to dynamically
 * allocated memory, a pointer to the free() function can be passed. If NULL
 * is passed, no function will be called for entries that are going to be
 * removed from the hash map.
 *
 * @param entry
 *  Points to a hash map entry.
 */
typedef void (*hashmap_destroy_t)(void *entry);

/**
 * Type of a function visiting hash map entries.
 *
 * @param entry
 *  The entry to visit.
 * @param data
 *  Optional data for the visitor as passed to hashmap_foreach().
 * @return
 *  TRUE if iteration should continue, FALSE otherwise.
 */
typedef BOOL (*hashmap_visitor_t)(void *entry, void *data);

/**
 * Creates an empty hash map. A hash map manages a collection of entries which
 * can be indexed by a key. The key is expected to be part of the entry, so an
 * entry is typically a structure containing one member being the key.
 *
 * @param key_offset
 *  The offset pointing to the key within the entry structure. Use the
 *  offsetof() macro to compute it, e.g. use offsetof(struct sometype_t,key) if
 *  the structure type sometype_t contains a member "key" which is to be used
 *  as the key for the hash map.
 * @param num_buckets
 *  The number of buckets to use. The larger the value, the smaller the risk of
 *  hash collisions (more than one entry in a bucket) but the larger the memory
 *  footprint. It follows that it is sensible to choose a value that is equal
 *  to the estimated number of entries the hash table will hold. This value may
 *  not be zero.
 * @param hash
 *  A pointer to the hash function to be used. It may not be NULL.
 * @param cmp
 *  A pointer to the function to be used for key comparison. It may not be NULL.
 * @param destroy
 *  A pointer to the function destroying entries when they are replaced or
 *  removed from the map. It constitutes some sort of an entry destructor. If
 *  NULL, nothing is done when an entry goes away.
 * @return
 *  A pointer to the hash map.
 */
struct hashmap_t *
hashmap_create(size_t key_offset, size_t num_buckets, hash_t hash, compare_t cmp, hashmap_destroy_t destroy);

/**
 * Returns the number of entries in a hash map.
 *
 * @param map
 *  The hash map.
 * @return
 *  The number of entries in the hash map.
 */
size_t
hashmap_size(struct hashmap_t const *map);

/**
 * Puts an entry into a hash map. If an entry with the same key is already in
 * the hash map, it is replaced, otherwise the entry passed will be added to
 * the hash map.
 *
 * Note that it is forbidden to insert an entry into a map that is currently
 * being destroyed.
 *
 * @param map
 *  The hash map.
 * @param entry
 *  A pointer to the entry to be put into the map. It may not be NULL.
 */
void
hashmap_put(struct hashmap_t *map, void *entry);

/**
 * Removes an entry from a hash map. The entry is destroyed.
 *
 * @param map
 *  The hash map.
 * @param key
 * 	The key of the entry to be removed.
 * @return
 *  TRUE if the entry has been found and removed, FALSE otherwise.
 */
BOOL
hashmap_remove(struct hashmap_t *map, void const *key);

/**
 * Looks up an entry in the hash map given its key.
 *
 * @param map
 *  The hash map.
 * @param key
 *  A pointer to the key of the entry to be found. It may not be NULL.
 * @return
 *  A pointer to the entry in the hash map or NULL if there is no entry with
 *  the given key in the hash map.
 */
void *
hashmap_get(struct hashmap_t *map, void const *key);

/**
 * Iterates over all entries in a hash map and calls a visitor on them. If a
 * visitor returns FALSE, the iteration is aborted and FALSE is returned.
 *
 * @param map
 *  The hash map.
 * @param visitor
 *  The visitor.
 * @param data
 *  Optional data to be passed as the second argument to the visitor.
 * @return
 *  TRUE if all visitor calls returned TRUE, FALSE otherwise.
 */
BOOL
hashmap_foreach(struct hashmap_t *map, hashmap_visitor_t visitor, void *data);

/**
 * Converts a hash map to an array. Each entry of the array is a pointer to a
 * hash map entry. This indirection is necessary as the hash map does not know
 * the size of its entries.
 *
 * @param map
 *  The hash map.
 * @return
 *  The array.
 */
array_t *
hashmap_to_array(struct hashmap_t *map);

/**
 * Destroys the map and all of its entries.
 *
 * @param map
 *  The hash map to destroy.
 */
void
hashmap_destroy(struct hashmap_t *map);

#endif
