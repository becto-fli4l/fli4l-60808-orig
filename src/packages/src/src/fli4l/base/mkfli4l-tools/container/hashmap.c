/*****************************************************************************
 *  @file hashmap.c
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

#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <container/hashmap.h>
#include <stdlib.h>
#include <string.h>

#define MODULE "container::hashmap"

/**
 * Describes a hash map bucket.
 */
struct hashmap_bucket_t {
    /**
     * Number of allocated entries in this bucket.
     */
    size_t num_alloc_entries;
    /**
     * Number of real entries in this bucket.
     */
    size_t num_entries;
    /**
     * Number of entries in the free list.
     */
    size_t num_free_entries;
    /**
     * Points to an array of pointers to the bucket entries.
     */
    void **entries;
    /**
     * Points to an array of pointers to free bucket entries.
     */
    void ***free_entries;
};

/**
 * Describes a hash map.
 */
struct hashmap_t {
    /**
     * The offset pointing to the key within the entry structure.
     */
    size_t key_offset;
    /**
     * The hash function.
     */
    hash_t hash;
    /**
     * The function comparing keys.
     */
    compare_t cmp;
    /**
     * The entry destructor.
     */
    hashmap_destroy_t destroy;
    /**
     * The number of buckets in the hash map. This value never changes.
     */
    size_t num_buckets;
    /**
     * Points to an array of buckets.
     */
    struct hashmap_bucket_t *buckets;
    /**
     * TRUE if map is being destroyed, else FALSE.
     */
    BOOL in_destroy;
};

/**
 * Puts an entry into an array.
 *
 * @param entry
 *  The entry to copy.
 * @param data
 *  The array.
 * @return
 *  Always TRUE.
 */
static BOOL
put_entry_into_array(void *entry, void *data)
{
    append_new_elem((array_t *) data, entry);
    return TRUE;
}

/**
 * Returns a pointer to the key member within an entry.
 *
 * @param entry
 *  The entry.
 * @param offset
 *  The offset to the key member from the beginning of the entry structure.
 * @return
 *  A pointer to the key member.
 */
static void const *
hashmap_get_key(void const *entry, size_t offset)
{
    return ((char const *) entry) + offset;
}

/**
 * Searches a bucket for an entry with a given key.
 *
 * @param map
 *  The hash map.
 * @param bucket
 *  The bucket to search.
 * @param key
 *  The key to search for.
 * @return
 *  A pointer to the bucket entry or NULL if an appropriate entry could not be
 *  found.
 */
static void **
hashmap_search_bucket(struct hashmap_t *map, struct hashmap_bucket_t *bucket, void const *key)
{
    size_t const key_offset = map->key_offset;
    compare_t const cmp = map->cmp;
    void **p;
    size_t i;

    for (p = bucket->entries, i = 0; i < bucket->num_alloc_entries; ++p, ++i) {
        if (*p == NULL) {
            continue;
        }
        if ((*cmp)(key, hashmap_get_key(*p, key_offset))) {
            return p;
        }
    }

    return NULL;
}

struct hashmap_t *
hashmap_create(size_t key_offset, size_t num_buckets, hash_t hash, compare_t cmp, hashmap_destroy_t destroy)
{
    struct hashmap_t *map;
    struct hashmap_bucket_t *bucket;
    size_t i;

    my_assert(num_buckets != 0);
    my_assert(hash != NULL);
    my_assert(cmp != NULL);

    map = (struct hashmap_t *) safe_malloc(sizeof(struct hashmap_t));
    map->key_offset = key_offset;
    map->hash = hash;
    map->cmp = cmp;
    map->destroy = destroy;
    map->num_buckets = num_buckets;
    map->buckets = (struct hashmap_bucket_t *)
        safe_malloc(num_buckets * sizeof(struct hashmap_bucket_t));
    map->in_destroy = FALSE;

    for (bucket = map->buckets, i = 0; i < num_buckets; ++bucket, ++i) {
        bucket->num_alloc_entries = 0;
        bucket->num_entries = 0;
        bucket->num_free_entries = 0;
        bucket->entries = NULL;
        bucket->free_entries = NULL;
    }

    return map;
}

