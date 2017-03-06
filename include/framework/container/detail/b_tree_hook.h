// b_tree_hook.h

#ifndef _FRAMEWORK_CONTAINER_DETAIL_B_TREE_HOOK_H_
#define _FRAMEWORK_CONTAINER_DETAIL_B_TREE_HOOK_H_

#include "framework/container/detail/hook.h"
#include "framework/generic/NativePointer.h"

#include <boost/operators.hpp>

namespace framework
{
    namespace container
    {

        template <
            typename _Ty, 
            typename _Pt = framework::generic::NativePointerTraits<_Ty>
        >
        struct b_tree_hook
            : Hook<_Ty, _Pt>
        {
        public:
            typedef _Ty value_type;
            typedef _Ty node_type;
            typedef b_tree_hook<_Ty, _Pt> hook_type;
            typedef typename _Pt::pointer pointer;
            typedef pointer & parent_ref;

        public:
            b_tree_hook()
                : parent_(0)
                , left_(0)
                , right_(0)
            {
            }

            ~b_tree_hook()
            {
                assert(not_linked());
            }

            pointer & parent()
            {
                return parent_;
            }

            pointer parent() const
            {
                return parent_;
            }

            pointer & left()
            {
                return left_;
            }

            pointer left() const
            {
                return left_;
            }

            pointer & right()
            {
                return right_;
            }

            pointer right() const
            {
                return right_;
            }

            bool not_linked() const
            {
                return parent_ == pointer(0);
            };

            bool is_linked() const
            {
                return parent_ != pointer(0);
            };

            void unlink()
            {
                parent_ = pointer(0);
                left_ = pointer(0);
                right_ = pointer(0);
            };

        public:
            /* interoperability with bidir_node_iterator */

            static void increment(
                pointer & x)
            {
                if (x->right() != pointer(0)) {
                    x = x->right();
                    while (x->left() != pointer(0))
                        x = x->left();
                } else {
                    pointer y = x->parent();
                    while (x == y->right()) {
                        x = y;
                        y = y->parent();
                    }
                    if (x->right() != y)
                        x = y;
                }
            }

            static void decrement(
                pointer & x)
            {
                // x不能为头，也不能为最左边节点
                if (x->left() != pointer(0)) {
                    pointer y = x->left();
                    while (y->right() != pointer(0))
                        y = y->right();
                    x = y;
                } else {
                    pointer y = x->parent();
                    while (x == y->left()) {
                        x = y;
                        y = y->parent();
                    }
                    x = y;
                }
            }

            static pointer minimum(
                pointer x)
            {
                while (x->left() != pointer(0))x = x->left();
                return x;
            }

            static pointer maximum(
                pointer x)
            {
                while (x->right() != pointer(0))
                    x = x->right();
                return x;
            }

            template<
                typename KeyFromValue,
                typename CompatibleKey,
                typename CompatibleCompare
            >
            static pointer find(
			pointer top,
            pointer y,
            KeyFromValue const & key,
            CompatibleKey const & x,
            CompatibleCompare const & comp)
            {
                pointer y0 = y;

                while (top) {
                    if (!comp(key(*top), x)) {
                        y = top;
                        top = top->left();
                    } else {
                        top = top->right();
                    }
                }

                return (y == y0 || comp(x, key(*y))) ? y0 : y;
            }

            template<
                typename KeyFromValue,
                typename CompatibleKey,
                typename CompatibleCompare
            >
            static pointer lower_bound(
            pointer top,
            pointer y,
            KeyFromValue const & key,
            CompatibleKey const & x,
            CompatibleCompare const & comp)
            {
                while (top) {
                    if (!comp(key(*top),x)) {
                        y = top;
                        top = top->left();
                    }
                    else top = top->right();
                }

                return y;
            }

            template<
                typename KeyFromValue,
                typename CompatibleKey,
                typename CompatibleCompare
            >
            static pointer upper_bound(
            pointer top,
            pointer y,
            KeyFromValue const & key,
            CompatibleKey const & x,
            CompatibleCompare const & comp)
            {
                while (top) {
                    if (comp(x, key(*top))) {
                        y = top;
                        top = top->left();
                    }
                    else top = top->right();
                }

                return y;
            }

            template<
                typename KeyFromValue,
                typename CompatibleKey,
                typename CompatibleCompare
            >
            static std::pair<pointer, pointer> equal_range(
            pointer top,
            pointer y,
            KeyFromValue const & key,
            CompatibleKey const & x,
            CompatibleCompare const & comp)
            {
                while (top) {
                    if (comp(key(*top), x)) {
                        top = top->right();
                    }
                    else if (comp(x, key(*top))) {
                        y = top;
                        top = top->left();
                    } else {
                        return std::pair<pointer, pointer>(
                            lower_bound(top->left(), top, key, x, comp),
                            upper_bound(top->right(), y, key, x, comp));
                    }
                }

                return std::pair<pointer, pointer>(y, y);
            }

        private:
            pointer parent_;
            pointer left_;
            pointer right_;
        };

        template<typename _Ty, typename _Pt>
        class btree_iterator
            : public boost::bidirectional_iterator_helper<
                btree_iterator<_Ty, _Pt>, 
                _Ty, 
                typename _Pt::difference_type, 
                typename _Pt::const_pointer, 
                typename _Pt::const_reference
            >
        {
        public:
            btree_iterator()
            {
            }

            explicit btree_iterator(
                typename _Pt::pointer node)
                : node_(node)
            {
            }

            typename _Pt::const_reference operator * () const
            {
                return *node_;
            }

            btree_iterator & operator++()
            {
                _Ty::increment(node_);
                return *this;
            }

            btree_iterator & operator--()
            {
                _Ty::decrement(node_);
                return *this;
            }

            typename _Pt::pointer get_node() const
            {
                return node_;
            }

        private:
            typename _Pt::pointer node_;
        };

        template<typename _Ty, typename _Pt>
        bool operator == (
            btree_iterator<_Ty, _Pt> const & x,
            btree_iterator<_Ty, _Pt> const & y)
        {
            return x.get_node() == y.get_node();
        }

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_DETAIL_B_TREE_HOOK_H_
