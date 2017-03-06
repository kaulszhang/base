// Rtsp.h

#ifndef _UTIL_PROTOCOL_RTSP_ERROR_H_
#define _UTIL_PROTOCOL_RTSP_ERROR_H_

namespace util
{
    namespace protocol
    {

        namespace rtsp_error {

            enum errors
            {
                already_bind        = 1,    //   1  rtsp已经绑定
                not_bind,                   //   2  rtsp已经绑定
                already_open,               //   3  rtsp已经打开连接
                not_open,                   //   4  rtsp连接未打开
                busy_work,                  //   5  rtsp有任务正在进行
                format_error,               //   6  rtsp头格式错误

                continue_                           = 100, // Continue

                ok                                  = 200, // OK
                created                             = 201, // Created
                low_on_storage_space                = 250, // Low on Storage Space

                multiple_choices                    = 300, // Multiple Choices
                moved_permanently                   = 301, // Moved Permanently
                moved_temporarily                   = 302, // Moved Temporarily
                see_other                           = 303, // See Other
                not_modified                        = 304, // Not Modified
                use_proxy                           = 305, // Use Proxy

                bad_request                         = 400, // Bad Request
                unauthorized                        = 401, // Unauthorized
                payment_required                    = 402, // Payment Required
                forbidden                           = 403, // Forbidden
                not_found                           = 404, // Not Found
                method_not_allowed                  = 405, // Method Not Allowed
                not_acceptable                      = 406, // Not Acceptable
                proxy_authentication_required       = 407, // Proxy Authentication Required
                request_time_out                    = 408, // Request Time-out
                gone                                = 410, // Gone
                length_required                     = 411, // Length Required
                precondition_failed                 = 412, // Precondition Failed
                request_entity_too_large            = 413, // Request Entity Too Large
                request_uri_too_large               = 414, // Request-URI Too Large
                unsupported_media_type              = 415, // Unsupported Media Type
                parameter_not_understood            = 451, // Parameter Not Understood
                conference_not_found                = 452, // Conference Not Found
                not_enough_bandwidth                = 453, // Not Enough Bandwidth
                session_not_found                   = 454, // Session Not Found
                method_not_valid_in_this_state      = 455, // Method Not Valid in This State
                header_field_not_valid_for_resource = 456, // Header Field Not Valid for Resource
                invalid_range                       = 457, // Invalid Range
                parameter_is_read_only              = 458, // Parameter Is Read-Only
                aggregate_operation_not_allowed     = 459, // Aggregate operation not allowed
                only_aggregate_operation_allowed    = 460, // Only aggregate operation allowed
                unsupported_transport               = 461, // Unsupported transport
                destination_unreachable             = 462, // Destination unreachable

                internal_server_error               = 500, // Internal Server Error
                not_implemented                     = 501, // Not Implemented
                bad_gateway                         = 502, // Bad Gateway
                service_unavailable                 = 503, // Service Unavailable
                gateway_time_out                    = 504, // Gateway Time-out
                rtsp_version_not_supported          = 505, // RTSP Version not supported
                option_not_supported                = 551, // Option not supported
            };

            namespace detail {

                class rtsp_category
                    : public boost::system::error_category
                {
                public:
                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "rtsp";
                    }

