// DataViewError.h

#ifndef _UTIL_DATAVIEW_DATAVIEW_ERROR_H_
#define _UTIL_DATAVIEW_DATAVIEW_ERROR_H_

namespace util
{
    namespace dataview
    {

        namespace error {

            enum errors
            {
                error_succeed = 0,
                error_invalid_param,
                error_unsupported,
                error_no_such_item,
                error_item_already_exist,
                error_data_type,
            };

            namespace detail {

                class dataview_category
                    : public boost::system::error_category
                {
                public:
                    dataview_category()
                    {
                        register_category(*this);
                    }

                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "dataview";
                    }

                    std::string message(int value) const
                    {
                        switch (value) {
                            case error_succeed:
                                return "dataview operate success";
                            case error_invalid_param:
                                return "dataview invalid parameter";
                            case error_unsupported:
                                return "dataview not supported";
                            case error_no_such_item:
                                return "dataview no such item";
                            case error_item_already_exist:
                                return "error item already exist";
                            case error_data_type:
                                return "error data type";
                            default:
                                return "dataview other error";
                        }
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::dataview_category instance;
                return instance;
            }

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

            static boost::system::error_category const & dataview_category = get_category();

        } // namespace error

    } // namespace dataview
} // namespace util

namespace boost
{
    namespace system
    {
        template<>
        struct is_error_code_enum<util::dataview::error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using util::dataview::error::make_error_code;
#endif

    }
}

#endif // _UTIL_DATAVIEW_DATAVIEW_ERROR_H_
