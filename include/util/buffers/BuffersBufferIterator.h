// BuffersBufferIterator.h

#ifndef _UTIL_BUFFERS_BUFFERS_BUFFER_ITERATOR_H_
#define _UTIL_BUFFERS_BUFFERS_BUFFER_ITERATOR_H_

#include "util/buffers/BuffersPosition.h"

#include <boost/iterator/iterator_facade.hpp>

namespace util
{
    namespace buffers
    {

        template <
            typename Buffer, 
            typename BufferIterator
        >
        class BuffersBufferIterator
            : public boost::iterator_facade<
                BuffersBufferIterator<Buffer, BufferIterator>,
                Buffer const, 
                boost::forward_traversal_tag
            >
        {
        public:
            typedef BuffersLimit<BufferIterator> Limit;

            typedef BuffersPosition<Buffer, BufferIterator> Position;

        public:
            BuffersBufferIterator(
                BufferIterator const & beg, 
                BufferIterator const & cur, 
                BufferIterator const & end, 
                size_t off = 0)
                : limit_(beg, end)
                , beg_(limit_, beg)
                , cur_(limit_, cur, off)
                , end_(limit_, end)
            {
            }

            BuffersBufferIterator(
                BufferIterator const & beg, 
                BufferIterator const & cur, 
                BufferIterator const & end)
                : limit_(beg, end)
                , beg_(limit_, beg)
                , cur_(limit_, cur)
                , end_(limit_, end)
            {
            }

            BuffersBufferIterator(
                BufferIterator const & beg, 
                BufferIterator const & end)
                : limit_(beg, end)
                , beg_(limit_, beg)
                , cur_(limit_, beg)
                , end_(limit_, end)
            {
            }

            BuffersBufferIterator()
            {
            }

        public:
            BuffersBufferIterator(
                Limit limit, 
                Position const & beg, 
                Position const & end)
                : limit_(limit)
                , beg_(beg)
                , cur_(beg)
                , end_(end)
            {
                cur_.set_end(limit_, end);
            }

            BuffersBufferIterator(
                Limit limit, 
                Position const & beg)
                : limit_(limit)
                , beg_(beg)
                , cur_(beg)
                , end_(limit, limit.end)
            {
            }

        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                cur_.increment_buffer(limit_, end_);
            }

            bool equal(
                BuffersBufferIterator const & r) const
            {
                return cur_.equal(r.cur_);
            }

            Buffer const & dereference() const
            {
                return cur_.dereference_buffer();
            }

        private:
            Limit limit_;
            Position beg_;
            Position cur_;
            Position end_;
        };

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_BUFFER_ITERATOR_H_
