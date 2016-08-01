#include "array_map.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>

#define __UNUSED __attribute__((unused))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


typedef struct A_ITEM_
{
    int key;
    int val;
} A_ITEM;

ARRAY_MAP_POOL_TYPE(A_ITEM_POOL, A_ITEM *);

#define A_ITEM_MAP_POOL_KEY_CMP(item, key) ((item)->key - (key))
#define A_ITEM_MAP_POOL_INITIALIZER(item, key) ((item)->key = (key))
#define A_ITEM_MAP_POOL_FINALIZER(item)
ARRAY_MAP_POOL_GEN(item_pool, A_ITEM_POOL, int, A_ITEM *, A_ITEM_MAP_POOL_KEY_CMP, A_ITEM_MAP_POOL_INITIALIZER, A_ITEM_MAP_POOL_FINALIZER)

static A_ITEM *item_pool_get_item_at(A_ITEM_POOL *pool, int index)
{
    assert_in_range(index, 0, pool->amp_len - 1);
    return pool->amp_item[index];
}

#define ITEM_BUF_NUM 4

A_ITEM item_buf[ITEM_BUF_NUM];
A_ITEM *item_ptr[ITEM_BUF_NUM];
A_ITEM_POOL item_pool;

static void item_pool_init(void)
{
    int i;
    for (i = 0; i < ITEM_BUF_NUM; ++i)
    {
        item_ptr[i] = &item_buf[i];
    }
    ARRAY_MAP_POOL_INIT(&item_pool, item_ptr, ITEM_BUF_NUM);
}

static void test_array_map_pool_bsearch(void **state __UNUSED)
{
    A_ITEM test_buf[] = {
            { .key = 2 },
            { .key = 4 },
            { .key = 6 },
            { .key = 8 },
    };
    const unsigned int N = ARRAY_SIZE(test_buf);
    A_ITEM *test_item[N];
    unsigned int i;
    for (i = 0; i < N; ++i)
    {
        test_item[i] = &test_buf[i];
    }
    A_ITEM_POOL test_pool;
    ARRAY_MAP_POOL_INIT(&test_pool, test_item, N);

    /* Test case: Search empty map */
    {
        int index;
        bool found = ARRAY_MAP_POOL_BSEARCH(item_pool, &test_pool, 2, &index);
        assert_false(found);
        assert_int_equal(index, 0);
    }

    /* Test case: Search existent key */
    test_pool.amp_len = N;
    for (i = 0; i < N; ++i)
    {
        int index;
        bool found = ARRAY_MAP_POOL_BSEARCH(item_pool, &test_pool,
                (i + 1) * 2, &index);
        assert_true(found);
        assert_int_equal(index, i);
    }

    /* Test case: Search nonexistent key */
    {
        int test_key[] = { 1, 3, 5, 7, 9};
        int test_index[] = { 0, 1, 2, 3, 4};
        for (i = 0; i < ARRAY_SIZE(test_key); ++i)
        {
            int index;
            bool found = ARRAY_MAP_POOL_BSEARCH(item_pool, &test_pool,
                    test_key[i], &index);
            assert_false(found);
            assert_int_equal(index, test_index[i]);
        }
    }
}

static void test_array_map_pool_get(void **state __UNUSED)
{
    item_pool_init();
    typedef struct TEST_DATA_
    {
        int insert;
        int len;
        int data[ITEM_BUF_NUM];
    } TEST_DATA;
    TEST_DATA td[ITEM_BUF_NUM] = {
            { 3, 1, {3} },
            { 1, 2, {1, 3}},
            { 4, 3, {1, 3, 4}},
            { 2, 4, {1, 2, 3, 4}},
    };

    /* Test case: Allocate new item */
    A_ITEM *item;
    int i, j;
    for (i = 0; i < ITEM_BUF_NUM; ++i)
    {
        item = ARRAY_MAP_POOL_GET(item_pool, &item_pool, td[i].insert);
        assert_non_null(item);
        assert_int_equal(td[i].len, i + 1);
        for (j = 0; j < td[i].len; ++j)
        {
            assert_int_equal(item_pool_get_item_at(&item_pool, j)->key,
                    td[i].data[j]);
        }
    }

    /* Test case: Get item by existed key */
    item = ARRAY_MAP_POOL_GET(item_pool, &item_pool, 1);
    assert_non_null(item);

    /* Test case: No empty space to allocate */
    item = ARRAY_MAP_POOL_GET(item_pool, &item_pool, 5);
    assert_null(item);
}

