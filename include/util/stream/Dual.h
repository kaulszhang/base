// Dual.h

#ifndef _UTIL_STREAM_DUAL_H_
#define _UTIL_STREAM_DUAL_H_

#include "util/stream/Source.h"
#include "util/stream/Sink.h"

namespace util
{
    namespace stream
    {

        class Dual
            : public Source, public Sink
        {
        public:
            Dual(
                boost::asio::io_service & io_svc)
                : Source(io_svc)
                , Sink(io_svc)
            {
            }
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_DUAL_H_
