// AWEMemoryPool.cpp

#include "framework/Framework.h"
#include "framework/memory/AWEMemoryPool.h"
#include "framework/logger/Logger.h"
using namespace framework::logger;

#if defined _WIN32 && _WIN32_WINNT >= 0x0500

#include <boost/thread/mutex.hpp>

#include <assert.h>

#include <windows.h>

namespace framework
{
    namespace memory
    {

        static size_t const VIRTUAL_MEMORY_SIZE = 0x40000000; // 1G

        BOOL AWESetLockPagesPrivilege(HANDLE hProcess, BOOL Enable)
        {
            HANDLE Token = NULL;
            BOOL Result = FALSE;
            TOKEN_PRIVILEGES Info = {0};

            // 打开令牌
            Result = OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &Token);
            if (!Result)
                return FALSE;

            // 设置权限信息
            Info.PrivilegeCount = 1;
            Info.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

            // 获得锁定内存权限的ID5H
            Result = LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &(Info.Privileges[0].Luid));
            if (!Result) {
                CloseHandle(Token);
                return FALSE;
            }

            // 调整权限
            Result = AdjustTokenPrivileges(Token, FALSE, (PTOKEN_PRIVILEGES)&Info,0, NULL, NULL);
            if ((!Result) || (GetLastError() != ERROR_SUCCESS)) {
                CloseHandle(Token);
                return FALSE;
            }

