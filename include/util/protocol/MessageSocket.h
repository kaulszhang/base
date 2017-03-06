// MessageSocket.h

#ifndef _UTIL_PROTOCOL_MESSAGE_SOCKET_H_
#define _UTIL_PROTOCOL_MESSAGE_SOCKET_H_

#include <util/buffers/StreamBuffer.h>

#include <framework/network/TcpSocket.h>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace util
{
    namespace protocol
    {

        class MessageParser;

        typedef util::buffers::StreamBuffer<boost::uint8_t> StreamBuffer;

        namespace detail
        {

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            struct raw_msg_read_handler;

            template <
                typename ConstBufferSequence, 
                typename Handler
            >
            struct raw_msg_write_handler;

            template <
                typename Message, 
                typename Handler
            >
            struct msg_read_handler;

            template <
                typename Message, 
                typename Handler
            >
            struct msg_write_handler;

        } // namespace detail

        class MessageSocket
            : public framework::network::TcpSocket
        {
        public:
            MessageSocket(
                boost::asio::io_service & io_svc, 
                MessageParser & parser, 
                void * ctx = NULL);

            ~MessageSocket();

        public:
            void set_read_parallel(
                bool b);

            void set_write_parallel(
                bool b);

        public:
            void close();

            boost::system::error_code close(
                boost::system::error_code & ec);

        public:
            template <
                typename MutableBufferSequence
            >
            size_t read_raw_msg(
                MutableBufferSequence const & buffers, 
                boost::system::error_code & ec);

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            void async_read_raw_msg(
                MutableBufferSequence const & buffers, 
                Handler const & handler);

        public:
            template <
                typename ConstBufferSequence
            >
            size_t write_raw_msg(
                ConstBufferSequence const & buffers, 
                boost::system::error_code & ec);

            template <
                typename ConstBufferSequence, 
                typename Handler
            >
            void async_write_raw_msg(
                ConstBufferSequence const & buffers, 
                Handler const & handler);

        public:
            template <
                typename Message
            >
            size_t read_msg(
                Message & msg, 
                boost::system::error_code & ec);

            template <
                typename Message, 
                typename Handler
            >
            void async_read_msg(
                Message & msg, 
                Handler const & handler);

        public:
            template <
                typename Message
            >
            size_t write_msg(
                Message const & msg, 
                boost::system::error_code & ec);

            template <
                typename Message, 
                typename Handler
            >
            void async_write_msg(
                Message const & msg, 
                Handler const & handler);

        private:
            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            friend struct detail::raw_msg_read_handler;

            template <
                typename ConstBufferSequence, 
                typename Handler
            >
            friend struct detail::raw_msg_write_handler;

            template <
                typename Message, 
                typename Handler
            >
            friend struct detail::msg_read_handler;

            template <
                typename Message, 
                typename Handler
            >
            friend struct detail::msg_write_handler;

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            void handle_read_raw_msg(
                MutableBufferSequence const & buffers, 
                Handler const & handler, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

            template <
                typename ConstBufferSequence, 
                typename Handler
            >
            void handle_write_raw_msg(
                ConstBufferSequence const & buffers, 
                Handler const & handler, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

            template <
                typename Message, 
                typename Handler
            >
            void handle_read_msg(
                Message & msg, 
                Handler const & handler, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

            template <
                typename Message, 
                typename Handler
            >
            void handle_write_msg(
                Message const & msg, 
                Handler const & handler, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

            template <
                typename Handler
            >
            void response(
            Handler const & handler, 
            boost::system::error_code ec, 
            size_t bytes_transferred);

        protected:
            bool read_free() const
            {
                return read_status_.size == 0;
            }

            bool write_free() const
            {
                return write_status_.size == 0;
            }

        protected:
            boost::mutex mutex_;
            boost::condition_variable cond_;

            StreamBuffer snd_buf_;
            StreamBuffer rcv_buf_;
            std::STREAMSDK_CONTANINER<size_t> pend_rcv_sizes_;
            StreamBuffer rcv_data_;
            std::STREAMSDK_CONTANINER<size_t> pend_data_sizes_;

        private:
            MessageParser & parser_;
            void * ctx_;

            typedef boost::function<void (
                boost::system::error_code ec, 
                size_t bytes_transferred
                )> wait_resp_t;

            struct MessageStatus
            {
                MessageStatus()
                    : size(0)
                    , pos(0)
                    , wait(0)
                {
                }

                size_t size;
                size_t pos;
                // 对于 write , wait 用来记录延迟 write 的数据字节数
                // 对于 read , wait 用来记录 raw buffer 的空间大小
                size_t wait;
                wait_resp_t resp;

                size_t left() const
                {
                    return size - pos;
                }
            };

            MessageStatus read_status_;
            MessageStatus write_status_;

            bool non_block_;
            bool read_parallel_;
            bool write_parallel_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_SOCKET_H_
