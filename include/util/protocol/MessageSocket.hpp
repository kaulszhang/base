// MessageSocket.hpp

#ifndef _UTIL_PROTOCOL_MESSAGE_SOCKET_HPP_
#define _UTIL_PROTOCOL_MESSAGE_SOCKET_HPP_

#include "util/protocol/MessageSocket.h"
#include "util/protocol/MessageParser.h"
#include "util/protocol/MessageTraits.h"
#include "util/protocol/MessageDefine.h"

#include <util/buffers/BuffersCopy.h>
#include <util/buffers/BuffersSize.h>
#include <util/buffers/SubBuffers.h>

#include <framework/network/AsioHandlerHelper.h>
#include <framework/network/TcpSocket.h>

namespace util
{
    namespace protocol
    {

        namespace detail
        {

            template <
                typename MutableBufferSequence, 
                typename Handler
            >
            struct raw_msg_read_handler
            {
                raw_msg_read_handler(
                    MessageSocket & socket, 
                    MutableBufferSequence const & buffers, 
                    Handler handler)
                    : socket_(socket)
                    , buffers_(buffers)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code ec, 
                    size_t bytes_transferred)
                {
                    socket_.handle_read_raw_msg(buffers_, handler_, ec, bytes_transferred);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(raw_msg_read_handler, handler_)

            private:
                MessageSocket & socket_;
                MutableBufferSequence buffers_;
                Handler handler_;
            };

            template <
                typename ConstBufferSequence, 
                typename Handler
            >
            struct raw_msg_write_handler
            {
                raw_msg_write_handler(
                    MessageSocket & socket, 
                    ConstBufferSequence const & buffers, 
                    Handler handler)
                    : socket_(socket)
                    , buffers_(buffers)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code ec, 
                    size_t bytes_transferred)
                {
                    socket_.handle_write_raw_msg(buffers_, handler_, ec, bytes_transferred);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(raw_msg_write_handler, handler_)

            private:
                MessageSocket & socket_;
                ConstBufferSequence buffers_;
                Handler handler_;
            };

        } // namespace detail

        template <
            typename MutableBufferSequence
        >
        size_t MessageSocket::read_raw_msg(
            MutableBufferSequence const & buffers, 
            boost::system::error_code & ec)
        {
            assert(read_parallel_);
            if (!pend_rcv_sizes_.empty()) {
                // control messages should be handle before data messages
                ec = boost::asio::error::would_block;
                return 0;
            }
            if (pend_data_sizes_.size() > 0) {
                size_t size = pend_data_sizes_.front();
                pend_data_sizes_.pop_front();
                assert(size <= rcv_data_.size());
                util::buffers::buffers_copy(buffers, rcv_data_.data(size));
                rcv_data_.consume(size);
                ec.clear();
                return size;
            }
            if (read_status_.size == 0) {
                parser_.reset();
                read_status_.size = parser_.size();
                read_status_.pos = 0;
                read_status_.wait = util::buffers::buffers_size(buffers);
                if (read_status_.size > read_status_.wait) {
                    ec = boost::asio::error::no_buffer_space;
                    return 0;
                }
            }
            while (true) {
                size_t bytes_read = read_some(
                    util::buffers::sub_buffers(buffers, read_status_.pos, read_status_.size - read_status_.pos), 
                    ec);
                read_status_.pos += bytes_read;
                if (read_status_.pos == read_status_.size) {
                    if (!parser_.ok()) {
                        parser_.parse(boost::asio::buffer(*buffers.begin(), read_status_.pos));
                        read_status_.size = parser_.size();
                        if (read_status_.size > read_status_.wait) {
                            ec = boost::asio::error::no_buffer_space;
                            return 0;
                        }
                    } else if (parser_.msg_def()->cls == MessageDefine::control_message) {
                        boost::mutex::scoped_lock lc(mutex_);
                        util::buffers::buffers_copy(rcv_buf_.prepare(read_status_.size), buffers);
                        rcv_buf_.commit(read_status_.size);
                        pend_rcv_sizes_.push_back(read_status_.size);
                        cond_.notify_all();
                        if (!read_status_.resp.empty()) {
                            read_status_.resp(ec, read_status_.pos);
                        }
                        read_status_.size = 0;
                        ec = boost::asio::error::would_block;
                        return 0;
                    } else {
                        read_status_.size = 0;
                        return read_status_.pos;
                    }
                } else if (ec) {
                    return 0;
                }
            }
        }

