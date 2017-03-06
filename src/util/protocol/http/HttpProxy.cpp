// HttpProxy.cpp

#include "util/Util.h"
#include "util/protocol/http/HttpProxy.h"
#include "util/protocol/http/HttpError.h"

#include <framework/logger/Logger.h>
#include <framework/logger/LoggerFormatRecord.h>
#include <framework/logger/LoggerSection.h>
#include <framework/system/LogicError.h>
#include <framework/string/Url.h>
#include <framework/string/Format.h>
using namespace framework::logger;
using namespace framework::network;
using namespace framework::string;
using namespace framework::system::logic_error;

#include <boost/bind.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/completion_condition.hpp>
using namespace boost::system;

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("HttpProxy", 2);

        std::string const HttpProxy::state_str[] = {
            "stopped", 
            "receiving_request_head", 
            "preparing", 
            "connectting", 
            "sending_request_head", 
            "transferring_request_data", 
            "local_processing", 
            "receiving_response_head", 
            "sending_response_head", 
            "transferring_response_data", 
            "exiting", 
        };

        std::string HttpProxy::Size::to_string() const
        {
            switch (type_) {
                case 0:
                    return "none";
                case 1:
                    return get_bool() ? "true" : "false";
                case 2:
                    return format(s1_);
                case 3:
                    return "{" + format(s1_) + ", " + format(s2_) + "}";
                default:
                    return "ERROR";
            }
        }

        HttpProxy::HttpProxy(
            boost::asio::io_service & io_svc)
            : state_(stopped)
            , watch_state_(watch_stopped)
            , http_to_client_(io_svc)
            , http_to_server_(NULL)
        {
            static size_t gid = 0;
            id_ = gid++;
        }

        HttpProxy::~HttpProxy()
        {
            close();
            if (http_to_server_) {
                delete http_to_server_;
                http_to_server_ = NULL;
            }
        }

        void HttpProxy::start()
        {
            handle_async(error_code(), Size());
        }

        void HttpProxy::close()
        {
            boost::system::error_code ec;
            http_to_client_.close(ec);
            if (http_to_server_) {
                http_to_server_->close(ec);
            }
        }

        void HttpProxy::cancel()
        {
            if (http_to_server_)
                http_to_server_->cancel();
            http_to_client_.cancel();
        }

        error_code HttpProxy::cancel(
            error_code & ec)
        {
            if (http_to_server_)
                http_to_server_->cancel(ec);
            http_to_client_.cancel(ec);
            return ec;
        }
        
        /*
        Local:
            receive_request_head
            prepare
            transfer_request_data
            local_process
            receive_response_head
            send_response_head
            transfer_response_data

        Proxy:
            receive_request_head
            prepare
            connect
            send_request_head
            transfer_request_data
            local_process
            receive_response_head
            send_response_head
            transfer_response_data
        */

        void HttpProxy::handle_async(
            boost::system::error_code const & ec, 
            Size const & bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_async] (id = %u, status = %s, ec = %s, bytes_transferred = %s)" 
                %id_ % state_str[state_] % ec.message() % bytes_transferred.to_string());

            if (watch_state_ == broken) {
                on_finish();
                delete this;
                return;
            }

            if (ec) {
                if (state_ == receiving_request_head) {
                    error_code ec1;
                    response_.clear_data();
                    bool block = !http_to_client_.get_non_block(ec1);
                    if (block)
                        http_to_client_.set_non_block(true, ec1);
                    boost::asio::read(http_to_client_, response_.data(), boost::asio::transfer_at_least(4096), ec1);
                    if (block)
                        http_to_client_.set_non_block(false, ec1);
                    if (response_.data().size() > 4096) {
                        LOG_HEX(Logger::kLevelDebug, 
                            boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), 4096);
                        LOG_STR(Logger::kLevelDebug, (format(response_.data().size() - 4096) + " bytes remain").c_str());
                    } else {
                        LOG_HEX(Logger::kLevelDebug, 
                            boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), response_.data().size());
                    }
                }
                on_error(ec);
                switch (state_) {
                    case receiving_request_head:
                    case transferring_request_data:
                    case sending_response_head:
                    case transferring_response_data:
                        on_finish();
                        state_ = exiting;
                        if (watch_state_ == watching) {
                            error_code ec1;
                            http_to_client_.cancel(ec1);
                        } else {
                            delete this;
                        }
                        break;
                    case local_processing:
                        if (is_local() && !response_.head().content_length.is_initialized() && bytes_transferred.is_size_t()) {
                            response_.head().content_length.reset(bytes_transferred.get_size_t());
                        }
                    case preparing:
                    case connectting:
                    case sending_request_head:
                    case receiving_response_head:
                        state_ = sending_response_head;
                        response_error(ec, boost::bind(&HttpProxy::handle_async, this, _1, _2));
                        break;
                    default:
                        assert(0);
                        break;
                }
                //if (watch_state_ != watching) {
                //    delete this;
                //}
                return;
            }

            switch (state_) {
                case stopped:
                    state_ = receiving_request_head;
                    response_.head() = HttpResponseHead();
                    http_to_client_.async_read(request_.head(), 
                        boost::bind(&HttpProxy::handle_async, this, _1, _2));
                    break;
                case receiving_request_head:
                    state_ = preparing;
                    if (watch_state_ == watch_stopped 
                        && request_.head().content_length.get_value_or(0) == 0) {
                            watch_state_ = watching;
                            http_to_client_.async_read_some(boost::asio::null_buffers(), 
                                boost::bind(&HttpProxy::handle_watch, this, _1));
                    }
                    on_receive_request_head(
                        request_.head(), 
                        boost::bind(&HttpProxy::handle_async, this, _1, _2));
                    break;
                case preparing:
                    if (bytes_transferred.get_bool()) {
                        if (!http_to_server_)
                            http_to_server_ = new HttpSocket(http_to_client_.get_io_service());
                        state_ = connectting;
                        http_to_server_->async_connect(request_.head().host.get(), 
                            boost::bind(&HttpProxy::handle_async, this, _1, Size()));
                    } else {
                        state_ = transferring_request_data;
                        transfer_request_data(
                            boost::bind(&HttpProxy::handle_async, this, _1, _2));
                    }
                    break;
                case connectting:
                    state_ = sending_request_head;
                    http_to_server_->async_write(request_.head(), 
                        boost::bind(&HttpProxy::handle_async, this, _1, _2));
                    break;
                case sending_request_head:
                    state_ = transferring_request_data;
                    transfer_request_data(
                        boost::bind(&HttpProxy::handle_async, this, _1, _2));
                    break;
                case transferring_request_data:
                    if (is_local()) {
                        on_receive_request_data(transfer_buf_);
                        transfer_buf_.consume(transfer_buf_.size());
                    }
                    state_ = local_processing;
                    if (watch_state_ == watch_stopped) {
                        watch_state_ = watching;
                        http_to_client_.async_read_some(boost::asio::null_buffers(), 
                            boost::bind(&HttpProxy::handle_watch, this, _1));
                    }
                    response_.head().connection = request_.head().connection;
                    local_process(
                        boost::bind(&HttpProxy::handle_async, this, _1, _2));
                    break;
                case local_processing:
                    if (is_local()) {
                        state_ = receiving_response_head;
                        on_receive_response_head(response_.head());
                        if (!response_.head().content_length.is_initialized()) {
                            if (bytes_transferred.is_size_t())
                                response_.head().content_length.reset(bytes_transferred.get_size_t());
                            else
                                response_.head().connection.reset(http_field::Connection());
                        }
                        if (!response_.head().connection.is_initialized()) {
                            response_.head().connection.reset(http_field::Connection());
                        }
                        handle_async(ec, Size());
                    } else {
                        state_ = receiving_response_head;
                        http_to_server_->async_read(response_.head(), 
                            boost::bind(&HttpProxy::handle_async, this, _1, _2));
                    }
                    break;
                case receiving_response_head:
                    if (!is_local()) {
                        on_receive_response_head(response_.head());
                        if (!response_.head().connection.is_initialized()) {
                            response_.head().connection.reset(http_field::Connection());
                        }
                        if (response_.head().err_msg.empty())
                            response_.head().err_msg = "OK";
                    }
                    state_ = sending_response_head;
                    http_to_client_.async_write(response_.head(), 
                        boost::bind(&HttpProxy::handle_async, this, _1, _2));
                    break;
                case sending_response_head:
                    state_ = transferring_response_data;
                    transfer_response_data(
                        boost::bind(&HttpProxy::handle_async, this, _1, _2));
                    break;
                case transferring_response_data:
                    on_finish();
                    if (!response_.head().connection
                        || response_.head().connection.get() == http_field::Connection::close) {
                            state_ = exiting;
                            if (watch_state_ != watching) {
                                delete this;
                            } else {
                                error_code ec;
                                http_to_client_.shutdown(boost::asio::socket_base::shutdown_send, ec);
                                http_to_client_.cancel(ec);
                            }
                    } else {
                        state_ = stopped;
                        if (watch_state_ != watching) {
                            // restart
                            watch_state_ = watch_stopped;
                            handle_async(ec, Size());
                        } else {
                            error_code ec;
                            http_to_client_.cancel(ec);
                        }
                    }
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        void HttpProxy::handle_watch(
            boost::system::error_code const & ec)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_watch] (id = %u, status = %s, ec = %s)" 
                %id_ % state_str[state_] % ec.message());

            if (state_ == exiting) {
                delete this;
            } else if (state_ == stopped) {
                // restart
                watch_state_ = watch_stopped;
                start();
            } else if (ec 
                && ec != boost::asio::error::eof
                && ec != boost::asio::error::operation_aborted) {
                    watch_state_ = broken;
                    on_broken_pipe();
            } else {
                watch_state_ = watch_stopped;
            }
        }

        void HttpProxy::local_process(
            response_type const & resp)
        {
            resp(boost::system::error_code(), Size());
        }

        void HttpProxy::on_broken_pipe()
        {
            error_code ec;
            on_error(ec);
            cancel(ec);
        }

        void HttpProxy::response_error(
            error_code const & ec, 
            response_type const & resp)
        {
            HttpResponseHead & head = response_.head();
            error_code ec1;
            if (ec.category() == http_error::get_category()) {
                ec1 = ec;
            } else if (ec.category() == boost::asio::error::get_system_category()
                || ec.category() == boost::asio::error::get_netdb_category()
                || ec.category() == boost::asio::error::get_addrinfo_category()
                || ec.category() == boost::asio::error::get_misc_category()) {
                ec1 = http_error::service_unavailable;
            } else {
                ec1 = http_error::internal_server_error;
            }
            head.err_code = ec1.value();
            head.err_msg = ec1.message();
            if (!head.content_length.is_initialized())
                head.content_length.reset(0);
            response_.head().connection = http_field::Connection::close;
            http_to_client_.async_write(response_.head(), 
                boost::bind(resp, _1, _2));
        }

    } // namespace protocol
} // namespace util
