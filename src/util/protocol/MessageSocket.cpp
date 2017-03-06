// MessageSocket.cpp

#include "util/Util.h"

#include <boost/function.hpp>

#include "util/protocol/MessageSocket.h"
#include "util/protocol/MessageSocket.hpp"

#include "util/serialization/Array.h"

namespace util
{
    namespace protocol
    {

        MessageSocket::MessageSocket(
            boost::asio::io_service & io_svc, 
            MessageParser & parser, 
            void * ctx)
            : framework::network::TcpSocket(io_svc)
            , parser_(parser)
            , ctx_(ctx)
            , non_block_(false)
            , read_parallel_(false)
            , write_parallel_(false)
        {
        }

        MessageSocket::~MessageSocket()
        {
            boost::system::error_code ec;
            close(ec);
        }

        void MessageSocket::close()
        {
            snd_buf_.reset();
            rcv_buf_.reset();
            super::close();
        }

        boost::system::error_code MessageSocket::close(
            boost::system::error_code & ec)
        {
            snd_buf_.reset();
            rcv_buf_.reset();
            return framework::network::TcpSocket::close(ec);
        }

        void MessageSocket::set_read_parallel(
            bool b)
        {
            read_parallel_ = b;
        }

        void MessageSocket::set_write_parallel(
            bool b)
        {
            write_parallel_ = b;
        }

    } // namespace protocol
} // namespace util
