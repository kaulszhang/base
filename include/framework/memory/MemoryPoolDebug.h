// MemoryPoolDebug.h

#ifndef _FRAMEWORK_MEMORY_MEMORY_POOL_DEBUG_H_
#define _FRAMEWORK_MEMORY_MEMORY_POOL_DEBUG_H_

#include "framework/container/Ordered.h"

namespace framework
{
    namespace memory
    {

        class MemoryPoolDebug
        {
        private:
            struct DebugStack;

        public:
            struct DebugInfo
            {
                DebugInfo()
                    : stack(NULL)
                {
                }

                DebugStack * stack;
            };

        public:
            MemoryPoolDebug();

            MemoryPoolDebug(
                MemoryPoolDebug const & r);

            ~MemoryPoolDebug();

        public:
            void mark(
                DebugInfo & info);

            void unmark(
                DebugInfo & info);

            void dump();

            void dump(
                DebugInfo const & info, 
                void * addr, 
                size_t size);

        private:
            void output_title(
                size_t num);

            void output_title(
                void * addr, 
                size_t size);

            void output_memory(
                void * addr, 
                size_t size);

            void output(
                char const * msg);

            void output_symbols(
                DebugStack * ds);

            DebugStack * push(
                void ** addrs, 
                size_t num);

        private:
            struct DebugStackItem;
            struct DebugStackSet;

            DebugStack * cds_;
            DebugStackSet * dss_;
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_MEMORY_POOL_DEBUG_H_
