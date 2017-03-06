// Format.h

/** 将各种类型的数据格式化为字符串的函数模板和类模板
*/

#ifndef _FRAMEWORK_STRING_FORMAT_H_
#define _FRAMEWORK_STRING_FORMAT_H_

#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_array.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/mpl/if.hpp>

#include <framework/system/LogicError.h>

#include <sstream>
#include <functional>

namespace framework
{
    namespace string
    {

        template <typename T>
        struct Formator;

        template <typename T>
        inline std::string format(
            T const & v);

        template <typename T>
        inline boost::system::error_code format2(
            std::string & str, 
            T const & v);

        template <typename T>
        inline std::string to_string(
            T const & v)
        {
            return v.to_string();
        }

        // 函数模板，可以不要指定数据类型，编译器自动决定
        inline std::string operator + (
            char const * left, 
            std::string const & right)
        {
            return std::string(left) + right;
        }

        struct format_pointer
        {
            template<typename T>
            static std::string invoke(
                T const * const & v)
            {
                return Formator<T>()(*v);
            }

            template<typename T>
            static std::string invoke(
                T * const & v)
            {
                return Formator<T>()(*v);
            }

            template<typename T>
            static std::string invoke(
                T const & v)
            {
                return Formator<typename T::value_type>()(*v);
            }

            static std::string invoke(
                void * v)
            {
                std::ostringstream oss;
                oss << v;
                return oss.str();
            }
        };

        struct format_enum
        {
            template<typename T>
            static std::string invoke(
                T const & v)
            {
                const int i = static_cast<int>(v);
                return format(i);
            }
        };

        struct format_non_pointer
        {
            /// 处理基本类型序列化
            struct format_primitive
            {
                template<typename T>
                static std::string invoke(
                    T const & v)
                {
                    std::ostringstream oss;
                    oss << v;
                    return oss.str();
                }

                static std::string invoke(
                    unsigned char const & v)
                {
                    return format((int)v);
                }

                static std::string invoke(
                    bool const & v)
                {
                    return v ? "true" : "false";
                }
            };

            /// 处理标准类型（非基本类型）序列化
            struct format_standard
            {
                template<typename T>
                static std::string invoke(
                    T const & v)
                {
                    return to_string(v);
                }
            };

            template<typename T>
            static std::string invoke(
                T const & v)
            {
                /// 根据类型类别（基本类型，标准类型），分别处理序列化
                typedef typename boost::mpl::if_<
                    boost::is_fundamental<T>, 
                    format_primitive, 
                    format_standard
                >::type invoke_type;
                return invoke_type::invoke(v);
            }
        };

        template <typename T>
        struct Formator
            : std::unary_function<T, std::string>
        {
            std::string operator () (
                T const & v) const
            {
                typedef typename boost::mpl::if_<
                    boost::is_pointer<T>, 
                    format_pointer, 
                    BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                    boost::is_enum<T>, 
                    format_enum, 
                    format_non_pointer
                    >::type
                >::type invoke_type;
                return invoke_type::invoke(v);
            }
        };

        template <typename T>
        struct Formator2
            : std::binary_function<std::string, T, boost::system::error_code>
        {
            boost::system::error_code operator () (
                std::string & str, 
                T const & v) const
            {
                str = Formator<T>()(v);
                return framework::system::logic_error::succeed;
            }
        };

        template <typename T>
        inline std::string format(
            T const & v)
        {
            return Formator<T>()(v);
        }

        template <typename T>
        inline boost::system::error_code format2(
            std::string & str, 
            T const & v)
        {
            str = Formator<T>()(v);
            return framework::system::logic_error::succeed;
        }

        template <>
        struct Formator<std::string>
            : std::binary_function<std::string, std::string, boost::system::error_code>
        {
        public:
            std::string operator () (
                std::string const & v)
            {
                return v;
            }
        };

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_FORMAT_H_
