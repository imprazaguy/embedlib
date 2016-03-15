#include "rbtree.h"
#include <assert.h>


void rb_rotate(RB_ROOT *root, RB_NODE *node, int direction)
{
    RB_NODE *pivot = node->rb_child[direction ^ 1];
    node->rb_child[direction ^ 1] = pivot->rb_child[direction];
    if (pivot->rb_child[direction] != NULL)
    {
        rb_set_parent(pivot->rb_child[direction], node);
    }
    RB_NODE *node_parent = rb_parent(node);
    rb_set_parent(pivot, node_parent);
    if (node_parent == NULL)
    {
        root->rb_root = pivot;
    }
    else
    {
        int cdir = (node_parent->rb_child[direction] == node
                ? direction : (direction ^ 1));
        node_parent->rb_child[cdir] = pivot;
    }
    pivot->rb_child[direction] = node;
    rb_set_parent(node, pivot);
}

void rb_insert_color(RB_ROOT *root, RB_NODE *node)
{
    RB_NODE *parent;
    while ((parent = rb_parent(node)) != NULL && rb_color(parent) == RB_RED)
    {
        RB_NODE *gparent = rb_parent(parent);
        int dir = (gparent->rb_child[RB_LEFT] == parent ? RB_LEFT : RB_RIGHT);
        RB_NODE *uncle = gparent->rb_child[dir ^ 1];
        if (uncle != NULL && rb_color(uncle) == RB_RED)
        {
            /* Case 1: Uncle is red.
             */
            rb_set_color(parent, RB_BLACK);
            rb_set_color(uncle, RB_BLACK);
            rb_set_color(gparent, RB_RED);
            node = gparent;
        }
        else
        {
            if (parent->rb_child[dir ^ 1] == node)
            {
                /* Case 2: Uncle is black and the path from grandparent to
                 * child is not straight.
                 */
                rb_rotate(root, parent, dir);
                RB_NODE *t = node;
                node = parent;
                parent = t;
            }
            /* Case 3: Uncle is black and the path from grandparent to child
             * is straight.
             */
            rb_set_color(parent, RB_BLACK);
            rb_set_color(gparent, RB_RED);
            rb_rotate(root, gparent, dir ^ 1);
        }
    }
    rb_set_color(root->rb_root, RB_BLACK);
}

void rb_remove_color(RB_ROOT *root, RB_NODE *parent, RB_NODE *node)
{
    while ((node == NULL || rb_color(node) == RB_BLACK)
            && node != root->rb_root)
    {
        int dir = (parent->rb_child[RB_LEFT] == node ? RB_LEFT : RB_RIGHT);
        RB_NODE *sibling = parent->rb_child[dir ^ 1];
        /* Because the node deleted in rb_remove() must be black to go here,
         * its original sibling cannot be NULL; otherwise, it violates the
         * rules.
         */
        if (rb_color(sibling) == RB_RED)
        {
            /* Case 1: Sibling is red.
             */
            rb_set_color(sibling, RB_BLACK);
            rb_set_color(parent, RB_RED);
            rb_rotate(root, parent, dir);
            sibling = parent->rb_child[dir ^ 1];
        }
        if (sibling->rb_child[dir ^ 1] == NULL
                || rb_color(sibling->rb_child[dir ^ 1]) == RB_BLACK)
        {
            if (sibling->rb_child[dir] == NULL
                    || rb_color(sibling->rb_child[dir]) == RB_BLACK)
            {
                /* Case 2: Sibling is black and both its children are black.
                */
                rb_set_color(sibling, RB_RED);
                node = parent;
                parent = rb_parent(node);
                continue;
            }

            /* Case 3: Sibling is black and its child in the same postition
             * as node is red and its child in the opposite position as
             * node is black.
             */
            rb_set_color(sibling->rb_child[dir], RB_BLACK);
            assert(sibling->rb_child[dir] != NULL);
            rb_set_color(sibling, RB_RED);
            rb_rotate(root, sibling, dir ^ 1);
            sibling = parent->rb_child[dir ^ 1];
        }

        /* Case 4:
        */
        rb_set_color(sibling, rb_color(parent));
        rb_set_color(parent, RB_BLACK);
        rb_set_color(sibling->rb_child[dir ^ 1], RB_BLACK);
        assert(sibling->rb_child[dir ^ 1] != NULL);
        rb_rotate(root, parent, dir);
        node = root->rb_root;
        break;
    }
    if (node != NULL)
    {
        rb_set_color(node, RB_BLACK);
    }
}

