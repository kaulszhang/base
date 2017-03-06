// Http.h

#ifndef _UTIL_PROTOCOL_HTTP_ERROR_H_
#define _UTIL_PROTOCOL_HTTP_ERROR_H_

namespace util
{
    namespace protocol
    {

        namespace http_error {

            enum errors
            {
                already_bind        = 1,    //   1  http已经绑定
                not_bind,                   //   2  http已经绑定
                already_open,               //   3  http已经打开连接
                not_open,                   //   4  http连接未打开
                busy_work,                  //   5  http有任务正在进行
                format_error,               //   6  http头格式错误
                redirect_error,             //   7  http不能重定向
                keepalive_error,             //  7  http不能复用连接

                continue_                       = 100,   // Continue
                witching_protocols              = 101,   // witching Protocols

                ok                              = 200,   // OK
                created                         = 201,   // Created
                accepted                        = 202,   // Accepted
                non_authoritative_information   = 203,   // Non-Authoritative Information
                no_content                      = 204,   // No Content
                reset_content                   = 205,   // Reset Content
                partial_content                 = 206,   // Partial Content

                multiple_choices                = 300,   // Multiple Choices
                moved_permanently               = 301,   // Moved Permanently
                moved_temporarily               = 302,   // Moved Temporarily
                see_other                       = 303,   // See Other
                not_modified                    = 304,   // Not Modified
                use_proxy                       = 305,   // Use Proxy
                temporary_redirect              = 307,   // Temporary Redirect

                bad_request                     = 400,   // Bad Request
                unauthorized                    = 401,   // Unauthorized
                payment_required                = 402,   // Payment Required
                forbidden                       = 403,   // Forbidden
                not_found                       = 404,   // Not Found
                method_not_allowed              = 405,   // Method Not Allowed
                not_acceptable                  = 406,   // Not Acceptable
                proxy_authentication_required   = 407,   // Proxy Authentication Required
                request_time_out                = 408,   // Request Time-out
                conflict                        = 409,   // Conflict
                gone                            = 410,   // Gone
                length_required                 = 411,   // Length Required
                precondition_failed             = 412,   // Precondition Failed
                request_entity_too_large        = 413,   // Request Entity Too Large
                request_uri_too_large           = 414,   // Request-URI Too Large
                unsupported_media_type          = 415,   // Unsupported Media Type
                requested_range_not_satisfiable = 416,   // Requested range not satisfiable
                expectation_failed              = 417,   // Expectation Failed

                internal_server_error           = 500,   // Internal Server Error
                not_implemented                 = 501,   // Not Implemented
                bad_gateway                     = 502,   // Bad Gateway
                service_unavailable             = 503,   // Service Unavailable
                gateway_time_out                = 504,   // Gateway Time-out
                http_version_not_supported      = 505,   // HTTP Version not supported
            };

            namespace detail {

                class http_category
                    : public boost::system::error_category
                {
                public:
                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "http";
                    }

                    std::string message(int value) const
                    {
                        switch (value) {
                        case http_error::already_bind:
                            return "HTTP server has already binded";
                        case http_error::not_bind:
                            return "HTTP server has not binded";
                        case http_error::already_open:
                            return "HTTP session has already opened";
                        case http_error::not_open:
                            return "HTTP session has not opened";
                        case http_error::busy_work:
                            return "HTTP client is busy working";
                        case http_error::format_error:
                            return "HTTP packet format error";
                        case http_error::redirect_error:
                            return "HTTP redirect error";
                        case http_error::keepalive_error:
                            return "HTTP keepalive error";

                        case http_error::continue_:
                            return "Http: Continue";
                        case http_error::witching_protocols:
                            return "Http: witching Protocols";
                        case http_error::ok:
                            return "Http: OK";
                        case http_error::created:
                            return "Http: Created";
                        case http_error::accepted:
                            return "Http: Accepted";
                        case http_error::non_authoritative_information:
                            return "Http: Non-Authoritative Information";
                        case http_error::no_content:
                            return "Http: No Content";
                        case http_error::reset_content:
                            return "Http: Reset Content";
                        case http_error::partial_content:
                            return "Http: Partial Content";
                        case http_error::multiple_choices:
                            return "Http: Multiple Choices";
                        case http_error::moved_permanently:
                            return "Http: Moved Permanently";
                        case http_error::moved_temporarily:
                            return "Http: Moved Temporarily";
                        case http_error::see_other:
                            return "Http: See Other";
                        case http_error::not_modified:
                            return "Http: Not Modified";
                        case http_error::use_proxy:
                            return "Http: Use Proxy";
                        case http_error::temporary_redirect:
                            return "Http: Temporary Redirect";
                        case http_error::bad_request:
                            return "Http: Bad Request";
                        case http_error::unauthorized:
                            return "Http: Unauthorized";
                        case http_error::payment_required:
                            return "Http: Payment Required";
                        case http_error::forbidden:
                            return "Http: Forbidden";
                        case http_error::not_found:
                            return "Http: Not Found";
                        case http_error::method_not_allowed:
                            return "Http: Method Not Allowed";
                        case http_error::not_acceptable:
                            return "Http: Not Acceptable";
                        case http_error::proxy_authentication_required:
                            return "Http: Proxy Authentication Required";
                        case http_error::request_time_out:
                            return "Http: Request Time-out";
                        case http_error::conflict:
                            return "Http: Conflict";
                        case http_error::gone:
                            return "Http: Gone";
                        case http_error::length_required:
                            return "Http: Length Required";
                        case http_error::precondition_failed:
                            return "Http: Precondition Failed";
                        case http_error::request_entity_too_large:
                            return "Http: Request Entity Too Large";
                        case http_error::request_uri_too_large:
                            return "Http: Request-URI Too Large";
                        case http_error::unsupported_media_type:
                            return "Http: Unsupported Media Type";
                        case http_error::requested_range_not_satisfiable:
                            return "Http: Requested range not satisfiable";
                        case http_error::expectation_failed:
                            return "Http: Expectation Failed";
                        case http_error::internal_server_error:
                            return "Http: Internal Server Error";
                        case http_error::not_implemented:
                            return "Http: Not Implemented";
                        case http_error::bad_gateway:
                            return "Http: Bad Gateway";
                        case http_error::service_unavailable:
                            return "Http: Service Unavailable";
                        case http_error::gateway_time_out:
                            return "Http: Gateway Time-out";
                        case http_error::http_version_not_supported:
                            return "Http: HTTP Version not supported";
                        default:
                            return "http error";
                        }
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::http_category instance;
                return instance;
            }

            static boost::system::error_category const & http_category = get_category();

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

        } // namespace http_error

    } // namespace protocol
} // namespace util

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<util::protocol::http_error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using util::protocol::http_error::make_error_code;
#endif

    }
}

#endif // _UTIL_PROTOCOL_HTTP_ERROR_H_
