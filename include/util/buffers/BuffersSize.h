// BuffersSize.h

#ifndef _UTIL_BUFFERS_BUFFERS_SIZE_H_
#define _UTIL_BUFFERS_BUFFERS_SIZE_H_

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template <typename ConstBufferSequence>
        size_t buffers_size(
            ConstBufferSequence const & buf)
        {
            using namespace boost::asio;
            typename ConstBufferSequence::const_iterator iter = buf.begin();
            typename ConstBufferSequence::const_iterator end = buf.end();
            size_t total_size = 0;
            for (; iter != end; ++iter) {
                total_size += boost::asio::buffer_size(buffer(*iter));
            }
            return total_size;
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_SIZE_H_
