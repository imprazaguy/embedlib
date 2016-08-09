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

/**
 * @defgroup array_utils Array utilities
 *
 * @brief Utility library implemented by array.
 */

#ifndef ARRAY_MAP_H_
#define ARRAY_MAP_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup array_map_pool Array map pool
 * @ingroup array_utils
 *
 * @brief An associate container for allocation/deallocation with an unique key.
 *
 * Array map pool is designed for allocation/deallocation a object with a
 * unique key. #ARRAY_MAP_POOL_GEN uses a initializer and a finalizer when
 * allocating and deallocating a object. Array map pool doesn't use extra
 * storages to store keys. The key should be stored in the objects contained
 * in array map pool.
 *
 * @note For efficiency and usability, it is highly recommended to use pointer
 * to objects as the type of contained objects in the array map pool.
 * @{
 */
/**
 * @brief Define type for a array map pool.
 * @param name  Type name of the array map pool.
 * @param type  Type of objects contained in the array map pool.
 */
#define ARRAY_MAP_POOL_TYPE(name, type) \
typedef struct \
{ \
    type *amp_item; \
    uint32_t amp_size; \
    uint32_t amp_len; \
} name

/**
 * @brief Initialize a array map pool.
 * @param map  Pointer to the array map pool.
 * @param buf  Pointer to the buffer of objects contained in the array map.
 * @param siz  Maximal number of objects in \a buf.
 */
#define ARRAY_MAP_POOL_INIT(map, buf, siz) \
do { \
    (map)->amp_item = (buf); \
    (map)->amp_size = (siz); \
    (map)->amp_len = 0; \
} while (0)
/**@}*/

#define ARRAY_MAP_POOL_BSEARCH(name, map, key, index) name##_array_map_pool_bsearch(map, key, index)

/**
 * @addtogroup array_map_pool
 * @{
 */
/**
 * @brief Get an object with a given key from the array map pool.
 *
 * If the array map pool doesn't have an object with the given \a key, an new
 * object is allocated and initialized by the initializer provided by
 * #ARRAY_MAP_POOL_GEN.
 * @param map  Pointer to the array map pool.
 * @param key  Key to object.
 * @return  The created/existing object with specified \a key, or \c NULL if
 * no free buffer.
 */
#define ARRAY_MAP_POOL_GET(name, map, key) name##_array_map_pool_get(map, key)

/**
 * @brief Free an object from the array map pool.
 *
 * If the array map pool has the object with the given \a key, the object is
 * applied with \a finalizer provided by #ARRAY_MAP_POOL_GEN and then
 * deallocated.
 * @param map  Pointer to the array map pool.
 * @param key  Key of object.
 */
#define ARRAY_MAP_POOL_FREE(name, map, key) name##_array_map_pool_free(map, key)

/**
 * @brief Find the object in the array map pool with specified \a key.
 * @param map  Pointer to the array map pool.
 * @param key  Key of object.
 * @return  The object with the specified \a key if found; otherwise, \c NULL;
 */
#define ARRAY_MAP_POOL_FIND(name, map, key) name##_array_map_pool_find(map, key)
/**@}*/

#define ARRAY_MAP_POOL_GENERATE_BSEARCH_PROTO(name, map_type, key_type) \
bool name##_array_map_pool_bsearch(map_type *map, key_type key, int *index)
#define ARRAY_MAP_POOL_GENERATE_BSEARCH(name, map_type, key_type, key_cmp) \
ARRAY_MAP_POOL_GENERATE_BSEARCH_PROTO(name, map_type, key_type) \
{ \
    if (map->amp_len == 0) \
    { \
        *index = 0; \
    } \
    else \
    { \
        int low = 0; \
        int high = map->amp_len - 1; \
        while (low < high) \
        { \
            int med = (low + high) / 2; \
            int c = key_cmp(map->amp_item[med], key); \
            if (c < 0) \
            { \
                low = med + 1; \
            } \
            else if (c > 0) \
            { \
                high = med - 1; \
            } \
            else \
            { \
                *index = med; \
                return true; \
            } \
        } \
        { \
            int c = key_cmp(map->amp_item[low], key); \
            if (c == 0) \
            { \
                *index = low; \
                return true; \
            } \
            else if (c < 0) \
            { \
                *index = low + 1; \
            } \
            else \
            { \
                *index = low; \
            } \
        } \
    } \
    return false; \
}

#define ARRAY_MAP_POOL_GENERATE_GET_PROTO(name, map_type, key_type, type) \
type name##_array_map_pool_get(map_type *map, key_type key)
#define ARRAY_MAP_POOL_GENERATE_GET(name, map_type, key_type, type, initializer) \
ARRAY_MAP_POOL_GENERATE_GET_PROTO(name, map_type, key_type, type) \
{ \
    int index; \
    if (!ARRAY_MAP_POOL_BSEARCH(name, map, key, &index)) \
    { \
        if (map->amp_len >= map->amp_size) \
        { \
            return (type) 0; \
        } \
        type new_item = map->amp_item[map->amp_len]; \
        initializer(new_item, key); \
        int i; \
        for (i = map->amp_len; i > index; --i) \
        { \
            map->amp_item[i] = map->amp_item[i - 1]; \
        } \
        map->amp_item[index] = new_item; \
        ++map->amp_len; \
    } \
    return map->amp_item[index]; \
}

#define ARRAY_MAP_POOL_GENERATE_FREE_PROTO(name, map_type, key_type) \
void name##_array_map_pool_free(map_type *map, key_type key)
#define ARRAY_MAP_POOL_GENERATE_FREE(name, map_type, key_type, type, finalizer) \
ARRAY_MAP_POOL_GENERATE_FREE_PROTO(name, map_type, key_type) \
{ \
    int index; \
    if (ARRAY_MAP_POOL_BSEARCH(name, map, key, &index)) \
    { \
        uint32_t i; \
        finalizer(map->amp_item[index]); \
        type free_item = map->amp_item[index]; \
        for (i = index; i < map->amp_len - 1; ++i) \
        { \
            map->amp_item[i] = map->amp_item[i + 1]; \
        } \
        map->amp_item[map->amp_len - 1] = free_item; \
        --map->amp_len; \
    } \
}

#define ARRAY_MAP_POOL_GENERATE_FIND_PROTO(name, map_type, key_type, type) \
type name##_array_map_pool_find(map_type *map, key_type key)
#define ARRAY_MAP_POOL_GENERATE_FIND(name, map_type, key_type, type) \
ARRAY_MAP_POOL_GENERATE_FIND_PROTO(name, map_type, key_type, type) \
{ \
    type found = (type) 0; \
    int index; \
    if (ARRAY_MAP_POOL_BSEARCH(name, map, key, &index)) \
    { \
        found = map->amp_item[index]; \
    } \
    return found; \
}

/**
 * @addtogroup array_map_pool
 * @{
 */
/**
 * @brief Generate declaration for a array map pool.
 * @param name  Prefix name.
 * @param map_type  Type of the array map pool.
 * @param key_type  Type of key.
 * @param type  Type of objects contained in the array map pool.
 */
#define ARRAY_MAP_POOL_GEN_PROTO(name, map_type, key_type, type) \
ARRAY_MAP_POOL_GENERATE_BSEARCH_PROTO(name, map_type, key_type); \
ARRAY_MAP_POOL_GENERATE_GET_PROTO(name, map_type, key_type, type); \
ARRAY_MAP_POOL_GENERATE_FREE_PROTO(name, map_type, key_type, type); \
ARRAY_MAP_POOL_GENERATE_FIND_PROTO(name, map_type, key_type, type);

/**
 * @brief Generate implementation for a array map pool.
 * @param name  Prefix name.
 * @param map_type  Type of array map pool.
 * @param key_type  Type of key.
 * @param type  Type of objects contained in the array map pool.
 * @param key_cmp  Comparator between key and objects. It takes two parameters.
 * The first parameter is the object; the second parameter is the key.
 * @param initializer  Initializer applied on objects allocated. It takes two
 * takes two parameters. The first parameter is the object allocated; the
 * second parameter is the key.
 * @param finalizer  Finalizer applied on objects deallocated. It takes one
 * parameter, the object to free.
 */
#define ARRAY_MAP_POOL_GEN(name, map_type, key_type, type, key_cmp, initializer, finalizer) \
ARRAY_MAP_POOL_GENERATE_BSEARCH(name, map_type, key_type, key_cmp) \
ARRAY_MAP_POOL_GENERATE_GET(name, map_type, key_type, type, initializer) \
ARRAY_MAP_POOL_GENERATE_FREE(name, map_type, key_type, type, finalizer) \
ARRAY_MAP_POOL_GENERATE_FIND(name, map_type, key_type, type)
/**@}*/

/**
 * @defgroup array_map Array map
 * @ingroup array_utils
 *
 * @brief An associate container with unique keys.
 *
 * Array map is a general associate container with unique keys. Array map
 * doesn't use extra storages to store keys. The key should be stored in the
 * values contained by array map.
 * @{
 */
/**
 * @brief Define type for a array map.
 * @param name  Type name of the array map.
 * @param type  Type of values contained in the array map.
 */
#define ARRAY_MAP_TYPE(name, type) \
typedef struct \
{ \
    type *am_item; \
    uint32_t am_size; \
    uint32_t am_len; \
} name

/**
 * @brief Initialize a array map.
 * @param map  Pointer to the array map.
 * @param buf  Pointer to the buffer of values contained in the array map.
 * @param siz  Maximal number of values in \a buf.
 */
#define ARRAY_MAP_INIT(map, buf, siz) \
do { \
    (map)->am_item = (buf); \
    (map)->am_size = (siz); \
    (map)->am_len = 0; \
} while (0)
/**@}*/

#define ARRAY_MAP_BSEARCH(name, map, key, index) name##_array_map_bsearch(map, key, index)

/**
 * @addtogroup array_map
 * @{
 */
/**
 * @brief Insert an value with a given key into the array map.
 *
 * If \a key is already existed in the array map, \a value will not be
 * inserted.
 * @param map  Pointer to the array map.
 * @param key  Key associated with the value.
 * @param value  Value to insert.
 * @return  \c true if insertion is successful; otherwise, \c false if key is
 * already existed or the map is full.
 */
#define ARRAY_MAP_INSERT(name, map, key, value) name##_array_map_insert(map, key, value)

/**
 * @brief Remove an value from the array map.
 * @param map  Pointer to the array map.
 * @param key  Key associated with the value.
 */
#define ARRAY_MAP_REMOVE(name, map, key) name##_array_map_remove(map, key)

/**
 * @brief Find the value in the array map with specified \a key.
 * @param map  Pointer to the array map.
 * @param key  Key associated with value.
 * @param pvalue  Returned address of the value found.
 * @return  \c true if found; otherwise, \c false;
 */
#define ARRAY_MAP_FIND(name, map, key, pvalue) name##_array_map_find(map, key, pvalue)
/**@}*/

#define ARRAY_MAP_GENERATE_BSEARCH_PROTO(name, map_type, key_type) \
bool name##_array_map_bsearch(map_type *map, key_type key, int *index)
#define ARRAY_MAP_GENERATE_BSEARCH(name, map_type, key_type, key_cmp) \
ARRAY_MAP_GENERATE_BSEARCH_PROTO(name, map_type, key_type) \
{ \
    if (map->am_len == 0) \
    { \
        *index = 0; \
    } \
    else \
    { \
        int low = 0; \
        int high = map->am_len - 1; \
        while (low < high) \
        { \
            int med = (low + high) / 2; \
            int c = key_cmp(map->am_item[med], key); \
            if (c < 0) \
            { \
                low = med + 1; \
            } \
            else if (c > 0) \
            { \
                high = med - 1; \
            } \
            else \
            { \
                *index = med; \
                return true; \
            } \
        } \
        { \
            int c = key_cmp(map->am_item[low], key); \
            if (c == 0) \
            { \
                *index = low; \
                return true; \
            } \
            else if (c < 0) \
            { \
                *index = low + 1; \
            } \
            else \
            { \
                *index = low; \
            } \
        } \
    } \
    return false; \
}

#define ARRAY_MAP_GENERATE_INSERT_PROTO(name, map_type, key_type, type) \
bool name##_array_map_insert(map_type *map, key_type key, type value)
#define ARRAY_MAP_GENERATE_INSERT(name, map_type, key_type, type) \
ARRAY_MAP_GENERATE_INSERT_PROTO(name, map_type, key_type, type) \
{ \
    int index; \
    if (!ARRAY_MAP_BSEARCH(name, map, key, &index)) \
    { \
        if (map->am_len >= map->am_size) \
        { \
            return false; \
        } \
        int i; \
        for (i = map->am_len; i > index; --i) \
        { \
            map->am_item[i] = map->am_item[i - 1]; \
        } \
        map->am_item[index] = value; \
        ++map->am_len; \
        return true; \
    } \
    return false; \
}

#define ARRAY_MAP_GENERATE_REMOVE_PROTO(name, map_type, key_type) \
void name##_array_map_remove(map_type *map, key_type key)
#define ARRAY_MAP_GENERATE_REMOVE(name, map_type, key_type, type) \
ARRAY_MAP_GENERATE_REMOVE_PROTO(name, map_type, key_type) \
{ \
    int index; \
    if (ARRAY_MAP_BSEARCH(name, map, key, &index)) \
    { \
        uint32_t i; \
        for (i = index; i < map->am_len - 1; ++i) \
        { \
            map->am_item[i] = map->am_item[i + 1]; \
        } \
        --map->am_len; \
    } \
}

#define ARRAY_MAP_GENERATE_FIND_PROTO(name, map_type, key_type, type) \
bool name##_array_map_find(map_type *map, key_type key, type *value)
#define ARRAY_MAP_GENERATE_FIND(name, map_type, key_type, type) \
ARRAY_MAP_GENERATE_FIND_PROTO(name, map_type, key_type, type) \
{ \
    int index; \
    if (ARRAY_MAP_BSEARCH(name, map, key, &index)) \
    { \
        *value = map->am_item[index]; \
        return true; \
    } \
    return false; \
}

/**
 * @addtogroup array_map
 * @{
 */
/**
 * @brief Generate declaration for a array map.
 * @param name  Prefix name.
 * @param map_type  Type of the array map.
 * @param key_type  Type of key.
 * @param type  Type of value contained in the array map.
 */
#define ARRAY_MAP_GEN_PROTO(name, map_type, key_type, type) \
ARRAY_MAP_GENERATE_BSEARCH_PROTO(name, map_type, key_type); \
ARRAY_MAP_GENERATE_INSERT_PROTO(name, map_type, key_type, type); \
ARRAY_MAP_GENERATE_REMOVE_PROTO(name, map_type, key_type, type); \
ARRAY_MAP_GENERATE_FIND_PROTO(name, map_type, key_type, type);

/**
 * @brief Generate implementation for a array map.
 * @param name  Prefix name.
 * @param map_type  Type of array map.
 * @param key_type  Type of key.
 * @param type  Type of values contained in the array map.
 * @param key_cmp  Comparator between key and objects. It takes two parameters.
 * The first parameter is the value; the second parameter is the key.
 */
#define ARRAY_MAP_GEN(name, map_type, key_type, type, key_cmp) \
ARRAY_MAP_GENERATE_BSEARCH(name, map_type, key_type, key_cmp) \
ARRAY_MAP_GENERATE_INSERT(name, map_type, key_type, type) \
ARRAY_MAP_GENERATE_REMOVE(name, map_type, key_type, type) \
ARRAY_MAP_GENERATE_FIND(name, map_type, key_type, type)
/**@}*/

#endif /* ARRAY_MAP_H_ */
