// Error.h

#ifndef _FRAMEWORK_PROCESS_ERROR_H_
#define _FRAMEWORK_PROCESS_ERROR_H_

namespace framework
{
    namespace process
    {

        namespace error {

            enum errors
            {
                already_open,             // 进程已经打开
                not_open,                 // 进程未打开
                not_alive,                // 进程不在活动
                still_alive,              // 进程还在活动
            };

            namespace detail {

                class process_category
                    : public boost::system::error_category
                {
                public:
                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "process";
                    }

                    std::string message(int value) const
                    {
                        if (value == error::already_open)
                            return "Process has already opened";
                        if (value == error::not_open)
                            return "Process has not opened";
                        if (value == error::not_alive)
                            return "Process not alive";
                        if (value == error::still_alive)
                            return "Process still alive";
                        return "Process other error";
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::process_category instance;
                return instance;
            }

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

            static boost::system::error_category const & process_category = get_category();

        } // namespace error

    } // namespace process
} // namespace framework

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<framework::process::error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using framework::process::error::make_error_code;
#endif

    }
}

#endif // _FRAMEWORK_PROCESS_ERROR_H_
