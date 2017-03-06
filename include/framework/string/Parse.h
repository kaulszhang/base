// Parse.h

/** 从格式化字符串解析出各种数据类型的函数模板
*/

#ifndef _FRAMEWORK_STRING_PARSE_H_
#define _FRAMEWORK_STRING_PARSE_H_

#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_array.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/mpl/if.hpp>

#include <framework/system/LogicError.h>

#include <sstream>
#include <functional>
#include <iterator>

namespace framework
{
    namespace string
    {

        template <typename T>
        struct Parser2;

        template <typename T>
        inline boost::system::error_code parse2(
            std::string const & str, 
            T & v);

        template <typename T>
        boost::system::error_code
            inline from_string(
            T & v, 
            std::string const & str)
        {
            return v.from_string(str);
        }

        struct parse2_pointer
        {
            template<typename T>
            static boost::system::error_code invoke(
                std::string const & str, 
                T * & v)
            {
                v = new T;
                return Parser2<T>()(str, *v);
            }

            template<typename T>
            static boost::system::error_code invoke(
                std::string const & str, 
                T const * & v)
            {
                v = new T;
                return Parser2<T>()(str, *v);
            }

            template<typename T>
            static boost::system::error_code invoke(
                std::string const & str, 
                T const & v)
            {
                v = new T;
                return Parser2<T>()(str, *v);
            }

            static boost::system::error_code invoke(
                std::string const & str, 
                void const * & v)
            {
                std::istringstream iss(str);
                void * v1;
                iss >> v1;
                if (iss.good()) {
                    v = v1;
                }
                return iss.bad() ? 
                    framework::system::logic_error::invalid_argument
                    : framework::system::logic_error::succeed;
            }

            static boost::system::error_code invoke(
                std::string const & str, 
                void * & v)
            {
                std::istringstream iss(str);
                iss >> v;
                return iss.bad() ? 
                    framework::system::logic_error::invalid_argument
                    : framework::system::logic_error::succeed;
            }
        };

        struct parse2_enum
        {
            template<typename T>
            boost::system::error_code
                static invoke(
                std::string const & str, 
                T & v)
            {
                int i;
                boost::system::error_code ec = parse2(str, i);
                if (ec) {
                    return ec;
                }
                v = static_cast<T>(i);
                return framework::system::logic_error::succeed;
            }
        };

        struct parse2_non_pointer
        {
            /// 处理基本类型序列化
            struct parse2_primitive
            {
                template<typename T>
                static boost::system::error_code invoke(
                    std::string const & str, 
                    T & v)
                {
                    std::istringstream iss(str);
                    iss >> v;
                    return iss.bad() ? 
                        framework::system::logic_error::invalid_argument
                        : framework::system::logic_error::succeed;
                }

                static boost::system::error_code invoke(
                    std::string const & str, 
                    bool & v)
                {
                    v = str == "1" || str == "true";
                    return framework::system::logic_error::succeed;
                }

                static boost::system::error_code invoke(
                    std::string const & str, 
                    unsigned char & v)
                {
                    int i;
                    boost::system::error_code ec = parse2(str, i);
                    if (ec) {
                        return ec;
                    }
                    v = static_cast<unsigned char>(i);
                    return framework::system::logic_error::succeed;
                }
            };

            /// 处理标准类型（非基本类型）序列化
            struct parse2_standard
            {
                template<typename T>
                static boost::system::error_code invoke(
                    std::string const & str, 
                    T & v)
                {
                    return from_string(v, str);
                }
            };

            template<typename T>
            static boost::system::error_code invoke(
                std::string const & str, 
                T & v)
            {
                /// 根据类型类别（基本类型，标准类型），分别处理序列化
                typedef typename boost::mpl::if_<
                    boost::is_fundamental<T>, 
                    parse2_primitive, 
                    parse2_standard
                >::type invoke_type;
                return invoke_type::invoke(str, v);
            }
        };

        template <typename T>
        struct Parser2
            : std::binary_function<std::string, T, boost::system::error_code>
        {
            boost::system::error_code operator () (
                std::string const & str, 
                T & v) const
            {
                typedef typename boost::mpl::if_<
                    boost::is_pointer<T>, 
                    parse2_pointer, 
                    BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                    boost::is_enum<T>, 
                    parse2_enum, 
                    parse2_non_pointer
                    >::type
                >::type invoke_type;
                return invoke_type::invoke(str, v);
            }
        };

        template <typename T>
        struct Parser
            : std::unary_function<T, std::string>
        {
            T operator () (
                std::string const & str)
            {
                T v;
                Parser2<T>()(str, v);
                return v;
            }
        };

        template <>
        struct Parser2<std::string>
            : std::binary_function<std::string, std::string, boost::system::error_code>
        {
            boost::system::error_code operator () (
                std::string const & v, 
                std::string & t)
            {
                t = v;
                return boost::system::error_code();
            }
        };

        // 函数模板，输入格式化字符串，输出参数化类型的数据
        template <typename T>
        inline T parse(
            std::string const & str)
        {
            T v = T();
            Parser2<T>()(str, v);
            return v;
        }

        template <typename T>
        inline boost::system::error_code parse2(
            std::string const & str, 
            T & v)
        {
            return Parser2<T>()(str, v);
        }

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_PARSE_H_
