/*****************************************************************************
 *  @file test_hashmap.c
 *  Functions for testing hash maps.
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
#include <container/hashmap.h>
#include <libmkfli4l/str.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "container.hashmap"

/**
 * A sample map entry.
 */
struct myentry_t {
    /**
     * A string.
     */
    char const *str;
    /**
     * An integer value.
     */
    int i;
};

/**
 * Another sample map entry.
 */
struct myentry2_t {
    /**
     * A string.
     */
    char const *str;
    /**
     * Points to the map.
     */
    struct hashmap_t *map;
    /**
     * TRUE if map is being destroyed.
     */
    BOOL in_destroy;
};

/**
 * A hash map visitor.
 *
 * @param entry
 *  The hash map entry.
 * @param data
 *  A pointer to an integer receiving the sum of the entries' values.
 * @return
 *  TRUE if the entry's value is not zero, FALSE otherwise.
 */
static BOOL
my_visitor(void *entry, void *data)
{
    struct myentry_t *my_entry = (struct myentry_t *) entry;
    if (my_entry->i != 0) {
        *((int *) data) += my_entry->i;
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/**
 * Compares two myentry_t objects in an array.
 *
 * @param lhs
 *  The left entry.
 * @param rhs
 *  The right entry.
 * @retval 0 if both entries are equal.
 * @retval -1 if the left entry is less than the right one.
 * @retval 1 if the right entry is less than the left one.
 */
static int
my_qsort_cmp_func(void const *lhs, void const *rhs)
{
    struct myentry_t const *l = *(struct myentry_t const **) lhs;
    struct myentry_t const *r = *(struct myentry_t const **) rhs;
    return l->i - r->i;
}

/**
 * Tests the creation and destruction of empty hash maps.
 */
mu_test_begin(test_hashmap_create_destroy)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_string, compare_string_keys, free);
    mu_assert_eq_uint(0, hashmap_size(map));
    hashmap_destroy(map);
mu_test_end()

/**
 * Tests adding the same entry twice, where the key is a string.
 */
mu_test_begin(test_hashmap_put_get_str1)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_string, compare_string_keys, free);
    struct myentry_t *entry1;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests adding two different entries with the same key, where the key is a
 * string.
 */
mu_test_begin(test_hashmap_put_get_str2)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_string, compare_string_keys, free);
    struct myentry_t *entry1;
    struct myentry_t *entry2;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    entry2 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry2->str = "A";
    entry2->i = 2;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_put(map, entry2);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry2 == hashmap_get(map, &entry2->str));

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests adding two different entries with different keys, where the key is a
 * string.
 */
mu_test_begin(test_hashmap_put_get_str3)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_string, compare_string_keys, free);
    struct myentry_t *entry1;
    struct myentry_t *entry2;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    entry2 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry2->str = "B";
    entry2->i = 2;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_put(map, entry2);
    mu_assert_eq_uint(2, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));
    mu_assert(entry2 == hashmap_get(map, &entry2->str));

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests adding the same entry twice, where the key is a case-insensitive
 * string.
 */
mu_test_begin(test_hashmap_put_get_ci_str1)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_ci_string, compare_ci_string_keys, free);
    struct myentry_t *entry1;
    struct myentry_t *entry2;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    entry2 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry2->str = "a";
    entry2->i = 1;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_put(map, entry2);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry2 == hashmap_get(map, &entry2->str));

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests adding two different entries with the same key, where the key is a
 * case-insensitive string.
 */
mu_test_begin(test_hashmap_put_get_ci_str2)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_ci_string, compare_ci_string_keys, free);
    struct myentry_t *entry1;
    struct myentry_t *entry2;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    entry2 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry2->str = "a";
    entry2->i = 2;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_put(map, entry2);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry2 == hashmap_get(map, &entry2->str));

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests adding two different entries with different keys, where the key is a
 * case-insensitive string.
 */
