// LittleEndianBinaryOArchive.h

#ifndef _UTIL_ARCHIVE_NET_BINARY_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_NET_BINARY_O_ARCHIVE_H_

#include "util/archive/StreamOArchive.h"
#include "util/serialization/Array.h"

#include <framework/system/BytesOrder.h>
#include <framework/system/NumberBits24.h>

#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

#include <ostream>

#ifdef BOOST_LITTLE_ENDIAN

#  include "util/archive/BinaryOArchive.h"

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class LittleEndianBinaryOArchive
            : public BinaryOArchive<_Elem, _Traits, LittleEndianBinaryOArchive<_Elem, _Traits> >
        {
            typedef BinaryOArchive<_Elem, _Traits, LittleEndianBinaryOArchive<_Elem, _Traits> > super;
        public:
            LittleEndianBinaryOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : super(*os.rdbuf())
            {
            }

            LittleEndianBinaryOArchive(
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

        /// 网络字节顺序序列化类
        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class LittleEndianBinaryOArchive
            : public StreamOArchive<LittleEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamOArchive<LittleEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>;

        public:
            /// 从ostream构造
            LittleEndianBinaryOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : StreamOArchive<LittleEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>(*os.rdbuf())
            {
            }

            LittleEndianBinaryOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamOArchive<LittleEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
            {
            }

            /// 序列化通用类型实现
            template <typename T>
            void save(
                T const & t)
            {
                // 先转换字节顺序
                T t1 = (T)framework::system::BytesOrder::host_to_little_endian(t);
                this->save_binary((_Elem const *)&t1, sizeof(T));
            }

            void save(
                framework::system::UInt24 const & t)
            {
                // 先转换字节顺序
                framework::system::UInt24 t1 = framework::system::BytesOrder::host_to_little_endian((boost::uint32_t)t);
                this->save_binary((_Elem const *)&t1.bytes(), 3);
            }

            using StreamOArchive<LittleEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>::save;

            /// 判断某个类型是否可以优化数组的序列化
            /// 只有char类型能够直接序列化数组，不需要转换字节顺序
            template<class T>
            struct use_array_optimization
                : boost::integral_constant<bool, sizeof(T) == 1>
            {
            };

            // 序列化数组，直接二进制批量写入，是针对char数组的优化实现
            template<class T>
            void save_array(
                framework::container::Array<T> const & a, 
                typename boost::enable_if<use_array_optimization<T> >::type * = NULL)
            {
                this->save_binary((_Elem *)a.address(), a.count());
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
        struct use_array_optimization<util::archive::LittleEndianBinaryOArchive<_Elem, _Traits>, T>
            : util::archive::LittleEndianBinaryOArchive<_Elem, _Traits>::template use_array_optimization<T>
        {
        };
    }
}

#endif // _UTIL_ARCHIVE_NET_BINARY_O_ARCHIVE_H_
