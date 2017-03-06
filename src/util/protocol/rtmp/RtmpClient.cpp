// RtmpClient.cpp

#include "util/Util.h"

#include <boost/function.hpp>

#include "util/protocol/rtmp/RtmpClient.h"
#include "util/protocol/rtmp/RtmpSocket.hpp"
#include "util/protocol/rtmp/RtmpError.h"
#include "util/protocol/rtmp/RtmpMessage.hpp"
#include "util/protocol/rtmp/RtmpMessageDataCommand.h"
using namespace util::protocol::rtmp_error;

#include <framework/system/LogicError.h>
#include <framework/logger/Logger.h>
#include <framework/string/Url.h>
#include <framework/network/NetName.h>
using namespace framework::logger;
using namespace framework::string;
using namespace framework::network;
using namespace framework::system::logic_error;

#include <boost/bind.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read.hpp>
using namespace boost::system;

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE("util.protocol.RtmpClient");

        static char const SERVICE_NAME[] = "1935";

        std::string const RtmpClient::con_status_str[] = {
            "closed", 
            "connectting", 
            "net_connectting", 
            "established", 
            "broken", 
        };

        std::string const RtmpClient::req_status_str[] = {
            "finished", 
            "send_pending", 
            "sending_req", 
            "recving_resp", 
        };

        RtmpClient::RtmpClient(
            boost::asio::io_service & io_svc)
            : RtmpSocket(io_svc)
            , status_(closed)
            , request_status_(finished)
        {
            static size_t gid = 0;
            id_ = gid++;
            addr_.svc(SERVICE_NAME);
        }

        RtmpClient::~RtmpClient()
        {
            error_code ec;
            if (status_ >= established) {
                status_ = broken;
            }
            RtmpSocket::close(ec);
            status_ = closed;
        }

        boost::system::error_code RtmpClient::connect(
            framework::string::Url const & url, 
            boost::system::error_code & ec)
        {
            //LOG_DEBUG("[connect] (id = %u, url = %s)" 
            //    % id_ % url.to_string());

            if (requests_.empty()) {
                make_connect_requests(url);
                request_status_ = send_pending;
            }
            resume_connect(ec);
            return ec;
        }

        void RtmpClient::async_connect(
            framework::string::Url const & url, 
            response_type const & resp)
        {
            //LOG_DEBUG("[async_connect] (id = %u, url = %s)" 
            //    % id_ % url.to_string());

            resp_ = resp;
            make_connect_requests(url);

            error_code ec;
            handle_async_connect(ec);
        }

        void RtmpClient::close()
        {
            error_code ec;
            RtmpSocket::close();
            status_ = closed;
            dump("close", ec);
        }

        error_code RtmpClient::close(
            error_code & ec)
        {
            RtmpSocket::close(ec);
            status_ = closed;
            dump("close", ec);
            return ec;
        }

        boost::system::error_code RtmpClient::play(
            boost::system::error_code & ec)
        {
            //LOG_DEBUG("[play] (id = %u, url = %s)" % id_);

            if (requests_.empty()) {
                make_play_requests(content_);
                request_status_ = send_pending;
            }
            resume_request(ec);
            return ec;
        }

        void RtmpClient::async_play(
            response_type const & resp)
        {
            //LOG_DEBUG("[async_play] (id = %u, url = %s)" % id_);

            resp_ = resp;
            make_play_requests(content_);
            async_reqeust();
        }

        boost::system::error_code RtmpClient::publish(
            boost::system::error_code & ec)
        {
            //LOG_DEBUG("[publish] (id = %u, url = %s)" % id_);

            if (requests_.empty()) {
                make_publish_requests(content_);
                request_status_ = send_pending;
            }
            resume_request(ec);
            return ec;
        }

        void RtmpClient::async_publish(
            response_type const & resp)
        {
            //LOG_DEBUG("[async_publish] (id = %u, url = %s)" % id_);

            resp_ = resp;
            make_publish_requests(content_);
            async_reqeust();
        }

        void RtmpClient::async_reqeust()
        {
            request_status_ = send_pending;
            handle_async_reqeust(error_code());
        }

        void RtmpClient::make_connect_requests(
            framework::string::Url const & url)
        {
            url_ = url;
            std::string::size_type pos = url_.path().rfind('/');
            content_ = url_.path().substr(pos + 1);
            url_.path(url_.path().substr(0, pos));

            addr_.from_string(url_.host_svc());

            requests_.resize(1);

            {
                RtmpMessage & msg = requests_[0];
                msg.chunk = 3;
                RtmpCommandMessage0 & cmd = msg.get<RtmpCommandMessage0>();
                cmd.CommandName = "connect";
                cmd.TransactionID = 1;
                RtmpAmfObject & obj = 
                    cmd.CommandObject.get<RtmpAmfObject>();
                obj["app"] = url_.path().substr(1);
                obj["flashVer"] = "WIN 10,0,12,36";
                obj["swfUrl"]; // = UNDEFINED
                obj["tcUrl"] = url_.to_string();
                obj["fpad"] = RtmpAmfValue(RtmpAmfType::BOOL);
                obj["capabilities"] = 15;
                obj["audioCodecs"] = 3191;
                obj["videoCodecs"] = 252;
                obj["videoFunction"] = 1;
                obj["pageUrl"]; // = UNDEFINED
                obj["objectEncoding"] = (double)3;

                while (url_.param_begin() != url_.param_end()) {
                    obj[url_.param_begin()->key()] = url_.param_begin()->value();
                    url_.param(url_.param_begin()->key(), "");
                }

                obj["tcUrl"] = url_.to_string();
            }
        }

        void RtmpClient::make_play_requests(
            std::string const & content)
        {
            requests_.resize(2);

            {
                RtmpMessage & msg = requests_[0];
                msg.chunk = 3;
                RtmpCommandMessage0 & cmd = msg.get<RtmpCommandMessage0>();
                cmd.CommandName = "createStream";
                cmd.TransactionID = (double)0;
                cmd.CommandObject = RtmpAmfValue(RtmpAmfType::_NULL);
            }

            {
                RtmpMessage & msg = requests_[1];
                msg.chunk = 3;
                msg.stream = 1;
                RtmpCommandMessage0 & cmd = msg.get<RtmpCommandMessage0>();
                cmd.CommandName = "play";
                cmd.TransactionID = (double)0;
                cmd.CommandObject = RtmpAmfValue(RtmpAmfType::_NULL);
                cmd.OptionalArguments.push_back(content);
            }
        }

        void RtmpClient::make_publish_requests(
            std::string const & content)
        {
            requests_.resize(2);

            {
                RtmpMessage & msg = requests_[0];
                msg.chunk = 3;
                RtmpCommandMessage0 & cmd = msg.get<RtmpCommandMessage0>();
                cmd.CommandName = "createStream";
                cmd.TransactionID = (double)0;
                cmd.CommandObject = RtmpAmfValue(RtmpAmfType::_NULL);
            }

            {
                RtmpMessage & msg = requests_[1];
                msg.chunk = 3;
                msg.stream = 1;
                RtmpCommandMessage0 & cmd = msg.get<RtmpCommandMessage0>();
                cmd.CommandName = "publish";
                cmd.TransactionID = (double)0;
                cmd.CommandObject = RtmpAmfValue(RtmpAmfType::_NULL);
                cmd.OptionalArguments.push_back(content);
                cmd.OptionalArguments.push_back("live");
            }
        }

        void RtmpClient::resume_connect(
            boost::system::error_code & ec)
        {
            switch (status_) {
                case closed:
                    status_ = connectting;
                case connectting:
                    if (RtmpSocket::connect(addr_, ec))
                        break;
                    status_ = net_connectting;
                case net_connectting:
                    resume_request(ec);
                    if (ec)
                        break;
                    status_ = established;
                    break;
                case established:
                case broken:
                    assert(0);
                    break;
            }
        }

        void RtmpClient::resume_request(
            boost::system::error_code & ec)
        {
            while (true) {
                switch (request_status_) {
                case send_pending:
                    request_status_ = sending_req;
                case sending_req:
                    if (write_msg(requests_.front(), ec) == 0)
                        break;
                    request_status_ = recving_resp;
                case recving_resp:
                    if (read_msg(response_, ec) == 0)
                        break;
                    if (process_protocol_message(response_, proto_responses_)) {
                        request_status_ = sending_req;
                        if (write_msgs(proto_responses_, ec) == 0)
                            break;
                        request_status_ = recving_resp;
                        break;
                    }
                    if (!post_response(ec)) {
                        request_status_ = recving_resp;
                        break;
                    }
                    request_status_ = finished;
                    requests_.pop_front();
                    if (!requests_.empty()) {
                        request_status_ = send_pending;
                    }
                    break;
                default:
                    assert(false);
                    break;
                }
                if (ec || request_status_ == finished) {
                    break;
                }
            }
        }

        void RtmpClient::handle_async_connect(
            error_code const & ec)
        {
            LOG_SECTION();

            dump("handle_async_connect", ec);

            if (ec) {
                status_ = broken;
                response(ec);
                return;
            }

            switch (status_) {
                case closed:
                    status_ = connectting;
                    RtmpSocket::async_connect(addr_, 
                        boost::bind(&RtmpClient::handle_async_connect, this, _1));
                    break;
                case connectting:
                    status_ = net_connectting;
                    async_reqeust();
                    break;
                case net_connectting:
                    status_ = established;
                    response(ec);
                    break;
                case established:
                case broken:
                    assert(0);
                    break;
            }
        }

        void RtmpClient::handle_async_reqeust(
            error_code ec)
        {
            LOG_SECTION();

            dump_request("handle_async_reqeust", ec);

            if (ec) {
                if (request_status_ == recving_resp) {
                    error_code ec1;
                    boost::asio::streambuf buf;
                    bool block = !get_non_block(ec1);
                    if (block)
                        set_non_block(true, ec1);
                    boost::asio::read(*this, buf, boost::asio::transfer_at_least(4096), ec1);
                    if (block)
                        set_non_block(false, ec1);
                    //LOG_DATA(Debug, ("recving_resp_head", buf.data()));
                }
                status_ = broken;
                requests_.clear();
                response(ec);
                return;
            }

            switch (request_status_) {
                case send_pending:
                    request_status_ = sending_req;
                    async_write_msg(requests_.front(), 
                        boost::bind(&RtmpClient::handle_async_reqeust, this, _1));
                    break;
                case sending_req:
                    request_status_ = recving_resp;
                    async_read_msg(response_, 
                        boost::bind(&RtmpClient::handle_async_reqeust, this, _1));
                    break;
                case recving_resp:
                    proto_responses_.clear();
                    if (process_protocol_message(response_, proto_responses_)) {
                        request_status_ = sending_req;
                        async_write_msgs(proto_responses_, 
                            boost::bind(&RtmpClient::handle_async_reqeust, this, _1));
                        break;
                    }
                    if (!post_response(ec)) {
                        request_status_ = sending_req;
                        handle_async_reqeust(ec);
                        break;
                    }
                    request_status_ = finished;
                    requests_.pop_front();
                    if (requests_.empty()) {
                        response(ec);
                    } else {
                        request_status_ = send_pending;
                        handle_async_reqeust(ec);
                    }
                    break;
                default:
                    assert(false);
                    break;
            }
        }

        bool RtmpClient::post_response(
            boost::system::error_code & ec)
        {
            if (response_.type != RCMT_CommandMessage0 
                && response_.type != RCMT_CommandMessage3) {
                    return false;
            }
                RtmpCommandMessage const & cmd(
                    response_.type == RCMT_CommandMessage0 
                    ? (RtmpCommandMessage const &)response_.as<RtmpCommandMessage0>() 
                    : (RtmpCommandMessage const &)response_.as<RtmpCommandMessage3>());
                std::string const & cmd_name = cmd.CommandName.as<RtmpAmfString>().StringData;
                if (cmd_name == "_result") {
                    return true;
                } else if (cmd_name == "_error") {
                    ec = rtmp_error::format_error;
                    return true;
                } else {
                    RtmpMessage const & req_msg = requests_[0];
                    RtmpCommandMessage0 const & req_cmd = req_msg.as<RtmpCommandMessage0>();
                    std::string const & req_cmd_name = req_cmd.CommandName.as<RtmpAmfString>().StringData;
                    if (req_cmd_name == "play") {
                        if (cmd_name == "onStatus") {
                            RtmpAmfObject const & arg = cmd.OptionalArguments.front().as<RtmpAmfObject>();
                            if (arg["code"] == "NetStream.Play.Start") {
                                return true;
                            }
                        }
                    }
                    if (req_cmd_name == "publish") {
                        if (cmd_name == "onStatus") {
                            RtmpAmfObject const & arg = cmd.OptionalArguments.front().as<RtmpAmfObject>();
                            if (arg["code"] == "NetStream.Publish.Start") {
                                return true;
                            }
                        }
                    }
                }
            return false;
        }

        void RtmpClient::response(
            boost::system::error_code const & ec)
        {
            response_type tmp;
            tmp.swap(resp_);
            tmp(ec);
        }

        void RtmpClient::tick(
            boost::system::error_code & ec)
        {
            if (request_status_ != finished) {
                resume_request(ec);
                if (ec)
                    return;
            }
            if (proto_responses_.empty()) {
                RtmpSocket::tick(proto_responses_);
                if (read_msg(response_, ec)) {
                    process_protocol_message(response_, proto_responses_);
                    response_.reset();
                }
            }
            if (!proto_responses_.empty() && write_msgs(proto_responses_, ec) == 0) {
                return;
            }
            proto_responses_.clear();
        }

       void RtmpClient::dump(
            char const * function, 
            boost::system::error_code const & ec)
        {
            //LOG_TRACE("[%s] (id = %u, status = %s, ec = %s)" 
            //    % function % id_ % con_status_str[status_] % ec.message());
        }

       static std::string const & command_name(
           RtmpMessage const & msg)
       {
           return msg.as<RtmpCommandMessage0>().CommandName.as<RtmpAmfString>().StringData;
       }

        void RtmpClient::dump_request(
            char const * function, 
            boost::system::error_code const & ec)
        {
            //LOG_TRACE("[%s] (id = %u, req_name = %s, req_status = %s, ec = %s)" 
            //    % function % id_ % command_name(requests_.front()) % req_status_str[request_status_] % ec.message());
        }

        void RtmpClient::close_socket(
            error_code & ec)
        {
            RtmpSocket::close(ec);
        }

    } // namespace protocol
} // namespace util
