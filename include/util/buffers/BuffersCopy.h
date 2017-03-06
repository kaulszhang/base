// BuffersCopy.h

#ifndef _UTIL_BUFFERS_BUFFERS_COPY_H_
#define _UTIL_BUFFERS_BUFFERS_COPY_H_

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template <typename MutableBufferSequence, typename ConstBufferSequence>
        size_t buffers_copy(
            MutableBufferSequence const & dst, 
            ConstBufferSequence const & src, 
            size_t size = size_t(-1), 
            size_t dst_off = 0, 
            size_t src_off = 0)
        {
            using namespace boost::asio;
            typename MutableBufferSequence::const_iterator idst = dst.begin();
            typename MutableBufferSequence::const_iterator edst = dst.end();
            typename ConstBufferSequence::const_iterator isrc = src.begin();
            typename ConstBufferSequence::const_iterator esrc = src.end();
            size_t bytes_copied = 0;
            if (size > 0 && idst != edst && isrc != esrc) {
                mutable_buffer bdst = buffer(*idst);
                while (dst_off) {
                    if (dst_off >= buffer_size(bdst)) {
                        if (++idst == edst)
                            break;
                        dst_off -= buffer_size(bdst);
                        bdst = buffer(*idst);
                    } else {
                        bdst = bdst + dst_off;
                        dst_off = 0;
                    }
                }
                if (dst_off)
                    return 0;
                const_buffer bsrc = buffer(*isrc);
                while (src_off) {
                    if (src_off >= buffer_size(bsrc)) {
                        if (++isrc == esrc)
                            break;
                        src_off -= buffer_size(bsrc);
                        //bsrc = buffer(*idst);
                        bsrc = buffer(*isrc);
                    } else {
                        bsrc = bsrc + src_off;
                        src_off = 0;
                    }
                }
                if (src_off)
                    return 0;
                while (1) {
                    size_t this_size = size;
                    if (this_size > buffer_size(bdst)) {
                        this_size = buffer_size(bdst);
                    }
                    if (this_size > buffer_size(bsrc)) {
                        this_size = buffer_size(bsrc);
                    }
                    memcpy(buffer_cast<void *>(bdst), buffer_cast<void const *>(bsrc), this_size);
                    bytes_copied += this_size;
                    size -= this_size;
                    if (size == 0)
                        break;
                    if (this_size == buffer_size(bdst)) {
                        bsrc = bsrc + this_size;
                        if (++idst == edst)
                            break;
                        bdst = buffer(*idst);
                    }
                    if (this_size == buffer_size(bsrc)) {
                        bdst = bdst + this_size;
                        if (++isrc == esrc)
                            break;
                        bsrc = buffer(*isrc);
                    }
                }
            }
            return bytes_copied;
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_COPY_H_
