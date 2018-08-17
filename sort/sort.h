/*
The MIT License (MIT)

Copyright (c) 2018 Kuan-Chung Huang

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

#ifndef SORT_H_
#define SORT_H_

#include <stddef.h>
#include <stdbool.h>

#define BUBBLE_SORT_GEN_RPOTO(name, type) \
void bubble_sort_##name(type data[], size_t num_data);

#define BUBBLE_SORT_GEN(name, type, cmp) \
void bubble_sort_##name(type data[], size_t num_data) \
{ \
    size_t i, j; \
    for (i = 0; i < num_data; ++i) \
    { \
        bool sorted = true; \
        for (j = num_data - 1; j > i; --j) \
        { \
            if (cmp(data[j], data[j - 1]) < 0) \
            { \
                type t = data[j - 1]; \
                data[j - 1] = data[j]; \
                data[j] = t; \
                sorted = false; \
            } \
        } \
        if (sorted) \
        { \
            break; \
        } \
    } \
}

#define BUBBLE_SORT(name, data, num) bubble_sort_##name(dat, num)

#endif /* SORT_H_ */
