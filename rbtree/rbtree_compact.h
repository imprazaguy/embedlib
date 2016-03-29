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
 * @defgroup rbtree Red black tree
 *
 * @brief A red black tree implementation optimized for code size and memory.
 *
 * This red black tree implementation is optimized for code size and memory
 * usages. It removes the parent pointer and embeds the color bit into child
 * pointers to make memory usages as small as possible. The speed of tree
 * iteration is not downgraded because #RB_PATH context compensates the
 * removal of the parent pointer.
 */

#ifndef RBTREE_COMPACT_H_
#define RBTREE_COMPACT_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RB_RED      0
#define RB_BLACK    1

#define RB_LEFT     0
#define RB_RIGHT    1

/**
 * @addtogroup rbtree
 * @{
 */
/**
 * @brief Red black tree node.
 *
 * The structure using red black tree needs to embed this as a member.
 */
typedef struct RB_NODE_
{
    intptr_t rb_child_color[2];
} RB_NODE;

/**@brief Red black tree root. */
typedef struct RB_ROOT_
{
    RB_NODE *rb_root;
} RB_ROOT;

/**
 * @brief Initializer for red black tree root.
 * @param root Pointer to the red black tree root.
 */
#define RB_ROOT_INITIALIZER(root) { NULL }
/**
 * @brief Initialize a red black tree root.
 * @param root Pointer to the red black tree root.
 */
#define RB_ROOT_INIT(root) do { (root)->rb_root = NULL; } while (0)

/**
 * @brief Get the container of a red black tree node.
 * @param node Address of #RB_NODE embedded in the container.
 * @param type Container type.
 * @param field Member name of the #RB_NODE in the container.
 * @return Pointer to the containter.
 */
#define RB_ENTRY(node, type, field) \
    ((type *)((char *)(node) - offsetof(type, field)))
/**@}*/

static inline RB_NODE *rb_child(RB_NODE *node, int dir)
{
    return (RB_NODE *)(node->rb_child_color[dir] & ~0x1);
}

static inline void rb_set_child(RB_NODE *node, int dir, RB_NODE *child)
{
    node->rb_child_color[dir] = (((intptr_t) child) | (node->rb_child_color[dir] & 0x1));
}

static inline void rb_set_left_child_color(RB_NODE *node, RB_NODE *child, int color)
{
    node->rb_child_color[RB_LEFT] = ((intptr_t) child | color);
}

static inline void rb_copy_left_child_color(RB_NODE *dest, RB_NODE *src)
{
    dest->rb_child_color[RB_LEFT] = src->rb_child_color[RB_LEFT];
}

static inline void rb_set_right_child(RB_NODE *node, RB_NODE *child)
{
    node->rb_child_color[RB_RIGHT] = (intptr_t) child;
}

static inline void rb_copy_right_child(RB_NODE *dest, RB_NODE *src)
{
    dest->rb_child_color[RB_RIGHT] = src->rb_child_color[RB_RIGHT];
}

static inline int rb_color(RB_NODE *node)
{
    return (node->rb_child_color[RB_LEFT] & 0x1);
}

static inline void rb_set_color(RB_NODE *node, int color)
{
    node->rb_child_color[RB_LEFT] = ((node->rb_child_color[RB_LEFT] & ~0x1) | color);
}

typedef struct RB_PATH_ENTRY_
{
    RB_NODE *parent;
    int dir;
} RB_PATH_ENTRY;

/**
 * @addtogroup rbtree
 * @{
 */
/**
 * @brief The context for iteration.
 * @sa rb_first, rb_last, rb_next, rb_prev
 */
typedef struct RB_PATH_
{
    RB_PATH_ENTRY path[sizeof(void *) * 8];
    RB_PATH_ENTRY *cur;
} RB_PATH;

/**
 * @brief Initialize the context for iteration.
 * @param rp Pointer to a #RB_PATH object.
 */
#define RB_PATH_INIT(rp) \
    do { (rp)->cur = (rp)->path; (rp)->cur->parent = NULL; } while(0)