mu_test_begin(test_hashmap_put_get_ci_str3)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_ci_string, compare_ci_string_keys, free);
    struct myentry_t *entry1;
    struct myentry_t *entry2;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    entry2 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry2->str = "B";
    entry2->i = 2;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_put(map, entry2);
    mu_assert_eq_uint(2, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));
    mu_assert(entry2 == hashmap_get(map, &entry2->str));

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests adding the same entry twice, where the key is an integer.
 */
mu_test_begin(test_hashmap_put_get_int1)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, i), 16, hash_int, compare_int_keys, free);
    struct myentry_t *entry1;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->i));

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->i));

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests adding two different entries with the same key, where the key is an
 * integer.
 */
mu_test_begin(test_hashmap_put_get_int2)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, i), 16, hash_int, compare_int_keys, free);
    struct myentry_t *entry1;
    struct myentry_t *entry2;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    entry2 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry2->str = "B";
    entry2->i = 1;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->i));

    hashmap_put(map, entry2);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry2 == hashmap_get(map, &entry2->i));

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests adding two different entries with different keys, where the key is an
 * integer.
 */
mu_test_begin(test_hashmap_put_get_int3)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, i), 16, hash_int, compare_int_keys, free);
    struct myentry_t *entry1;
    struct myentry_t *entry2;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    entry2 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry2->str = "B";
    entry2->i = 2;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->i));

    hashmap_put(map, entry2);
    mu_assert_eq_uint(2, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->i));
    mu_assert(entry2 == hashmap_get(map, &entry2->i));

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests adding the same entry twice, where the key is a string and the entries
 * are managed by the caller.
 */
mu_test_begin(test_hashmap_put_get_str_managed1)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_string, compare_string_keys, NULL);
    struct myentry_t *entry1;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_destroy(map);
    free(entry1);
mu_test_end()

/**
 * Tests adding two different entries with the same key, where the key is a
 * string and the entries are managed by the caller.
 */
mu_test_begin(test_hashmap_put_get_str_managed2)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_string, compare_string_keys, NULL);
    struct myentry_t *entry1;
    struct myentry_t *entry2;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    entry2 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry2->str = "A";
    entry2->i = 2;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_put(map, entry2);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry2 == hashmap_get(map, &entry2->str));

    hashmap_destroy(map);
    free(entry1);
    free(entry2);
mu_test_end()

/**
 * Tests adding two different entries with different keys, where the key is a
 * string.
 */
mu_test_begin(test_hashmap_put_get_str_managed3)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_string, compare_string_keys, NULL);
    struct myentry_t *entry1;
    struct myentry_t *entry2;

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = "A";
    entry1->i = 1;

    entry2 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry2->str = "B";
    entry2->i = 2;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    hashmap_put(map, entry2);
    mu_assert_eq_uint(2, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));
    mu_assert(entry2 == hashmap_get(map, &entry2->str));

    hashmap_destroy(map);
    free(entry1);
    free(entry2);
mu_test_end()

/**
 * Tests iteration over a hash map.
 */
mu_test_begin(test_hashmap_foreach)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, i), 2, hash_int, compare_int_keys, free);
    int i;
    int sum;
    int halfsum;
    int const num_entries = 20;

    for (i = 1; i <= num_entries; ++i) {
        struct myentry_t *entry
            = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
        entry->str = "";
        entry->i = i;
        hashmap_put(map, entry);
    }

    sum = 0;
    mu_assert_eq_int(TRUE, hashmap_foreach(map, my_visitor, &sum));
    mu_assert_eq_int(num_entries * (num_entries + 1) / 2, sum);

    i = 1;
    ((struct myentry_t *) hashmap_get(map, &i))->i = 0;

    halfsum = 0;
    for (i = 2; i <= num_entries; i += 2) {
        halfsum += i;
    }

    sum = 0;
    mu_assert_eq_int(FALSE, hashmap_foreach(map, my_visitor, &sum));
    mu_assert_eq_int(halfsum, sum);

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests copying a hash map into an array.
 */
