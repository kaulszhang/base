// HttpClient.cpp

#include "util/Util.h"
#include "util/stream/StreamTransfer.h"
#include "util/stream/StlStream.h"
#include "util/protocol/http/HttpClient.h"
#include "util/protocol/http/HttpError.h"
#include "util/protocol/http/HttpRequest.h"
#include "util/protocol/http/HttpResponse.h"
using namespace util::stream;
using namespace util::protocol::http_error;

#include <framework/logger/Logger.h>
#include <framework/system/LogicError.h>
#include <framework/logger/LoggerFormatRecord.h>
#include <framework/logger/LoggerSection.h>
#include <framework/string/Url.h>
#include <framework/network/NetName.h>
using namespace framework::logger;
using namespace framework::string;
using namespace framework::network;
using namespace framework::system::logic_error;

#include <boost/bind.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
using namespace boost::system;
using namespace boost::asio;

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE("HttpClient");

        static char const SERVICE_NAME[] = "http";

        std::string const HttpClient::con_status_str[] = {
            "closed", 
            "connectting", 
            "established", 
            "ready", 
            "broken", 
        };

        std::string const HttpClient::req_status_str[] = {
            "send_pending", 
            "sending_req_head", 
            "sending_req_data", 
            "recv_pending", 
            "recving_resp_head", 
            "opened", 
            "recving_resp_data", 
            "finished", 
        };

        framework::network::NetName util::protocol::HttpClient::g_proxy_addr_;

        void HttpClient::set_http_proxy(std::string const & host)
        {
            //g_proxy_addr_
            if(host.empty())
                g_proxy_addr_.host("");
            else
            {
                g_proxy_addr_.svc("80");
                g_proxy_addr_.from_string(host);
            }
        }

        HttpClient::HttpClient(
            boost::asio::io_service & io_svc)
            : HttpSocketEx(io_svc)
            , status_(closed)
            , is_keep_alive_(false)
            , req_id_(0)
            , num_sent_(0)
        {

            static size_t gid = 0;
            id_ = gid++;
            if(!g_proxy_addr_.host().empty())
                addr_ = g_proxy_addr_;
        }

        HttpClient::~HttpClient()
        {
            error_code ec;
            if (status_ >= established) {
                status_ = broken;
                broken_error_ = boost::asio::error::operation_aborted;
            }
            while (handle_next(ec))
                resume(false, ec);

            HttpSocketEx::close(ec);
            status_ = closed;
            broken_error_.clear();
        }

        error_code HttpClient::bind_host(
            std::string const & host, 
            error_code & ec)
        {
            return bind_host(NetName(host), ec);
        }

        error_code HttpClient::bind_host(
            std::string const & host, 
            std::string const & svc, 
            error_code & ec)
        {
            return bind_host(NetName(host, svc), ec);
        }

        error_code HttpClient::bind_host(
            NetName const & addr, 
            error_code & ec)
        {
            ec.clear();
            if (addr == addr_) {
                return ec;
            }
            if (status_ != closed) {
                return ec = already_open;
            }
            if (addr.host().empty()) {
                return ec;
            }
            
            //if host == localhost not proxy
            if(!addr_.host().empty() && addr.host() != "127.0.0.1")
                return ec;

            addr_ = addr;
            ec.clear();
            return ec;
        }

        error_code HttpClient::open(
            HttpRequest const & request, 
            error_code & ec)
        {
            LOG_F(Logger::kLevelDebug, "[open] (id = %u, url = %s)" 
                % id_ % request.head().path.c_str());

            post_reqeust(request, false, ec);
            if (!ec) {
                resume(requests_.size() > 1, ec);
            }
            return ec;
        }

        error_code HttpClient::reopen(
            error_code & ec)
        {
            assert(requests_.size() == 1);
            HttpRequest request = requests_[0];
            close(ec);
            return open(request, ec);
        }

        bool HttpClient::is_open(
            error_code & ec)
        {
            if (status_ == ready) {
                ec = error_code();
                return true;
            }
            resume(false, ec);
            return !ec;
        }

        void HttpClient::async_open(
            HttpRequest const & request, 
            response_type const & resp)
        {
            LOG_F(Logger::kLevelDebug, "[async_open] (id = %u, url = %s)" 
                % id_ % request.head().path);

            error_code ec;
            post_reqeust(request, false, resp, ec);
            if (ec) {
                get_io_service().post(
                    boost::bind(resp, ec));
                return;
            }
            async_resume();
        }

        void HttpClient::async_reopen(
            response_type const & resp)
        {
            assert(requests_.size() == 1);
            HttpRequest request = requests_[0];
            error_code ec;
            close(ec);
            return async_open(request, resp);
        }

        error_code HttpClient::fetch(
            HttpRequest const & request, 
            boost::system::error_code & ec)
        {
            LOG_F(Logger::kLevelDebug, "[fetch] (id = %u, url = %s)" 
                % id_ % request.head().path.c_str());

            post_reqeust(request, true, ec);
            if (!ec) {
                resume(requests_.size() > 1, ec);
            }
            return ec;
        }

        error_code HttpClient::refetch(
            error_code & ec)
        {
            assert(requests_.size() == 1);
            HttpRequest request = requests_[0];
            close(ec);
            return fetch(request, ec);
        }

        bool HttpClient::is_fetch(
            error_code & ec)
        {
            resume(false, ec);
            return !ec;
        }

        void HttpClient::async_fetch(
            HttpRequest const & request, 
            response_type const & resp)
        {
            LOG_F(Logger::kLevelDebug, "[async_fetch] (id = %u, url = %s)" 
                % id_ % request.head().path);

            error_code ec;
            post_reqeust(request, true, resp, ec);
            if (ec) {
                get_io_service().post(
                    boost::bind(resp, ec));
                return;
            }
            async_resume();
        }

        void HttpClient::async_refetch(
            response_type const & resp)
        {
            assert(requests_.size() == 1);
            HttpRequest request = requests_[0];
            error_code ec;
            close(ec);
            return async_fetch(request, resp);
        }

        void HttpClient::close()
        {
            error_code ec;
            if (handle_next(ec))
                return;
            HttpSocketEx::close();
            status_ = closed;
            broken_error_.clear();
            dump("close", ec);
        }

        error_code HttpClient::close(
            error_code & ec)
        {
            if (handle_next(ec))
                return ec;
            HttpSocketEx::close(ec);
            status_ = closed;
            broken_error_.clear();
            dump("close", ec);
            return ec;
        }

        error_code HttpClient::post_reqeust(
            HttpRequest const & request, 
            bool is_fetch, 
            response_type const & resp, 
            error_code & ec)
        {
            if (requests_.size() == 1 && requests_[0].is_fetch && requests_[0].status == finished) {
                // 兼容老版本，fetch不需要close的机制，但是只用于非串行请求的情形
                assert(status_ == closed || status_ == broken);
                status_ = closed;
                broken_error_.clear();
                requests_.clear();
            } else if (status_ == broken) {
                ec = broken_error_;
                return ec;
            } else if (requests_.empty()) {
                is_keep_alive_ = (http_field::Connection::keep_alive 
                    == request.head().connection.get_value_or(http_field::Connection::close));
            } else {
                if (is_keep_alive_) {
                    is_keep_alive_ &= (http_field::Connection::keep_alive 
                        == request.head().connection.get_value_or(http_field::Connection::close));
                } else {
                    if (http_field::Connection::keep_alive 
                        == request.head().connection.get_value_or(http_field::Connection::close)) {
                            ec = keepalive_error;
                    } else {
                            ec = busy_work;
                    }
                    return ec;
                }
            }
            ec.clear();
            if (resp.empty()) {
                requests_.push_back(Request(req_id_++, request, is_fetch));
            } else {
                requests_.push_back(Request(req_id_++, request, is_fetch, resp));
            }
            Request & request2 = requests_.back();
            if (request2.data().size())
                request2.head().content_length.reset(request2.data().size());
            if (status_ == ready)
                status_ = established;
            if (status_ == established) {
                request2.stat.resolve_time = 0;
                request2.stat.connect_time = 0;
            }
            return ec;
        }

        error_code HttpClient::resume(
            bool pending, 
            error_code & ec)
        {
            if (requests_.empty()) {
                return ec = not_open;
            }
            ec = broken_error_;
            if (status_ == closed || status_ == connectting) {
                resume_connect(ec);
            }
            if (status_ >= established) {
                if (!pending && requests_[0].status != opened && 
                    requests_[0].status != finished) {
                        resume_request(false, ec);
                }
                if (num_sent_ > 0 && num_sent_ < requests_.size()) {
                    error_code ec1 = broken_error_;
                    if (requests_[num_sent_].status < recv_pending)
                        resume_request(true, ec1);
                    if (pending) {
                        ec = ec1 ? ec1 : boost::asio::error::would_block;
                    }
                }
                // 优化is_open
                if (status_ == established && num_sent_ == requests_.size() 
                    && requests_[0].status >= recving_resp_data) {
                        status_ = ready;
                }
            }
            return ec;
        }

        error_code HttpClient::resume_connect(
            error_code & ec)
        {
            ec.clear();

            switch (status_) {
                case closed:
                    dump("resume_connect1", ec);
                    status_ = connectting;
                    if (!addr_.host().empty()) {
                        connect(addr_, ec);
                    } else if (requests_[0].head().host.is_initialized()) {
                        NetName addr("", 0, requests_[0].head().protocol.get());
                        addr.from_string(requests_[0].head().host.get());
                        connect(addr, ec);
                    } else {
                        ec = not_bind;
                    }
                    if (ec) {
                        if (ec == boost::asio::error::in_progress) {
                            ec = boost::asio::error::would_block;
                        }
                        // 如果失败，最后会把状态设置成broken，这里都设置为connectting
                    } else {
                        dump("resume_connect", ec);
                        status_ = established;
                    }
                    break;
                case connectting:
                    {
                        NetName addr("", 0, requests_[0].head().protocol.get());
                        if(addr_.host().empty())
                            addr.from_string(requests_[0].head().host.get());	
                        else 
                            addr = addr_;
                        connect(addr, ec);
                        dump("resume_connect3", ec);
                        if (ec) {
                            if (ec == boost::asio::error::in_progress) {
                                ec = boost::asio::error::would_block;
                            }
                        } else {
                            dump("resume_connect", ec);
                            status_ = established;
                        }
                    }
                    break;
                default:
                    assert(0);
            }

            if (ec && ec != boost::asio::error::would_block) {
                dump("resume_connect2", ec);
                status_ = broken;
                broken_error_ = ec;
            }

            return ec;
        }

        error_code HttpClient::resume_request(
            bool pending, 
            error_code & ec)
        {
            Request & request = requests_[pending ? num_sent_ : 0];
            Statistics & stat = request.stat;

            if (ec) {
                if (!pending)
                    post_handle_request(request, ec);
                return ec;
            }

            switch (request.status) {
                case send_pending:
                    dump_request(request, "resume_request", ec);
                    stat.send_pend_time = stat.elapse();
                    request.status = sending_req_head;
                case sending_req_head:
                    write(request.head(), ec);
                    dump_request(request, "resume_request", ec);
                    if (ec) {
                        break;
                    }
                    stat.request_head_time = stat.elapse();
                    request.status = sending_req_data;
                case sending_req_data:
                    if (request.data().size()) {
                        boost::asio::write(*this, request.data(), boost::asio::transfer_all(), ec);
                        dump_request(request, "resume_request", ec);
                        if (ec) {
                            break;
                        }
                    }
                    stat.request_data_time = stat.elapse();
                    ++num_sent_;
                    request.status = recv_pending;
                    if (pending) {
                        break;
                    }
                case recv_pending:
                    dump_request(request, "resume_request", ec);
                    stat.recv_pend_time = stat.elapse();
                    request.status = recving_resp_head;
                case recving_resp_head:
                    read(response_.head(), ec);
                    dump_request(request, "resume_request", ec);
                    if (ec) {
                        if (ec != boost::asio::error::would_block) {
                            // 临时用上response_.data()，把已有的数据拿过来看看
                            error_code ec1;
                            response_.clear_data();
                            bool block = !get_non_block(ec1);
                            if (block)
                                set_non_block(true, ec1);
                            boost::asio::read(*this, response_.data(), boost::asio::transfer_at_least(4096), ec1);
                            if (block)
                                set_non_block(false, ec1);
                            if (response_.data().size() > 4096) {
                                LOG_HEX(Logger::kLevelDebug, 
                                    boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), 4096);
                                LOG_STR(Logger::kLevelDebug, (format(response_.data().size() - 4096) + " bytes remain").c_str());
                            } else {
                                LOG_HEX(Logger::kLevelDebug, 
                                    boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), response_.data().size());
                            }
                        }
                        break;
                    }

                    {
                        util::protocol::HttpResponseHead head = response_.head();
                        std::ostringstream oss;
                        head.get_content(oss);
                        LOG_STR(framework::logger::Logger::kLevelDebug, oss.str().c_str());
                    }

                    stat.response_head_time = stat.elapse();
                    if (handle_redirect(request, ec)) {
                        close_socket(ec);
                        status_ = closed;
                        request.status = send_pending;
                        return resume(false, ec);
                    }
                    if (request.is_fetch) {
                        request.status = recving_resp_data;
                        response_.clear_data();
                    } else {
                        request.status = opened;
                        break;
                    }
                case recving_resp_data:
                    if (request.is_fetch) {
                        if (response_.head().content_length.is_initialized()) {
                            boost::uint64_t content_length = 
                                response_.head().content_length.get() - response_.data().size();
                            if (content_length > 0) {
                                content_length = boost::asio::read(
                                    *this, response_.data().prepare(content_length), 
                                    boost::asio::transfer_at_least(content_length), ec);
                                response_.data().commit(content_length);
                                dump_request(request, "resume_request", ec);
                            }
                            if (ec) {
                                break;
                            }
                        } else {
                            boost::asio::read(*this, response_.data(), 
                                boost::asio::transfer_all(), ec);
                            dump_request(request, "resume_request", ec);
                            if (ec == boost::asio::error::eof) {
                                ec = error_code();
                            } else {
                                break;
                            }
                        }
                        stat.response_data_time = stat.elapse();
                        request.status = finished;
                    } else {
                        ec = already_open;
                        break;
                    }
                default:
                    break;
            }

            if (!pending && ec != boost::asio::error::would_block) {
                post_handle_request(request, ec);
            }

            return ec;
        }

        void HttpClient::async_resume()
        {
            if (requests_.empty()) {
                return;
            }
            if (status_ == closed) {
                handle_async_connect(error_code());
                return;
            }
            if (status_ >= established) {
                if (requests_[0].status == send_pending || requests_[0].status == recv_pending)
                    handle_async_reqeust(false, broken_error_);
                if (num_sent_ > 0 && num_sent_ < requests_.size() 
                    && requests_[num_sent_].status == send_pending) {
                        handle_async_reqeust(true, broken_error_);
                }
            }
        }

        void HttpClient::handle_async_connect(
            error_code const & ec)
        {
            LOG_SECTION();

            dump("handle_async_connect", ec);

            if (ec) {
                status_ = broken;
                broken_error_ = ec;
                async_resume();
                return;
            }

            switch (status_) {
                case closed:
                    status_ = connectting;
                    if (!addr_.host().empty()) {
                        async_connect(addr_, 
                            boost::bind(&HttpClient::handle_async_connect, this, _1));
                    } else if (requests_[0].head().host.is_initialized()) {
                        NetName addr("", 0, requests_[0].head().protocol.get());
                        addr.from_string(requests_[0].head().host.get());
                        async_connect(addr,
                            boost::bind(&HttpClient::handle_async_connect, this, _1));
                    } else {
                        error_code ec1 = not_bind;
                        status_ = broken;
                        broken_error_ = ec1;
                        handle_async_reqeust(false, ec1);
                    }
                    break;
                case connectting:
                    status_ = established;
                    async_resume();
                    break;
                case established:
                case ready:
                case broken:
                    assert(0);
                    break;
            }
        }

        template <typename Stream>
        static error_code const & commit_stream(
            Stream & stream, 
            error_code const & ec, 
            size_t bytes_transferred)
        {
            stream.commit(bytes_transferred);
            return ec;
        }

        void HttpClient::handle_async_reqeust(
            bool pending, 
            error_code const & ec)
        {
            LOG_SECTION();

            Request & request = requests_[pending ? num_sent_ : 0];
            Statistics & stat = request.stat;

            dump_request(request, "handle_async_reqeust", ec);

            if (pending && num_sent_ == 0) {
                pending = false;
            }

            if (ec) {
                if (request.status == recving_resp_head) {
                    error_code ec1;
                    response_.clear_data();
                    bool block = !get_non_block(ec1);
                    if (block)
                        set_non_block(true, ec1);
                    boost::asio::read(*this, response_.data(), boost::asio::transfer_at_least(4096), ec1);
                    if (block)
                        set_non_block(false, ec1);
                    if (response_.data().size() > 4096) {
                        LOG_HEX(Logger::kLevelDebug, 
                            boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), 4096);
                        LOG_STR(Logger::kLevelDebug, (format(response_.data().size() - 4096) + " bytes remain").c_str());
                    } else {
                        LOG_HEX(Logger::kLevelDebug, 
                            boost::asio::buffer_cast<unsigned char const *>(response_.data().data()), response_.data().size());
                    }
                }
                if (!request.is_fetch 
                    || ec != boost::asio::error::eof
                    || request.status != recving_resp_data 
                    || response_.head().content_length.is_initialized()) {
                        if (!pending) {
                            error_code ec1 = ec;
                            post_handle_request(request, ec1);
                            response_request(request, ec1);
                        }
                        return;
                }
            }

            switch (request.status) {
                case send_pending:
                    stat.send_pend_time = stat.elapse();
                    request.status = sending_req_head;
                    async_write(request.head(), 
                        boost::bind(&HttpClient::handle_async_reqeust, this, pending, _1));
                    break;
                case sending_req_head:
                    stat.request_head_time = stat.elapse();
                    request.status = sending_req_data;
                    if (request.data().size()) {
                        boost::asio::async_write(*this, request.data(), 
                            boost::bind(&HttpClient::handle_async_reqeust, this, pending, _1));
                        break;
                    }
                case sending_req_data:
                    stat.request_data_time = stat.elapse();
                    if (++num_sent_ < requests_.size()) {
                        assert(requests_[num_sent_].status == send_pending);
                        if (requests_[num_sent_].is_async) {
                            handle_async_reqeust(true, error_code());
                        }
                    }
                    if (pending) {
                        request.status = recv_pending;
                        break;
                    }
                case recv_pending:
                    stat.recv_pend_time = stat.elapse();
                    request.status = recving_resp_head;
                    async_read(response_.head(), 
                        boost::bind(&HttpClient::handle_async_reqeust, this, pending, _1));
                    break;
                case recving_resp_head:
                    stat.response_head_time = stat.elapse();
                    {
                        error_code ec1;
                        if (handle_redirect(request, ec1)) {
                            error_code ec2;
                            close_socket(ec2);
                            status_ = closed;
                            request.status = send_pending;
                            return async_resume();
                        }
                    }
                    if (request.is_fetch) {
                        request.status = recving_resp_data;
                        response_.clear_data();
                        if (response_.head().content_length.is_initialized()) {
                            boost::uint64_t content_length = response_.head().content_length.get();
                            if (content_length > 0) {
                                boost::asio::async_read(*this, response_.data().prepare(content_length), 
                                    boost::asio::transfer_at_least(content_length), 
                                    boost::bind(&HttpClient::handle_async_reqeust, this, pending, boost::bind(commit_stream<boost::asio::streambuf>, boost::ref(response_.data()), _1, _2)));
                                break;
                            // } else { no break;
                            }
                        } else {
                            boost::asio::async_read(*this, response_.data(), 
                                boost::asio::transfer_all(), 
                                boost::bind(&HttpClient::handle_async_reqeust, this, pending, _1));
                            break;
                        }
                    } else {
                        request.status = opened;
                        error_code ec1;
                        post_handle_request(request, ec1);
                        response_request(request, ec1);
                        break;
                    }
                case recving_resp_data:
                    stat.response_data_time = stat.elapse();
                    assert(request.is_fetch);
                    if (request.is_fetch) {
                        request.status = finished;
                        error_code ec1;
                        post_handle_request(request, ec1);
                        response_request(request, ec1);
                    }
                    break;
                default:
                    break;
            }
        }

        bool HttpClient::handle_redirect(
            Request & request, 
            error_code & ec)
        {
            size_t const redirect_codes[] = {
                http_error::moved_permanently, 
                http_error::moved_temporarily, 
                http_error::see_other,
                http_error::temporary_redirect, 
            };
            size_t const * iter = std::find(redirect_codes, 
                redirect_codes + sizeof(redirect_codes) / sizeof(redirect_codes[0]), 
                response_.head().err_code);
            if (iter == redirect_codes + sizeof(redirect_codes) / sizeof(redirect_codes[0])) {
                return false;
            }
            if (requests_.size() > 1) {
                ec = redirect_error;
                return false;
            }
            if (!response_.head().location.is_initialized()) {
                ec = format_error;
                return false;
            }
            Url location(response_.head().location.get());
            printf("Url http 302 url:%s\n", response_.head().location.get().c_str());

            if (!location.is_valid()) {
                location.from_string("http://" + request.head().host.get() + response_.head().location.get());
            }

            if (!location.host().empty()) {
                request.head().protocol = location.protocol();
                request.head().host.reset(location.host_svc());

                //防止绑定127.0.0.1后,优先使用127.0.0.1的ip
                if(!g_proxy_addr_.host().empty())
                    addr_ = g_proxy_addr_;
                else 
                    addr_.host("");
            }
            if (!location.path().empty()) {
                request.head().path = location.path_all();
            }
            // 重定向之后不能keep alive
            request.head().connection = http_field::Connection::close;
            return true;
        }

        error_code HttpClient::read_finish(
            error_code & ec, 
            boost::uint64_t bytes_transferred)
        {
            Request & request = requests_[0];
            dump_request(request, "read_finish", ec);
            assert(request.status == opened);
            {
                error_code ec1;
                handle_next(ec1);
            }
            return ec;
        }

        void HttpClient::post_handle_request(
            Request & request, 
            error_code & ec)
        {
            dump_request(request, "post_handle_request", ec);

            if (ec) {
                Statistics stat = request.stat;
                stat.last_error = ec;
                switch (request.status) {
                    case send_pending:
                        stat.send_pend_time = stat.elapse();
                        break;
                    case sending_req_head:
                        stat.request_head_time = stat.elapse();
                        break;
                    case sending_req_data:
                        stat.request_data_time = stat.elapse();
                        break;
                    case recv_pending:
                        stat.recv_pend_time = stat.elapse();
                        break;
                    case recving_resp_head:
                        stat.response_head_time = stat.elapse();
                        break;
                    case opened:
                        break;
                    case recving_resp_data:
                        stat.response_data_time = stat.elapse();
                        break;
                    default:
                        assert(0);
                        break;
                }
                // 假装发生了请求，因为后面会统一减一次num_sent_
                if (request.status < recv_pending)
                    ++num_sent_;
                if (status_ != broken) {
                    status_ = broken;
                    close_socket(broken_error_);
                    broken_error_ = ec;
                    dump("post_handle_request", ec);
                }
                request.status = finished;
            }

            if (!ec && (response_.head().err_code < ok 
                || response_.head().err_code >= multiple_choices)) {
                    ec = http_error::errors(response_.head().err_code);
                    dump_request(request, "post_handle_request", ec);
            }

            if ((status_ == established || status_ == ready)
                && request.status >= opened 
                && response_.head().connection
                && response_.head().connection.get() == http_field::Connection::close) {
                    if (request.status > opened) {
                        close_socket(broken_error_);
                        status_ = broken;
                        broken_error_ = keepalive_error;
                        dump("post_handle_request", ec);
                    }
                    is_keep_alive_ = false;
            }
        }

        void HttpClient::response_request(
            Request & request, 
            error_code const & ec)
        {
            dump_request(request, "response_request", ec);

            response_type resp;
            resp.swap(request.resp);
            get_io_service().post(
                boost::bind(resp, ec));
        }

        bool HttpClient::handle_next(
            error_code & ec)
        {
            if (requests_.empty()) {
                return false;
            }
            dump_request(requests_[0], "handle_next", ec);
            if (requests_[0].status == opened) {
                requests_[0].stat.response_data_time = requests_[0].stat.elapse();
                requests_[0].status = finished;
                // close_socket if not keep_alive
                error_code ec1;
                post_handle_request(requests_[0], ec1);
            }
            if (requests_[0].status != finished) {
                error_code ec1 = boost::asio::error::operation_aborted;
                post_handle_request(requests_[0], ec1);
            }
            assert(requests_[0].status == finished);
            requests_.pop_front();
            --num_sent_;
            if (requests_.empty()) {
                return false;
            }
            Request & request = requests_.front();
            if (status_ == ready)
                status_ = established;
            if (request.is_async) {
                async_resume();
            }
            return true;
        }

        void HttpClient::dump(
            char const * function, 
            boost::system::error_code const & ec)
        {
            LOG_F(Logger::kLevelDebug1, "[%s] (id = %u, status = %s, ec = %s)" 
                % function % id_ % con_status_str[status_] % ec.message());
        }

        void HttpClient::dump_request(
            Request const & request, 
            char const * function, 
            boost::system::error_code const & ec)
        {
            LOG_F(Logger::kLevelDebug1, "[%s] (id = %u, req_id = %u, req_status = %s, ec = %s)" 
                % function % id_ % request.id % req_status_str[request.status] % ec.message());
        }

        void HttpClient::close_socket(
            error_code & ec)
        {
            HttpSocketEx::close(ec);
        }

    } // namespace protocol
} // namespace util
