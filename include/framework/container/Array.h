// array.h

#ifndef _FRAMEWORK_CONTAINER_ARRAY_H_
#define _FRAMEWORK_CONTAINER_ARRAY_H_

#include <boost/operators.hpp>

namespace framework
{
    namespace container
    {

        template<class T>
        class Array
        {
        public:    
            typedef T value_type;
            typedef value_type * iterator;
            typedef value_type const * const_iterator;

            Array()
                : pointer_(NULL)
                , count_(0)
            {
            }

            Array(
                value_type * t, 
                std::size_t s)
                : pointer_(t)
                , count_(s)
            {
            }

            Array(
                Array const & r)
                : pointer_(r.pointer_)
                , count_(r.count_)
            {
            }

            Array & operator=(
                Array const & r)
            {
                pointer_ = r.pointer_;
                count_ = r.count_;
                return *this;
            }

            value_type * address() const
            {
                return pointer_;
            }

            std::size_t size() const
            {
                return count_;
            }

            std::size_t count() const
            {
                return count_;
            }

            bool empty() const
            {
                return count_ == 0;
            }

            iterator begin()
            {
                return iterator(pointer_);
            }

            const_iterator begin() const
            {
                return const_iterator(pointer_);
            }

            iterator end()
            {
                return iterator(pointer_ + count_);
            }

            const_iterator end() const
            {
                return const_iterator(pointer_ + count_);
            }

        private:
            value_type * pointer_;
            std::size_t count_;
        };

        /// 指定数组指针和数组大小构造array
        template<class T>
        inline Array<T> const make_array(T * t, std::size_t s){
            return Array<T>(t, s);
        }

        /// 从定常数组构造array
#if !BOOST_WORKAROUND(BOOST_MSVC, < 1301)
        template<class T, std::size_t s>
        inline Array<T> const make_array(T (&t)[s]){
            return Array<T>(&t[0], s);
        }
#endif

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_ARRAY_H_
