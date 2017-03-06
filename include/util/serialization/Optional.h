// Optional.h

#ifndef _UTIL_SERIALIZATION_OPTIONAL_H_
#define _UTIL_SERIALIZATION_OPTIONAL_H_

#include <boost/optional.hpp>

namespace util
{
    namespace serialization
    {

        template<
            class _T
        >
        struct optional_ref
        {
            optional_ref(
                _T & t, 
                size_t ver)
                : t_(t)
                , ver_(ver)
            {
            }

            void reset()
            {
                t_ = _T();
            }

            template <
                typename Archive
            >
            void serialize( 
                Archive & ar)
            {
                size_t ver = ar.version();
                if (ver_ > 0)
                    ar.version(ver_);
                ar & t_;
                if (ver_ > 0)
                    ar.version(ver);
            }

        private:
            _T & t_;
            size_t ver_;
        };

        template<
            class _T
        >
        optional_ref<_T> make_optional(
            _T & t, 
            size_t ver = 0)
        {
            return optional_ref<_T>(t, ver);
        }

    }  // namespace serialize
} // namespace util

#endif // _UTIL_SERIALIZATION_OPTIONAL_H_
