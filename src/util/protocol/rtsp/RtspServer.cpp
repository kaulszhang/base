// RtspServer.cpp

#include "util/Util.h"
#include "util/stream/StreamTransfer.h"
#include "util/protocol/rtsp/RtspServer.h"
#include "util/protocol/rtsp/RtspError.h"
using namespace util::stream;

#include <framework/logger/Logger.h>
#include <framework/logger/LoggerFormatRecord.h>
#include <framework/logger/LoggerSection.h>
#include <framework/system/LogicError.h>
#include <framework/string/Url.h>
using namespace framework::logger;
using namespace framework::network;
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

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("RtspServer", 2);

        static size_t const DATA_BUFFER_SIZE = 10240;

        RtspServer::RtspServer(
            boost::asio::io_service & io_svc)
            : RtspSocket(io_svc)
        {
            static size_t gid = 0;
            id_ = gid++;
        }

        RtspServer::~RtspServer()
        {
            close();
        }

        void RtspServer::start()
        {
            async_read(request_.head(), 
                boost::bind(&RtspServer::handle_receive_request_head, this, _1, _2));
        }

        void RtspServer::handle_receive_request_head(
            error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_receive_request_head] id =%u, ec = %s, bytes_transferred = %d" 
                % id_ % ec % bytes_transferred);

            if (ec) {
                handle_error(ec);
                return;
            }

            size_t content_length = request_.head().content_length.get_value_or(0);
            if (content_length) {
                boost::asio::async_read(
                    (RtspSocket &)(*this), 
                    request_.data(), 
                    boost::asio::transfer_at_least(content_length), 
                    boost::bind(&RtspServer::handle_receive_request_data, this, _1, _2));
            } else {
                handle_receive_request_data(boost::system::error_code(), 0);
            }
        }

        void RtspServer::handle_receive_request_data(
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_receive_request_data] id =%u, ec = %s, bytes_transferred = %d" 
                % id_ % ec % bytes_transferred);

            if (ec) {
                handle_error(ec);
                return;
            }

            response_.head() = RtspResponseHead();
            local_process(
                boost::bind(&RtspServer::handle_local_process, this, _1));
        }

        void RtspServer::handle_local_process(
            error_code const & ec)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_local_process] id =%u, ec = %s" % id_ % ec);

            if (ec) {
                response_error(ec);
                return;
            }

            response_.head().err_msg = "OK";
            response_.head()["CSeq"] = request_.head()["CSeq"];
            response_.head().content_length.reset(response_.data().size());

            async_write(response_.head(), 
                boost::bind(&RtspServer::handle_send_response_head, this, _1, _2));
        }

        void RtspServer::handle_send_response_head(
            error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_send_response_head] id =%u, ec = %s, bytes_transferred = %d" 
                % id_ % ec % bytes_transferred);

            if (ec) {
                handle_error(ec);
                return;
            }

            if (response_.data().size()) {
                boost::asio::async_write(
                    (RtspSocket &)(*this), 
                    response_.data(), 
                    boost::bind(&RtspServer::handle_send_response_data, this, _1, _2));
            } else {
                handle_send_response_data(boost::system::error_code(), 0);
            }
        }

        void RtspServer::handle_send_response_data(
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_send_response_data] id =%u, ec = %s, bytes_transferred = %d" 
                % id_ % ec % bytes_transferred);

            if (ec) {
                handle_error(ec);
                return;
            }

            on_finish();

            request_.clear_data();
            response_.clear_data();

            start();
        }

        void RtspServer::handle_response_error(
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_response_error] id =%u, ec = %s, bytes_transferred = %d" 
                % id_ % ec % bytes_transferred);

            delete this;
        }

       void RtspServer::handle_error(
            error_code const & ec)
        {
            LOG_F(Logger::kLevelDebug, "[handle_error] id =%u, ec = %s" % id_ % ec);

            on_error(ec);
            delete this;
        }

        void RtspServer::response_error(
            error_code const & ec)
        {
            RtspResponseHead & head = response_.head();
            head = RtspResponseHead(); // clear
            if (ec.category() == rtsp_error::get_category()) {
                head.err_code = ec.value();
            } else if (ec.category() == boost::asio::error::get_system_category()
                || ec.category() == boost::asio::error::get_netdb_category()
                || ec.category() == boost::asio::error::get_addrinfo_category()
                || ec.category() == boost::asio::error::get_misc_category()) {
                response_.head().err_code = rtsp_error::service_unavailable;
            } else {
                head.err_code = rtsp_error::internal_server_error;
            }
            head.err_msg = ec.message();
            head.content_length.reset(0);
            on_error(ec);
            RtspSocket::async_write(response_.head(), 
                boost::bind(&RtspServer::handle_response_error, this, _1, _2));
        }

    } // namespace protocol
} // namespace util