/**
 * @brief Test if a red black tree is empty.
 * @param root Pointer to a red black tree root.
 * @return \c TRUE if empty; otherwise, \c FALSE.
 */
#define RB_EMPTY(root) ((root)->rb_root == NULL)
/**@}*/

RB_NODE *rb_iter(RB_ROOT *root, int dir, RB_PATH *rp);
RB_NODE *rb_iter_next(RB_NODE *node, int dir, RB_PATH *rp);

/**
 * @addtogroup rbtree
 * @{
 */
/**
 * @brief Get first node in the tree.
 * @param root Tree root.
 * @param rp The returned context used for iteration.
 * @return The first node, or \c NULL if the tree is empty.
 */
static inline RB_NODE *rb_first(RB_ROOT *root, RB_PATH *rp)
{
    return rb_iter(root, RB_LEFT, rp);
}

/**
 * @brief Get last node in the tree.
 * @param root Tree root.
 * @param rp The returned context used for iteration.
 * @return The last node, or \c NULL if the tree is empty.
 */
static inline RB_NODE *rb_last(RB_ROOT *root, RB_PATH *rp)
{
    return rb_iter(root, RB_RIGHT, rp);
}

/**
 * @brief Get node's successor in the tree.
 * @param node The node in the tree.
 * @param rp The returned context used for iteration.
 * @return The node's successor in the tree, or \c NULL if the node is the
 * last node.
 */
static inline RB_NODE *rb_next(RB_NODE *node, RB_PATH *rp)
{
    return rb_iter_next(node, RB_RIGHT, rp);
}

/**
 * @brief Get node's predecessor in the tree.
 * @param node The node in the tree.
 * @param rp The returned context used for iteration.
 * @return The node's predecessor in the tree, or \c NULL if the node is the
 * first node.
 */
static inline RB_NODE *rb_prev(RB_NODE *node, RB_PATH *rp)
{
    return rb_iter_next(node, RB_LEFT, rp);
}
/**@}*/

void rb_insert_color(RB_ROOT *root, RB_PATH *rp);

/**
 * @addtogroup rbtree
 * @{
 */
/**
 * @brief Remove a node rom the red black tree.
 *
 * The iteration context \p rp will be changed after calling this function,
 * and should not be used for iteration anymore.
 * @param root Red black tree root.
 * @param node The red black tree node to be removed.
 * @param rp The corresponding iteration context of \p node. It should be
 * generated by #rb_first, #rb_last, #rb_next, #rb_prev, or #RB_FIND.
 */
void rb_remove(RB_ROOT *root, RB_NODE *node, RB_PATH *rp);

/**
 * @brief Insert a node into the red black tree.
 *
 * A duplicate node is not inserted.
 * @param name Identifier.
 * @param root Pointer to the red black tree root.
 * @param node The node to be inserted.
 * @return \a node if successfully inserted, or the pointer to the duplicate
 * node already in the tree.
 */
#define RB_INSERT(name, root, node) name##_rb_insert(root, node)
/**
 * @brief Remove a node by key from the red black tree.
 * @param name Identifier.
 * @param root Pointer to the red black tree root.
 * @param key The key of the node to be removed.
 * @return Pointer to the container removed, or \c NULL if the node is not
 * found.
 */
#define RB_REMOVE(name, root, key) name##_rb_remove(root, key)
/**
 * @brief Find a node in the red black tree.
 * @param name Identifier.
 * @param root Pointer to the red black tree node.
 * @param key The key to search nodes.
 * @param rp Pointer to #RB_PATH to store the context for iteration.
 * @return Pointer to the container, or \c NULL if not found.
 */
#define RB_FIND(name, root, key, rp) name##_rb_find(root, key, rp)
/**@}*/

