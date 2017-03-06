// rb_tree_hook.h

#ifndef _FRAMEWORK_CONTAINER_DETAIL_RBTREE_HOOK_H_
#define _FRAMEWORK_CONTAINER_DETAIL_RBTREE_HOOK_H_

#include "framework/container/detail/b_tree_hook.h"
#include "framework/generic/NativePointer.h"

namespace framework
{
    namespace container
    {

        enum ordered_color
        {
            red = false, 
            black = true
        };

        enum ordered_side
        {
            to_left = false, 
            to_right = true
        };

        template <
            typename _Ty, 
            typename _Pt = framework::generic::NativePointerTraits<_Ty>
        >
        struct rb_tree_hook
            : b_tree_hook<_Ty, _Pt>
        {
        public:
            typedef _Ty value_type;
            typedef _Ty node_type;
            typedef b_tree_hook<_Ty, _Pt> super;
            typedef rb_tree_hook<_Ty, _Pt> hook_type;
            typedef typename _Pt::pointer pointer;
            typedef typename _Pt::const_pointer const_pointer;
            typedef pointer & parent_ref;

        public:
            ordered_color & color()
            {
                return color_;
            }

            ordered_color color() const
            {
                return color_;
            }

        public:
            /* algorithmic stuff */

            static void rotate_left(
                pointer x,
                parent_ref root)
            {
                pointer y = x->right();
                x->right() = y->left();
                if (y->left() != pointer(0))
                    y->left()->parent() = x;
                y->parent() = x->parent();

                if (x == root)
                    root = y;
                else if (x == x->parent()->left())
                    x->parent()->left() = y;
                else
                    x->parent()->right() = y;
                y->left() = x;
                x->parent() = y;
            }

            static void rotate_right(
                pointer x,
                parent_ref root)
            {
                pointer y = x->left();
                x->left() = y->right();
                if (y->right() != pointer(0))
                    y->right()->parent() = x;
                y->parent() = x->parent();

                if (x == root)
                    root = y;
                else if (x == x->parent()->right())
                    x->parent()->right() = y;
                else
                    x->parent()->left() = y;
                y->right() = x;
                x->parent() = y;
            }

            static void rebalance(
                pointer x,
                parent_ref root)
            {
                x->color() = red;
                while(x != root && x->parent()->color() == red) {
                    if (x->parent() == x->parent()->parent()->left()) {
                        pointer y = x->parent()->parent()->right();
                        if (y != pointer(0) && y->color() == red) {
                            x->parent()->color() = black;
                            y->color() = black;
                            x->parent()->parent()->color() = red;
                            x = x->parent()->parent();
                        } else {
                            if (x == x->parent()->right()) {
                                x = x->parent();
                                rotate_left(x, root);
                            }
                            x->parent()->color() = black;
                            x->parent()->parent()->color() = red;
                            rotate_right(x->parent()->parent(),root);
                        }
                    } else {
                        pointer y = x->parent()->parent()->left();
                        if (y != pointer(0) && y->color() == red) {
                            x->parent()->color() = black;
                            y->color() = black;
                            x->parent()->parent()->color() = red;
                            x = x->parent()->parent();
                        } else {
                            if (x == x->parent()->left()) {
                                x = x->parent();
                                rotate_right(x, root);
                            }
                            x->parent()->color() = black;
                            x->parent()->parent()->color() = red;
                            rotate_left(x->parent()->parent(),root);
                        }
                    }
                }
                root->color() = black;
            }

            static void link(
                pointer x, 
                ordered_side side, 
                pointer position, 
                pointer header)
            {
                if (side == to_left) {
                    position->left() = x;  /* also makes leftmost = x when parent == header */
                    if (position == header) {
                        header->parent() = x;
                        header->right() = x;
                    } else if (position == header->left()) {
                        header->left() = x;  /* maintain leftmost pointing to min node */
                    }
                } else {
                    position->right() = x;
                    if (position == header->right()) {
                        header->right() = x; /* maintain rightmost pointing to max node */
                    }
                }
                x->parent() = position;
                x->left() = pointer(0);
                x->right() = pointer(0);
                rb_tree_hook::rebalance(x, header->parent());
            }

            static pointer rebalance_for_erase(
                pointer z, 
                parent_ref root, 
                pointer & leftmost, 
                pointer & rightmost)
            {
                pointer y = z;
                pointer x = pointer(0);
                pointer x_parent = pointer(0);
                if (y->left() == pointer(0)) {    /* z has at most one non-null child. y == z. */
                    x = y->right();               /* x might be null */
                } else {
                    if (y->right() == pointer(0)) { /* z has exactly one non-null child. y == z. */
                        x = y->left();              /* x is not null */
                    } else {                       /* z has two non-null children.  Set y to */
                        y = y->right();             /* z's successor. x might be null.        */
                        while(y->left() != pointer(0))
                            y = y->left();
                        x = y->right();
                    }
                }
                if (y != z) {
                    z->left()->parent() = y;   /* relink y in place of z. y is z's successor */
                    y->left() = z->left();
                    if (y != z->right()) {
                        x_parent = y->parent();
                        if (x != pointer(0))
                            x->parent() = y->parent();
                        y->parent()->left() = x; /* y must be a child of left */
                        y->right() = z->right();
                        z->right()->parent() = y;
                    } else {
                        x_parent = y;
                    }

                    if (root == z)
                        root = y;
                    else if (z->parent()->left() == z)
                        z->parent()->left() = y;
                    else
                        z->parent()->right() = y;
                    y->parent() = z->parent();
                    ordered_color c = y->color();
                    y->color() = z->color();
                    z->color() = c;
                    y = z;                    /* y now points to node to be actually deleted */
                } else {                     /* y == z */
                    x_parent = y->parent();
                    if (x != pointer(0))
                        x->parent() = y->parent();   
                    if (root == z) {
                        root = x;
                    } else {
                        if (z->parent()->left() == z)
                            z->parent()->left() = x;
                        else
                            z->parent()->right() = x;
                    }
                    if (leftmost == z) {
                        if (z->right() == pointer(0)) { /* z->left() must be null also */
                            leftmost = z->parent();
                        } else {              
                            leftmost = b_tree_hook<_Ty, _Pt>::minimum(x);      /* makes leftmost == header if z == root */
                        }
                    }
                    if (rightmost == z) {
                        if (z->left() == pointer(0)) {  /* z->right() must be null also */
                            rightmost = z->parent();
                        } else {                   /* x == z->left() */
                            rightmost = b_tree_hook<_Ty, _Pt>::maximum(x); /* makes rightmost == header if z == root */
                        }
                    }
                }
                if (y->color() != red) {
                    while(x != root&&(x == pointer(0)|| x->color() == black)) {
                        if (x == x_parent->left()) {
                            pointer w = x_parent->right();
                            if (w->color() == red) {
                                w->color() = black;
                                x_parent->color() = red;
                                rotate_left(x_parent, root);
                                w = x_parent->right();
                            }
                            if ((w->left() == pointer(0) || w->left()->color() == black) &&
                                (w->right() == pointer(0) || w->right()->color() == black)) {
                                    w->color() = red;
                                    x = x_parent;
                                    x_parent = x_parent->parent();
                            } 
                            else {
                                if (w->right() == pointer(0) || w->right()->color() == black) {
									if (w->left() != pointer(0))
                                        w->left()->color() = black;
                                    w->color() = red;
                                    rotate_right(w, root);
                                    w = x_parent->right();
                                }
                                w->color() = x_parent->color();
                                x_parent->color() = black;
                                if (w->right() != pointer(0))
                                    w->right()->color() = black;
                                rotate_left(x_parent, root);
                                break;
                            }
                        } 
                        else {                   /* same as above, with right <-> left */
                            pointer w = x_parent->left();
                            if (w->color() == red) {
                                w->color() = black;
                                x_parent->color() = red;
                                rotate_right(x_parent, root);
                                w = x_parent->left();
                            }
                            if ((w->right() == pointer(0) || w->right()->color() == black) &&
                                (w->left() == pointer(0) || w->left()->color() == black)) {
                                    w->color() = red;
                                    x = x_parent;
                                    x_parent = x_parent->parent();
                            } else {
                                if (w->left() == pointer(0) || w->left()->color() == black) {
                                    if (w->right() != pointer(0))
                                        w->right()->color() = black;
                                    w->color() = red;
                                    rotate_left(w, root);
                                    w = x_parent->left();
                                }
                                w->color() = x_parent->color();
                                x_parent->color() = black;
                                if (w->left() != pointer(0))
                                    w->left()->color() = black;
                                rotate_right(x_parent, root);
                                break;
                            }
                        }
                    }
                    if (x != pointer(0))
                        x->color() = black;
                }
                return y;
            }

            static void restore(
                pointer x, 
                pointer position, 
                pointer header)
            {
                if (position->left() == pointer(0) || position->left() == header) {
                    link(x, to_left, position, header);
                } else {
                    decrement(position);
                    link(x, to_right, position, header);
                }
            }

            static void decrement(
                pointer & x)
            {
                if (x->color() == red && x->parent()->parent() == x) {
                    x = x->right();
                } else {
                    super::decrement(x);
                }
            }

        private:
            ordered_color color_; 
        };

    } // namespace container
} // namespace framework

#endif // #ifndef _FRAMEWORK_CONTAINER_DETAIL_RBTREE_HOOK_H_
