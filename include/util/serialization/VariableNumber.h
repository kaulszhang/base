// VariableNumber.h

#ifndef _UTIL_SERIALIZATION_VARIABLE_NUMBER_H_
#define _UTIL_SERIALIZATION_VARIABLE_NUMBER_H_

#include "util/serialization/Serialization.h"

#include <framework/system/VariableNumber.h>

namespace util
{
    namespace serialization
    {

        template<
            typename Archive, 
            typename _Ty
        >
        struct is_primitive<Archive, framework::system::VariableNumber<_Ty> >
            : boost::true_type
        {
        };

    }
}

#endif // _UTIL_SERIALIZATION_VARIABLE_NUMBER_H_