#define RB_GENERATE_INSERT_PROTO(name, type) \
type *name##_rb_insert(RB_ROOT *root, type *node)
#define RB_GENERATE_INSERT(name, type, field, cmp) \
RB_GENERATE_INSERT_PROTO(name, type) \
{ \
    RB_PATH rp; \
    RB_PATH_INIT(&rp); \
    RB_NODE *p = root->rb_root; \
    if (p != NULL) \
    { \
        int dir; \
        do \
        { \
            type *ent = RB_ENTRY(p, type, field); \
            int c = cmp(node, ent); \
            if (c < 0) \
            { \
                dir = RB_LEFT; \
            } \
            else if (c > 0) \
            { \
                dir = RB_RIGHT; \
            } \
            else \
            { \
                return ent; \
            } \
            ++rp.cur; \
            rp.cur->parent = p; \
            rp.cur->dir = dir; \
            p = rb_child(p, dir); \
        } while (p != NULL);\
        rb_set_child(rp.cur->parent, dir, &node->field); \
    } \
    else \
    { \
        root->rb_root = &node->field; \
    } \
    rb_set_left_child_color(&node->field, NULL, RB_RED); \
    rb_set_right_child(&node->field, NULL); \
    rb_insert_color(root, &rp); \
    return node; \
}

#define RB_GENERATE_FIND_PROTO(name, key_type, type) \
type *name##_rb_find(RB_ROOT *root, key_type key, RB_PATH *rp)
#define RB_GENERATE_FIND(name, key_type, type, field, key_cmp) \
RB_GENERATE_FIND_PROTO(name, key_type, type) \
{ \
    RB_PATH_INIT(rp); \
    RB_NODE *p = root->rb_root; \
    while (p != NULL) \
    { \
        int dir; \
        type *ent = RB_ENTRY(p, type, field); \
        int c = key_cmp(key, ent); \
        if (c < 0) \
        { \
            dir = RB_LEFT; \
        } \
        else if (c > 0) \
        { \
            dir = RB_RIGHT; \
        } \
        else \
        { \
            return ent; \
        } \
        ++rp->cur; \
        rp->cur->parent = p; \
        rp->cur->dir = dir; \
        p = rb_child(p, dir); \
    } \
    return NULL; \
}

#define RB_GENERATE_REMOVE_PROTO(name, key_type, type) \
type *name##_rb_remove(RB_ROOT *root, key_type key)
#define RB_GENERATE_REMOVE(name, key_type, type, field) \
RB_GENERATE_REMOVE_PROTO(name, key_type, type) \
{ \
    RB_PATH rp; \
    type *node = RB_FIND(name, root, key, &rp); \
    if (node != NULL) \
    { \
        rb_remove(root, &node->field, &rp); \
    } \
    return node; \
}

/**
 * @addtogroup rbtree
 * @{
 */
/**
 * @brief Generator for red black tree declaration.
 *
 * Use this macros to generate declartions of red black tree related
 * functions.
 * @param name Identifier.
 * @param key_type Type of key.
 * @param type Type of structure containing #RB_NODE.
 */
#define RB_GEN_PROTO(name, key_type, type) \
RB_GENERATE_INSERT_PROTO(name, type); \
RB_GENERATE_REMOVE_PROTO(name, key_type, type); \
RB_GENERATE_FIND_PROTO(name, key_type, type);

/**
 * @brief Generator for red black tree implementation.
 *
 * Use this macros to generate implementations of red black tree related
 * functions.
 * @param name Identifier.
 * @param key_type Type of key.
 * @param type Type of the container of #RB_NODE.
 * @param field Member name of #RB_NODE in the container.
 * @param key_cmp Comparator for key and node. The comparator should be a
 * function or a macro that has two arguments with types as \a key_type and
 * \a type respectively and returns a value less than, equal to, or greater
 * than zero respectively, if the first argument is less than, equal to, or
 * greater than the second argument.
 * @param cmp Comparator for two nodes. The comparator should be a function
 * or a macro that has two arguments with types as \a type and returns a value
 * less than, equal to, or greater than zero respectively, if the first
 * argument is less than, equal to, or greater than the second argument.
 */
#define RB_GEN(name, key_type, type, field, key_cmp, cmp) \
RB_GENERATE_INSERT(name, type, field, cmp) \
RB_GENERATE_FIND(name, key_type, type, field, key_cmp) \
RB_GENERATE_REMOVE(name, key_type, type, field)
/**@}*/

#endif /* RBTREE_COMPACT_H_ */
