// HttpPacket.h

#ifndef _UTIL_PROTOCOL_HTTP_PACKET_H_
#define _UTIL_PROTOCOL_HTTP_PACKET_H_

#include "util/protocol/http/HttpHead.h"

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        class HttpPacket
        {
        protected:
            HttpPacket(
                HttpHead & head)
                : head_(&head)
            {
            }

            HttpPacket & operator=(
                HttpPacket const & r);

        public:
            HttpHead & head()
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
            HttpHead * head_;
            boost::asio::streambuf data_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_PACKET_H_
