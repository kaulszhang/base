// BigEndianBinaryIArchive.h

#ifndef _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_I_ARCHIVE_H_

#include "util/archive/StreamIArchive.h"
#include "util/serialization/Array.h"

#include <framework/system/BytesOrder.h>
#include <framework/system/NumberBits24.h>
#include <framework/system/VariableNumber.h>

#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

#include <istream>

#ifdef BOOST_BIG_ENDIAN

#  include "util/archive/BinaryIArchive.h"

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class BigEndianBinaryIArchive
            : public BinaryIArchive<_Elem, _Traits>
        {
        public:
            BigEndianBinaryIArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : BinaryIArchive<_Elem, _Traits>(*is.rdbuf())
            {
            }

            BigEndianBinaryIArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : BinaryIArchive<_Elem, _Traits>(buf)
            {
            }
        };

    } // namespace archive
} // namespace util

#else // BOOST_BIG_ENDIAN

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class BigEndianBinaryIArchive
            : public StreamIArchive<BigEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamIArchive<BigEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>;

        public:
            BigEndianBinaryIArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : StreamIArchive<BigEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>(*is.rdbuf())
            {
            }

            BigEndianBinaryIArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamIArchive<BigEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
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
                if (this->state()) return;
                t = (T)framework::system::BytesOrder::big_endian_to_host(t);
            }

            void load(
                framework::system::UInt24 & t)
            {
                framework::system::UInt24 t1;
                this->load_binary((_Elem *)t1.bytes() + 1, 3);
                // 执行字节顺序转换
                if (this->state()) return;
                t = framework::system::BytesOrder::big_endian_to_host((boost::uint32_t)t1);
            }

            template<class T>
            void load(
                framework::system::VariableNumber<T> & t)
            {
                boost::uint8_t byte = 0;
                this->load_binary((_Elem *)&byte, sizeof(byte));
                if (this->state()) return;
                framework::system::VariableNumber<T> t1(byte);
                this->load_binary((_Elem *)t1.bytes() + sizeof(T) + 1 - t1.size(), t1.size() - 1);
                // 执行字节顺序转换
                if (this->state()) return;
                t = framework::system::BytesOrder::big_endian_to_host(t1).decode();
            }

            using StreamIArchive<BigEndianBinaryIArchive<_Elem, _Traits>, _Elem, _Traits>::load;

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

#endif // BOOST_BIG_ENDIAN

namespace util
{
    namespace serialization
    {
        template <
            typename _Elem, 
            typename _Traits, 
            typename T
        >
        struct use_array_optimization<util::archive::BigEndianBinaryIArchive<_Elem, _Traits>, T>
            : util::archive::BigEndianBinaryIArchive<_Elem, _Traits>::template use_array_optimization<T>
        {
        };
    }
}

#endif // _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_I_ARCHIVE_H_
