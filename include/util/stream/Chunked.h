// Chunked.h

#ifndef _UTIL_STREAM_CHUNKED_H_
#define _UTIL_STREAM_CHUNKED_H_

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        class Chunked
            : public Dual
        {
        public:
            Chunked(
                Dual & dual)
                : Dual(dual.get_io_service())
                , dual_(dual)
                , snd_left_(0)
                , rcv_left_(0)
            {
            }

        public:
            void close()
            {
                snd_left_ = 0;
                snd_buf_.reset();
                rcv_left_ = 0;
                rcv_buf_.reset();
            }

        private:
            virtual std::size_t private_read_some(
                StreamMutableBuffers const & buffers,
                boost::system::error_code & ec);

            virtual void private_async_read_some(
                StreamMutableBuffers const & buffers, 
                StreamHandler const & handler);

            virtual std::size_t private_write_some(
                StreamConstBuffers const & buffers, 
                boost::system::error_code & ec);

            virtual void private_async_write_some(
                StreamConstBuffers const & buffers, 
                StreamHandler const & handler);

        private:
            Dual & dual_;
            size_t snd_left_;
            boost::asio::streambuf snd_buf_;
            size_t rcv_left_;
            boost::asio::streambuf rcv_buf_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_STREAM_CHUNKED_H_
