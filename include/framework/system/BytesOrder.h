// BytesOrder.h

#ifndef _FRAMEWORK_SYSTEM_BYTES_ORDER_H_
#define _FRAMEWORK_SYSTEM_BYTES_ORDER_H_

#include <boost/detail/endian.hpp>

namespace framework
{
    namespace system
    {

        namespace BytesOrder
        {

            namespace detail
            {

                inline boost::uint16_t rotate(
                    boost::uint16_t v)
                {
                    return v >> 8 | v << 8;
                }

                inline boost::uint32_t rotate(
                    boost::uint32_t v)
                {
                    // 1 2 3 4
                    // 2 3 4 1
                    // 4 1 2 3
                    return (((v >> 24 | v << 8) & 0x00ff00ff) 
                        | ((v >> 8 | v << 24) & 0xff00ff00));
                }

                inline boost::uint64_t rotate(
                    boost::uint64_t v)
                {
                    // 1 2 3 4 5 6 7 8
                    // 2 3 4 5 6 7 8 1
                    // 4 5 6 7 8 1 2 3
                    // 6 7 8 1 2 3 4 5
                    // 8 1 2 3 4 5 6 7
                    return (((v >> 56 | v << 8) & 0x000000ff000000ffULL) 
                        | ((v >> 40 | v << 24) & 0x0000ff000000ff00ULL)
                        | ((v >> 24 | v << 40) & 0x00ff000000ff0000ULL)
                        | ((v >> 8 | v << 56) & 0xff000000ff000000ULL));
                }

                template <
                    typename _Ty, 
                    size_t _Sz = sizeof(_Ty)
                >
                struct native_rotate;

                template <
                    typename _Ty
                >
                struct native_rotate<_Ty, 1>
                {
                    static inline _Ty invoke(
                        _Ty v)
                    {
                        return v;
                    }
                };

                template <
                    typename _Ty
                >
                struct native_rotate<_Ty, 2>
                {
                    static inline _Ty invoke(
                        _Ty v)
                    {
                        boost::uint16_t r = rotate(*(boost::uint16_t*)&v);
                        return *(_Ty *)&r;
                    }
                };

                template <
                    typename _Ty
                >
                struct native_rotate<_Ty, 4>
                {
                    static inline _Ty invoke(
                        _Ty v)
                    {
                        boost::uint32_t r = rotate(*(boost::uint32_t*)&v);
                        return *(_Ty *)&r;
                    }
                };

                template <
                    typename _Ty
                >
                struct native_rotate<_Ty, 8>
                {
                    static inline _Ty invoke(
                        _Ty v)
                    {
                        boost::uint64_t r = rotate(*(boost::uint64_t*)&v);
                        return *(_Ty *)&r;
                    }
                };

            } // namespace detail

            template <
                typename _Ty
            >
            inline _Ty rotate(
                _Ty v)
            {
                return detail::native_rotate<_Ty>::invoke(v);
            }

#if (defined BOOST_BIG_ENDIAN)

            template <
                typename _Ty
            >
            inline _Ty host_to_big_endian(
                _Ty v)
            {
                return v;
            }

            template <
                typename _Ty
            >
            inline _Ty big_endian_to_host(
                _Ty v)
            {
                return v;
            }

            template <
                typename _Ty
            >
            inline _Ty host_to_little_endian(
                _Ty v)
            {
                return rotate(v);
            }

            template <
                typename _Ty
            >
            inline _Ty little_endian_to_host(
                _Ty v)
            {
                return rotate(v);
            }

#elif (defined BOOST_LITTLE_ENDIAN)

            template <
                typename _Ty
            >
            inline _Ty host_to_big_endian(
                _Ty v)
            {
                return rotate(v);
            }

            template <
                typename _Ty
            >
            inline _Ty big_endian_to_host(
                _Ty v)
            {
                return rotate(v);
            }

            template <
                typename _Ty
            >
            inline _Ty host_to_little_endian(
                _Ty v)
            {
                return v;
            }

            template <
                typename _Ty
            >
            inline _Ty little_endian_to_host(
                _Ty v)
            {
                return v;
            }

#else

 # error "Unknown endian type!"

#endif

            // host <--> big_endian

            inline boost::uint16_t host_to_big_endian_short(
                boost::uint16_t v)
            {
                return host_to_big_endian(v);
            }

            inline boost::uint32_t host_to_big_endian_long(
                boost::uint32_t v)
            {
                return host_to_big_endian(v);
            }

            inline boost::uint64_t host_to_big_endian_longlong(
                boost::uint64_t v)
            {
                return host_to_big_endian(v);
            }

            inline boost::uint16_t big_endian_to_host_short(
                boost::uint16_t v)
            {
                return big_endian_to_host(v);
            }

            inline boost::uint32_t big_endian_to_host_long(
                boost::uint32_t v)
            {
                return big_endian_to_host(v);
            }

            inline boost::uint64_t big_endian_to_host_longlong(
                boost::uint64_t v)
            {
                return big_endian_to_host(v);
            }

            // host <--> little_endian

            inline boost::uint16_t host_to_little_endian_short(
                boost::uint16_t v)
            {
                return host_to_little_endian(v);
            }

            inline boost::uint32_t host_to_little_endian_long(
                boost::uint32_t v)
            {
                return host_to_little_endian(v);
            }

            inline boost::uint64_t host_to_little_endian_longlong(
                boost::uint64_t v)
            {
                return host_to_little_endian(v);
            }

            inline boost::uint16_t little_endian_to_host_short(
                boost::uint16_t v)
            {
                return little_endian_to_host(v);
            }

            inline boost::uint32_t little_endian_to_host_long(
                boost::uint32_t v)
            {
                return little_endian_to_host(v);
            }

            inline boost::uint64_t little_endian_to_host_longlong(
                boost::uint64_t v)
            {
                return little_endian_to_host(v);
            }

            // host <--> net

            inline boost::uint16_t net_to_host_short(
                boost::uint16_t v)
            {
                return big_endian_to_host_short(v);
            }

            inline boost::uint32_t net_to_host_long(
                boost::uint32_t v)
            {
                return big_endian_to_host_long(v);
            }

            inline boost::uint64_t net_to_host_longlong(
                boost::uint64_t v)
            {
                return big_endian_to_host_longlong(v);
            }

            inline boost::uint16_t host_to_net_short(
                boost::uint16_t v)
            {
                return host_to_big_endian_short(v);
            }

            inline boost::uint32_t host_to_net_long(
                boost::uint32_t v)
            {
                return host_to_big_endian_long(v);
            }

            inline boost::uint64_t host_to_net_longlong(
                boost::uint64_t v)
            {
                return host_to_big_endian_longlong(v);
            }

        }

    } // namespace system
} // namespace boost

#endif // _FRAMEWORK_SYSTEM_BYTES_ORDER_H_
