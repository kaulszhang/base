// BuffersFind.h

#ifndef _UTIL_BUFFERS_BUFFERS_BYTE_ITERATOR_H_
#define _UTIL_BUFFERS_BUFFERS_BYTE_ITERATOR_H_

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
        class BuffersByteIterator
            : public boost::iterator_facade<
                BuffersByteIterator<Buffer, BufferIterator>,
                typename BufferByteType<Buffer>::type, 
                boost::forward_traversal_tag
            >
        {
        public:
            typedef BuffersLimit<BufferIterator> Limit;

            typedef BuffersPosition<Buffer, BufferIterator> Position;

        public:
            BuffersByteIterator(
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

            BuffersByteIterator(
                BufferIterator const & beg, 
                BufferIterator const & cur, 
                BufferIterator const & end)
                : limit_(beg, end)
                , beg_(limit_, beg)
                , cur_(limit_, cur)
                , end_(limit_, end)
            {
            }

            BuffersByteIterator(
                BufferIterator const & beg, 
                BufferIterator const & end)
                : limit_(beg, end)
                , beg_(limit_, beg)
                , cur_(limit_, beg)
                , end_(limit_, end)
            {
            }

        public:
            BuffersByteIterator(
                Limit limit, 
                Position const & beg, 
                Position const & end)
                : limit_(limit)
                , beg_(beg)
                , cur_(beg)
                , end_(end)
            {
                cur_.set_end(limit, end);
            }

            BuffersByteIterator(
                Limit limit, 
                Position const & beg)
                : limit_(limit)
                , beg_(beg)
                , cur_(beg)
                , end_(limit, limit.end)
            {
            }

            BuffersByteIterator()
            {
            }

        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                cur_.increment_byte(limit_, end_);
            }

            bool equal(
                BuffersByteIterator const & r) const
            {
                return cur_.equal(r.cur_);
            }

            typename Position::Byte & dereference() const
            {
                return cur_.dereference_byte();
            }

        private:
            Limit limit_;
            Position beg_;
            Position cur_;
            Position end_;
        };

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_BYTE_ITERATOR_H_
