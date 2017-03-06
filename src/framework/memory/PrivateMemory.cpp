// PageAlloc.cpp

#include "framework/Framework.h"
#include "framework/memory/PrivateMemory.h"
#include "framework/system/ErrorCode.h"
#include "framework/logger/LoggerFormatRecord.h"

#include <stdio.h>

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#else
#  include <unistd.h>
#  include <sys/mman.h>
#  ifndef MAP_ANONYMOUS
#    define MAP_ANONYMOUS MAP_ANON
#  endif
#endif

FRAMEWORK_LOGGER_DECLARE_MODULE("PrivateMemory");

namespace framework
{

    namespace memory
    {

        void * PrivateMemory::alloc_block(
            size_t size)
        {
#ifdef _WIN32
            void * addr = VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
#else
            void * addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (addr == MAP_FAILED) {
                addr = NULL;
            }
            if (addr) {
                char * p = (char *)addr;
                char * pe = p + size;
                size_t n = page_size();
                while (p < pe) {
                    *p = '\0';
                    p += n;
                }
            }
#endif
            if (addr == NULL) {
                LOG_F(framework::logger::Logger::kLevelAlarm, 
                    ("alloc page failed"));
            }
            return addr;
        }

        void PrivateMemory::free_block(
            void * addr, 
            size_t size)
        {
#ifdef _WIN32
            VirtualFree(addr, 0, MEM_RELEASE);
#else
            munmap(addr, size);
#endif
        }

        void * PrivateMemory::alloc_with_id(
            size_t id, 
            size_t size)
        {
            if ( private_pool_ == NULL )
            {
                void * ptrpool = alloc_block( align_page( page_size() ) );
                private_pool_ = new ( ptrpool ) InternalData;
                private_pool_->alloc_pos_ = align_object( sizeof( InternalData ) );
            }
            else
            {// 找到了，则返回空，表示已经存在
                if ( get_by_id( id ) ) return NULL;
            }

            void * objptr = ( char * )private_pool_ + private_pool_->alloc_pos_ + align_object( sizeof( PriMemItem ) );
            size = align_object( size + align_object( sizeof( PriMemItem ) ) );

            assert( private_pool_->alloc_pos_ + size < page_size() );

            private_pool_->primems_.push( 
                new ( ( char * )private_pool_ + private_pool_->alloc_pos_ ) PriMemItem( id, size ) );
            private_pool_->alloc_pos_ += size;

            return objptr;
        }

        void * PrivateMemory::get_by_id(
            size_t id)
        {
            if ( private_pool_ == NULL ) return NULL;
            
            for ( OrderedUnidirList< PriMemItem >::pointer p = private_pool_->primems_.first(); p; p = private_pool_->primems_.next(p)) 
            {
                if ( p->n_ == id ) return ( ( char * )p + align_object( sizeof( PriMemItem ) ) );
            }

            return NULL;
        }

        void PrivateMemory::close( boost::system::error_code & ec )
        {
            if ( private_pool_ )
            {
                private_pool_->primems_.clear();
                free_block( private_pool_, align_page( page_size() ) );
            }
        }

    }
}