mu_test_begin(test_hashmap_to_array)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, i), 2, hash_int, compare_int_keys, free);
    array_t *array;
    void **beg;
    void **end;
    array_iterator_t it;
    int i;

    for (i = 0; i < 20; ++i) {
        struct myentry_t *entry
            = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
        entry->str = "";
        entry->i = i;
        hashmap_put(map, entry);
    }

    array = hashmap_to_array(map);
    beg = get_array_start(array);
    end = get_array_end(array);
    qsort(beg, end - beg, sizeof(*beg), my_qsort_cmp_func);

    init_array_iterator(&it, array);
    for (i = 0; i < 20; ++i) {
        struct myentry_t *entry = (struct myentry_t *) get_next_elem(&it);
        mu_assert(entry != NULL);
        mu_assert_eq_int(i, entry->i);
    }

    mu_assert(get_next_elem(&it) == NULL);
    free_array(array);

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests removing an entry from a hash map.
 */
mu_test_begin(test_hashmap_remove)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_string, compare_string_keys, free);
    struct myentry_t *entry1;
    char const *key = "A";

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = key;
    entry1->i = 1;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &key));

    mu_assert(hashmap_remove(map, &key));
    mu_assert_eq_uint(0, hashmap_size(map));
    mu_assert(NULL == hashmap_get(map, &key));

    mu_assert(!hashmap_remove(map, &key));
    mu_assert_eq_uint(0, hashmap_size(map));
    mu_assert(NULL == hashmap_get(map, &key));

    hashmap_destroy(map);
mu_test_end()

/**
 * Tests removing a managed entry from a hash map.
 */
mu_test_begin(test_hashmap_remove_managed)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry_t, str), 16, hash_string, compare_string_keys, NULL);
    struct myentry_t *entry1;
    char const *key = "A";

    entry1 = (struct myentry_t *) safe_malloc(sizeof(struct myentry_t));
    entry1->str = key;
    entry1->i = 1;

    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &key));

    mu_assert(hashmap_remove(map, &key));
    mu_assert_eq_uint(0, hashmap_size(map));
    mu_assert(NULL == hashmap_get(map, &key));

    mu_assert(!hashmap_remove(map, &key));
    mu_assert_eq_uint(0, hashmap_size(map));
    mu_assert(NULL == hashmap_get(map, &key));

    hashmap_destroy(map);
    free(entry1);
mu_test_end()

static void myentry2_free1(void *p)
{
    struct myentry2_t *entry = (struct myentry2_t *) p;
    if (strcmp(entry->str, "\001\050") == 0) {
        char const *key = "\002\003";
        hashmap_remove(entry->map, &key);
    }
    free(entry);
}

/**
 * Tests removing an entry while another removal is pending.
 */
mu_test_begin(test_hashmap_remove_nested)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry2_t, str), 16, hash_string, compare_string_keys, myentry2_free1);
    struct myentry2_t *entry1;
    struct myentry2_t *entry2;
    char const *key1 = "\001\050";
    char const *key2 = "\002\003";

    /* ensure hash collision */
    mu_assert_eq_int(hash_string(&key1), hash_string(&key2));

    entry1 = (struct myentry2_t *) safe_malloc(sizeof(struct myentry2_t));
    entry1->str = key1;
    entry1->map = map;
    hashmap_put(map, entry1);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry1 == hashmap_get(map, &entry1->str));

    entry2 = (struct myentry2_t *) safe_malloc(sizeof(struct myentry2_t));
    entry2->str = key2;
    entry2->map = map;
    hashmap_put(map, entry2);
    mu_assert_eq_uint(2, hashmap_size(map));
    mu_assert(entry2 == hashmap_get(map, &entry2->str));

    mu_assert(hashmap_remove(map, &entry1->str));
    mu_assert_eq_uint(0, hashmap_size(map));
    mu_assert(NULL == hashmap_get(map, &key1));
    mu_assert(NULL == hashmap_get(map, &key2));

    hashmap_destroy(map);
