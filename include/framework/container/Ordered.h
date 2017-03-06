// ordered.h

#ifndef _FRAMEWORK_CONTAINER_ORDERED_H_
#define _FRAMEWORK_CONTAINER_ORDERED_H_

#include "framework/container/detail/rb_tree_hook.h"
#include "framework/container/detail/ordered.h"
#include "framework/container/detail/identity.h"

#include <boost/iterator/reverse_iterator.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/if.hpp>

namespace framework
{
    namespace container
    {

        template <
            typename _Ty, 
            typename _Pt = framework::generic::NativePointerTraits<_Ty>
        >
        struct OrderedHook
        {
            typedef rb_tree_hook<_Ty, _Pt> type;
        };

        template <
            typename _Ty, 
            typename _Key = identity<_Ty>, 
            typename _Ls = std::less<BOOST_DEDUCED_TYPENAME _Key::result_type>, 
            typename _Cat = ordered_unique_tag>
        class Ordered
        {
        public:
            typedef _Ty node_type;
            typedef typename node_type::pointer_traits pointer_traits;
            typedef typename node_type::hook_type hook_type;
            typedef typename pointer_traits::pointer pointer;
            typedef typename pointer_traits::const_pointer const_pointer;
            typedef typename _Key::result_type key_type;
            typedef node_type value_type;

        public:
            typedef btree_iterator<node_type, pointer_traits> iterator;
            typedef iterator const_iterator;
            typedef typename boost::reverse_iterator<iterator> reverse_iterator;
            typedef typename boost::reverse_iterator<const_iterator> const_reverse_iterator;

        public:
            Ordered()
            {
                empty_initialize();
            }

            ~Ordered(void)
            {
                clear();
            }

            iterator begin()
            {
                return make_iterator(leftmost());
            }

            const_iterator begin() const
            {
                return make_iterator(leftmost());
            }

            iterator end()
            {
                return make_iterator(header());
            }

            const_iterator end() const
            {
                return make_iterator(header());
            }

            reverse_iterator rbegin()
            {
                return make_reverse_iterator(end());
            }

            const_reverse_iterator rbegin() const
            {
                return make_reverse_iterator(end());
            }

            reverse_iterator rend()
            {
                return make_reverse_iterator(begin());
            }

            const_reverse_iterator rend() const
            {
                return make_reverse_iterator(begin());
            }

            const_iterator cbegin() const
            {
                return begin();
            }

            const_iterator cend() const
            {
                return end();
            }

            const_reverse_iterator crbegin() const
            {
                return rbegin();
            }

            const_reverse_iterator crend() const
            {
                return rend();
            }

            pointer deref(
                iterator i)
            {
                return i.get_node() == header() ? pointer(0) : i.get_node();
            }

            pointer first()
            {
                return leftmost() == header() ? pointer(0) : leftmost();
            }

            const_pointer first() const
            {
                return leftmost() == header() ? const_pointer(0) : leftmost();
            }

            pointer next(
                pointer p) const
            {
                node_type::increment(p);
                return p == header() ? pointer(0) : p;
            }

            const_pointer next(
                const_pointer p) const
            {
                pointer p1 = pointer(p);
                node_type::increment(p1);
                return p1 == header() ? const_pointer(0) : p1;
            }

            pointer last()
            {
                return rightmost() == header() ? pointer(0) : rightmost();
            }

            const_pointer last() const
            {
                return rightmost() == header() ? const_pointer(0) : rightmost();
            }

            pointer prev(
                pointer p) const
            {
                if (p == leftmost())
                    return pointer(0);
                node_type::decrement(p);
                return p == header() ? pointer(0) : p;
            }

            const_pointer prev(
                const_pointer p) const
            {
                if (p == leftmost())
                    return const_pointer(0);
                pointer p1 = pointer(p);
                node_type::decrement(p1);
                return p1 == header() ? const_pointer(0) : p1;
            }

            /* capacity */

            bool empty() const
            {
                return header()->parent() == pointer(0);
            }

            size_t size() const
            {
                return count_all_nodes(root());
            }

            std::pair<iterator, bool> insert(
                pointer v)
            {
                pointer v1 = insert_(v);
                if (v1 == v) {
                    return std::pair<iterator, bool> (make_iterator(v), true);
                } else {
                    return std::pair<iterator, bool> (make_iterator(v1), false);
                }
            }

