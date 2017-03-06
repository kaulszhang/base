// BigEndianBinaryOArchive.h

#ifndef _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_O_ARCHIVE_H_

#include "util/archive/StreamOArchive.h"
#include "util/serialization/Array.h"

#include <framework/system/BytesOrder.h>
#include <framework/system/NumberBits24.h>
#include <framework/system/VariableNumber.h>

#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

#include <ostream>

#ifdef BOOST_BIG_ENDIAN

#  include "util/archive/BinaryOArchive.h"

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class BigEndianBinaryOArchive
            : public BinaryOArchive<_Elem, _Traits>
        {
        public:
            BigEndianBinaryOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : BinaryOArchive<_Elem, _Traits>(*os.rdbuf())
            {
            }

            BigEndianBinaryOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : BinaryOArchive<_Elem, _Traits>(buf)
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

        /// �����ֽ�˳�����л���
        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class BigEndianBinaryOArchive
            : public StreamOArchive<BigEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamOArchive<BigEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>;

        public:
            /// ��ostream����
            BigEndianBinaryOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : StreamOArchive<BigEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>(*os.rdbuf())
            {
            }

            BigEndianBinaryOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamOArchive<BigEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
            {
            }

            /// ���л�ͨ������ʵ��
            template <typename T>
            void save(
                T const & t)
            {
                // ��ת���ֽ�˳��
                T t1 = (T)framework::system::BytesOrder::host_to_big_endian(t);
                this->save_binary((_Elem const *)&t1, sizeof(T));
            }

            void save(
                framework::system::UInt24 const & t)
            {
                // ��ת���ֽ�˳��
                framework::system::UInt24 t1 = framework::system::BytesOrder::host_to_big_endian((boost::uint32_t)t);
                this->save_binary((_Elem const *)t1.bytes() + 1, 3);
            }

            template <typename T>
            void save(
                framework::system::VariableNumber<T> const & t)
            {
                // ��ת���ֽ�˳��
                framework::system::VariableNumber<T> t1 = framework::system::BytesOrder::host_to_big_endian(t.encode());
                this->save_binary((_Elem const *)t1.bytes() + sizeof(T) - t1.size(), t1.size());
            }

            using StreamOArchive<BigEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>::save;
            
            /// �ж�ĳ�������Ƿ�����Ż���������л�
            /// ֻ��_Elem�����ܹ�ֱ�����л����飬����Ҫת���ֽ�˳��
            template<class T>
            struct use_array_optimization
                : boost::integral_constant<bool, sizeof(T) == 1>
            {
            };

            // ���л����飬ֱ�Ӷ���������д�룬�����_Elem������Ż�ʵ��
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
        struct use_array_optimization<util::archive::BigEndianBinaryOArchive<_Elem, _Traits>, T>
            : util::archive::BigEndianBinaryOArchive<_Elem, _Traits>::template use_array_optimization<T>
        {
        };
    }
}

#endif // _UTIL_ARCHIVE_BIG_ENDIAN_BINARY_O_ARCHIVE_H_
