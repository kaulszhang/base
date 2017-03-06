// StreamTransfer.h

#ifndef _UTIL_STREAM_STREAM_TRANSFER_H_
#define _UTIL_STREAM_STREAM_TRANSFER_H_

#include "util/stream/detail/transfer_buffers.h"

#include <framework/network/AsioHandlerHelper.h>

#include <boost/mpl/if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/non_type.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>

namespace util
{
    namespace stream
    {

        typedef std::pair<std::size_t, std::size_t> transfer_size;

        namespace detail
        {
            class transfer_all_t
            {
            public:
                typedef bool result_type;

                template <typename Error>
                bool operator()(
                    bool is_read, 
                    const Error& err, 
                    transfer_size const & bytes_transferred)
                {
                    return !!err;
                }
            };

            class transfer_at_least_t
            {
            public:
                typedef bool result_type;

                explicit transfer_at_least_t(std::size_t minimum)
                    : minimum_(minimum)
                {
                }

                template <typename Error>
                bool operator()(bool is_read, const Error& err, transfer_size const & bytes_transferred)
                {
                    if (is_read)
                        return !!err || bytes_transferred.first >= minimum_;
                    else
                        return !!err || bytes_transferred.second >= minimum_;
                }

            private:
                std::size_t minimum_;
            };
        }

        inline detail::transfer_all_t transfer_all()
        {
            return detail::transfer_all_t();
        }

        inline detail::transfer_at_least_t transfer_at_least(std::size_t minimum)
        {
            return detail::transfer_at_least_t(minimum);
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename MutableBufferSequence, 
            typename CompletionCondition
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            MutableBufferSequence const & buffers, 
            CompletionCondition completion_condition, 
            boost::system::error_code & ec, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            detail::transfer_buffers<boost::asio::const_buffer, 
                boost::asio::mutable_buffer, MutableBufferSequence> tmp(buffers);
            size_t total_buffer_size = tmp.total_buffer_size();
            if (high_level > total_buffer_size)
                high_level = total_buffer_size;
            transfer_size total_transferred(0, 0);
            bool read_end = false;
            while (!read_end) {
                size_t level = total_transferred.first - total_transferred.second;
                if (level < low_level) {
                    // 没达到低水平线，继续输入
                    std::size_t bytes_transferred = r.read_some(tmp.write_buffers(), ec);
                    total_transferred.first += bytes_transferred;
                    if (completion_condition(true, ec, total_transferred))
                        read_end = true;
                } else if (level >= high_level) {
                    // 达到高水平线，继续输出
                    std::size_t bytes_transferred = w.write_some(tmp.read_buffers(), ec);
                    total_transferred.second += bytes_transferred;
                    if (completion_condition(false, ec, total_transferred))
                        return total_transferred;
                } else {
                    // 介于两者之间，应该是抢先，现在的实现是输入优先
                    std::size_t bytes_transferred = r.read_some(tmp.write_buffers(), ec);
                    total_transferred.first += bytes_transferred;
                    if (completion_condition(true, ec, total_transferred))
                        read_end = true;
                }
            }
            while (total_transferred.second < total_transferred.first) {
                std::size_t bytes_transferred = w.write_some(tmp.read_buffers(), ec);
                total_transferred.second += bytes_transferred;
                if (completion_condition(false, ec, total_transferred))
                    return total_transferred;
            }
            ec = boost::system::error_code();
            return total_transferred;
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename MutableBufferSequence, 
            typename CompletionCondition
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            MutableBufferSequence const & buffers, 
            CompletionCondition completion_condition, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            boost::system::error_code ec;
            transfer_size bytes_transferred = 
                transfer(r, w, buffers, completion_condition, ec, low_level, high_level);
            boost::asio::detail::throw_error(ec);
            return bytes_transferred;
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename MutableBufferSequence
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            MutableBufferSequence const & buffers, 
            boost::system::error_code & ec, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            return transfer(r, w, buffers, transfer_all(), ec, low_level, high_level);
        }

        template <
            typename SyncReadStream, 
            typename SyncWriteStream, 
            typename MutableBufferSequence
        >
        transfer_size transfer(
            SyncReadStream & r, 
            SyncWriteStream & w, 
            MutableBufferSequence const & buffers, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            return transfer(r, w, buffers, transfer_all(), low_level, high_level);
        }

        namespace detail
        {

            template <
                typename AsyncReadStream, 
                typename AsyncWriteStream, 
                typename MutableBufferSequence,
                typename CompletionCondition, 
                typename TransferHandler
            >
            class transfer_handler
            {
            public:
                typedef util::stream::detail::transfer_buffers<
                    boost::asio::const_buffer, boost::asio::mutable_buffer, MutableBufferSequence> buffers_type;

                typedef boost::intrusive_ptr<transfer_handler> transfer_handler_ptr;

                transfer_handler(
                    AsyncReadStream & read_stream, 
                    AsyncWriteStream & write_stream, 
                    buffers_type const & buffers,
                    CompletionCondition completion_condition, 
                    TransferHandler handler, 
                    size_t low_level, 
                    size_t high_level)
                    : read_stream_(read_stream)
                    , write_stream_(write_stream)
                    , buffers_(buffers)
                    , completion_condition_(completion_condition)
                    , handler_(handler)
                    , strand_(read_stream.get_io_service())
                    , low_level_(low_level)
                    , high_level_(high_level)
                    , total_transferred_(0, 0)
                    , read_end_(false)
                    , reading_(false)
                    , write_end_(false)
                    , writing_(false)
                    , ref_count_(0)
                {
                    size_t total_buffer_size = buffers_.total_buffer_size();
                    if (high_level_ > total_buffer_size)
                        high_level_ = total_buffer_size;
                }