            std::pair<iterator, bool> insert(
                iterator w, 
                pointer v)
            {
                pointer v1 = insert_(w.get_node(), v);
                if (v1 == v) {
                    return std::pair<iterator, bool> (make_iterator(v), true);
                } else {
                    return std::pair<iterator, bool> (make_iterator(v1), false);
                }
            }

            size_t erase(
                key_type const & x)
            {
                std::pair<iterator, iterator> p = equal_range(x);
                size_t s = 0;
                while (p.first != p.second) {
                    p.first = erase(p.first);
                    ++s;
                }
                return s;
            }

            pointer erase(
                pointer p)
            {
                pointer p1 = p;
                node_type::increment(p1);
                erase_(p);
                return p1 == header() ? pointer(0) : p1;
            }

            iterator erase(
                iterator p)
            {
                erase_(p++.get_node());
                return p;
            }

            iterator erase(
                iterator first,
                iterator last)
            {
                while (first != last) {
                    first = erase(first);
                }
                return first;
            }

            void clear()
            {
                delete_all_nodes(root());
                empty_initialize();
            }

            template<typename CompatibleKey>
            iterator find(
                const CompatibleKey & x) const
            {
                return make_iterator(node_type::find(root(), header(), key_, x, comp_));
            }

            template<typename CompatibleKey, typename CompatibleCompare>
            iterator find(
                const CompatibleKey & x,
                const CompatibleCompare & comp) const
            {
                return make_iterator(node_type::find(root(), header(), key_, x, comp));
            }

            template<typename CompatibleKey>
            size_t count(CompatibleKey const & x) const
            {
                return count(x, comp_);
            }

            template<typename CompatibleKey, typename CompatibleCompare>
            size_t count(CompatibleKey const & x, CompatibleCompare const & comp) const
            {
                std::pair<iterator, iterator> p = node_type::equal_range(x, comp);
                size_t n = std::distance(p.first, p.second);
                return n;
            }

            template<typename CompatibleKey>
            iterator lower_bound(
                CompatibleKey const & x) const
            {
                return make_iterator(
                    node_type::lower_bound(root(), header(), key_, x, comp_));
            }

            template<typename CompatibleKey, typename CompatibleCompare>
            iterator lower_bound(
                CompatibleKey const & x,
                CompatibleCompare const & comp) const
            {
                return make_iterator(
                    node_type::lower_bound(root(), header(), key_, x, comp));
            }

            template<typename CompatibleKey>
            iterator upper_bound(
                CompatibleKey const & x) const
            {
                return make_iterator(
                    node_type::upper_bound(root(), header(), key_, x, comp_));
            }

            template<typename CompatibleKey, typename CompatibleCompare>
            iterator upper_bound(
                CompatibleKey const & x,
                CompatibleCompare const & comp) const
            {
                return make_iterator(
                    node_type::upper_bound(root(), header(), key_, x, comp));
            }

            template<typename CompatibleKey>
            std::pair<iterator, iterator> equal_range(
                CompatibleKey const & x) const
            {
                std::pair<pointer, pointer> p =
                    node_type::equal_range(root(), header(), key_, x, comp_);
                return std::pair<iterator, iterator>(
                    make_iterator(p.first), make_iterator(p.second));
            }

            template<typename CompatibleKey, typename CompatibleCompare>
            std::pair<iterator, iterator> equal_range(
                CompatibleKey const & x,
                CompatibleCompare const & comp) const
            {
                std::pair<pointer, pointer> p =
                    node_type::equal_range(root(), header(), key_, x, comp);
                return std::pair<iterator, iterator>(
                    make_iterator(p.first), make_iterator(p.second));
            }

            /* range */

            template<typename LowerBounder, typename UpperBounder>
            std::pair<iterator, iterator> range(
                LowerBounder lower,
                UpperBounder upper) const
            {
                using namespace boost;
                typedef typename mpl::if_<
                    is_same<LowerBounder, unbounded_type>,
                    typename mpl::if_<
                    is_same<UpperBounder, unbounded_type>,
                    both_unbounded_tag,
                    lower_unbounded_tag
                    >::type,
                    typename mpl::if_<
                    is_same<UpperBounder, unbounded_type>,
                    upper_unbounded_tag,
                    none_unbounded_tag
                    >::type
                >::type dispatch;

                return range(lower, upper, dispatch());
            }

            bool exist(
                const_pointer x) const
            {
                std::pair<iterator, iterator> pair = equal_range(key_(*x));
                while (pair.first != pair.second) {
                    if ((const_pointer)pair.first.get_node() == x)
                        return true;
                    ++pair.first;
                }
                return false;
            }

