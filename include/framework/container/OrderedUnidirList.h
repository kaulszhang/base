// OrderedUnidirList.h

#ifndef _FRAMEWORK_CONTAINER_ORDERED_UNIDIR_LIST_H_
#define _FRAMEWORK_CONTAINER_ORDERED_UNIDIR_LIST_H_

#include "framework/container/detail/unidir_hook.h"
#include "framework/container/detail/ordered.h"
#include "framework/container/detail/identity.h"

namespace framework
{
    namespace container
    {

        template <
            typename _Ty, 
            typename _Pt = framework::generic::NativePointerTraits<_Ty>
        >
        struct OrderedUnidirListHook
        {
            typedef unidir_hook<_Ty, _Pt> type;
        };

        template <
            typename _Ty, 
            typename _Key = identity<_Ty>, 
            typename _Ls = std::less<BOOST_DEDUCED_TYPENAME _Key::result_type>
        >
        class OrderedUnidirList
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
            typedef unidir_iterator<node_type, pointer_traits> iterator;
            typedef unidir_const_iterator<node_type, pointer_traits> const_iterator;

        public:
            OrderedUnidirList()
            {
            }

            ~OrderedUnidirList(void)
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
                return make_iterator(pointer(0));
            }

            const_iterator end() const
            {
                return make_iterator(const_pointer(0));
            }

            const_iterator cbegin() const
            {
                return begin();
            }

            const_iterator cend() const
            {
                return end();
            }

            pointer first()
            {
                return header()->next();
            }

            const_pointer first() const
            {
                return header()->next();
            }

            pointer next(
                pointer p)
            {
                return p->next();
            }

            const_pointer next(
                const_pointer p) const
            {
                return p->next();
            }

            /* capacity */

            bool empty() const
            {
                return header()->next() == const_pointer(0);
            }

            size_t size() const
            {
                size_t n = 0;
                const_pointer p = header()->next();
                while (p != const_pointer(0)) {
                    n++;
                    p = p->next();
                }
                return n;
            }

            void clear()
            {
                while (header()->is_linked()) {
                    header()->unlink_next();
                }
            }

            void insert(
                pointer v)
            {
                push(v);
            }

            void insert(
                iterator w, 
                pointer v)
            {
                push(v);
            }

            pointer erase(
                pointer x)
            {
                pointer p = header();
                while (p->next() != x) {
                    p = p->next();
                }
                pointer x1 = next(x);
                p->unlink_next();
                return x1;
            }

            void push(
                pointer v)
            {
                assert(v->not_linked());
                pointer * p = &header()->next();
                key_type key = key_(*v);
                while ((*p) && comp_(key_(**p), key))
                    p = &(*p)->next();
                v->next() = *p;
                *p = v;
            }

            void pop()
            {
                assert(!empty());
                header()->unlink_next();
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
            _Key key_;
            _Ls comp_;
            hook_type header_;
        };

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_ORDERED_UNIDIR_LIST_H_