            // 成功返回
            CloseHandle(Token);
            return TRUE;
        }

        size_t AWEMemoryPool::page_per_block_ = 1; // 每块页面数
        size_t AWEMemoryPool::size_block_ = 0; // 每块页面数
        size_t AWEMemoryPool::num_physical_block_ = 1000; // 块数
        size_t AWEMemoryPool::num_virtual_block_ = 1000; // 物理内存块数
        size_t AWEMemoryPool::num_physical_block_used_ = 0; // 物理内存使用块数
        size_t AWEMemoryPool::max_physical_block_used_ = 0; // 最大物理内存使用块数
        size_t AWEMemoryPool::num_virtual_block_used_ = 0; // 虚拟内存使用块数
        size_t AWEMemoryPool::max_virtual_block_used_ = 0; // 最大虚拟内存使用块数
        void * AWEMemoryPool::page_nums_ = NULL; // 页面号数组
        void * AWEMemoryPool::virtual_addr_ = NULL; // 虚拟地址
        AWEMemoryPool::Block * AWEMemoryPool::physical_blocks_ = NULL;
        AWEMemoryPool::Block * AWEMemoryPool::virtual_blocks_ = NULL;
        AWEMemoryPool::Block * AWEMemoryPool::free_physical_blocks_ = NULL;
        AWEMemoryPool::Block AWEMemoryPool::free_virtual_blocks_ = {
            NULL, NULL, &AWEMemoryPool::free_virtual_blocks_, &AWEMemoryPool::free_virtual_blocks_
        };
        boost::mutex * AWEMemoryPool::mutex_ = NULL;

        int AWEMemoryPool::init(
            size_t size, 
            size_t num)
        {
            num_physical_block_ = num;

            HANDLE hProcess = GetCurrentProcess();

            SYSTEM_INFO sSysInfo;
            GetSystemInfo(&sSysInfo);

            page_per_block_ = (size - 1) / sSysInfo.dwPageSize + 1;
            size_block_ = page_per_block_ * sSysInfo.dwPageSize;
            num_virtual_block_ = VIRTUAL_MEMORY_SIZE / size_block_;
            page_per_block_ = page_per_block_;

            ULONG_PTR NumberOfPages = page_per_block_ * num_physical_block_;
            ULONG_PTR PFNArraySize = NumberOfPages * sizeof(ULONG_PTR);

            page_nums_ = HeapAlloc(GetProcessHeap(), 0, PFNArraySize);
            physical_blocks_ = (Block *) HeapAlloc(GetProcessHeap(), 0, num_physical_block_ * sizeof(Block));
            virtual_blocks_ = (Block *) HeapAlloc(GetProcessHeap(), 0, num_virtual_block_ * sizeof(Block));
            if (page_nums_ == NULL || physical_blocks_ == NULL || virtual_blocks_ == NULL) {
                return 1;
            }

            if (!AWESetLockPagesPrivilege(hProcess, TRUE)) {
                return 2;
            }

            ULONG_PTR NumberOfPagesInitial = NumberOfPages;
            if (!AllocateUserPhysicalPages(hProcess, &NumberOfPages, (ULONG_PTR *)page_nums_)) {
                return 3;
            }

            if (NumberOfPagesInitial != NumberOfPages) {
                return 4;
            }

            virtual_addr_ = VirtualAlloc(NULL, size_block_ * num_virtual_block_, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE);
            if (virtual_addr_ == NULL) {
                return 5;
            }

            ULONG_PTR * p_page_nums_ = (ULONG_PTR *)page_nums_;
            for (size_t i = 0; i < num_physical_block_; i++, p_page_nums_ += page_per_block_) {
                physical_blocks_[i].pfns = p_page_nums_;
                physical_blocks_[i].addr = NULL;
                physical_blocks_[i].next = &physical_blocks_[i + 1];
                physical_blocks_[i].ref = 0;
            }
            free_physical_blocks_ = &physical_blocks_[0];
            physical_blocks_[num_physical_block_ - 1].next = NULL;

            char * p_virtual = (char *)virtual_addr_;
            for (size_t i = 0; i < num_virtual_block_; i++, p_virtual += size_block_) {
                virtual_blocks_[i].pfns = NULL;
                virtual_blocks_[i].addr = p_virtual;
                virtual_blocks_[i].next = &virtual_blocks_[i + 1];
                virtual_blocks_[i].prev = &virtual_blocks_[i - 1];
            }
            free_virtual_blocks_.next = &virtual_blocks_[0];
            virtual_blocks_[0].prev = &free_virtual_blocks_;
            free_virtual_blocks_.prev = &virtual_blocks_[num_virtual_block_ - 1];
            virtual_blocks_[num_virtual_block_ - 1].next = &free_virtual_blocks_;

            mutex_ = new boost::mutex;

            return 0;
        }

        void AWEMemoryPool::term(void)
        {

        }

        struct unref_block_list
        {
            unref_block_list * next;
            unref_block_list * prev;
        };

        AWEMemoryPool::WeakPtr AWEMemoryPool::alloc()
        {
            boost::mutex::scoped_lock lock(*mutex_);

            if (!free_physical_blocks_)
                return (NULL);

            Block * blk = free_physical_blocks_;
            free_physical_blocks_ = free_physical_blocks_->next;

            blk->next = NULL;
            blk->hold = 1;

            num_physical_block_used_++;
            if (num_physical_block_used_ > max_physical_block_used_)
                max_physical_block_used_ = num_physical_block_used_;

            return (blk);
        }

        void AWEMemoryPool::free(
            WeakPtr & ptr)
        {
            unhold(ptr.blk_);
            ptr.blk_ = NULL;
        }

        void AWEMemoryPool::hold(
            Block * blk)
        {
            assert(blk);

            boost::mutex::scoped_lock lock(*mutex_);

            assert(blk->hold > 0);

            blk->hold++;
        }

        void AWEMemoryPool::unhold(
            Block * blk)
        {
            assert(blk);

            boost::mutex::scoped_lock lock(*mutex_);

            assert(blk->hold > 0);

            blk->hold--;

            if (blk->hold == 0) {
                assert(blk->ref == 0);
                blk->next = free_physical_blocks_;
                free_physical_blocks_ = blk;
                num_physical_block_used_--;
            }
        }

        void AWEMemoryPool::ref(Block * blk)
        {
            assert(blk);

            boost::mutex::scoped_lock lock(*mutex_);

            assert(blk->hold > 0);

            if (blk->ref == 0) {
                // 寻找
                Block * vblk = NULL;
                if (blk->addr) {
                    // 如果映射还没有释放，检查关联是否仍然匹配
                    vblk = virtual_blocks_ + (((size_t)blk->addr - (size_t)virtual_addr_) / size_block_);
                    if (vblk->pfns != blk->pfns) {
                        blk->addr = NULL;
                        vblk = NULL;
                    } else {
                        // 从free链表中取出
                        vblk->next->prev = vblk->prev;
                        vblk->prev->next = vblk->next;
                        vblk->prev = vblk->next = NULL;
                    }
                }
                if (vblk == NULL && free_virtual_blocks_.next != &free_virtual_blocks_) {
                    vblk = free_virtual_blocks_.next;
                    if (MapUserPhysicalPages(vblk->addr, page_per_block_, (PULONG_PTR)blk->pfns)) {
                        // 从free链表中取出
                        vblk->next->prev = &free_virtual_blocks_;
                        free_virtual_blocks_.next = vblk->next;
                        vblk->prev = vblk->next = NULL;
                        // 建立关联
                        vblk->pfns = blk->pfns;
                        blk->addr = vblk->addr;
                    } else {
                        vblk = NULL;
                    }
                }

                if (vblk) {
                    num_virtual_block_used_++;
                    if (num_virtual_block_used_ > max_virtual_block_used_)
                        max_virtual_block_used_ = num_virtual_block_used_;
                }
            }

            assert(blk->addr != NULL);

            if (blk->addr)
                blk->ref++;
        }

        void AWEMemoryPool::unref(Block * blk)
        {
            boost::mutex::scoped_lock lock(*mutex_);

            assert(blk->ref > 0);

            if (blk->ref == 0)
                return;

            blk->ref--;

            if (blk->ref == 0) {
                Block * vblk = virtual_blocks_ + (((size_t)blk->addr - (size_t)virtual_addr_) / size_block_);
                vblk->prev = free_virtual_blocks_.prev;
                vblk->next = &free_virtual_blocks_;
                free_virtual_blocks_.prev->next = vblk;
                free_virtual_blocks_.prev = vblk;
                num_virtual_block_used_--;
            }
        }

    } // namespace memory
} // namespace framework

#endif // _WIN32 && _WIN32_WINNT >= 0x0500
