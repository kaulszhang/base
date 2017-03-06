// RtmpSocket.hpp

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_HPP_
#define _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_HPP_

#include "util/protocol/MessageSocket.hpp"
#include "util/protocol/rtmp/RtmpSocket.h"
#include "util/protocol/rtmp/RtmpMessageHelper.h"

#include <util/archive/ArchiveBuffer.h>

#include <framework/network/AsioHandlerHelper.h>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

namespace util
{
    namespace protocol
    {

        namespace detail
        {

            template <typename Handler>
            struct rtmp_connect_handler
            {
                typedef void result_type;

                rtmp_connect_handler(
                     RtmpSocket & socket, 
                     Handler const & handler)
                     : socket_(socket)
                     , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code const & ec)
                {
                    socket_.handle_connect(ec, handler_);
                }

                void operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    socket_.handle_connect(ec, handler_);
                }

            private:
                RtmpSocket & socket_;
                Handler handler_;
            };

            template <typename Handler>
            struct rtmp_accept_handler
            {
                rtmp_accept_handler(
                    RtmpSocket & socket, 
                    Handler const & handler)
                    : socket_(socket)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code const & ec)
                {
                    socket_.handle_accept(ec, handler_);
                }

                void operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    socket_.handle_accept(ec, handler_);
                }

            private:
                RtmpSocket & socket_;
                Handler handler_;
            };

        } // namespace detail

        template <typename Handler>
        void RtmpSocket::async_connect(
            framework::network::NetName const & addr, 
            Handler const & handler)
        {
            MessageSocket::async_connect(addr, 
                detail::rtmp_connect_handler<Handler>(*this, handler));
        }

        template <typename Handler>
        void RtmpSocket::handle_connect(
            boost::system::error_code const & ec, 
            Handler const & handler)
        {
            if (ec) {
                handler(ec);
                return;
            }

            switch (status_) {
                case closed:
                    make_c01(snd_buf_);
                    status_ = handshake1;
                    boost::asio::async_write(*this, snd_buf_.data(), 
                        boost::asio::transfer_all(), 
                        detail::rtmp_connect_handler<Handler>(*this, handler));
                    break;
                case handshake1:
                    snd_buf_.consume(snd_buf_.size());
                    status_ = handshake2;
                    boost::asio::async_read(*this, rcv_buf_.prepare(1 + HANDSHAKE_SIZE * 2), 
                        boost::asio::transfer_all(), 
                        detail::rtmp_connect_handler<Handler>(*this, handler));
                    break;
                case handshake2:
                    rcv_buf_.commit(1 + HANDSHAKE_SIZE * 2);
                    check_s012(rcv_buf_);
                    status_ = handshake3;
                    make_c2(snd_buf_);
                    rcv_buf_.consume(1 + HANDSHAKE_SIZE * 2);
                    boost::asio::async_write(*this, snd_buf_.data(), 
                        boost::asio::transfer_all(), 
                        detail::rtmp_connect_handler<Handler>(*this, handler));
                    break;
                case handshake3:
                    snd_buf_.consume(snd_buf_.size());
                    status_ = established;
                    handler(ec);
                    break;
                default:
                    assert(false);
                    break;
            }
        }

        //template <typename Handler>
        //void RtmpSocket::async_accept(
        //    framework::network::Acceptor & acceptor, 
        //    Handler const & handler)
        //{
        //    MessageSocket::async_accept(acceptor, 
        //        detail::rtmp_accept_handler<Handler>(*this, handler));
        //}

        //template <typename Handler>
        //void RtmpSocket::handle_accept(
        //    boost::system::error_code const & ec, 
        //    Handler const & handler)
        //{
        //    if (ec) {
        //        handler(ec);
        //        return;
        //    }

        //    switch (status_) {
        //        case closed:
        //            status_ = handshake1;
        //            boost::asio::async_read(*this, rcv_buf_.prepare(1 + HANDSHAKE_SIZE), 
        //                boost::asio::transfer_all(), 
        //                detail::rtmp_accept_handler<Handler>(*this, handler));
        //            break;
        //        case handshake1:
        //            rcv_buf_.commit(1 + HANDSHAKE_SIZE);
        //            check_c01(rcv_buf_);
        //            rcv_buf_.consume(1 + HANDSHAKE_SIZE);
        //            status_ = handshake2;
        //            make_s012(snd_buf_);
        //            boost::asio::async_write(*this, snd_buf_.data(), 
        //                boost::asio::transfer_all(), 
        //                detail::rtmp_accept_handler<Handler>(*this, handler));
        //            break;
        //        case handshake2:
        //            snd_buf_.consume(snd_buf_.size());
        //            status_ = handshake3;
        //            boost::asio::async_read(*this, rcv_buf_.prepare(HANDSHAKE_SIZE), 
        //                boost::asio::transfer_all(), 
        //                detail::rtmp_accept_handler<Handler>(*this, handler));
        //            break;
        //        case handshake3:
        //            rcv_buf_.commit(HANDSHAKE_SIZE);
        //            check_c2(rcv_buf_);
        //            rcv_buf_.consume(HANDSHAKE_SIZE);
        //            status_ = established;
        //            handler(ec);
        //            break;
        //        default:
        //            assert(false);
        //            break;
        //    }
        //}

        template <
            typename MutableBufferSequence
        >
        void RtmpRawChunk::from_data(
            RtmpMessageHeaderEx & header, 
            MutableBufferSequence const & buffers, 
            boost::uint32_t size, 
            RtmpMessageContext * ctx)
        {
            boost::uint8_t const * p = NULL;
            size_t n = pos;
            typename MutableBufferSequence::const_iterator iter = buffers.begin();
            for (; iter != buffers.end(); ++iter) {
                if (n < boost::asio::buffer_size(*iter)) {
                    p = boost::asio::buffer_cast<boost::uint8_t const *>(*iter) + n;
                    n = boost::asio::buffer_size(*iter) - n;
                    break;
                } else {
                    n -= boost::asio::buffer_size(*iter);
                }
            }
            assert(p);
            RtmpChunkBasicHeader h;
            h.one_byte = p[0];
            if (h.cs_id0 < 2) {
                if (h.cs_id0 == 0) {
                    h.cs_id1 = get_char(buffers, iter, p, n);
                } else {
                    boost::uint8_t b1 = get_char(buffers, iter, p, n);
                    boost::uint8_t b2 = get_char(buffers, iter, p, n);
                    h.cs_id2 = (boost::uint16_t)b1 << 8 | b2;
                }
            }
            assert(pos == 0 || cs_id == h.cs_id());
            cs_id = h.cs_id();
            finish = ctx->read.chunk(cs_id).put_data(ctx->read.chunk_size());
            if (finish) {
                RtmpChunkHeader chunk;
                typename MutableBufferSequence::const_iterator iter = buffers.begin();
                boost::uint8_t * p = boost::asio::buffer_cast<boost::uint8_t *>(*iter);
                size_t n = boost::asio::buffer_size(*iter);
                util::archive::ArchiveBuffer<boost::uint8_t> buf(p, n, n);
                RtmpMessageTraits::i_archive_t ia(buf);
                ia >> chunk;
                ctx->read.from_chunk(header, chunk);
                header.chunk_size = ctx->read.chunk_size();
            }
            pos += size;
        }

        template <
            typename MutableBufferSequence
        >
        boost::uint8_t RtmpRawChunk::get_char(
            MutableBufferSequence const & buffers, 
            typename MutableBufferSequence::const_iterator & iter, 
            boost::uint8_t const *& p, 
            size_t & n)
        {
            while (n == 0) {
                ++iter;
                assert(iter != buffers.end());
                p = boost::asio::buffer_cast<boost::uint8_t const *>(*iter);
                n = boost::asio::buffer_size(*iter);
            }
            --n;
            return *p++;
        }

        template <
            typename MutableBufferSequence
        >
        size_t RtmpSocket::read_raw_msg(
            RtmpMessageHeaderEx & header, 
            MutableBufferSequence const & buffers, 
            boost::system::error_code & ec)
        {
            if (help_raw_chunk_.cs_id == 0) {
                help_raw_chunk_.pos = 0;
                help_raw_chunk_.cs_id = 1;
            }
            size_t bytes_read = 0;
            while ((bytes_read = MessageSocket::read_raw_msg(
                util::buffers::sub_buffers(buffers, help_raw_chunk_.pos), ec))) {
                    help_raw_chunk_.from_data(header, buffers, bytes_read, &context_);
                    if (help_raw_chunk_.finish) {
                        help_raw_chunk_.cs_id = 0;
                        help_raw_chunk_.finish = false;
                        return help_raw_chunk_.pos;
                    }
            }
            return 0;
        }

        namespace detail
        {

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            struct rtmp_raw_msg_read_handler
            {
                rtmp_raw_msg_read_handler(
                    RtmpSocket & socket, 
                    RtmpMessageHeaderEx & header, 
                    MutableBufferSequence const & buffers, 
                    Handler handler)
                    : socket_(socket)
                    , header_(header)
                    , buffers_(buffers)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code ec, 
                    size_t bytes_transferred) const
                {
                    socket_.handle_read_raw_msg(header_, buffers_, handler_, ec, bytes_transferred);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(rtmp_raw_msg_read_handler, handler_)

            private:
                RtmpSocket & socket_;
                RtmpMessageHeaderEx & header_;
                MutableBufferSequence buffers_;
                Handler handler_;
            };

        } // namespace detail

        template <
            typename MutableBufferSequence, 
            typename Handler
        >
        void RtmpSocket::async_read_raw_msg(
            RtmpMessageHeaderEx & header, 
            MutableBufferSequence const & buffers, 
            Handler const & handler)
        {
            assert(help_raw_chunk_.cs_id == 0);
            help_raw_chunk_.pos = 0;
            help_raw_chunk_.cs_id = 1;
            MessageSocket::async_read_raw_msg(
                util::buffers::sub_buffers(buffers, help_raw_chunk_.pos), 
                detail::rtmp_raw_msg_read_handler<MutableBufferSequence, Handler>(*this, header, buffers, handler));
        }

        template <
            typename MutableBufferSequence, 
            typename Handler
        >
        void RtmpSocket::handle_read_raw_msg(
            RtmpMessageHeaderEx & header, 
            MutableBufferSequence const & buffers, 
            Handler const & handler, 
            boost::system::error_code ec, 
            size_t bytes_transferred)
        {
            if (ec) {
                if (ec == boost::asio::error::would_block) {
                    assert(help_raw_chunk_.pos == 0);
                    help_raw_chunk_.cs_id = 0;
                }
                handler(ec, 0);
                return;
            }
            help_raw_chunk_.from_data(header, buffers, bytes_transferred, &context_);
            if (help_raw_chunk_.finish) {
                help_raw_chunk_.cs_id = 0;
                help_raw_chunk_.finish = false;
                handler(ec, help_raw_chunk_.pos);
                return;
            }
            MessageSocket::async_read_raw_msg(
                util::buffers::sub_buffers(buffers, help_raw_chunk_.pos), 
                detail::rtmp_raw_msg_read_handler<MutableBufferSequence, Handler>(*this, header, buffers, handler));
        }

        template <
            typename ConstBufferSequence
        >
        size_t RtmpSocket::write_raw_msg(
            RtmpMessageHeaderEx const & header, 
            ConstBufferSequence const & buffers, 
            boost::system::error_code & ec)
        {
            if (MessageSocket::write_free()) {
                RtmpChunkHeader chunk;
                context_.write.to_chunk(header, chunk);
            }
            return MessageSocket::write_raw_msg(buffers, ec);
        }

        template <
            typename MutableBufferSequence, 
            typename Handler
        >
        void RtmpSocket::async_write_raw_msg(
            RtmpMessageHeaderEx const & header, 
            MutableBufferSequence const & buffers, 
            Handler const & handler)
        {
            RtmpChunkHeader chunk;
            context_.write.to_chunk(header, chunk);
            MessageSocket::async_write_raw_msg(buffers, handler);
        }

        namespace detail
        {

            template <
                typename Handler
            >
            struct rtmp_msg_read_handler
            {
                rtmp_msg_read_handler(
                    RtmpSocket & socket, 
                    RtmpMessage & msg, 
                    Handler handler)
                    : socket_(socket)
                    , msg_(msg)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code ec, 
                    size_t bytes_transferred) const
                {
                    socket_.handle_read_msg(msg_, handler_, ec, bytes_transferred);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(rtmp_msg_read_handler, handler_)

            private:
                RtmpSocket & socket_;
                RtmpMessage & msg_;
                Handler handler_;
            };

        } // namespace detail

        template <
            typename Handler
        >
        void RtmpSocket::async_read_msg(
            RtmpMessage & msg, 
            Handler const & handler)
        {
            MessageSocket::async_read_msg(help_chunk_, 
                detail::rtmp_msg_read_handler<Handler>(*this, msg, handler));
        }

        template <
            typename Handler
        >
        void RtmpSocket::handle_read_msg(
            RtmpMessage & msg, 
            Handler const & handler, 
            boost::system::error_code ec, 
            size_t bytes_transferred)
        {
            if (help_chunk_.finish) {
                help_chunk_.finish = false;
                RtmpChunkMessage & cm(context_.read.chunk(help_chunk_.cs_id));
                boost::uint32_t size = cm.data.size();
                msg.from_data(cm.data, &context_);
                handler(ec, size);
                return;
            }
            if (ec) {
                handler(ec, 0);
                return;
            }
            MessageSocket::async_read_msg(help_chunk_, 
                detail::rtmp_msg_read_handler<Handler>(*this, msg, handler));
        }

        struct RtmpMessageVector
        {
            RtmpMessageVector(
                std::vector<RtmpMessage> const & msgs)
                : msgs_(msgs)
            {
            }

            void to_data(
                StreamBuffer & buf, 
                void * ctx) const;

        private:
            std::vector<RtmpMessage> const & msgs_;
        };

        template <typename Handler>
        void RtmpSocket::async_write_msgs(
            std::vector<RtmpMessage> const & msgs, 
            Handler const & handler)
        {
            async_write_msg(RtmpMessageVector(msgs), handler);
        }


    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_SOCKET_HPP_