        private:
            pointer insert_(
                pointer v)
            {
                assert(v->not_linked());
                //if (v->is_linked()) {
                //    Util::logger.Log(Util::Logger::kLevelError, 0, "[ordered::insert_] failed 1");
                //    __asm int 3;
                //    return pointer(0);
                //}
                link_info inf;
                if (!link_point(key_(*v), inf, _Cat())) {
                    //Util::logger.Log(Util::Logger::kLevelError, 0, "[ordered::insert_] failed 2");
                    //__asm int 3;
                    return inf.pos;
                }
                node_type::link(v, inf.side, inf.pos, header());
                return v;
            }

            pointer insert_(
                pointer p, 
                pointer v)
            {
                assert(v->not_linked());
                //if (v->is_linked()) {
                //    Util::logger.Log(Util::Logger::kLevelError, 0, "[ordered::insert_] failed 1");
                //    __asm int 3;
                //    return pointer(0);
                //}
                link_info inf;
                if (!hinted_link_point(key_(*v), p, inf, _Cat())) {
                    //Util::logger.Log(Util::Logger::kLevelError, 0, "[ordered::insert_] failed 2");
                    //__asm int 3;
                    return inf.pos;
                }
                node_type::link(v, inf.side, inf.pos, header());
                return v;
            }

            void erase_(
                pointer x)
            {
                assert(exist(x));
                //if (!exist(x)) {
                //    __asm int 3;
                //    Util::logger.Log(Util::Logger::kLevelError, 0, "[ordered::erase_] failed");
                //    return;
                //}
                node_type::rebalance_for_erase(
                    x, header()->parent(), header()->left(), header()->right());
                x->unlink();
            }

        private:
            pointer header() const
            {
                return const_cast<_Ty *>(header_.get());
            }

            pointer root() const
            {
                return header()->parent();
            }

            pointer leftmost() const
            {
                return header()->left();
            }

            pointer rightmost() const
            {
                return header()->right();
            }

            void empty_initialize()
            {
                header()->color() = red;
                /* used to distinguish header() from root, in iterator.operator++ */

                header()->parent() = pointer(0);
                header()->left() = header();
                header()->right() = header();
            }

            struct link_info
            {
                link_info():side(to_left) {}

                ordered_side side;
                pointer  pos;
            };

            bool link_point(
                key_type const & k, 
                link_info & inf, 
                ordered_unique_tag)
            {
                pointer y = header();
                pointer x = root();
                bool c = true;
                while (x) {
                    y = x;
                    c = comp_(k, key_(*x));
                    x = c ? x->left() : x->right();
                }
                pointer yy = y;
                if (c) {
                    if (yy == leftmost()) {
                        inf.side = to_left;
                        inf.pos = y;
                        return true;
                    } else {
                        node_type::decrement(yy);
                    }
                }
                if (comp_(key_(*yy), k)) {
                    inf.side = c ? to_left : to_right;
                    inf.pos = y;
                    return true;
                } else {
                    inf.pos = yy;
                    return false;
                }
            }

            bool link_point(
                key_type const & k, 
                link_info & inf, 
                ordered_non_unique_tag)
            {
                pointer y = header();
                pointer x = root();
                bool c = true;
                while (x) {
                    y = x;
                    c = comp_(k, key_(*x));
                    x = c ? x->left() : x->right();
                }
                inf.side = c ? to_left : to_right;
                inf.pos = y;
                return true;
            }

            bool lower_link_point(
                key_type k,
                link_info & inf,
                ordered_non_unique_tag)
            {
                pointer y = header();
                pointer x = root();
                bool c = false;
                while (x) {
                    y = x;
                    c = comp_(key_(*x), k);
                    x = c ? x->right() : x->left();
                }
                inf.side = c ? to_right : to_left;
                inf.pos = y;
                return true;
            }

            bool hinted_link_point(
                key_type const & k,
                pointer position,
                link_info & inf,
                ordered_unique_tag)
            {
                if (position == header()->left()) { 
                    if (!empty() && comp_(k, key_(*position))) {
                        inf.side = to_left;
                        inf.pos = position;
                        return true;
                    } else {
                        return link_point(k, inf, ordered_unique_tag());
                    }
                } else if (position == header()) { 
                    if (comp_(key_(*rightmost()), k)) {
                        inf.side = to_right;
                        inf.pos = rightmost();
                        return true;
                    } else {
                        return link_point(k, inf, ordered_unique_tag());
                    }
                } else {
                    pointer before = position;
                    node_type::decrement(before);
                    if (comp_(key_(*before), k) && comp_(k, key_(*position))) {
                        if (before->right() == pointer(0)) {
                            inf.side = to_right;
                            inf.pos = before;
                            return true;
                        } else {
                            inf.side = to_left;
                            inf.pos = position;
                            return true;
                        }
                    } else {
                        return link_point(k, inf, ordered_unique_tag());
                    }
                }
            }

