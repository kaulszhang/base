// MmsError.h

#ifndef _UTIL_PROTOCOL_MMS_ERROR_H_
#define _UTIL_PROTOCOL_MMS_ERROR_H_

#include "boost/system/error_code.hpp"

namespace util
{
    namespace protocol
    {

        namespace mms_error {

            enum errors
            {
                succeed             = 0,
                already_bind        = 1,    //   1  mms已经绑定
                not_bind,                   //   2  mms已经绑定
                already_open,               //   3  mms已经打开连接
                not_open,                   //   4  mms连接未打开
                busy_work,                  //   5  mms有任务正在进行
                format_error,               //   6  mms头格式错误
                redirect_error,             //   7  mms不能重定向
                keepalive_error,             //  7  mms不能复用连接
                action_error,               //  播放动作错误

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
                mms_version_not_supported      = 505,   // MMS Version not supported

                ///
                not_acceptable_packet,
            };

            namespace detail {

                class mms_category
                    : public boost::system::error_category
                {
                public:
                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "mms";
                    }

                    std::string message(int value) const
                    {
                        switch (value) {
                        case mms_error::already_bind:
                            return "MMS server has already binded";
                        case mms_error::not_bind:
                            return "MMS server has not binded";
                        case mms_error::already_open:
                            return "MMS session has already opened";
                        case mms_error::not_open:
                            return "MMS session has not opened";
                        case mms_error::busy_work:
                            return "MMS client is busy working";
                        case mms_error::format_error:
                            return "MMS packet format error";
                        case mms_error::redirect_error:
                            return "MMS redirect error";
                        case mms_error::keepalive_error:
                            return "MMS keepalive error";

                        case mms_error::continue_:
                            return "Mms: Continue";
                        case mms_error::witching_protocols:
                            return "Mms: witching Protocols";
                        case mms_error::ok:
                            return "Mms: OK";
                        case mms_error::created:
                            return "Mms: Created";
                        case mms_error::accepted:
                            return "Mms: Accepted";
                        case mms_error::non_authoritative_information:
                            return "Mms: Non-Authoritative Information";
                        case mms_error::no_content:
                            return "Mms: No Content";
                        case mms_error::reset_content:
                            return "Mms: Reset Content";
                        case mms_error::partial_content:
                            return "Mms: Partial Content";
                        case mms_error::multiple_choices:
                            return "Mms: Multiple Choices";
                        case mms_error::moved_permanently:
                            return "Mms: Moved Permanently";
                        case mms_error::moved_temporarily:
                            return "Mms: Moved Temporarily";
                        case mms_error::see_other:
                            return "Mms: See Other";
                        case mms_error::not_modified:
                            return "Mms: Not Modified";
                        case mms_error::use_proxy:
                            return "Mms: Use Proxy";
                        case mms_error::temporary_redirect:
                            return "Mms: Temporary Redirect";
                        case mms_error::bad_request:
                            return "Mms: Bad Request";
                        case mms_error::unauthorized:
                            return "Mms: Unauthorized";
                        case mms_error::payment_required:
                            return "Mms: Payment Required";
                        case mms_error::forbidden:
                            return "Mms: Forbidden";
                        case mms_error::not_found:
                            return "Mms: Not Found";
                        case mms_error::method_not_allowed:
                            return "Mms: Method Not Allowed";
                        case mms_error::not_acceptable:
                            return "Mms: Not Acceptable";
                        case mms_error::proxy_authentication_required:
                            return "Mms: Proxy Authentication Required";
                        case mms_error::request_time_out:
                            return "Mms: Request Time-out";
                        case mms_error::conflict:
                            return "Mms: Conflict";
                        case mms_error::gone:
                            return "Mms: Gone";
                        case mms_error::length_required:
                            return "Mms: Length Required";
                        case mms_error::precondition_failed:
                            return "Mms: Precondition Failed";
                        case mms_error::request_entity_too_large:
                            return "Mms: Request Entity Too Large";
                        case mms_error::request_uri_too_large:
                            return "Mms: Request-URI Too Large";
                        case mms_error::unsupported_media_type:
                            return "Mms: Unsupported Media Type";
                        case mms_error::requested_range_not_satisfiable:
                            return "Mms: Requested range not satisfiable";
                        case mms_error::expectation_failed:
                            return "Mms: Expectation Failed";
                        case mms_error::internal_server_error:
                            return "Mms: Internal Server Error";
                        case mms_error::not_implemented:
                            return "Mms: Not Implemented";
                        case mms_error::bad_gateway:
                            return "Mms: Bad Gateway";
                        case mms_error::service_unavailable:
                            return "Mms: Service Unavailable";
                        case mms_error::gateway_time_out:
                            return "Mms: Gateway Time-out";
                        case mms_error::mms_version_not_supported:
                            return "Mms: MMS Version not supported";
                        default:
                            return "mms error";
                        }
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::mms_category instance;
                return instance;
            }

            static boost::system::error_category const & mms_category = get_category();

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

        } // namespace mms_error

    } // namespace protocol
} // namespace util

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<util::protocol::mms_error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using util::protocol::mms_error::make_error_code;
#endif

    }
}

#endif // _UTIL_PROTOCOL_MMS_ERROR_H_
