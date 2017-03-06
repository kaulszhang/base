// Buffers.h

#ifndef _UTIL_BUFFERS_BUFFERS_H_
#define _UTIL_BUFFERS_BUFFERS_H_

#include <framework/container/Array.h>

#include <algorithm>

namespace util
{
    namespace buffers
    {

        template <
            typename Buffer, 
            size_t max_size
        >
        class Buffers
            : public framework::container::Array<Buffer const>
        {
        public:
            typedef framework::container::Array<Buffer const> super;

        public:
            Buffers()
                : super(buffers_, 0)
            {
            }

            Buffers(
                Buffer const * buffers, 
                size_t count)
                : super(buffers_, count)
            {
                assert(count <= max_size);
                std::copy(buffers, buffers + count, buffers_);
            }

        public:
            void add(
                Buffer const & buffer)
            {
                assert(super::count() < max_size);
                buffers_[super::count()] = buffer;
                (super &)(*this) = super(buffers_, super::count() + 1);
            }

        public:
            Buffers & operator+=(
                Buffer const & r)
            {
                add(r);
                return *this;
            }

            Buffers & operator=(
                Buffers const & r)
            {
                std::copy(r.buffers_, r.buffers_ + r.count(), buffers_);
                (super &)(*this) = super(buffers_, r.count());
                return *this;
            }

        private:
            Buffer buffers_[max_size];
        };

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_H_
