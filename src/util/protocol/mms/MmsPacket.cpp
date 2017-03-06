// MmsPacket.cpp

#include "util/Util.h"
#include "util/protocol/mms/MmsPacket.h"
#include "util/protocol/mms/MmsError.h"

using namespace framework::system::logic_error;
using namespace boost::system;

namespace util
{
    namespace protocol
    {

        MmsPacket & MmsPacket::operator=(
            MmsPacket const & r)
        {
            // head_²»±ä
            data_.reset();
            memcpy(
                boost::asio::buffer_cast<void *>(data_.prepare(r.data_.size())), 
                boost::asio::buffer_cast<void const *>(r.data_.data()), 
                r.data_.size());
            data_.commit(r.data_.size());
            return *this;
        }

    } // namespace protocol
} // namespace util