            bool hinted_link_point(
                key_type const & k,
                pointer position,
                link_info& inf,
                ordered_non_unique_tag)
            {
                if (position == header()->left()) { 
                    if (!empty() && !comp_(key_(*position), k)) {
                        inf.side = to_left;
                        inf.pos = position;
                        return true;
                    } else {
                        return lower_link_point(k, inf, ordered_non_unique_tag());
                    }
                } else if (position == header()) {
                    if (!comp_(k, key_(*rightmost()))) {
                        inf.side = to_right;
                        inf.pos = rightmost();
                        return true;
                    } else return link_point(k, inf, ordered_non_unique_tag());
                } else{
                    pointer before = position;
                    node_type::decrement(before);
                    if (!comp_(k, key_(*before))) {
                        if (!comp_(key_(*position), k)) {
                            if (before->right() == pointer(0)) {
                                inf.side = to_right;
                                inf.pos = before;
                                return true;
                            } else {
                                inf.side = to_left;
                                inf.pos = position;
                                return true;
                            }
                        } else {
                            return lower_link_point(k, inf, ordered_non_unique_tag());
                        }
                    } else {
                        return link_point(k, inf, ordered_non_unique_tag());
                    }
                }
            }

            void delete_all_nodes(
                pointer x)
            {
                if (!x)
                    return;
                delete_all_nodes(x->left());
                delete_all_nodes(x->right());
                x->unlink();
            }

            size_t count_all_nodes(
                const_pointer x) const
            {
                if (!x)
                    return 0;
                return 1 +
                    count_all_nodes(x->left()) + 
                    count_all_nodes(x->right());
            }

            template<typename LowerBounder, typename UpperBounder>
            std::pair<iterator, iterator> range(
                LowerBounder lower, 
                UpperBounder upper, 
                none_unbounded_tag) const
            {
                pointer y = header();
                pointer z = root();

                while (z) {
                    if (!lower(key_(*z))) {
                        z = z->right();
                    } else if (!upper(key_(*z))) {
                        y = z;
                        z = z->left();
                    } else {
                        return std::pair<iterator, iterator>(
                            make_iterator(
                            lower_range(z->left(), z, lower)),
                            make_iterator(
                            upper_range(z->right(), y, upper)));
                    }
                }

                return std::pair<iterator, iterator>(make_iterator(y), make_iterator(y));
            }

            template<typename LowerBounder, typename UpperBounder>
            std::pair<iterator, iterator> range(
                LowerBounder, 
                UpperBounder upper, 
                lower_unbounded_tag) const
            {
                return std::pair<iterator, iterator>(
                    begin(),
                    make_iterator(upper_range(root(), header(), upper)));
            }

            template<typename LowerBounder, typename UpperBounder>
            std::pair<iterator, iterator> range(
                LowerBounder lower, 
                UpperBounder, 
                upper_unbounded_tag) const
            {
                return std::pair<iterator, iterator>(
                    make_iterator(lower_range(root(), header(), lower)),
                    end());
            }

            template<typename LowerBounder, typename UpperBounder>
            std::pair<iterator, iterator>
                range(LowerBounder, UpperBounder, both_unbounded_tag) const
            {
                return std::pair<iterator, iterator>(begin(), end());
            }

            template<typename LowerBounder>
            pointer lower_range(
                pointer top, 
                pointer y, 
                LowerBounder lower) const
            {
                while (top) {
                    if (lower(key_(*top))) {
                        y = top;
                        top = top->left();
                    } else {
                        top = top->right();
                    }
                }

                return y;
            }

            template<typename UpperBounder>
            pointer upper_range(
                pointer top, 
                pointer y, 
                UpperBounder upper) const
            {
                while (top) {
                    if (!upper(key_(*top))) {
                        y = top;
                        top = top->left();
                    } else top = top->right();
                }

                return y;
            }

        private:
            iterator make_iterator(
                pointer node)
            {
                return iterator(node);
            }

            const_iterator make_iterator(
                pointer node) const
            {
                return const_iterator(node);
            }

            _Key key_;
            _Ls comp_;
            hook_type header_;
        };

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_ORDERED_H_
