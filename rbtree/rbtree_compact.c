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

#include "rbtree_compact.h"


RB_NODE *rb_rotate(RB_NODE *node, int direction)
{
    RB_NODE *pivot = rb_child(node, direction ^ 1);
    rb_set_child(node, direction ^ 1, rb_child(pivot, direction));
    rb_set_child(pivot, direction, node);
    return pivot;
}

void rb_insert_color(RB_ROOT *root, RB_PATH *rp)
{
    RB_NODE *parent;
    while ((parent = rp->cur->parent) != NULL && rb_color(parent) == RB_RED)
    {
        /* NOTE: gparent can't be NULL; otherwise, it implies parent is root
         * and black, contradicting the loop condition.
         */
        RB_NODE *gparent = rp->cur[-1].parent;
        int dir = rp->cur[-1].dir;
        RB_NODE *uncle = rb_child(gparent, dir ^ 1);
        if (uncle != NULL && rb_color(uncle) == RB_RED)
        {
            /* Case 1: Uncle is red.
             */
            rb_set_color(parent, RB_BLACK);
            rb_set_color(uncle, RB_BLACK);
            rb_set_color(gparent, RB_RED);
            rp->cur -= 2;
        }
        else
        {
            if (rp->cur->dir != dir)
            {
                /* Case 2: Uncle is black and the path from grandparent to
                 * child is not straight.
                 */
                parent = rb_rotate(parent, dir);
                rb_set_child(gparent, dir, parent);
            }
            /* Case 3: Uncle is black and the path from grandparent to child
             * is straight.
             */
            rb_set_color(parent, RB_BLACK);
            rb_set_color(gparent, RB_RED);
            RB_NODE *t = rb_rotate(gparent, dir ^ 1);
            RB_NODE *ggparent = rp->cur[-2].parent;
            if (ggparent != NULL)
            {
                rb_set_child(ggparent, rp->cur[-2].dir, t);
            }
            else
            {
                root->rb_root = t;
            }
            break;
        }
    }
    rb_set_color(root->rb_root, RB_BLACK);
}


void rb_remove_color(RB_ROOT *root, RB_NODE *node, RB_PATH *rp)
{
    while ((node == NULL || rb_color(node) == RB_BLACK)
            && node != root->rb_root)
    {
        RB_NODE *parent = rp->cur->parent;
        int dir = rp->cur->dir;
        RB_NODE *sibling = rb_child(parent, dir ^ 1);
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
            RB_NODE *t = rb_rotate(parent, dir);
            RB_NODE *gparent = rp->cur[-1].parent;
            if (gparent != NULL)
            {
                rb_set_child(gparent, rp->cur[-1].dir, t);
            }
            else
            {
                root->rb_root = t;
            }
            rp->cur[1] = rp->cur[0];
            rp->cur->parent = t;
            rp->cur->dir = dir;
            ++rp->cur;
            sibling = rb_child(parent, dir ^ 1);
        }
        RB_NODE *nibling, *nibling2;
        if ((nibling = rb_child(sibling, dir ^ 1)) == NULL
                || rb_color(nibling) == RB_BLACK)
        {
            if ((nibling2 = rb_child(sibling, dir)) == NULL
                    || rb_color(nibling2) == RB_BLACK)
            {
                /* Case 2: Sibling is black and both its children are black.
                 */
                rb_set_color(sibling, RB_RED);
                node = parent;
                --rp->cur;
                continue;
            }

            /* Case 3: Sibling is black and its child in the same direction
             * is red and its child in the opposite direction is black.
             */
            rb_set_color(nibling2, RB_BLACK);
            rb_set_color(sibling, RB_RED);
            nibling = sibling;
            RB_NODE *t = rb_rotate(sibling, dir ^ 1);
            rb_set_child(parent, dir ^ 1, t);
            sibling = t;
        }

        /* Case 4: Sibling is black and its child in the opposite direction
         * is red.
         */
        rb_set_color(sibling, rb_color(parent));
        rb_set_color(parent, RB_BLACK);
        rb_set_color(nibling, RB_BLACK);
        RB_NODE *t = rb_rotate(parent, dir);
        RB_NODE *gparent = rp->cur[-1].parent;
        if (gparent != NULL)
        {
            rb_set_child(gparent, rp->cur[-1].dir, t);
        }
        else
        {
            root->rb_root = t;
        }
        node = root->rb_root;
        break;
    }
    if (node != NULL)
    {
        rb_set_color(node, RB_BLACK);
    }
}

void rb_remove(RB_ROOT *root, RB_NODE *node, RB_PATH *rp)
{
    RB_NODE *replacement;
    RB_NODE *transplanter;
    RB_NODE *node_parent = rp->cur->parent;
    int node_dir = rp->cur->dir;
    int deleted_color = rb_color(node);
    if (rb_child(node, RB_LEFT) == NULL) /* node has at most one right child */
    {
        transplanter = rb_child(node, RB_RIGHT);
        replacement = transplanter;
    }
    else if (rb_child(node, RB_RIGHT) == NULL) /* node has one left child */
    {
        transplanter = rb_child(node, RB_LEFT);
        replacement = transplanter;
    }
    else /* node has two children */
    {
        RB_PATH_ENTRY *node_rpe = ++rp->cur;
        rp->cur->parent = node;
        rp->cur->dir = RB_RIGHT;
        RB_NODE *successor = rb_child(node, RB_RIGHT);
        RB_NODE *left;
        while ((left = rb_child(successor, RB_LEFT)) != NULL)
        {
            ++rp->cur;
            rp->cur->parent = successor;
            rp->cur->dir = RB_LEFT;
            successor = left;
        }
        deleted_color = rb_color(successor);
        RB_NODE *child = rb_child(successor, RB_RIGHT);
        replacement = child;

        RB_NODE *parent = rp->cur->parent; /* successor's parent */
        if (parent != node)
        {
            rb_set_child(parent, rp->cur->dir, child);
            rb_copy_right_child(successor, node);
        }
        rb_copy_left_child_color(successor, node);
        node_rpe->parent = successor;
        transplanter = successor;
    }
    if (node_parent != NULL)
    {
        rb_set_child(node_parent, node_dir, transplanter);
    }
    else
    {
        root->rb_root = transplanter;
    }

    if (deleted_color == RB_BLACK)
    {
        rb_remove_color(root, replacement, rp);
    }
}

RB_NODE *rb_iter(RB_ROOT *root, int dir, RB_PATH *rp)
{
    RB_PATH_INIT(rp);
    RB_NODE *p = root->rb_root;
    if (p != NULL)
    {
        RB_NODE *q;
        while ((q = rb_child(p, dir)) != NULL)
        {
            ++rp->cur;
            rp->cur->parent = p;
            rp->cur->dir = dir;
            p = q;
        }
    }
    return p;
}

RB_NODE *rb_iter_next(RB_NODE *node, int dir,  RB_PATH *rp)
{
    RB_NODE *p = NULL;
    if (node != NULL)
    {
        p = rb_child(node, dir);
        if (p != NULL)
        {
            ++rp->cur;
            rp->cur->parent = node;
            rp->cur->dir = dir;

            RB_NODE *q;
            while ((q = rb_child(p, dir ^ 1)) != NULL)
            {
                ++rp->cur;
                rp->cur->parent = p;
                rp->cur->dir = (dir ^ 1);
                p = q;
            }
        }
        else
        {
            while (rp->cur->parent != NULL)
            {
                if (rp->cur->dir == (dir ^ 1))
                {
                    p = rp->cur->parent;
                    --rp->cur;
                    break;
                }
                --rp->cur;
            }
        }
    }
    return p;
}

