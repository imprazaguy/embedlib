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

int main(void)
{
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_array_queue),
            cmocka_unit_test(test_array_queue_iterator),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
