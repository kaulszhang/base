// RtmpError.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_ERROR_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_ERROR_H_

#include "boost/system/error_code.hpp"

namespace util
{
    namespace protocol
    {

        namespace rtmp_error {

            enum errors
            {
                succeed             = 0,
                busy_work, 
                unkown_command, 
                format_error, 
            };

            namespace detail {

                class rtmp_category
                    : public boost::system::error_category
                {
                public:
                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "rtmp";
                    }

                    std::string message(int value) const
                    {
                        switch (value) {
                        case busy_work:
                            return "rtmp: busy work";
                        case unkown_command:
                            return "rtmp: unkown command";
                        case format_error:
                            return "rtmp: format error";
                        default:
                            return "rtmp error";
                        }
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::rtmp_category instance;
                return instance;
            }

            static boost::system::error_category const & rtmp_category = get_category();

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
        struct is_error_code_enum<util::protocol::rtmp_error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using util::protocol::rtmp_error::make_error_code;
#endif

    }
}

#endif // _UTIL_PROTOCOL_RTMP_RTMP_ERROR_H_