static inline void rb_change_child(RB_NODE *parent, RB_NODE *old_child,
        RB_NODE *new_child)
{
    int dir = (parent->rb_child[RB_LEFT] == old_child ? RB_LEFT : RB_RIGHT);
    parent->rb_child[dir] = new_child;
}

void rb_remove(RB_ROOT *root, RB_NODE *node)
{
    RB_NODE *replacement;
    RB_NODE *transplanter;
    RB_NODE *parent;
    RB_NODE *node_parent = rb_parent(node);
    int deleted_color = rb_color(node);
    if (node->rb_child[RB_LEFT] == NULL) /* node has at most one right child */
    {
        transplanter = node->rb_child[RB_RIGHT];
        replacement = transplanter;
        if (transplanter != NULL)
        {
            rb_set_parent(transplanter, node_parent);
        }
        parent = node_parent;
    }
    else if (node->rb_child[RB_RIGHT] == NULL) /* node has one left child */
    {
        transplanter = node->rb_child[RB_LEFT];
        replacement = transplanter;
        rb_set_parent(transplanter, node_parent);
        parent = node_parent;
    }
    else /* node has two children */
    {
        RB_NODE *successor = node->rb_child[RB_RIGHT];
        RB_NODE *left;
        while ((left = successor->rb_child[RB_LEFT]) != NULL)
        {
            successor = left;
        }
        deleted_color = rb_color(successor);
        RB_NODE *child = successor->rb_child[RB_RIGHT];
        replacement = child;

        parent = rb_parent(successor);
        if (parent != node)
        {
            rb_change_child(parent, successor, child);
            if (child != NULL)
            {
                rb_set_parent(child, parent);
            }
            successor->rb_child[RB_RIGHT] = node->rb_child[RB_RIGHT];
            rb_set_parent(node->rb_child[RB_RIGHT], successor);
        }
        else
        {
            /* In this case, the successor replaces the original parent. */
            parent = successor;
        }
        successor->rb_child[RB_LEFT] = node->rb_child[RB_LEFT];
        rb_set_parent(node->rb_child[RB_LEFT], successor);
        successor->rb_parent_color = node->rb_parent_color;
        transplanter = successor;
    }
    if (node_parent != NULL)
    {
        rb_change_child(node_parent, node, transplanter);
    }
    else
    {
        root->rb_root = transplanter;
    }

    if (deleted_color == RB_BLACK)
    {
        rb_remove_color(root, parent, replacement);
    }
}

RB_NODE *rb_iter(RB_ROOT *root, int dir)
{
    RB_NODE *p = root->rb_root;
    if (p != NULL)
    {
        RB_NODE *q;
        while ((q = rb_child(p, dir)) != NULL)
        {
            p = q;
        }
    }
    return p;
}

RB_NODE *rb_iter_next(RB_NODE *node, int dir)
{
    RB_NODE *p = NULL;
    if (node != NULL)
    {
        p = rb_child(node, dir);
        if (p != NULL)
        {
            RB_NODE *q;
            while ((q = rb_child(p, dir ^ 1)) != NULL)
            {
                p = q;
            }
        }
        else
        {
            while ((p = rb_parent(node)) != NULL
                    && rb_child(p, dir) == node)
            {
                node = p;
            }
        }
    }
    return p;
}

