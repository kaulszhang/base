// identify.h

#ifndef _FRAMEWORK_CONTAINER_DETAIL_IDENTIFY_H_
#define _FRAMEWORK_CONTAINER_DETAIL_IDENTIFY_H_

namespace framework
{
    namespace container
    {

        template <
            typename _Ty
        >
        struct identity 
        {
            typedef _Ty result_type;

            _Ty const & operator () (
                _Ty const & t) const
            {
                return t;
            }
        };

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_DETAIL_IDENTIFY_H_
