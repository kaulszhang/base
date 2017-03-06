// MemoryPool.cpp

#include "framework/Framework.h"
#include "framework/memory/MemoryPool.h"
#include "framework/logger/LoggerFormatRecord.h"

FRAMEWORK_LOGGER_DECLARE_MODULE("MemoryPool");

namespace framework
{
    namespace memory
    {

        void * MemoryPool::alloc_block(
            size_t size)
        {
           // printf("[MemoryPool::alloc_block] size: %u consumption: %u peek: %u num_block: %u num_object: %u\n", 
           //     size, consumption_, peek_, num_block_, num_object_);
            size = MemoryPage::align_page(size);
            if (consumption_ + size > capacity_) {
                LOG_F(framework::logger::Logger::kLevelAlarm, 
                    ("capacity limit arrived"));
                return NULL;
            }
            void * addr = bm_->alloc_block(size);
            if (addr) {
                consumption_ += size;
                if (consumption_ > peek_) {
                    peek_ = consumption_;
                }
                ++num_block_;
            }
            return addr;
        }

        void MemoryPool::free_block(
            void * ptr, 
            size_t size)
        {
            // printf("[MemoryPool::free_block] size: %u consumption: %u peek: %u num_block: %u num_object: %u\n", 
            //    size, consumption_, peek_, num_block_, num_object_);
            size = MemoryPage::align_page(size);
            --num_block_;
            consumption_ -= size;
            bm_->free_block(ptr, size);
        }

    } // namespace memory
} // namespace framework
