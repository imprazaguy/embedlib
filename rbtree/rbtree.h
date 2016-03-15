#ifndef RBTREE_H_
#define RBTREE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RB_RED      0
#define RB_BLACK    1

#define RB_LEFT     0
#define RB_RIGHT    1

typedef struct RB_NODE_
{
    intptr_t rb_parent_color;
    struct RB_NODE_ *rb_child[2];
} RB_NODE;

typedef struct RB_ROOT_
{
    RB_NODE *rb_root;
} RB_ROOT;

#define RB_ROOT_INITIALIZER(root) { NULL }
#define RB_ROOT_INIT(root) do { (root)->rb_root = NULL; } while (0)

#define RB_ENTRY(node, type, field) \
    ((type *)((char *)(node) - offsetof(type, field)))

static inline RB_NODE *rb_parent(RB_NODE *node)
{
    return (RB_NODE *) (node->rb_parent_color & ~0x1);
}

static inline void rb_set_parent(RB_NODE *node, RB_NODE *parent)
{
    node->rb_parent_color = ((intptr_t) parent
            | (node->rb_parent_color & 0x1));
}

static inline void rb_set_parent_color(RB_NODE *node, RB_NODE *parent, int color)
{
    node->rb_parent_color = ((intptr_t) parent | (color & 0x1));
}

static inline RB_NODE *rb_child(RB_NODE *node, int dir)
{
    return node->rb_child[dir];
}

static inline void rb_set_child(RB_NODE *node, int dir, RB_NODE *child)
{
    node->rb_child[dir] = child;
}

static inline int rb_color(RB_NODE *node)
{
    return (node->rb_parent_color & 0x1);
}

static inline void rb_set_color(RB_NODE *node, int color)
{
    node->rb_parent_color = ((node->rb_parent_color & ~0x1) | color);
}

#define RB_EMPTY(root) ((root)->rb_root == NULL)

RB_NODE *rb_iter(RB_ROOT *root, int dir);

static inline RB_NODE *rb_first(RB_ROOT *root)
{
    return rb_iter(root, RB_LEFT);
}

static inline RB_NODE *rb_last(RB_ROOT *root)
{
    return rb_iter(root, RB_RIGHT);
}

RB_NODE *rb_iter_next(RB_NODE *node, int dir);

static inline RB_NODE *rb_next(RB_NODE *node)
{
    return rb_iter_next(node, RB_RIGHT);
}

static inline RB_NODE *rb_prev(RB_NODE *node)
{
    return rb_iter_next(node, RB_LEFT);
}

void rb_insert_color(RB_ROOT *root, RB_NODE *node);
void rb_remove(RB_ROOT *root, RB_NODE *node);

#define RB_INSERT(name, root, node) name##_rb_insert(root, node)
#define RB_REMOVE(name, root, key) name##_rb_remove(root, key)
#define RB_FIND(name, root, key) name##_rb_find(root, key)

#define RB_GENERATE_INSERT_PROTO(name, type) \
type *name##_rb_insert(RB_ROOT *root, type *node)
#define RB_GENERATE_INSERT(name, type, field, cmp) \
RB_GENERATE_INSERT_PROTO(name, type) \
{ \
    RB_NODE *parent; \
    RB_NODE *p = root->rb_root; \
    if (p != NULL) \
    { \
        int dir; \
        do \
        { \
            parent = p; \
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
            p = p->rb_child[dir]; \
        } while (p != NULL); \
        parent->rb_child[dir] = &node->field; \
    } \
    else \
    { \
        parent = NULL; \
        root->rb_root = &node->field; \
    } \
    rb_set_parent_color(&node->field, parent, RB_RED); \
    node->field.rb_child[RB_LEFT] = NULL; \
    node->field.rb_child[RB_RIGHT] = NULL; \
    rb_insert_color(root, &node->field); \
    return node; \
}

#define RB_GENERATE_FIND_PROTO(name, key_type, type) \
type *name##_rb_find(RB_ROOT *root, key_type key)
#define RB_GENERATE_FIND(name, key_type, type, field, key_cmp) \
RB_GENERATE_FIND_PROTO(name, key_type, type) \
{ \
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
        p = p->rb_child[dir]; \
    } \
    return NULL; \
}

#define RB_GENERATE_REMOVE_PROTO(name, key_type, type) \
type *name##_rb_remove(RB_ROOT *root, key_type key)
#define RB_GENERATE_REMOVE(name, key_type, type, field) \
RB_GENERATE_REMOVE_PROTO(name, key_type, type) \
{ \
    type *node = RB_FIND(name, root, key); \
    if (node != NULL) \
    { \
        rb_remove(root, &node->field); \
    } \
    return node; \
}

#define RB_GEN_PROTO(name, key_type, type) \
RB_GENERATE_INSERT_PROTO(name, type); \
RB_GENERATE_REMOVE_PROTO(name, key_type, type); \
RB_GENERATE_FIND_PROTO(name, key_type, type);

#define RB_GEN(name, key_type, type, field, key_cmp, cmp) \
RB_GENERATE_INSERT(name, type, field, cmp) \
RB_GENERATE_FIND(name, key_type, type, field, key_cmp) \
RB_GENERATE_REMOVE(name, key_type, type, field)

#endif /* RBTREE_H_ */
