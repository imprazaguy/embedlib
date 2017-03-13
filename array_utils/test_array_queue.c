#include "array_queue.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>

#define __UNUSED __attribute__((unused))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


typedef int A_ITEM;
ARRAY_QUEUE_TYPE_8(A_ITEM_QUEUE, A_ITEM);

#define ITEM_BUF_NUM 4
A_ITEM item_buf[ITEM_BUF_NUM];
A_ITEM_QUEUE item_queue;

static A_ITEM item_queue_get_item_at(A_ITEM_QUEUE *queue, int index)
{
    assert_in_range(index, 0, queue->aq_len - 1);
    int q_index = (queue->aq_front + index) % queue->aq_size;
    return queue->aq_item[q_index];
}

static void test_array_queue(void **state __UNUSED)
{
    ARRAY_QUEUE_INIT(&item_queue, item_buf, ITEM_BUF_NUM);

    bool ret;
    int i;
    int test_data[ITEM_BUF_NUM] = { 1, 2, 3, 4 };

    /* Test case: Enqueue items when queue is not full */
    for (i = 0; i < ITEM_BUF_NUM; ++i)
    {
        assert_false(ARRAY_QUEUE_IS_FULL(&item_queue));
        ARRAY_QUEUE_ENQUEUE_RET(&item_queue, i + 1, ret);
        assert_true(ret);
    }
    for (i = 0; i < ITEM_BUF_NUM; ++i)
    {
        assert_int_equal(item_queue_get_item_at(&item_queue, i), test_data[i]);
    }

    /* Test case: Enqueue items when queue is full */
    assert_true(ARRAY_QUEUE_IS_FULL(&item_queue));
    ARRAY_QUEUE_ENQUEUE_RET(&item_queue, ITEM_BUF_NUM + 1, ret);
    assert_false(ret);

    /* Test case: Dequeue items when queue is not empty */
    for (i = 0; i < ITEM_BUF_NUM / 2; ++i)
    {
        A_ITEM item;
        assert_false(ARRAY_QUEUE_IS_EMPTY(&item_queue));
        ARRAY_QUEUE_DEQUEUE_RET(&item_queue, &item, ret);
        assert_true(ret);
        assert_int_equal(item, test_data[i]);
    }
    assert_int_equal(item_queue.aq_front, ITEM_BUF_NUM / 2);
    for (i = ITEM_BUF_NUM / 2; i < ITEM_BUF_NUM; ++i)
    {
        A_ITEM item;
        assert_false(ARRAY_QUEUE_IS_EMPTY(&item_queue));
        ARRAY_QUEUE_DEQUEUE_RET(&item_queue, &item, ret);
        assert_true(ret);
        assert_int_equal(item, test_data[i]);
    }

    /* Test case: Dequeue items when queue is empty */
    {
        A_ITEM item;
        assert_true(ARRAY_QUEUE_IS_EMPTY(&item_queue));
        ARRAY_QUEUE_DEQUEUE_RET(&item_queue, &item, ret);
        assert_false(ret);
    }
}

static void test_array_queue_iterator(void **state __UNUSED)
{
    ARRAY_QUEUE_INIT(&item_queue, item_buf, ITEM_BUF_NUM);

    int i;

    /* Test case: Iterate empty queue */
    A_ITEM *iter = ARRAY_QUEUE_ITER(&item_queue);
    while (iter != ARRAY_QUEUE_ITER_END(&item_queue))
    {
        assert_true(0); /* shouldn't go here */
        ARRAY_QUEUE_ITER_NEXT(&item_queue, iter);
    }

    /* Test case: Iterate non-full queue */
    for (i = 0; i < ITEM_BUF_NUM / 2; ++i)
    {
        ARRAY_QUEUE_ENQUEUE(&item_queue, i + 1);
    }
    assert_false(ARRAY_QUEUE_IS_FULL(&item_queue));
    i = 0;
    iter = ARRAY_QUEUE_ITER(&item_queue);
    while (iter != ARRAY_QUEUE_ITER_END(&item_queue))
    {
        assert_int_equal(*iter, item_queue_get_item_at(&item_queue, i));
        ++i;
        ARRAY_QUEUE_ITER_NEXT(&item_queue, iter);
    }
    assert_int_equal(i, ITEM_BUF_NUM / 2);

    /* Test case: Iterate full queue */
    for (i = ITEM_BUF_NUM / 2; i < ITEM_BUF_NUM; ++i)
    {
        ARRAY_QUEUE_ENQUEUE(&item_queue, i + 1);
    }
    assert_true(ARRAY_QUEUE_IS_FULL(&item_queue));
    i = 0;
    iter = ARRAY_QUEUE_ITER(&item_queue);
    while (iter != ARRAY_QUEUE_ITER_END(&item_queue))
    {
        assert_int_equal(*iter, item_queue_get_item_at(&item_queue, i));
        ++i;
        ARRAY_QUEUE_ITER_NEXT(&item_queue, iter);
    }
    assert_int_equal(i, ITEM_BUF_NUM);
}

static void test_array_queue_iterator_remove(void **state __UNUSED)
{
    typedef struct TEST_DATA_STEP_ {
        int n_data;
        int data[ITEM_BUF_NUM];
    } TEST_DATA_STEP;
    typedef struct TEST_DATA_ {
        int n_step;
        TEST_DATA_STEP step[ITEM_BUF_NUM];
    } TEST_DATA;
    int i;
    TEST_DATA test_data[ITEM_BUF_NUM] = {
            [0] = {
                    4,
                    {
                            { 3, { 2, 3, 4} },
                            { 2, { 3, 4 } },
                            { 1, { 4 } },
                            { 0, { 0 } }
                    }
            },
            [1] = {
                    3,
                    {
                            { 3, { 1, 3, 4 } },
                            { 2, { 1, 4 } },
                            { 1, { 1 } }
                    }
            },
            [2] = {
                    2,
                    {
                            { 3, { 1, 2, 4} },
                            { 2, { 1, 2 } }
                    }
            },
            [3] = {
                    1,
                    {
                            { 3, { 1, 2, 3 } }
                    }
            },
    };

    /* Test case: From i = 0 to ITEM_BUF_NUM - 1, remove item from index i to
     * ITEM_BUF_NUM - 1 one by one.
     */
    int case_i;
    for (case_i = 0; case_i < ITEM_BUF_NUM; ++case_i)
    {
        ARRAY_QUEUE_INIT(&item_queue, item_buf, ITEM_BUF_NUM);
        for (i = 0; i < ITEM_BUF_NUM; ++i)
        {
            ARRAY_QUEUE_ENQUEUE(&item_queue, i + 1);
        }

        A_ITEM *iter = ARRAY_QUEUE_ITER(&item_queue);
        for (i = 0; i < case_i; ++i)
        {
            ARRAY_QUEUE_ITER_NEXT(&item_queue, iter);
        }

        int n_steps = 0;
        while (iter != ARRAY_QUEUE_ITER_END(&item_queue))
        {
            ARRAY_QUEUE_ITER_REMOVE(&item_queue, iter);

            assert_int_equal(item_queue.aq_len,
                    test_data[case_i].step[n_steps].n_data);
            for (i = 0; i < test_data[case_i].step[n_steps].n_data; ++i)
            {
                assert_int_equal(item_queue_get_item_at(&item_queue, i),
                        test_data[case_i].step[n_steps].data[i]);
            }
            ++n_steps;
        }
        assert_int_equal(n_steps, test_data[case_i].n_step);
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_array_queue),
            cmocka_unit_test(test_array_queue_iterator),
            cmocka_unit_test(test_array_queue_iterator_remove),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