                    std::string message(int value) const
                    {
                        switch (value) {
						case rtsp_error::already_bind:
							return "RTSP server has already binded";
						case rtsp_error::not_bind:
							return "RTSP server has not binded";
						case rtsp_error::already_open:
							return "RTSP session has already opened";
						case rtsp_error::not_open:
							return "RTSP session has not opened";
						case rtsp_error::busy_work:
                            return "RTSP client is busy working";
						case rtsp_error::format_error:
							return "RTSP packet format error";
                        case rtsp_error::continue_:
                            return "Rtsp Continue";
                        case rtsp_error::ok:
                            return "Rtsp OK";
                        case rtsp_error::created:
                            return "Rtsp Created";
                        case rtsp_error::low_on_storage_space:
                            return "Rtsp Low on Storage Space";
                        case rtsp_error::multiple_choices:
                            return "Rtsp Multiple Choices";
                        case rtsp_error::moved_permanently:
                            return "Rtsp Moved Permanently";
                        case rtsp_error::moved_temporarily:
                            return "Rtsp Moved Temporarily";
                        case rtsp_error::see_other:
                            return "Rtsp See Other";
                        case rtsp_error::not_modified:
                            return "Rtsp Not Modified";
                        case rtsp_error::use_proxy:
                            return "Rtsp Use Proxy";
                        case rtsp_error::bad_request:
                            return "Rtsp Bad Request";
                        case rtsp_error::unauthorized:
                            return "Rtsp Unauthorized";
                        case rtsp_error::payment_required:
                            return "Rtsp Payment Required";
                        case rtsp_error::forbidden:
                            return "Rtsp Forbidden";
                        case rtsp_error::not_found:
                            return "Rtsp Not Found";
                        case rtsp_error::method_not_allowed:
                            return "Rtsp Method Not Allowed";
                        case rtsp_error::not_acceptable:
                            return "Rtsp Not Acceptable";
                        case rtsp_error::proxy_authentication_required:
                            return "Rtsp Proxy Authentication Required";
                        case rtsp_error::request_time_out:
                            return "Rtsp Request Time-out";
                        case rtsp_error::gone:
                            return "Rtsp Gone";
                        case rtsp_error::length_required:
                            return "Rtsp Length Required";
                        case rtsp_error::precondition_failed:
                            return "Rtsp Precondition Failed";
                        case rtsp_error::request_entity_too_large:
                            return "Rtsp Request Entity Too Large";
                        case rtsp_error::request_uri_too_large:
                            return "Rtsp Request-URI Too Large";
                        case rtsp_error::unsupported_media_type:
                            return "Rtsp Unsupported Media Type";
                        case rtsp_error::parameter_not_understood:
                            return "Rtsp Parameter Not Understood";
                        case rtsp_error::conference_not_found:
                            return "Rtsp Conference Not Found";
                        case rtsp_error::not_enough_bandwidth:
                            return "Rtsp Not Enough Bandwidth";
                        case rtsp_error::session_not_found:
                            return "Rtsp Session Not Found";
                        case rtsp_error::method_not_valid_in_this_state:
                            return "Rtsp Method Not Valid in This State";
                        case rtsp_error::header_field_not_valid_for_resource:
                            return "Rtsp Header Field Not Valid for Resource";
                        case rtsp_error::invalid_range:
                            return "Rtsp Invalid Range";
                        case rtsp_error::parameter_is_read_only:
                            return "Rtsp Parameter Is Read-Only";
                        case rtsp_error::aggregate_operation_not_allowed:
                            return "Rtsp Aggregate operation not allowed";
                        case rtsp_error::only_aggregate_operation_allowed:
                            return "Rtsp Only aggregate operation allowed";
                        case rtsp_error::unsupported_transport:
                            return "Rtsp Unsupported transport";
                        case rtsp_error::destination_unreachable:
                            return "Rtsp Destination unreachable";
                        case rtsp_error::internal_server_error:
                            return "Rtsp Internal Server Error";
                        case rtsp_error::not_implemented:
                            return "Rtsp Not Implemented";
                        case rtsp_error::bad_gateway:
                            return "Rtsp Bad Gateway";
                        case rtsp_error::service_unavailable:
                            return "Rtsp Service Unavailable";
                        case rtsp_error::gateway_time_out:
                            return "Rtsp Gateway Time-out";
                        case rtsp_error::rtsp_version_not_supported:
                            return "Rtsp RTSP Version not supported";
                        case rtsp_error::option_not_supported:
                            return "Rtsp Option not supported";
						default:
							return "rtsp error";
                        }
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::rtsp_category instance;
                return instance;
            }

            static boost::system::error_category const & rtsp_category = get_category();

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

        } // namespace rtsp_error

    } // namespace protocol
} // namespace util

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<util::protocol::rtsp_error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using util::protocol::rtsp_error::make_error_code;
#endif

    }
}

#endif // _UTIL_PROTOCOL_RTSP_ERROR_H_