size_t
hashmap_size(struct hashmap_t const *map)
{
    struct hashmap_bucket_t *bucket;
    size_t i;
    size_t num_entries = 0;

    for (bucket = map->buckets, i = 0; i < map->num_buckets; ++bucket, ++i) {
        num_entries += bucket->num_entries;
    }

    return num_entries;
}

void
hashmap_put(struct hashmap_t *map, void *entry)
{
    unsigned hash;
    struct hashmap_bucket_t *bucket;
    void **p;

    my_assert(entry != NULL);
    my_assert(!map->in_destroy);

    hash = (*map->hash)(hashmap_get_key(entry, map->key_offset));
    bucket = &map->buckets[hash % map->num_buckets];

    p = hashmap_search_bucket(map, bucket, hashmap_get_key(entry, map->key_offset));
    if (p) {
        if (*p != entry) {
            if (map->destroy) {
                (*map->destroy)(*p);
            }
            *p = entry;
        }
    }
    else if (bucket->num_free_entries > 0) {
        *bucket->free_entries[0] = entry;
        ++bucket->num_entries;
        memmove(
            &bucket->free_entries[0],
            &bucket->free_entries[1],
            (bucket->num_free_entries - 1) * sizeof(void **)
        );
        --bucket->num_free_entries;
        bucket->free_entries = (void ***) safe_realloc(
            bucket->free_entries,
            bucket->num_free_entries * sizeof(void **)
        );
    }
    else {
        bucket->entries = (void **) safe_realloc(
            bucket->entries,
            (bucket->num_alloc_entries + 1) * sizeof(void *)
        );
        bucket->entries[bucket->num_alloc_entries++] = entry;
        ++bucket->num_entries;
    }
}

BOOL
hashmap_remove(struct hashmap_t *map, void const *key)
{
    unsigned hash;
    struct hashmap_bucket_t *bucket;
    void **p;

    my_assert(key != NULL);

    hash = (*map->hash)(key);
    bucket = &map->buckets[hash % map->num_buckets];

    p = hashmap_search_bucket(map, bucket, key);
    if (p) {
        void *entry = *p;
        bucket->free_entries = (void ***) safe_realloc(
            bucket->free_entries,
            (bucket->num_free_entries + 1) * sizeof(void **)
        );
        bucket->free_entries[bucket->num_free_entries++] = p;
        *p = NULL;
        --bucket->num_entries;

        if (map->destroy) {
            (*map->destroy)(entry);
        }
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void *
hashmap_get(struct hashmap_t *map, void const *key)
{
    unsigned hash;
    struct hashmap_bucket_t *bucket;
    void **p;

    my_assert(key != NULL);

    hash = (*map->hash)(key);
    bucket = &map->buckets[hash % map->num_buckets];

    p = hashmap_search_bucket(map, bucket, key);
    return p ? *p : NULL;
}

BOOL
hashmap_foreach(struct hashmap_t *map, hashmap_visitor_t visitor, void *data)
{
    struct hashmap_bucket_t *bucket;
    size_t i;
    BOOL result = TRUE;

    for (bucket = map->buckets, i = 0; result && i < map->num_buckets; ++bucket, ++i) {
        void **p;
        size_t j;

        for (p = bucket->entries, j = 0;
                result && j < bucket->num_alloc_entries;
                ++p, ++j) {
            if (*p != NULL) {
                result = (*visitor)(*p, data);
            }
        }
    }

    return result;
}

array_t *
hashmap_to_array(struct hashmap_t *map)
{
    array_t *result = init_array(hashmap_size(map));
    hashmap_foreach(map, &put_entry_into_array, result);
    return result;
}

void
hashmap_destroy(struct hashmap_t *map)
{
    hashmap_destroy_t const destroy = map->destroy;
    struct hashmap_bucket_t *bucket;
    size_t i;

    map->in_destroy = TRUE;

    if (destroy) {
        for (bucket = map->buckets, i = 0; i < map->num_buckets; ++bucket, ++i) {
            void **p;
            size_t j;

            for (p = bucket->entries, j = 0;
                    j < bucket->num_alloc_entries;
                    ++p, ++j) {
                if (*p != NULL) {
                    (*destroy)(*p);
                }
            }
        }
    }

    for (bucket = map->buckets, i = 0; i < map->num_buckets; ++bucket, ++i) {
        free(bucket->entries);
        free(bucket->free_entries);
    }

    free(map->buckets);
    free(map);
}