        template <
            typename MutableBufferSequence, 
            typename Handler
        >
        void MessageSocket::async_read_raw_msg(
            MutableBufferSequence const & buffers, 
            Handler const & handler)
        {
            assert(read_parallel_);
            if (!pend_rcv_sizes_.empty()) {
                // control messages should be handle before data messages
                response(handler, boost::asio::error::would_block, 0);
                return;
            }
            if (pend_data_sizes_.size() > 0) {
                size_t size = pend_data_sizes_.front();
                pend_data_sizes_.pop_front();
                assert(size <= rcv_data_.size());
                util::buffers::buffers_copy(buffers, rcv_data_.data(size));
                rcv_data_.consume(size);
                response(handler, boost::system::error_code(), size);
                return;
            }
            assert(read_status_.size == 0);
            parser_.reset();
            read_status_.size = parser_.size();
            read_status_.pos = 0;
            read_status_.wait = util::buffers::buffers_size(buffers);
            if (read_status_.size > read_status_.wait) {
                response(handler, boost::asio::error::no_buffer_space, 0);
                return;
            }

            async_read_some(
                util::buffers::sub_buffers(buffers, read_status_.pos, read_status_.left()), 
                detail::raw_msg_read_handler<MutableBufferSequence, Handler>(*this, buffers, handler));
        }

        template <
            typename MutableBufferSequence, 
            typename Handler
        >
        void MessageSocket::handle_read_raw_msg(
            MutableBufferSequence const & buffers, 
            Handler const & handler, 
            boost::system::error_code ec, 
            size_t bytes_transferred)
        {
            read_status_.pos += bytes_transferred;
            if (read_status_.pos == read_status_.size) {
                if (!parser_.ok()) {
                    parser_.parse(boost::asio::buffer(*buffers.begin(), read_status_.pos));
                    read_status_.size = parser_.size();
                    if (read_status_.size > read_status_.wait) {
                        handler(boost::asio::error::no_buffer_space, 0);
                        return;
                    }
                } else if (parser_.msg_def()->cls == MessageDefine::control_message) {
                    boost::mutex::scoped_lock lc(mutex_);
                    util::buffers::buffers_copy(rcv_buf_.prepare(read_status_.size), buffers);
                    rcv_buf_.commit(read_status_.size);
                    pend_rcv_sizes_.push_back(read_status_.size);
                    cond_.notify_all();
                    if (!read_status_.resp.empty()) {
                        read_status_.resp(ec, read_status_.pos);
                    }
                    read_status_.size = 0;
                    lc.unlock();
                    handler(boost::asio::error::would_block, 0);
                    return;
                } else {
                    read_status_.size = 0;
                    handler(ec, read_status_.pos);
                    return;
                }
            } else if (ec) {
                handler(ec, 0);
                return;
            }
            async_read_some(
                util::buffers::sub_buffers(buffers, read_status_.pos, read_status_.left()), 
                detail::raw_msg_read_handler<MutableBufferSequence, Handler>(*this, buffers, handler));
        }

        template <
            typename ConstBufferSequence
        >
        size_t MessageSocket::write_raw_msg(
            ConstBufferSequence const & buffers, 
            boost::system::error_code & ec)
        {
            assert(write_parallel_);
            if (write_status_.size == 0) {
                while (snd_buf_.size()) {
                    size_t bytes_write = write_some(snd_buf_.data(), ec);
                    boost::mutex::scoped_lock lc(mutex_);
                    snd_buf_.consume(bytes_write);
                    if (snd_buf_.size() == 0) {
                        cond_.notify_all();
                        if (!write_status_.resp.empty()) {
                            write_status_.resp(ec, read_status_.pos);
                        }
                        break;
                    } else if (ec) {
                        return 0;
                    }
                }
                write_status_.size = util::buffers::buffers_size(buffers);
                write_status_.pos = 0;
            }
            while (true) {
                size_t bytes_write = write_some(
                    util::buffers::sub_buffers(buffers, write_status_.pos, write_status_.left()), 
                    ec);
                write_status_.pos += bytes_write;
                if (write_status_.pos == write_status_.size) {
                    write_status_.size = 0;
                    return write_status_.pos;
                } else if (ec) {
                    return 0;
                }
            }
        }