static int item_ptr_cmp(const void *a, const void *b)
{
    return ((intptr_t)*((A_ITEM **) a)) - ((intptr_t) *(A_ITEM **) b);
}

static void test_array_map_pool_free(void **state __UNUSED)
{
    item_pool_init();
    int i;
    for (i = 0; i < ITEM_BUF_NUM; ++i)
    {
        ARRAY_MAP_POOL_GET(item_pool, &item_pool, i + 1);
    }

    typedef struct TEST_DATA_
    {
        int remove;
        int len;
        int data[ITEM_BUF_NUM];
    } TEST_DATA;
    TEST_DATA td[] = {
            { 5, 4, {1, 2, 3, 4} },
            { 3, 3, {1, 2, 4} },
            { 1, 2, {2, 4} },
            { 4, 1, {2} },
            { 2, 0, {}},
    };

    /* Test case: Remove nonexistent key and existent key */
    {
        unsigned int i;
        int j;
        for (i = 0; i < ARRAY_SIZE(td); ++i)
        {
            ARRAY_MAP_POOL_FREE(item_pool, &item_pool, td[i].remove);
            assert_int_equal(item_pool.amp_len, td[i].len);
            for (j = 0; j < td[i].len; ++j)
            {
                assert_int_equal(item_pool_get_item_at(&item_pool, j)->key,
                        td[i].data[j]);
            }
        }
        /* Test allocation is not corrupted */
        A_ITEM *item_ptr[ITEM_BUF_NUM];
        for (i = 0; i < ITEM_BUF_NUM; ++i)
        {
            item_ptr[i] = item_pool.amp_item[i];
        }
        qsort(item_ptr, ITEM_BUF_NUM, sizeof (item_ptr[0]), item_ptr_cmp);
        for (i = 0; i < ITEM_BUF_NUM; ++i)
        {
            assert_ptr_equal(item_ptr[i], &item_buf[i]);
        }
    }
}


ARRAY_MAP_TYPE(A_ITEM_MAP, A_ITEM);

#define A_ITEM_MAP_KEY_CMP(item, key) ((item).key - (key))
ARRAY_MAP_GEN(item_map, A_ITEM_MAP, int, A_ITEM, A_ITEM_MAP_KEY_CMP)

static A_ITEM *item_map_get_item_at(A_ITEM_MAP *map, int index)
{
    assert_in_range(index, 0, map->am_len - 1);
    return &map->am_item[index];
}

A_ITEM item_map_buf[ITEM_BUF_NUM];
A_ITEM_MAP item_map;

static void item_map_init(void)
{
    ARRAY_MAP_INIT(&item_map, item_map_buf, ITEM_BUF_NUM);
}

static void test_array_map_bsearch(void **state __UNUSED)
{
    A_ITEM test_buf[] = {
            { .key = 2 },
            { .key = 4 },
            { .key = 6 },
            { .key = 8 },
    };
    const unsigned int N = ARRAY_SIZE(test_buf);
    A_ITEM_MAP test_map;
    ARRAY_MAP_INIT(&test_map, test_buf, N);

    /* Test case: Search empty map */
    {
        int index;
        bool found = ARRAY_MAP_BSEARCH(item_map, &test_map, 2, &index);
        assert_false(found);
        assert_int_equal(index, 0);
    }

    /* Test case: Search existent key */
    test_map.am_len = N;
    unsigned int i;
    for (i = 0; i < N; ++i)
    {
        int index;
        bool found = ARRAY_MAP_BSEARCH(item_map, &test_map,
                (i + 1) * 2, &index);
        assert_true(found);
        assert_int_equal(index, i);
    }

    /* Test case: Search nonexistent key */
    {
        int test_key[] = { 1, 3, 5, 7, 9};
        int test_index[] = { 0, 1, 2, 3, 4};
        for (i = 0; i < ARRAY_SIZE(test_key); ++i)
        {
            int index;
            bool found = ARRAY_MAP_BSEARCH(item_map, &test_map,
                    test_key[i], &index);
            assert_false(found);
            assert_int_equal(index, test_index[i]);
        }
    }
}

