// unidir_hook.h

#ifndef _FRAMEWORK_CONTAINER_DETAIL_UNIDIR_HOOK_H_
#define _FRAMEWORK_CONTAINER_DETAIL_UNIDIR_HOOK_H_

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
        class unidir_hook
            : public Hook<_Ty, _Pt>
        {
        public:
            typedef unidir_hook<_Ty, _Pt> hook_type;
            typedef typename _Pt::pointer pointer;
            typedef typename _Pt::const_pointer const_pointer;

        public:
            unidir_hook()
            {
                next_ = pointer(0);
            }

            ~unidir_hook(void)
            {
                next_ = pointer(0);
            }

            void unlink()
            {
                next_ = pointer(0);
            }

            void unlink_next()
            {
                pointer & next_next = static_cast<unidir_hook &>(*next_).next_;
                next_ = next_next;
                next_next = pointer(0);
            }

            void link_next(
                pointer p)
            {
                static_cast<unidir_hook &>(*p).next_ = next_;
                next_ = p;
            }

            bool not_linked() const
            {
                return (const_pointer)next_ == const_pointer(0);
            };

            bool is_linked() const
            {
                return (const_pointer)next_ != const_pointer(0);
            };

            pointer & next()
            {
                return next_;
            }

            const_pointer next() const
            {
                return next_;
            }

        private:
            pointer next_;
        };

        template<typename _Ty, typename _Pt>
        class unidir_iterator
            : public boost::forward_iterator_helper<
                unidir_iterator<_Ty, _Pt>, 
                _Ty, 
                typename _Pt::difference_type, 
                typename _Pt::pointer, 
                typename _Pt::reference
            >
        {
        public:
            unidir_iterator()
            {
            }

            explicit unidir_iterator(
                typename _Pt::pointer node)
                : node_(node)
            {
            }

            typename _Pt::reference operator * () const
            {
                return *node_;
            }

            unidir_iterator & operator++()
            {
                node_ = node_->next();
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
            unidir_iterator<_Ty, _Pt> const & x,
            unidir_iterator<_Ty, _Pt> const & y)
        {
            return x.get_node() == y.get_node();
        }

        template<typename _Ty, typename _Pt>
        class unidir_const_iterator
            : public boost::forward_iterator_helper<
                unidir_const_iterator<_Ty, _Pt>, 
                _Ty, 
                typename _Pt::difference_type, 
                typename _Pt::const_pointer, 
                typename _Pt::const_reference
            >
        {
        public:
            unidir_const_iterator()
            {
            }

            explicit unidir_const_iterator(
                typename _Pt::const_pointer node)
                : node_(node)
            {
            }

            typename _Pt::const_reference operator * () const
            {
                return *node_;
            }

            unidir_const_iterator & operator++()
            {
                node_ = node_->next();
                return *this;
            }

            typename _Pt::const_pointer get_node() const
            {
                return node_;
            }

        private:
            typename _Pt::const_pointer node_;
        };

        template<typename _Ty, typename _Pt>
        bool operator == (
            unidir_const_iterator<_Ty, _Pt> const & x,
            unidir_const_iterator<_Ty, _Pt> const & y)
        {
            return x.get_node() == y.get_node();
        }

    }
}

#endif // _FRAMEWORK_CONTAINER_DETAIL_UNIDIR_HOOK_H_
