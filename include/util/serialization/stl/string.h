// string.h

#ifndef _UTIL_SERIALIZATION_STL_STRING_H_
#define _UTIL_SERIALIZATION_STL_STRING_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"
#include "util/serialization/Array.h"

#include <string>

namespace util
{
    namespace serialization
    {

        template<
            typename Ar, 
            class _Elem, 
            class _Traits, 
            class _Alloc
        >
        struct is_primitive<Ar, std::basic_string<_Elem, _Traits, _Alloc> >
            : boost::true_type
        {
        };

        template<
            class _Size, 
            class _Elem, 
            class _Traits, 
            class _Alloc
        >
        class SizedString
        {
        public:
            SizedString(
                std::basic_string<_Elem, _Traits, _Alloc> & string)
                : string_(string)
            {
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void save(Archive & ar) const
            {
                _Size size = string_.size();
                ar & size;
                ar & framework::container::make_array(&string_[0], size);
            }

            template <typename Archive>
            void load(Archive & ar)
            {
                _Size size = 0;
                ar & size;
                string_.resize(size);
                ar & framework::container::make_array(&string_[0], size);
            }

        private:
            std::basic_string<_Elem, _Traits, _Alloc> & string_;
        };

        template<
            class _Size, 
            class _Elem, 
            class _Traits, 
            class _Alloc
        >
        SizedString<_Size, _Elem, _Traits, _Alloc> const make_sized(
            std::basic_string<_Elem, _Traits, _Alloc> & str)
        {
            return SizedString<_Size, _Elem, _Traits, _Alloc>(str);
        }

    } // namespace serialization
} // namespace util

#endif // _UTIL_SERIALIZATION_STL_STRING_H_
