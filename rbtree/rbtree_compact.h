#ifndef RBTREE_COMPACT_H_
#define RBTREE_COMPACT_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RB_RED      0
#define RB_BLACK    1

#define RB_LEFT     0
#define RB_RIGHT    1

typedef struct RB_NODE_
{
    intptr_t rb_child_color[2];
} RB_NODE;

typedef struct RB_ROOT_
{
    RB_NODE *rb_root;
} RB_ROOT;

#define RB_ROOT_INITIALIZER(root) { NULL }
#define RB_ROOT_INIT(root) do { (root)->rb_root = NULL; } while (0)

#define RB_ENTRY(node, type, field) \
    ((type *)((char *)(node) - offsetof(type, field)))

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

typedef struct RB_PATH_
{
    RB_PATH_ENTRY path[sizeof(void *) * 8];
    RB_PATH_ENTRY *cur;
} RB_PATH;

#define RB_PATH_INIT(rp) \
    do { (rp)->cur = (rp)->path; (rp)->cur->parent = NULL; } while(0)

void rb_insert_color(RB_ROOT *root, RB_PATH *rp);
void rb_remove(RB_ROOT *root, RB_NODE *node, RB_PATH *rp);

#define RB_INSERT(name, root, node) name##_rb_insert(root, node)
#define RB_REMOVE(name, root, key) name##_rb_remove(root, key)
#define RB_FIND(name, root, key, rp) name##_rb_find(root, key, rp)

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

#define RB_GEN_PROTO(name, key_type, type) \
RB_GENERATE_INSERT_PROTO(name, type); \
RB_GENERATE_REMOVE_PROTO(name, key_type, type); \
RB_GENERATE_FIND_PROTO(name, key_type, type);

#define RB_GEN(name, key_type, type, field, key_cmp, cmp) \
RB_GENERATE_INSERT(name, type, field, cmp) \
RB_GENERATE_FIND(name, key_type, type, field, key_cmp) \
RB_GENERATE_REMOVE(name, key_type, type, field)

#endif /* RBTREE_COMPACT_H_ */
