// RtspPacket.h

#ifndef _UTIL_PROTOCOL_RTSP_PACKET_H_
#define _UTIL_PROTOCOL_RTSP_PACKET_H_

#include "util/protocol/rtsp/RtspHead.h"

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        class RtspPacket
        {
        protected:
            RtspPacket(
                RtspHead & head)
                : head_(&head)
            {
            }

            RtspPacket & operator=(
                RtspPacket const & r);

        public:
            RtspHead & head()
            {
                return *head_;
            }

            boost::asio::streambuf & data()
            {
                return data_;
            }

            void clear_data()
            {
                data_.reset();
            }

        private:
            RtspHead * head_;
            boost::asio::streambuf data_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_PACKET_H_