static void test_array_map_insert(void **state __UNUSED)
{
    item_map_init();
    typedef struct TEST_DATA_
    {
        A_ITEM insert;
        int len;
        int data[ITEM_BUF_NUM];
    } TEST_DATA;
    TEST_DATA td[ITEM_BUF_NUM] = {
            { {3, 1}, 1, {3} },
            { {1, 2}, 2, {1, 3}},
            { {4, 3}, 3, {1, 3, 4}},
            { {2, 4}, 4, {1, 2, 3, 4}},
    };

    /* Test case: Allocate new item */
    int i, j;
    bool inserted;
    for (i = 0; i < ITEM_BUF_NUM; ++i)
    {
        inserted = ARRAY_MAP_INSERT(item_map, &item_map, td[i].insert.key,
                td[i].insert);
        assert_true(inserted);
        assert_int_equal(td[i].len, i + 1);
        for (j = 0; j < td[i].len; ++j)
        {
            assert_int_equal(item_map_get_item_at(&item_map, j)->key,
                    td[i].data[j]);
        }
    }

    /* Test case: Get item by existed key */
    {
        A_ITEM item = { 1, 5 };
        inserted = ARRAY_MAP_INSERT(item_map, &item_map, item.key, item);
        assert_false(inserted);
        A_ITEM item2;
        bool found = ARRAY_MAP_FIND(item_map, &item_map, item.key, &item2);
        assert_true(found);
        assert_int_not_equal(item.val, item2.val);
    }

    /* Test case: No empty space to allocate */
    {
        A_ITEM item = { 5, 5 };
        inserted = ARRAY_MAP_INSERT(item_map, &item_map, item.key, item);
        assert_false(inserted);
    }
}

static void test_array_map_remove(void **state __UNUSED)
{
    item_map_init();
    int i;
    for (i = 0; i < ITEM_BUF_NUM; ++i)
    {
        A_ITEM item = { i + 1, i + 1 };
        bool inserted = ARRAY_MAP_INSERT(item_map, &item_map, item.key, item);
        assert_true(inserted);
    }

    typedef struct TEST_DATA_
    {
        int remove;
        int len;
        int data[ITEM_BUF_NUM];
    } TEST_DATA;
    TEST_DATA td[] = {
            { 5, 4, {1, 2, 3, 4} },
            { 3, 3, {1, 2, 4} },
            { 1, 2, {2, 4} },
            { 4, 1, {2} },
            { 2, 0, {}},
    };

    /* Test case: Remove nonexistent key and existent key */
    {
        unsigned int i;
        int j;
        for (i = 0; i < ARRAY_SIZE(td); ++i)
        {
            ARRAY_MAP_REMOVE(item_map, &item_map, td[i].remove);
            assert_int_equal(item_map.am_len, td[i].len);
            for (j = 0; j < td[i].len; ++j)
            {
                assert_int_equal(item_map_get_item_at(&item_map, j)->key,
                        td[i].data[j]);
            }
        }
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_array_map_pool_bsearch),
            cmocka_unit_test(test_array_map_pool_get),
            cmocka_unit_test(test_array_map_pool_free),
            cmocka_unit_test(test_array_map_bsearch),
            cmocka_unit_test(test_array_map_insert),
            cmocka_unit_test(test_array_map_remove),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