        template <
            typename ConstBufferSequence, 
            typename Handler
        >
        void MessageSocket::async_write_raw_msg(
            ConstBufferSequence const & buffers, 
            Handler const & handler)
        {
            assert(write_parallel_);
            if (snd_buf_.size()) {
                async_write_some(
                    snd_buf_.data(), 
                    detail::raw_msg_write_handler<ConstBufferSequence, Handler>(*this, buffers, handler));
                return;
            }
            assert(write_status_.size == 0);
            write_status_.size = util::buffers::buffers_size(buffers);
            write_status_.pos = 0;
            async_write_some(
                util::buffers::sub_buffers(buffers, write_status_.pos, write_status_.left()), 
                detail::raw_msg_write_handler<ConstBufferSequence, Handler>(*this, buffers, handler));
        }

        template <
            typename ConstBufferSequence, 
            typename Handler
        >
        void MessageSocket::handle_write_raw_msg(
            ConstBufferSequence const & buffers, 
            Handler const & handler, 
            boost::system::error_code ec, 
            size_t bytes_transferred)
        {
            if (write_status_.size == 0) {
                boost::mutex::scoped_lock lc(mutex_);
                snd_buf_.consume(bytes_transferred);
                bytes_transferred = 0;
                if (snd_buf_.size() == 0) {
                    cond_.notify_all();
                    if (!write_status_.resp.empty()) {
                        write_status_.resp(ec, read_status_.pos);
                    }
                    write_status_.size = util::buffers::buffers_size(buffers);
                    write_status_.pos = 0;
                } else if (ec) {
                    lc.unlock();
                    handler(ec, 0);
                    return;
                } else {
                    async_write_some(
                        snd_buf_.data(), 
                        detail::raw_msg_write_handler<ConstBufferSequence, Handler>(*this, buffers, handler));
                    return;
                }
            }
            write_status_.pos += bytes_transferred;
            if (write_status_.pos == write_status_.size) {
                write_status_.size = 0;
                handler(ec, read_status_.pos);
                return;
            } else if (ec) {
                handler(ec, 0);
                return;
            }
            async_write_some(
                util::buffers::sub_buffers(buffers, read_status_.pos, read_status_.left()), 
                detail::raw_msg_write_handler<ConstBufferSequence, Handler>(*this, buffers, handler));
        }

        namespace detail
        {

            template <
                typename Message, 
                typename Handler
            >
            struct msg_read_handler
            {
                msg_read_handler(
                    MessageSocket & socket, 
                    Message & msg, 
                    Handler handler)
                    : socket_(socket)
                    , msg_(msg)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code ec, 
                    size_t bytes_transferred)
                {
                    socket_.handle_read_msg(msg_, handler_, ec, bytes_transferred);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(msg_read_handler, handler_)

            private:
                MessageSocket & socket_;
                Message & msg_;
                Handler handler_;
            };

            template <
                typename Message, 
                typename Handler
            >
            struct msg_write_handler
            {
                msg_write_handler(
                    MessageSocket & socket, 
                    Message const & msg, 
                    Handler handler)
                    : socket_(socket)
                    , msg_(msg)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code ec, 
                    size_t bytes_transferred)
                {
                    socket_.handle_write_msg(msg_, handler_, ec, bytes_transferred);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(msg_write_handler, handler_)

            private:
                MessageSocket & socket_;
                Message const & msg_;
                Handler handler_;
            };

        } // namespace detail