mu_test_end()

static void myentry2_free2(void *p)
{
    struct myentry2_t *entry = (struct myentry2_t *) p;
    if (strcmp(entry->str, "A") == 0 && !entry->in_destroy) {
        hashmap_put(entry->map, entry);
    }
    else {
        free(entry);
    }
}

/**
 * Tests inserting an entry while a removal for the same key is pending.
 */
mu_test_begin(test_hashmap_put_while_remove1)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry2_t, str), 16, hash_string, compare_string_keys, myentry2_free2);
    struct myentry2_t *entry;

    entry = (struct myentry2_t *) safe_malloc(sizeof(struct myentry2_t));
    entry->str = "A";
    entry->map = map;
    entry->in_destroy = FALSE;
    hashmap_put(map, entry);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry == hashmap_get(map, &entry->str));

    mu_assert(hashmap_remove(map, &entry->str));
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry == hashmap_get(map, &entry->str));

    entry->in_destroy = TRUE;
    hashmap_destroy(map);
mu_test_end()

static void myentry2_free3(void *p)
{
    struct myentry2_t *entry = (struct myentry2_t *) p;
    if (!entry->in_destroy) {
        struct myentry2_t *entry_new
            = (struct myentry2_t *) safe_malloc(sizeof (struct myentry2_t));
        char *new_key = strsave(entry->str);
        ++new_key[0];
        entry_new->str = new_key;
        entry_new->map = entry->map;
        entry_new->in_destroy = FALSE;
        hashmap_put(entry_new->map, entry_new);
    }
    free((char *) entry->str);
    free(entry);
}

/**
 * Tests inserting an entry while a removal for some other key is pending.
 */
mu_test_begin(test_hashmap_put_while_remove2)
    struct hashmap_t *map = hashmap_create(offsetof(struct myentry2_t, str), 16, hash_string, compare_string_keys, myentry2_free3);
    struct myentry2_t *entry;
    char str[2] = "A";
    char *key = str;

    entry = (struct myentry2_t *) safe_malloc(sizeof(struct myentry2_t));
    entry->str = strsave(key);
    entry->map = map;
    entry->in_destroy = FALSE;
    hashmap_put(map, entry);
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(entry == hashmap_get(map, &key));

    mu_assert(hashmap_remove(map, &key));
    mu_assert_eq_uint(1, hashmap_size(map));
    mu_assert(!hashmap_get(map, &key));
    ++str[0];
    mu_assert(entry = hashmap_get(map, &key));

    entry->in_destroy = TRUE;
    hashmap_destroy(map);
mu_test_end()

void
test_hashmap(void)
{
    mu_run_test(test_hashmap_create_destroy);
    mu_run_test(test_hashmap_put_get_str1);
    mu_run_test(test_hashmap_put_get_str2);
    mu_run_test(test_hashmap_put_get_str3);
    mu_run_test(test_hashmap_put_get_ci_str1);
    mu_run_test(test_hashmap_put_get_ci_str2);
    mu_run_test(test_hashmap_put_get_ci_str3);
    mu_run_test(test_hashmap_put_get_int1);
    mu_run_test(test_hashmap_put_get_int2);
    mu_run_test(test_hashmap_put_get_int3);
    mu_run_test(test_hashmap_put_get_str_managed1);
    mu_run_test(test_hashmap_put_get_str_managed2);
    mu_run_test(test_hashmap_put_get_str_managed3);
    mu_run_test(test_hashmap_foreach);
    mu_run_test(test_hashmap_to_array);
    mu_run_test(test_hashmap_remove);
    mu_run_test(test_hashmap_remove_managed);
    mu_run_test(test_hashmap_remove_nested);
    mu_run_test(test_hashmap_put_while_remove1);
    mu_run_test(test_hashmap_put_while_remove2);
}
