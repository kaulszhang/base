// BuffersCompare.h

#ifndef _UTIL_BUFFERS_BUFFERS_COMPARE_H_
#define _UTIL_BUFFERS_BUFFERS_COMPARE_H_

#include "util/buffers/BuffersPosition.h"

namespace util
{
    namespace buffers
    {

        namespace detail
        {

            template <
                typename Buffer2, 
                typename Buffer1, 
                typename BufferIterator1, 
                typename BufferIterator2
            >
            int buffers_compare_impl(
                BuffersLimit<BufferIterator1> const & limit1, 
                BuffersLimit<BufferIterator2> const & limit2, 
                BuffersPosition<Buffer1, BufferIterator1> pos1, 
                BuffersPosition<Buffer1, BufferIterator1> const & end1, 
                size_t size = (size_t)-1)
            {
                char const * buf1 = "";
                size_t size1 = 0;
                char const * buf2 = "";
                size_t size2 = 0;
                long result = 0;
                BuffersPosition<Buffer1, BufferIterator2> pos2(limit2);
                while (size > 0 && !pos1.at_end() && !pos2.at_end()) {
                    buf1 = boost::asio::buffer_cast<char const *>(pos1.dereference_buffer());
                    size1 = boost::asio::buffer_size(pos1.dereference_buffer());
                    buf2 = boost::asio::buffer_cast<char const *>(pos2.dereference_buffer());
                    size2 = boost::asio::buffer_size(pos2.dereference_buffer());
                    if (size1 <= size2) {
                        if (size1 > size)
                            size1 = size;
                        result = (long)::memcmp((void *)buf1, buf2, size1);
                        size -= size1;
                        if (result || size == 0)
                            break;
                        pos2.increment_bytes(limit2, size1);
                        pos1.increment_buffer(limit1, end1);
                    } else {
                        if (size2 > size)
                            size2 = size;
                        result = (long)::memcmp((void *)buf1, buf2, size2);
                        size -= size2;
                        if (result || size == 0)
                            break;
                        pos1.increment_bytes(limit1, end1, size2);
                        pos2.increment_buffer(limit2);
                    }
                }
                if (result == 0 && size != 0) {
                    if (pos1.at_end() && pos2.at_end()) {
                    } else if (pos1.at_end()) {
                        result = -(int)pos2.dereference_byte();
                    } else {
                        result = (int)pos1.dereference_byte();
                    }
                }
                return result;
            }

            template <
                typename Buffer1, 
                typename BufferIterator1, 
                typename Buffer2, 
                typename BufferIterator2
            >
            int buffers_compare_impl(
                BuffersLimit<BufferIterator1> const & limit1, 
                BuffersLimit<BufferIterator2> const & limit2, 
                size_t size = (size_t)-1)
            {
                char const * buf1 = "";
                size_t size1 = 0;
                char const * buf2 = "";
                size_t size2 = 0;
                long result = 0;
                BuffersPosition<Buffer1, BufferIterator1> pos1(limit1);
                BuffersPosition<Buffer1, BufferIterator1> pos2(limit2);
                while (size > 0 && !pos1.at_end() && !pos2.at_end()) {
                    buf1 = boost::asio::buffer_cast<char const *>(pos1.dereference_buffer());
                    size1 = boost::asio::buffer_size(pos1.dereference_buffer());
                    if (size1 <= size2) {
                        if (size1 > size)
                            size1 = size;
                        result = (long)::memcpy((void *)buf1, buf2, size1);
                        size -= size1;
                        if (result || size == 0)
                            break;
                        buf2 += size1;
                        size2 -= size1;
                        size1 = 0;
                        pos1.increment_buffer(limit1);
                    } else {
                        if (size2 > size)
                            size2 = size;
                        result = (long)::memcpy((void *)buf1, buf2, size2);
                        size -= size2;
                        if (result || size == 0)
                            break;
                        buf1 += size2;
                        size1 -= size2;
                        size2 = 0;
                        pos2.increment_buffer(limit2);
                    }
                }
                if (result == 0 && size != 0) {
                    if (pos1.at_end() && pos2.at_end()) {
                    } else if (pos1.at_end()) {
                        result = -(int)*buf2;
                    } else {
                        pos1.increment_bytes(buf1 
                            - boost::asio::buffer_cast<char const *>(pos1.dereference_buffer()));
                        result = (int)*buf1;
                    }
                }
                return result;
            }

        } // namespace detail

        template <typename ConstBufferSequence1, typename ConstBufferSequence2>
        size_t buffers_compare(
            ConstBufferSequence1 const & buffers1, 
            ConstBufferSequence2 const & buffers2, 
            size_t size = size_t(-1))
        {
            typedef BuffersLimit<
                typename ConstBufferSequence1::const_iterator
            > Limit1;
            typedef BuffersLimit<
                typename ConstBufferSequence2::const_iterator
            > Limit2;
            return detail::buffers_compare_impl<
                typename ConstBufferSequence1::value_type, 
                typename ConstBufferSequence1::const_iterator, 
                typename ConstBufferSequence2::value_type, 
                typename ConstBufferSequence2::const_iterator
            >(Limit1(buffers1.begin(), buffers1.end()), 
                Limit2(buffers2.begin(), buffers2.end()), size);
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_COMPARE_H_