        template <
            typename Message
        >
        size_t MessageSocket::read_msg(
            Message & msg, 
            boost::system::error_code & ec)
        {
            if (read_parallel_) {
                boost::mutex::scoped_lock lc(mutex_);
                if (rcv_buf_.size()) {
                    size_t size = pend_rcv_sizes_.front();
                    pend_rcv_sizes_.pop_front();
                    assert(size <= rcv_buf_.size());
                    size_t left = rcv_buf_.size() - size;
                    rcv_buf_.pubseekoff(-(std::streamoff)left, std::ios::cur, std::ios::out);
                    msg.from_data(rcv_buf_, ctx_);
                    rcv_buf_.pubseekoff((std::streamoff)left, std::ios::cur, std::ios::out);
                    assert(left == rcv_buf_.size());
                    ec.clear();
                    return size;
                } else {
                    ec = boost::asio::error::would_block;
                    return 0;
                }
            }
            if (read_status_.size == 0) {
                parser_.reset();
                read_status_.size = parser_.size();
                read_status_.pos = 0;
            }
            while (true) {
                size_t bytes_read = read_some(
                    rcv_buf_.prepare(read_status_.left()), 
                    ec);
                read_status_.pos += bytes_read;
                rcv_buf_.commit(bytes_read);
                if (read_status_.pos == read_status_.size) {
                    if (!parser_.ok()) {
                        parser_.parse(rcv_buf_.data());
                        read_status_.size = parser_.size();
                    } else if (parser_.msg_def()->cls != MessageDefine::control_message) {
                        // save this message
                        util::buffers::buffers_copy(rcv_data_.prepare(read_status_.size), rcv_buf_.data());
                        rcv_data_.commit(read_status_.size);
                        rcv_buf_.consume(read_status_.size);
                        pend_data_sizes_.push_back(read_status_.size);
                        parser_.reset();
                        read_status_.size = parser_.size();
                        read_status_.pos = 0;
                    } else {
                        msg.from_data(rcv_buf_, ctx_);
                        assert(rcv_buf_.size() == 0);
                        read_status_.size = 0;
                        break;
                    }
                } else if (ec) {
                    break;
                }
            }
            return ec ? 0 : read_status_.pos;
        }

        template <
            typename Message, 
            typename Handler
        >
        void MessageSocket::async_read_msg(
            Message & msg, 
            Handler const & handler)
        {
            if (read_parallel_) {
                boost::mutex::scoped_lock lc(mutex_);
                if (rcv_buf_.size()) {
                    size_t size = pend_rcv_sizes_.front();
                    pend_rcv_sizes_.pop_front();
                    assert(size <= rcv_buf_.size());
                    size_t left = rcv_buf_.size() - size;
                    rcv_buf_.pubseekoff(-(std::streamoff)left, std::ios::cur, std::ios::out);
                    msg.from_data(rcv_buf_, ctx_);
                    rcv_buf_.pubseekoff((std::streamoff)left, std::ios::cur, std::ios::out);
                    assert(left == rcv_buf_.size());
                    response(handler, boost::system::error_code(), size);
                } else {
                    read_status_.resp = detail::msg_read_handler<Message, Handler>(*this, msg, handler);
                }
                return;
            }
            assert(read_status_.size == 0);
            parser_.reset();
            read_status_.size = parser_.size();
            read_status_.pos = 0;
            async_read_some(
                rcv_buf_.prepare(read_status_.left()), 
                detail::msg_read_handler<Message, Handler>(*this, msg, handler));
        }

        template <
            typename Message, 
            typename Handler
        >
        void MessageSocket::handle_read_msg(
            Message & msg, 
            Handler const & handler, 
            boost::system::error_code ec, 
            size_t bytes_transferred)
        {
            if (read_parallel_) {
                if (rcv_buf_.size() == 0) {
                    response(handler, ec, 0);
                } else {
                    assert(rcv_buf_.size() == bytes_transferred);
                    msg.from_data(rcv_buf_, ctx_);
                    assert(rcv_buf_.size() == 0);
                    response(handler, ec, bytes_transferred);
                }
                return;
            }
            read_status_.pos += bytes_transferred;
            rcv_buf_.commit(bytes_transferred);
            if (read_status_.pos == read_status_.size) {
                if (!parser_.ok()) {
                    parser_.parse(rcv_buf_.data());
                    read_status_.size = parser_.size();
                } else if (parser_.msg_def()->cls != MessageDefine::control_message) {
                    // save this message
                    util::buffers::buffers_copy(rcv_data_.prepare(read_status_.size), rcv_buf_.data());
                    rcv_data_.commit(read_status_.size);
                    rcv_buf_.consume(read_status_.size);
                    pend_data_sizes_.push_back(read_status_.size);
                    parser_.reset();
                    read_status_.size = parser_.size();
                    read_status_.pos = 0;
                } else {
                    msg.from_data(rcv_buf_, ctx_);
                    assert(rcv_buf_.size() == 0);
                    read_status_.size = 0;
                    handler(ec, read_status_.pos);
                    return;
                }
            } else if (ec) {
                handler(ec, 0);
                return;
            }
            async_read_some(
                rcv_buf_.prepare(read_status_.left()), 
                detail::msg_read_handler<Message, Handler>(*this, msg, handler));
        }

