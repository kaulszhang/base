// BackTrace.h

#ifndef _FRAMEWORK_DEBUGING_BACK_TRACE_H_
#define _FRAMEWORK_DEBUGING_BACK_TRACE_H_

namespace framework
{
    namespace debuging
    {

        bool back_trace_init();

        size_t back_trace(
            void ** addrs, 
            size_t num);

        char ** back_trace_symbols(
            void * const * addrs, 
            size_t num);

        void release_symbols(
            char ** ptr);

        void back_trace_symbols_fd(
            void * const * addrs, 
            size_t num, 
            int fd);

    } // namespace debuging
} // namespace framework

#endif // _FRAMEWORK_DEBUGING_BACK_TRACE_H_
