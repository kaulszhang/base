// TcpsSocket.cpp

#include "framework/Framework.h"
#include "framework/network/TcpsSocket.h"

namespace framework
{
    namespace network
    {

        boost::asio::detail::mutex TcpsSocket::mutex_;

    } // namespace network
} // namespace framework
