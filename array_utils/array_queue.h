/*
The MIT License (MIT)

Copyright (c) 2016 Kuan-Chung Huang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#ifndef ARRAY_QUEUE_H_
#define ARRAY_QUEUE_H_

#include <stdint.h>
#include <stdbool.h>

#define ARRAY_QUEUE_TYPE_IMPL(name, type, size_type) \
typedef struct \
{ \
    type *aq_item; \
    size_type aq_size; \
    size_type aq_front; \
    size_type aq_back; \
    size_type aq_len; \
} name

#define ARRAY_QUEUE_TYPE_8(name, type) ARRAY_QUEUE_TYPE_IMPL(name, type, uint8_t)
#define ARRAY_QUEUE_TYPE_16(name, type) ARRAY_QUEUE_TYPE_IMPL(name, type, uint16_t)
#define ARRAY_QUEUE_TYPE_32(name, type) ARRAY_QUEUE_TYPE_IMPL(name, type, uint32_t)

#define ARRAY_QUEUE_INIT(q, buf, siz) \
do { \
    (q)->aq_item = (buf); \
    (q)->aq_size = (siz); \
    (q)->aq_front = 0; \
    (q)->aq_back = 0; \
    (q)->aq_len = 0; \
} while (0)

#define ARRAY_QUEUE_IS_EMPTY(q) ((q)->aq_len == 0)

#define ARRAY_QUEUE_IS_FULL(q) ((q)->aq_len == (q)->aq_size)

#define ARRAY_QUEUE_FRONT(q) ((q)->aq_item[(q)->aq_front])

#define _ARRAY_QUEUE_RET(ret, val) ((ret) = (val))
#define _ARRAY_QUEUE_NO_RET(ret, val)

#define _ARRAY_QUEUE_ENQUEUE_IMPL(q, data, ret_func, ret) \
do { \
    if (ARRAY_QUEUE_IS_FULL(q)) \
    { \
        ret_func(ret, false); \
    } \
    else \
    { \
        (q)->aq_item[(q)->aq_back] = (data); \
        (q)->aq_back = ((q)->aq_back + 1) % (q)->aq_size; \
        ++(q)->aq_len; \
        ret_func(ret, true); \
    } \
} while (0)

#define _ARRAY_QUEUE_DEQUEUE_IMPL(q, pdata, ret_func, ret) \
do { \
    if (ARRAY_QUEUE_IS_EMPTY(q)) \
    { \
        ret_func(ret, false); \
    } \
    else \
    { \
        *(pdata) = (q)->aq_item[(q)->aq_front]; \
        (q)->aq_front = ((q)->aq_front + 1) % (q)->aq_size; \
        --(q)->aq_len; \
        ret_func(ret, true); \
    } \
} while (0)

#define ARRAY_QUEUE_ENQUEUE(q, data) _ARRAY_QUEUE_ENQUEUE_IMPL(q, data, _ARRAY_QUEUE_NO_RET,)
#define ARRAY_QUEUE_DEQUEUE(q, pdata) _ARRAY_QUEUE_DEQUEUE_IMPL(q, pdata, _ARRAY_QUEUE_NO_RET,)

#define ARRAY_QUEUE_ENQUEUE_RET(q, data, ret) _ARRAY_QUEUE_ENQUEUE_IMPL(q, data, _ARRAY_QUEUE_RET, ret)
#define ARRAY_QUEUE_DEQUEUE_RET(q, pdata, ret) _ARRAY_QUEUE_DEQUEUE_IMPL(q, pdata, _ARRAY_QUEUE_RET, ret)

#endif /* ARRAY_QUEUE_H_ */
