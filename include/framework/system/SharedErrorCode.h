// SharedErrorCode.h

#ifndef _FRAMEWORK_SYSTEM_SHARED_ERROR_CODE_H_
#define _FRAMEWORK_SYSTEM_SHARED_ERROR_CODE_H_

#include <boost/system/error_code.hpp>

namespace framework
{
    namespace system
    {

        class SharedErrorCode
        {
        public:
            SharedErrorCode(
                boost::system::error_code const & ec = boost::system::error_code())
                : value_(ec.value())
            {
                strncpy(category_, ec.category().name(), sizeof(category_));
            }

            template <class CodeEnum>
            SharedErrorCode(CodeEnum e,
                typename boost::enable_if<boost::system::is_error_code_enum<CodeEnum> >::type* = 0)
            {
                *this = make_error_code(e);
            }

        public:
            int value() const
            {
                return value_;
            }

            boost::system::error_category const & category() const
            {
                return boost::system::error_category::find_category(category_);
            }

            std::string message() const
            {
                return category().message(value());
            }

            operator boost::system::error_code() const
            {
                return boost::system::error_code(value_, 
                    boost::system::error_category::find_category(category_));
            }

            typedef void (*unspecified_bool_type)();
            static void unspecified_bool_true() {}

            operator unspecified_bool_type() const  // true if error
            { 
                return value_ == 0 ? 0 : unspecified_bool_true;
            }

        private:
            char category_[16];
            int value_;
        };

    } // namespace system
} // namespace boost

#endif // _FRAMEWORK_SYSTEM_SHARED_ERROR_CODE_H_
