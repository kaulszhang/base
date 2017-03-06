// Array.h

#ifndef _UTIL_SERIALIZATION_ARRAY_H_
#define _UTIL_SERIALIZATION_ARRAY_H_

#include "util/serialization/NVPair.h"
#include "util/serialization/SplitFree.h"

#include <framework/container/Array.h>

#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>

namespace util
{
    namespace serialization
    {

        template <class Archive, typename T>
        struct use_array_optimization : boost::mpl::false_ {};

    }  // namespace serialization
} // namespace util

namespace framework
{
    namespace container
    {
        /// ָʾArchive�Ƿ�֧��ĳ������T�������Ż����л�
        /// Ĭ�ϲ�֧��
        // Ĭ��ʵ��
        template <class Archive, typename T>
        void serialize_optimized(
            Archive & ar, 
            framework::container::Array<T> & arr, 
            boost::mpl::false_)
        {
            // ͨ��ѭ���������л�
            std::size_t c = arr.count();
            T * t = arr.address();
            while(0 < c--)
                ar & util::serialization::make_nvp("item", *t++);
        }

        // �Ż�ʵ��
        template <class Archive, typename T>
        void serialize_optimized(
            Archive & ar, 
            framework::container::Array<T> & arr, 
            boost::mpl::true_)
        {
            util::serialization::split_free(ar, arr);
        }

        // �Ż�ʵ�����л�
        template <class Archive, typename T>
        void save(
            Archive & ar, 
            framework::container::Array<T> const & arr)
        {
            ar.save_array(arr);
        }

        // �Ż�ʵ�ַ����л�
        template <class Archive, typename T>
        void load(
            Archive & ar, 
            framework::container::Array<T> & arr)
        {
            ar.load_array(arr);
        }

        template <class Archive, typename T>
        void serialize(
            Archive & ar, 
            framework::container::Array<T> & arr)
        {
            // ����use_array_optimizationָʾ�ֱ�����Ż�����Ż��汾�����л�����
            typedef typename boost::mpl::if_<
                util::serialization::use_array_optimization<Archive, T>, 
                boost::mpl::true_, 
                boost::mpl::false_
            >::type use_optimized;
            serialize_optimized(ar, arr, use_optimized());
        }

    }  // namespace container
} // namespace framework

/// Archive�ô˺�ָʾ��֧��ĳЩ���͵������Ż����л�
#ifndef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#define SERIALIZATION_USE_ARRAY_OPTIMIZATION(Archive) \
namespace util \
{ \
    namespace serialization \
    { \
        template <typename T> \
        struct use_array_optimization<Archive, T> \
            : Archive::use_array_optimization<T> \
        { \
        }; \
    } \
}

#else

#define SERIALIZATION_USE_ARRAY_OPTIMIZATION(Archive) \
namespace util \
{ \
    namespace serialization \
    { \
        template <> \
        struct use_array_optimization_impl<Archive> \
        { \
            struct result_ \
            { \
                template <typename T> \
                struct apply \
                    : Archive::use_array_optimization<T> \
                { \
                }; \
            }; \
        }; \
    } \
}

#endif

#endif // _UTIL_SERIALIZATION_ARRAY_H_