                friend void intrusive_ptr_add_ref(transfer_handler * p)
                {
                    ++p->ref_count_;
                }

                friend void intrusive_ptr_release(transfer_handler * p)
                {
                    if (--p->ref_count_ == 0) {
                        delete p;
                    }
                }

                size_t level() const
                {
                    return total_transferred_.first - total_transferred_.second;
                }

                struct read_handler_t
                {
                    read_handler_t(
                        transfer_handler & handler)
                        : handler_(&handler)
                    {
                    }

                    void operator()(
                        boost::system::error_code const & ec, 
                        std::size_t bytes_transferred)
                    {
                        handler_->handler_read(ec, bytes_transferred);
                    }

                    PASS_DOWN_ASIO_HANDLER_FUNCTION(read_handler_t, handler_->handler_)

                private:
                    transfer_handler_ptr handler_;
                };

                struct write_handler_t
                {
                    write_handler_t(
                        transfer_handler & handler)
                        : handler_(&handler)
                    {
                    }

                    void operator()(
                        boost::system::error_code const & ec, 
                        std::size_t bytes_transferred)
                    {
                        handler_->handler_write(ec, bytes_transferred);
                    }

                    PASS_DOWN_ASIO_HANDLER_FUNCTION(write_handler_t, handler_->handler_)

                private:
                    transfer_handler_ptr handler_;
                };

                void start()
                {
                    reading_ = true;
                    read_stream_.async_read_some(buffers_.write_buffers(), get_read_handler());
                }

                void handler_read(
                    const boost::system::error_code & ec,
                    std::size_t bytes_transferred)
                {
                    total_transferred_.first += bytes_transferred;
                    buffers_.commit(bytes_transferred);
                    if (completion_condition_(true, ec, total_transferred_)) {
                        read_end_ = true;
                        reading_ = false;
                    } else if (level() < high_level_ && !write_end_) {
                        reading_ = true;
                        read_stream_.async_read_some(buffers_.write_buffers(), get_read_handler());
                    } else {
                        reading_ = false;
                    }
                    if (!writing_) {
                        handler_write(boost::system::error_code(), 0);
                    }
                }

                void handler_write(
                    const boost::system::error_code & ec,
                    std::size_t bytes_transferred)
                {
                    if (write_end_)
                        return;
                    total_transferred_.second += bytes_transferred;
                    buffers_.consume(bytes_transferred);
                    if (completion_condition_(false, ec, total_transferred_)) {
                        writing_ = false;
                        write_end_ = true;
                        handler_(ec, std::pair<const std::size_t, const std::size_t>(total_transferred_));
                    } else if (level() > low_level_ || (read_end_ && level() > 0)) {
                        writing_ = true;
                        write_stream_.async_write_some(buffers_.read_buffers(), get_write_handler());
                    } else if (read_end_) {
                        writing_ = false;
                        write_end_ = true;
                        handler_(ec, std::pair<const std::size_t, const std::size_t>(total_transferred_));
                    } else {
                        writing_ = false;
                    }
                    if (!reading_ && !read_end_) {
                        handler_read(boost::system::error_code(), 0);
                    }
                }

                boost::asio::detail::wrapped_handler<boost::asio::strand, read_handler_t, boost::asio::detail::is_continuation_if_running> get_read_handler()
                {
                    return strand_.wrap(read_handler_t(*this));
                }

                boost::asio::detail::wrapped_handler<boost::asio::strand, write_handler_t, boost::asio::detail::is_continuation_if_running> get_write_handler()
                {
                    return strand_.wrap(write_handler_t(*this));
                }

               //private:
                AsyncReadStream & read_stream_;
                AsyncWriteStream & write_stream_;
                buffers_type buffers_;
                CompletionCondition completion_condition_;
                TransferHandler handler_;
                boost::asio::strand strand_;
                size_t low_level_;
                size_t high_level_;
                transfer_size total_transferred_;
                bool read_end_;
                bool reading_;
                bool write_end_;
                bool writing_;
                size_t ref_count_;
            };

        } // namespace detail

        template <
            typename AsyncReadStream, 
            typename AsyncWriteStream, 
            typename MutableBufferSequence, 
            typename CompletionCondition, 
            typename TransferHandler
        >
        void async_transfer(
            AsyncReadStream & r, 
            AsyncWriteStream & w, 
            MutableBufferSequence const & buffers, 
            CompletionCondition completion_condition, 
            TransferHandler handler, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            detail::transfer_buffers<boost::asio::const_buffer, 
                boost::asio::mutable_buffer, MutableBufferSequence> tmp(buffers);
            typedef detail::transfer_handler<AsyncReadStream, AsyncWriteStream, 
                MutableBufferSequence, CompletionCondition, TransferHandler> transfer_handler_t;
            typename transfer_handler_t::transfer_handler_ptr ptr(new transfer_handler_t(
                r, w, tmp, completion_condition, handler, low_level, high_level));
            ptr->start();
        }

        template <
            typename AsyncReadStream, 
            typename AsyncWriteStream, 
            typename MutableBufferSequence, 
            typename TransferHandler
        >
        void async_transfer(
            AsyncReadStream & r, 
            AsyncWriteStream & w, 
            MutableBufferSequence const & buffers, 
            TransferHandler handler, 
            size_t low_level = 0, 
            size_t high_level = size_t(-1))
        {
            return async_transfer(r, w, buffers, transfer_all(), handler, low_level, high_level);
        }

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_STREAM_TRANSFER_H_
