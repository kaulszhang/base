// LoggerParam.h

#ifndef _FRAMEWORK_LOGGER_PARAM_H_
#define _FRAMEWORK_LOGGER_PARAM_H_

namespace framework
{
    namespace logger
    {

        template <
            typename _Ty
        >
        struct LoggerParam
        {
            typedef _Ty const & store_type;
        };

        template <
            typename _Ty, 
            size_t size
        >
        struct LoggerParam<_Ty [size]>
        {
            typedef _Ty const * store_type;
        };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_PARAM_H_
