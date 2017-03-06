// BinaryIArchive.h

#ifndef _UTIL_ARCHIVE_BINARY_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_BINARY_I_ARCHIVE_H_

#include "util/archive/StreamIArchive.h"
#include "util/serialization/Array.h"

#include <framework/system/NumberBits24.h>

#include <boost/type_traits/is_fundamental.hpp>
#include <boost/utility/enable_if.hpp>

#include <istream>
#include <string>

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>, 
            typename _Archive = boost::mpl::false_
        >
        class BinaryIArchive
            : public StreamIArchive<typename boost::mpl::if_<
                boost::is_same<_Archive, boost::mpl::false_>, 
                BinaryIArchive<_Elem, _Traits>, 
                _Archive>::type, _Elem, _Traits>
        {
            typedef StreamIArchive<typename boost::mpl::if_<
                boost::is_same<_Archive, boost::mpl::false_>, 
                BinaryIArchive<_Elem, _Traits>, 
                _Archive>::type, _Elem, _Traits> super;

        public:
            BinaryIArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : super(*is.rdbuf())
            {
            }

            BinaryIArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : super(buf)
            {
            }

        public:
            using super::load_binary;

            /// 从流中读出变量
            template<class T>
            void load(
                T & t)
            {
                load_binary((_Elem *)&t, sizeof(T));
            }

            void load(
                framework::system::UInt24 & t)
            {
                t = framework::system::UInt24();
                load_binary((_Elem *)t.data(), 3);
            }

            using super::load;

            /// 判断某个类型是否可以优化数组的序列化
            /// 只有基本类型能够直接序列化数组
            template<class T>
            struct use_array_optimization
                : boost::is_fundamental<T>
            {
            };

            /// 从流中读出数组（优化）
            template<class T>
            void load_array(
                framework::container::Array<T> & a, 
                typename boost::enable_if<use_array_optimization<T> >::type * = NULL)
            {
                load_binary((_Elem *)a.address(), sizeof(T) * a.count());
            }
        };

    } // namespace archive
} // namespace util

namespace util
{
    namespace serialization
    {
        template <
            typename _Elem, 
            typename _Traits, 
            typename _Archive,
            typename T
        >
        struct use_array_optimization<util::archive::BinaryIArchive<_Elem, _Traits, _Archive>, T>
            : util::archive::BinaryIArchive<_Elem, _Traits, _Archive>::template use_array_optimization<T>
        {
        };
    }
}

#endif // _UTIL_ARCHIVE_BINARY_I_ARCHIVE_H_
