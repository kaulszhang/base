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

        /// ָʾһ�������Ƿ������л���������
        /// �������͵����л��������л���ֱ�ӵ������л��������л������save��load������
        template <
            typename Ar, 
            typename T
        >
        struct is_primitive
            : boost::is_fundamental<T>
        {
        };

        /// ָʾһ�������Ƿ������л���������
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

/// ����serialize��ֹ��ܵĺ�
#define SERIALIZATION_SPLIT_MEMBER() \
template<class Archive> \
void serialize( \
    Archive & ar) \
{ \
    util::serialization::split_member(ar, *this); \
}

#endif // _UTIL_SERIALIZATION_SERIALIZATION_H_
