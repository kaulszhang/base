// Serialization.h

#ifndef _UTIL_SERIALIZATION_SERIALIZATION_H_
#define _UTIL_SERIALIZATION_SERIALIZATION_H_

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>

namespace util
{
    namespace serialization
    {

        struct wrapper {};

        template <typename T>
        struct is_wrapper
            : boost::is_base_and_derived<wrapper, T>
        {};

        /// 指示一个类型是否是序列化基本类型
        /// 基本类型的序列化（反序列化）直接调用序列化（反序列化）类的save（load）方法
        template <
            typename Ar, 
            typename T
        >
        struct is_primitive
            : boost::is_fundamental<T>
        {
        };

        /// 指示一个类型是否是序列化基本类型
        template <
            typename T
        >
        struct is_sigle_unit
            : boost::false_type
        {
        };

        template<class Archive, class T>
        inline void serialize(Archive & ar, T & t){
            t.serialize(ar);
        }

    }  // namespace serialization
} // namespace util

/// 定义serialize拆分功能的宏
#define SERIALIZATION_SPLIT_MEMBER() \
template<class Archive> \
void serialize( \
    Archive & ar) \
{ \
    util::serialization::split_member(ar, *this); \
}

#endif // _UTIL_SERIALIZATION_SERIALIZATION_H_
