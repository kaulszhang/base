// List.h

#ifndef _FRAMEWORK_CONTAINER_LIST_H_
#define _FRAMEWORK_CONTAINER_LIST_H_

#include "framework/container/detail/bidir_hook.h"

#include <boost/iterator/reverse_iterator.hpp>

namespace framework
{
    namespace container
    {

        template <
            typename _Ty, 
            typename _Pt = framework::generic::NativePointerTraits<_Ty>
        >
        struct ListHook
        {
            typedef bidir_hook<_Ty, _Pt> type;
        };

        template <
            typename _Ty>
        class List
        {
        public:
            typedef _Ty node_type;
            typedef typename node_type::pointer_traits pointer_traits;
            typedef typename node_type::hook_type hook_type;
            typedef typename pointer_traits::pointer pointer;
            typedef typename pointer_traits::const_pointer const_pointer;
            typedef node_type value_type;

        public:
            typedef bidir_iterator<node_type, pointer_traits> iterator;
            typedef bidir_const_iterator<node_type, pointer_traits> const_iterator;
            typedef typename boost::reverse_iterator<iterator> reverse_iterator;
            typedef typename boost::reverse_iterator<const_iterator> const_reverse_iterator;

        public:
            List()
            {
            }

            ~List(void)
            {
                clear();
            }

            iterator begin()
            {
                return make_iterator(header()->next());
            }

            const_iterator begin() const
            {
                return make_iterator(header()->next());
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

            pointer first()
            {
                pointer p = header()->next();
                return p == header() ? pointer(0) : p;
            }

            const_pointer first() const
            {
                const_pointer p = header()->next();
                return p == header() ? const_pointer(0) : p;
            }

            pointer next(
                pointer p)
            {
                p = p->next();
                return p == header() ? pointer(0) : p;
            }

            const_pointer next(
                const_pointer p) const
            {
                p = p->next();
                return p == header() ? const_pointer(0) : p;
            }

            pointer last()
            {
                pointer p = header()->prev();
                return p == header() ? pointer(0) : p;
            }

            const_pointer last() const
            {
                const_pointer p = header()->prev();
                return p == header() ? const_pointer(0) : p;
            }

            pointer prev(
                pointer p)
            {
                p = p->prev();
                return p == header() ? pointer(0) : p;
            }

            const_pointer prev(
                const_pointer p) const
            {
                p = p->prev();
                return p == header() ? const_pointer(0) : p;
            }

            /* capacity */

            bool empty() const
            {
                return header()->next() == header();
            }

            size_t size() const
            {
                size_t n = 0;
                const_pointer p = header()->next();
                while (p != header()) {
                    n++;
                    p = p->next();
                }
                return n;
            }

            iterator erase(
                iterator position)
            {
                erase(position++.get_node());
                return position;
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

            void rotate(
                iterator mid)
            {
                rotate(begin(), mid, end());
            }

            void rotate(
                iterator first, 
                iterator mid, 
                iterator last)
            {
                node_type::rotate(first.get_node(), mid.get_node(), last.get_node());
            }

            void clear()
            {
                while (header()->is_linked()) {
                    node_type::erase(header()->next());
                }
            }

            void insert(
                pointer v)
            {
                insert(header(), v);
            }

            void insert(
                iterator w, 
                pointer v)
            {
                node_type::insert(w.get_node(), v);
            }

            void insert(
                reverse_iterator w, 
                pointer v)
            {
                insert(w.base(), v);
            }

            void insert(
                pointer w, 
                pointer v)
            {
                if (w == pointer(0))
                    w = header();
                node_type::insert(w, v);
            }

            void push_front(
                pointer v)
            {
                insert(header()->next(), v);
            }

            void push_back(
                pointer v)
            {
                insert(header(), v);
            }

            pointer erase(
                pointer x)
            {
                pointer x1 = next(x);
                node_type::erase(x);
                return x1 == header() ? pointer(0) : x1;
            }

            void pop_front()
            {
                assert(!empty());
                erase(first());
            }

            void pop_back()
            {
                assert(!empty());
                erase(last());
            }

        private:
            pointer header()
            {
                return header_.get();
            }

            const_pointer header() const
            {
                return header_.get();
            }

        private:
            iterator make_iterator(
                pointer node)
            {
                return iterator(node);
            }

            const_iterator make_iterator(
                const_pointer node) const
            {
                return const_iterator(node);
            }

        private:
            hook_type header_;
        };

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_LIST_H_
