// ArchiveError.h

#ifndef _UTIL_ARCHIVE_ARCHIVE_ERROR_H_
#define _UTIL_ARCHIVE_ARCHIVE_ERROR_H_

#include <exception>
#include <cassert>

namespace util
{
    namespace archive
    {
        namespace error
        {
            enum errors
            {
                incompatible_native_format, ///< 格式不对
                stream_error, ///< 输入/输出流错误
            }

            namespace detail {

                class archive_category
                    : public boost::system::error_category
                {
                public:
                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "archive";
                    }

                    std::string message(int value) const
                    {
                        if (value == error::incompatible_native_format)
                            return "Archive: incompatible native format";
                        if (value == error::stream_error)
                            return "Archive: stream error";
                        return "Archive: other error";
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::archive_category instance;
                return instance;
            }

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

            static boost::system::error_category const & archive_category = get_category();

        } // namespace error

    } // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_ARCHIVE_ERROR_H_
