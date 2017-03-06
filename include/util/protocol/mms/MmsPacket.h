// MmsPacket.h

#ifndef _UTIL_PROTOCOL_MMS_PACKET_H_
#define _UTIL_PROTOCOL_MMS_PACKET_H_

#include "util/protocol/mms/MmsHead.h"
#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        class MmsPacket
        {
        protected:
            MmsPacket(
                MmsHead & head)
                : head_(&head)
            {
            }

            MmsPacket & operator=(
                MmsPacket const & r);

        public:
            MmsHead & head()
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
            MmsHead * head_;
            boost::asio::streambuf data_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMS_PACKET_H_
