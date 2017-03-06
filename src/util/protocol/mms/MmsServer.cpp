// MmsServer.cpp

#include "util/Util.h"
#include "util/stream/StreamTransfer.h"
#include "util/protocol/mms/MmsServer.h"
#include "util/protocol/mms/MmsError.h"
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
#include <boost/asio/buffer.hpp>
#include <boost/asio/completion_condition.hpp>
using namespace boost::system;

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("MmsServer", 2);

        static size_t const DATA_BUFFER_SIZE = 10240;

        MmsServer::MmsServer(
            boost::asio::io_service & io_svc)
            : MmsSocket(io_svc)
        {
            static size_t gid = 0;
            id_ = gid++;
        }

        MmsServer::~MmsServer()
        {
            close();
        }

        void MmsServer::start()
        {
            async_read(request_.head(), 
                boost::bind(&MmsServer::handle_receive_request_head, this, _1, _2));
        }

        void MmsServer::handle_receive_request_head(
            error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_receive_request_head] id =%u, ec = %s, bytes_transferred = %d" 
                % id_ % ec % bytes_transferred);

            LOG_S( Logger::kLevelDebug, "[request_head]" << std::hex << \
                "\n\tHeader-->msg_len = " << request_.head().msg_len << \
                "\n\tHeader-->chunk_count = " << request_.head().chunk_count << \
                "\n\tHeader-->seq = " << request_.head().seq << \
                "\n\tHeader-->trunk_len = " << request_.head().trunk_len << \
                "\n\tHeader-->mid = " << request_.head().mid );

            if (ec) {
                handle_error(ec);
                return;
            }

            size_t content_length = request_.head().content_length;
            if (content_length) {
                boost::asio::async_read(
                    (MmsSocket &)(*this), 
                    request_.data(), 
                    boost::asio::transfer_at_least(content_length), 
                    boost::bind(&MmsServer::handle_receive_request_data, this, _1, _2));
            } else {
                handle_receive_request_data(boost::system::error_code(), 0);
            }
        }

        void MmsServer::handle_receive_request_data(
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_receive_request_data] id =%u, ec = %s, bytes_transferred = %d" 
                % id_ % ec % bytes_transferred);

            LOG_HEX( Logger::kLevelDebug, ( const unsigned char * )boost::asio::detail::buffer_cast_helper( request_.data().data() ), request_.data().size() );

            if (ec) {
                handle_error(ec);
                return;
            }

            response_.head() = MmsResponseHead();
            local_process(
                boost::bind(&MmsServer::handle_local_process, this, _1));
        }

        void MmsServer::process_response(
            error_code const & ec,
            boost::asio::streambuf const & resp_buf)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_local_process] id =%u, ec = %s" % id_ % ec);

            if (ec) {
                response_error(ec);
                return;
            }

            boost::asio::async_write(
                    (MmsSocket &)(*this), 
                    resp_buf.data(), 
                    boost::bind(&MmsServer::handle_send_response_data, this, _1, _2));
        }

        void MmsServer::handle_local_process(
            error_code const & ec)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_local_process] id =%u, ec = %s" % id_ % ec);

            if (ec) {
                response_error(ec);
                return;
            }

            //async_write(response_.head(), 
            //    boost::bind(&MmsServer::handle_send_response_head, this, _1, _2));
        }

        void MmsServer::handle_send_response_head(
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
                    (MmsSocket &)(*this), 
                    response_.data(), 
                    boost::bind(&MmsServer::handle_send_response_data, this, _1, _2));
            } else {
                handle_send_response_data(boost::system::error_code(), 0);
            }
        }

        void MmsServer::handle_send_response_data(
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

        void MmsServer::handle_response_error(
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            LOG_SECTION();

            LOG_F(Logger::kLevelDebug, "[handle_response_error] id =%u, ec = %s, bytes_transferred = %d" 
                % id_ % ec % bytes_transferred);

            delete this;
        }

       void MmsServer::handle_error(
            error_code const & ec)
        {
            LOG_F(Logger::kLevelDebug, "[handle_error] id =%u, ec = %s" % id_ % ec);

            on_error(ec);
            delete this;
        }

        void MmsServer::response_error(
            error_code const & ec)
        {
            MmsResponseHead & head = response_.head();
            head = MmsResponseHead(); // clear
            if (ec.category() == mms_error::get_category()) {
                head.err_code = ec.value();
            } else if (ec.category() == boost::asio::error::get_system_category()
                || ec.category() == boost::asio::error::get_netdb_category()
                || ec.category() == boost::asio::error::get_addrinfo_category()
                || ec.category() == boost::asio::error::get_misc_category()) {
                response_.head().err_code = mms_error::service_unavailable;
            } else {
                head.err_code = mms_error::internal_server_error;
            }
            //head.err_msg = ec.message();
            //head.content_length.reset(0);
            on_error(ec);
            MmsSocket::async_write(response_.head(), 
                boost::bind(&MmsServer::handle_response_error, this, _1, _2));
        }

    } // namespace protocol
} // namespace util
