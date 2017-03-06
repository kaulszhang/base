// NumberBits24.h

#ifndef _FRAMEWORK_SYSTEM_NUMBER_BITS_24_H_
#define _FRAMEWORK_SYSTEM_NUMBER_BITS_24_H_

#include "framework/system/BytesOrder.h"

namespace framework
{
    namespace system
    {

        class UInt24
        {
        public:
            UInt24(
                boost::uint32_t n = 0)
                : n_(n)
            {
            }
 
        public:
            operator boost::uint32_t() const
            {
                return n_;
            }

            boost::uint8_t * bytes()
            {
                return (boost::uint8_t *)&n_;
            }
 
            boost::uint8_t const * bytes() const
            {
                return (boost::uint8_t const *)&n_;
            }
 
            boost::uint8_t * data()
            {
#if (defined BOOST_BIG_ENDIAN)
                return (boost::uint8_t *)&n_ + 1;
#else
                return (boost::uint8_t *)&n_;
#endif
            }
 
            boost::uint8_t const * data() const
            {
#if (defined BOOST_BIG_ENDIAN)
                return (boost::uint8_t const *)&n_ + 1;
#else
                return (boost::uint8_t const *)&n_;
#endif
            }

        private:
            // big endian 后三个字节，第一个字节为0
            // little endian 前三个字节，最后一个字节为0
            boost::uint32_t n_;
        };

        namespace BytesOrder
        {

            namespace detail
            {

                inline framework::system::UInt24 rotate(
                    framework::system::UInt24 v)
                {
                    return framework::system::UInt24(rotate((boost::uint32_t)v));
                }

            }

        }

    }
}

#endif // _FRAMEWORK_SYSTEM_NUMBER_BITS_24_H_
