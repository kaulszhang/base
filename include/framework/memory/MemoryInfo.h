#ifndef _FRAMEWORK_MEMORY_INFO_H_
#define _FRAMEWORK_MEMORY_INFO_H_

namespace framework
{
    namespace memory
    {

        class MemoryInfo
        {
        public:
            static boost::uint64_t free_memory_size();
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_INFO_H_
