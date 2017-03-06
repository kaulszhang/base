// RtmpSocket.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_H_

#include "util/protocol/rtmp/RtmpMessage.h"
#include "util/protocol/rtmp/RtmpMessageContext.h"
#include "util/protocol/rtmp/RtmpChunkParser.h"
#include "util/protocol/rtmp/RtmpHandshake.h"
#include "util/protocol/MessageSocket.h"

namespace util
{
    namespace protocol
    {

        struct RtmpChunk
        {
            RtmpChunk()
                : finish(false)
                , cs_id(0)
            {
            }

            void from_data(
                StreamBuffer & buf, 
                void * vctx);

            bool finish;
            boost::uint16_t cs_id;
        };

        struct RtmpRawChunk
        {
            RtmpRawChunk()
                : finish(false)
                , pos(0)
                , cs_id(0)
            {
            }

            template <
                typename MutableBufferSequence
            >
            void from_data(
                RtmpMessageHeaderEx & header, 
                MutableBufferSequence const & buffers, 
                boost::uint32_t size, 
                RtmpMessageContext * ctx);

            template <
                typename MutableBufferSequence
            >
            static boost::uint8_t get_char(
                MutableBufferSequence const & buffers, 
                typename MutableBufferSequence::const_iterator & iter, 
                boost::uint8_t const *& p, 
                size_t & n);

            bool finish;
            size_t pos;
            boost::uint16_t cs_id;
        };


        namespace detail
        {

            template <typename Handler>
            struct rtmp_connect_handler;

            template <typename Handler>
            struct rtmp_accept_handler;

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            struct rtmp_raw_msg_read_handler;

            template <
                typename Handler
            >
            struct rtmp_msg_read_handler;

        }

        class RtmpSocket
            : public MessageSocket
            , private RtmpHandshake
        {
        public:
            RtmpSocket(
                boost::asio::io_service & io_svc);

            ~RtmpSocket();

        public:
            enum StatusEnum
            {
                closed = 0, 
                handshake1, 
                handshake2, 
                handshake3,
                established,
            };

        public:
            void connect(
                framework::network::NetName const & addr);

            boost::system::error_code connect(
                framework::network::NetName const & addr, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_connect(
                framework::network::NetName const & addr, 
                Handler const & handler);

        public:
            //void accept(
            //    framework::network::Acceptor & acceptor);

            //boost::system::error_code accept(
            //    framework::network::Acceptor & acceptor, 
            //    boost::system::error_code & ec);

            //template <typename Handler>
            //void async_accept(
            //    framework::network::Acceptor & acceptor, 
            //    Handler const & handler);

        public:
            template <
                typename MutableBufferSequence
            >
            size_t read_raw_msg(
                RtmpMessageHeaderEx & header, 
                MutableBufferSequence const & buffers, 
                boost::system::error_code & ec);

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            void async_read_raw_msg(
                RtmpMessageHeaderEx & header, 
                MutableBufferSequence const & buffers, 
                Handler const & handler);

        public:
            template <
                typename ConstBufferSequence
            >
            size_t write_raw_msg(
                RtmpMessageHeaderEx const & header, 
                ConstBufferSequence const & buffers, 
                boost::system::error_code & ec);

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            void async_write_raw_msg(
                RtmpMessageHeaderEx const & header, 
                MutableBufferSequence const & buffers, 
                Handler const & handler);

        public:
            size_t read_msg(
                RtmpMessage & msg, 
                boost::system::error_code & ec);

            template <
                typename Handler
            >
            void async_read_msg(
                RtmpMessage & msg, 
                Handler const & handler);

        public:
            size_t write_msgs(
                std::vector<RtmpMessage> const & msgs, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_write_msgs(
                std::vector<RtmpMessage> const & msgs, 
                Handler const & handler);

        public:
            RtmpMessageContext const & context() const
            {
                return context_;
            }

        protected:
            void tick(
                std::vector<RtmpMessage> & resp);

        protected:
            bool process_protocol_message(
                RtmpMessage const & msg, 
                std::vector<RtmpMessage> & resp);

        private:
            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            friend struct detail::rtmp_raw_msg_read_handler;

            template <
                typename Handler
            >
            friend struct detail::rtmp_msg_read_handler;

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            void handle_read_raw_msg(
                RtmpMessageHeaderEx & header, 
                MutableBufferSequence const & buffers, 
                Handler const & handler, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

            template <
                typename Handler
            >
            void handle_read_msg(
                RtmpMessage & msg, 
                Handler const & handler, 
                boost::system::error_code ec, 
                size_t bytes_transferred);

        private:
            template <typename Handler>
            friend struct detail::rtmp_connect_handler;

            template <typename Handler>
            friend struct detail::rtmp_accept_handler;

            template <typename Handler>
            void handle_connect(
                boost::system::error_code const & ec, 
                Handler const & handler);

            template <typename Handler>
            void handle_accept(
                boost::system::error_code const & ec, 
                Handler const & handler);

        private:
            StatusEnum status_;
            RtmpMessageContext context_;
            RtmpRawChunk help_raw_chunk_;
            RtmpChunk help_chunk_;
            RtmpChunkParser read_parser_;
            boost::system::error_code last_ec_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_H_