        template <
            typename Message
        >
        size_t MessageSocket::write_msg(
            Message const & msg, 
            boost::system::error_code & ec)
        {
            if (write_parallel_) {
                boost::mutex::scoped_lock lc(mutex_);
                if (write_status_.wait) {
                    if (snd_buf_.size() == 0) {
                        size_t size = write_status_.wait;
                        write_status_.wait = 0;
                        ec.clear();
                        return size;
                    } else {
                        ec = boost::asio::error::would_block;
                        return 0;
                    }
                } else {
                    assert(snd_buf_.size() == 0);
                    msg.to_data(snd_buf_, ctx_);
                    ec = boost::asio::error::would_block;
                    return 0;
                }
            }
            if (write_status_.size == 0) {
                assert(snd_buf_.size() == 0);
                msg.to_data(snd_buf_, ctx_);
                write_status_.size = snd_buf_.size();
                write_status_.pos = 0;
            }
            while (true) {
                size_t bytes_write = write_some(
                    snd_buf_.data(), 
                    ec);
                write_status_.pos += bytes_write;
                snd_buf_.consume(bytes_write);
                if (write_status_.pos == write_status_.size) {
                    assert(snd_buf_.size() == 0);
                    write_status_.size = 0;
                    return write_status_.pos;
                } else if (ec) {
                    return 0;
                }
            }
        }

        template <
            typename Message, 
            typename Handler
        >
        void MessageSocket::async_write_msg(
            Message const & msg, 
            Handler const & handler)
        {
            if (write_parallel_) {
                assert(write_status_.wait == 0);
                assert(snd_buf_.size() == 0);
                boost::mutex::scoped_lock lc(mutex_);
                msg.to_data(snd_buf_, ctx_);
                write_status_.wait = snd_buf_.size();
                write_status_.resp = detail::msg_write_handler<Message, Handler>(*this, msg, handler);
                return;
            }
            assert(write_status_.size == 0);
            assert(snd_buf_.size() == 0);
            msg.to_data(snd_buf_, ctx_);
            write_status_.size = snd_buf_.size();
            write_status_.pos = 0;
            async_write_some(
                snd_buf_.data(), 
                detail::msg_write_handler<Message, Handler>(*this, msg, handler));
        }

        template <
            typename Message, 
            typename Handler
        >
        void MessageSocket::handle_write_msg(
            Message const & msg, 
            Handler const & handler, 
            boost::system::error_code ec, 
            size_t bytes_transferred)
        {
            if (write_parallel_) {
                if (snd_buf_.size() == 0) {
                    assert(write_status_.wait == bytes_transferred);
                    write_status_.wait = 0;
                    response(handler, ec, bytes_transferred);
                } else {
                    response(handler, ec, 0);
                }
                return;
            }
            write_status_.pos += bytes_transferred;
            snd_buf_.consume(bytes_transferred);
            if (write_status_.pos == write_status_.size) {
                write_status_.size = 0;
                handler(ec, write_status_.pos);
                return;
            } else if (ec) {
                handler(ec, 0);
                return;
            }
            async_write_some(
                snd_buf_.data(), 
                detail::msg_write_handler<Message, Handler>(*this, msg, handler));
        }

        template <
            typename Handler
        >
        void MessageSocket::response(
        Handler const & handler, 
        boost::system::error_code ec, 
        size_t bytes_transferred)
        {
            get_io_service().post(
                boost::asio::detail::bind_handler(handler, ec, bytes_transferred));
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_SOCKET_HPP_
