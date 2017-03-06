// ZipError.h

#ifndef _UTIL_STREAM_COMPRESS_ERROR_H_
#define _UTIL_STREAM_COMPRESS_ERROR_H_

namespace util
{
    namespace stream
    {

        namespace error {

            enum errors
            {
                compress_success,
                compress_eos,
                compress_out_of_range,
                compress_invalid_state,
                compress_invalid_parameters,
                compress_not_supported,
                compress_invalid_format,
                compress_out_of_memory,
                compress_internal,
                compress_failure
            };

            namespace detail {

                class compress_category
                    : public boost::system::error_category
                {
                public:
                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "compress";
                    }

                    std::string message(int value) const
                    {
                        switch (value) {
                case compress_success:
                    return "compress success";
                case compress_eos:
                    return "compress eos";
                case compress_out_of_range:
                    return "compress out of range";
                case compress_invalid_state:
                    return "compress invalid state";
                case compress_invalid_parameters:
                    return "compress invalid parameters";
                case compress_not_supported:
                    return "compress not supported";
                case compress_invalid_format:
                    return "compress invalid format";
                case compress_out_of_memory:
                    return "compress out of memory";
                case compress_internal:
                    return "compress internal";
                case compress_failure:
                    return "compress failure";
                default:
                    return "compress other error";
                        }
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::compress_category instance;
                return instance;
            }

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

        } // namespace compress_error

    } // namespace stream
} // namespace util

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<util::stream::error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using util::stream::error::make_error_code;
#endif

    }
}

#endif // _UTIL_STREAM_COMPRESS_ERROR_H_
