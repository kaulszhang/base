// bidir_hook.h

#ifndef _FRAMEWORK_CONTAINER_DETAIL_BIDIR_HOOK_H_
#define _FRAMEWORK_CONTAINER_DETAIL_BIDIR_HOOK_H_

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
        class bidir_hook
            : public Hook<_Ty, _Pt>
        {
        public:
            typedef bidir_hook<_Ty, _Pt> hook_type;
            typedef typename _Pt::pointer pointer;
            typedef typename _Pt::const_pointer const_pointer;

        public:
            bidir_hook()
            {
                prev_ = next_ = this->get();
            }

            ~bidir_hook(void)
            {
                static_cast<bidir_hook &>(*next_).prev_ = prev_;
                static_cast<bidir_hook &>(*prev_).next_ = next_;
                next_ = prev_ = pointer(0);
            }

            // 将p插入到w前面
            static void insert(
                pointer w, 
                pointer p)
            {
                static_cast<bidir_hook &>(*p).next_ = w;
                static_cast<bidir_hook &>(*p).prev_ = static_cast<bidir_hook &>(*w).prev_;
                static_cast<bidir_hook &>(*static_cast<bidir_hook &>(*w).prev_).next_ = p;
                static_cast<bidir_hook &>(*w).prev_ = p;
            }

            static void rotate(
                pointer f, 
                pointer m, 
                pointer l)
            {
                // from: (pf)f-----(pm)m------(pl)l
                // to:   (pf)m-----(pl)f-----(pm)l
                if (f == m || m == l)
                    return;
                pointer pf = f->bidir_hook_prev();
                pointer pm = m->bidir_hook_prev();
                pointer pl = l->bidir_hook_prev();
                l->bidir_hook_next()->bidir_hook_prev() = pm; 
                pf->bidir_hook_next() = m;
                pm->bidir_hook_next() = l;
                m->bidir_hook_prev() = pf;
                f->bidir_hook_prev() = pl;
                pl->bidir_hook_next() = f;
            }

            static void erase(
                pointer p)
            {
                p->unlink();
            }

            void unlink()
            {
                static_cast<bidir_hook &>(*next_).prev_ = prev_;
                static_cast<bidir_hook &>(*prev_).next_ = next_;
                next_ = prev_ = this->get();
            }

            bool not_linked() const
            {
                return (const_pointer)next_ == (const_pointer)this->get();
            };

            bool is_linked() const
            {
                return (const_pointer)next_ != (const_pointer)this->get();
            };

            pointer & prev()
            {
                return prev_;
            }

            const_pointer prev() const
            {
                return prev_;
            }

            pointer & next()
            {
                return next_;
            }

            const_pointer next() const
            {
                return next_;
            }

        private:
            pointer & bidir_hook_prev()
            {
                return prev_;
            }

            const_pointer bidir_hook_prev() const
            {
                return prev_;
            }

            pointer & bidir_hook_next()
            {
                return next_;
            }

            const_pointer bidir_hook_next() const
            {
                return next_;
            }

        private:
            pointer prev_;
            pointer next_;
        };

        template<typename _Ty, typename _Pt>
        class bidir_iterator
            : public boost::bidirectional_iterator_helper<
                bidir_iterator<_Ty, _Pt>, 
                _Ty, 
                typename _Pt::difference_type, 
                typename _Pt::pointer, 
                typename _Pt::reference
            >
        {
        public:
            bidir_iterator()
            {
            }

            explicit bidir_iterator(
                typename _Pt::pointer node)
                : node_(node)
            {
            }

            typename _Pt::reference operator * () const
            {
                return *node_;
            }

            bidir_iterator & operator++()
            {
                node_ = node_->next();
                return *this;
            }

            bidir_iterator & operator--()
            {
                node_ = node_->prev();
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
            bidir_iterator<_Ty, _Pt> const & x,
            bidir_iterator<_Ty, _Pt> const & y)
        {
            return x.get_node() == y.get_node();
        }

        template<typename _Ty, typename _Pt>
        class bidir_const_iterator
            : public boost::bidirectional_iterator_helper<
                bidir_const_iterator<_Ty, _Pt>, 
                _Ty, 
                typename _Pt::difference_type, 
                typename _Pt::const_pointer, 
                typename _Pt::const_reference
            >
        {
        public:
            bidir_const_iterator()
            {
            }

            bidir_const_iterator(
                bidir_iterator<_Ty, _Pt> const & r)
                : node_(r.get_node())
            {
            }

            explicit bidir_const_iterator(
                typename _Pt::const_pointer node)
                : node_(node)
            {
            }

            typename _Pt::const_reference operator * () const
            {
                return *node_;
            }

            bidir_const_iterator & operator++()
            {
                node_ = node_->next();
                return *this;
            }

            bidir_const_iterator & operator--()
            {
                node_ = node_->prev();
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
            bidir_const_iterator<_Ty, _Pt> const & x,
            bidir_const_iterator<_Ty, _Pt> const & y)
        {
            return x.get_node() == y.get_node();
        }

    }
}

#endif // _FRAMEWORK_CONTAINER_DETAIL_BIDIR_HOOK_H_
