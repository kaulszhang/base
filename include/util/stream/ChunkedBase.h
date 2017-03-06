// ChunkedBase.h

#ifndef _UTIL_STREAM_CHUNKED_BASE_H_
#define _UTIL_STREAM_CHUNKED_BASE_H_

namespace util
{
    namespace stream
    {

        class ChunkedBase
        {
        public:
            struct eof_t {};

            static eof_t eof() { return eof_t(); }
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_CHUNKED_BASE_H_
