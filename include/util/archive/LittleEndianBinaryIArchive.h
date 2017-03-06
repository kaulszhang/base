// LittleEndianBinaryIArchive.h

#ifndef _UTIL_ARCHIVE_LITTLE_ENDIAN_BINARY_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_LITTLE_ENDIAN_BINARY_I_ARCHIVE_H_

#include "util/archive/StreamIArchive.h"
#include "util/serialization/Array.h"

#include <framework/system/BytesOrder.h>
#include <framework/system/NumberBits24.h>

#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

#include <istream>

#ifdef BOOST_LITTLE_ENDIAN

#  include "util/archive/BinaryIArchive.h"

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class LittleEndianBinaryIArchive
            : public BinaryIArchive<_Elem, _Traits, LittleEndianBinaryIArchive<_Elem, _Traits> >
        {
            typedef BinaryIArchive<_Elem, _Traits, LittleEndianBinaryIArchive<_Elem, _Traits> > super;
        public:
            LittleEndianBinaryIArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : super(*is.rdbuf())
            {
            }

            LittleEndianBinaryIArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : super(buf)
            {
            }
        };

    } // namespace archive
} // namespace util

#else // BOOST_LITTLE_ENDIAN

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class LittleEndianBinaryIArchive
            : public StreamIArchive<LittleEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>
        {
        public:
            LittleEndianBinaryIArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : StreamIArchive<LittleEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>(*is.rdbuf())
            {
            }

            LittleEndianBinaryIArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamIArchive<LittleEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
            {
            }

        public:
            /// 从流中读出变量
            template<class T>
            void load(
                T & t)
            {
                this->load_binary((_Elem *)&t, sizeof(T));
                // 执行字节顺序转换
                if (this->state())
                    return;
                t = (T)framework::system::BytesOrder::little_endian_to_host(t);
            }

            void load(
                framework::system::UInt24 & t)
            {
                framework::system::UInt24 t1;
                this->load_binary((_Elem *)&t1.bytes(), 3);
                // 执行字节顺序转换
                if (this->state())
                    return;
                t = framework::system::BytesOrder::little_endian_to_host((boost::uint32_t)t1);
            }

            /// 判断某个类型是否可以优化数组的读
            /// 只有char类型能够直接读数组，不需要转换字节顺序
            template<class T>
            struct use_array_optimization
                : boost::integral_constant<bool, sizeof(T) == 1>
            {
            };

            /// 读数组，直接二进制批量读取，针对char数组的优化实现
            template<class T>
            void load_array(
                framework::container::Array<T> & a, 
                typename boost::enable_if<use_array_optimization<T> >::type * = NULL)
            {
                this->load_binary((_Elem *)a.address(), a.count());
            }
        };

    } // namespace archive
} // namespace util

#endif // BOOST_LITTLE_ENDIAN

namespace util
{
    namespace serialization
    {
        template <
            typename _Elem, 
            typename _Traits, 
            typename T
        >
        struct use_array_optimization<util::archive::LittleEndianBinaryIArchive<_Elem, _Traits>, T>
            : util::archive::LittleEndianBinaryIArchive<_Elem, _Traits>::template use_array_optimization<T>
        {
        };
    }
}

#endif // _UTIL_ARCHIVE_LITTLE_ENDIAN_BINARY_I_ARCHIVE_H_
