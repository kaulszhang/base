// LogicError.h

#ifndef _FRAMEWORK_SYSTEM_LOGIC_ERROR_H_
#define _FRAMEWORK_SYSTEM_LOGIC_ERROR_H_

namespace framework
{
	namespace system
	{

        namespace logic_error {

            enum errors
            {
                succeed,                      //   0  ok
                succeed_with_info,            //   1  ok�����о�����Ϣ
                failed_some,                  //   2  ���ִ���
                skipped,                      //   3  ����
                not_supported,                //   4  ��֧�ֵĶ���
                invalid_argument,             //   5  ��Ч����ֵ
                item_not_exist,               //   6  ������ָ����
                item_already_exist,           //   7  ָ�����Ѿ�����
                no_permission,                //   8  ������Ĳ���
                operation_canceled,           //   9  ������ȡ��
                processing,                   //  10  �������ڽ���
                multiple_items,               //  11  ���ڶ���
                no_momery,                    //  12  �ڴ治��
                io_busy,                      //  13  �������æ
                time_out,                     //  14  ������ʱ
                try_again,                    //  15  δ׼����
                too_busy,                     //  16  ����̫��Ƶ��
                no_data,                      //  17  û����������������
                invalid_data_type,            //  18  �������Ͳ�ƥ��
                no_more_item,                 //  19  û�и������
                out_of_range,                 //  20  ������Χ
                unknown_error = 99,           //  99  δ֪����
            };

            namespace detail
            {

                class logic_category
                    : public boost::system::error_category
                {
				public:
                    const char* name() const BOOST_SYSTEM_NOEXCEPT
					{
						return "logic";
					}

					std::string message(int value) const
					{
						if (value == logic_error::succeed)
							return "Everything seems ok";
						if (value == logic_error::succeed_with_info)
							return "OK, but with warnings";
						if (value == logic_error::failed_some)
							return "There is something wrong";
						if (value == logic_error::skipped)
							return "The operation was skipped";
						if (value == logic_error::not_supported)
							return "The request is not supported now";
						if (value == logic_error::invalid_argument)
							return "The request has invalid argument";
						if (value == logic_error::item_not_exist)
							return "The target item does not exist";
						if (value == logic_error::item_already_exist)
							return "The inserting item already exists";
						if (value == logic_error::no_permission)
							return "The operation is not permitted";
						if (value == logic_error::operation_canceled)
							return "The operation is canceled";
						if (value == logic_error::processing)
							return "The operation can't complete soon";
						if (value == logic_error::multiple_items)
							return "There are multiple associated items";
						if (value == logic_error::no_momery)
							return "There is not enough memory";
						if (value == logic_error::io_busy)
							return "The io system is busy";
						if (value == logic_error::time_out)
							return "The operation is out of executing time";
						if (value == logic_error::try_again)
							return "The is something wrong, need try again";
						if (value == logic_error::too_busy)
							return "The requests come too busy";
						if (value == logic_error::no_data)
							return "It does not have any associated data";
						if (value == logic_error::invalid_data_type)
							return "The data type is invalid";
						if (value == logic_error::no_more_item)
							return "Iteratorring reach the end";
						if (value == logic_error::out_of_range)
							return "resule out of range";
						if (value == logic_error::unknown_error)
							return "A unknown error occured";
						return "logic logic_error";
					}
				};

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
				static detail::logic_category instance;
				return instance;
            }

            static boost::system::error_category const & logic_category = get_category();

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
					static_cast<int>(e), get_category());
            }

       } // namespace logic_error

    } // namespace system
} // namespace framework

namespace boost
{
    namespace system
    {
        template<>
        struct is_error_code_enum<framework::system::logic_error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using framework::system::logic_error::make_error_code;
#endif // _FRAMEWORK_SYSTEM_LOGIC_ERROR_H_
    }
}

#endif


