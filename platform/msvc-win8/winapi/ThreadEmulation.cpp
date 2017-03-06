// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include <Windows.h>

#define WINAPI_DECL     __declspec(dllexport)

#include "ThreadEmulation.h"
#include "Charset.h"
using namespace winapi;

#include <assert.h>
#include <vector>
#include <set>
#include <map>
#include <mutex>

using namespace std;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;

//extern "C"
//{
    
    // Thread local storage.
    typedef vector<void*> ThreadLocalData;

    static __declspec(thread) ThreadLocalData* currentThreadData = nullptr;

    struct TlsData
    {
        TlsData()
            : nextTlsIndex(0)
        {
        }

        DWORD nextTlsIndex;
        vector<DWORD> freeTlsIndices;
        mutex tlsAllocationLock;
        set<ThreadLocalData*> allThreadData;
    };

    static TlsData & tls_data()
    {
        static TlsData tls;
        return tls;
    }

    // Stored data for Thread.
    struct ThreadInfo
    {
        ThreadInfo(
            LPTHREAD_START_ROUTINE lpStartAddress, 
            LPVOID lpParameter, 
            DWORD dwCreationFlags, 
            HANDLE threadHandle, 
            HANDLE completionEvent)
            : lpStartAddress(lpStartAddress)
            , lpParameter(lpParameter)
            , dwCreationFlags(dwCreationFlags)
            , threadHandle(threadHandle)
            , completionEvent(completionEvent)
            , nPriority(0)
        {
            static DWORD gid  = 0;
            dwThreadId = ++gid;
        }

        LPTHREAD_START_ROUTINE lpStartAddress;
        LPVOID lpParameter;
        DWORD dwCreationFlags;
        HANDLE threadHandle;
        HANDLE completionEvent;
        int nPriority;
        DWORD dwThreadId;
    };

    struct ThreadData
    {
        map<HANDLE, ThreadInfo *> threads;
        mutex threadsLock;
    };
    
    static ThreadData & thread_data()
    {
        static ThreadData th;
        return th;
    }

    static DWORD my_tls()
    {
        static DWORD tls = TlsAlloc();
        return tls;
    }

    // Converts a Win32 thread priority to WinRT format.
    static WorkItemPriority GetWorkItemPriority(
        int nPriority)
    {
        if (nPriority < 0)
            return WorkItemPriority::Low;
        else if (nPriority > 0)
            return WorkItemPriority::High;
        else
            return WorkItemPriority::Normal;
    }

    // Helper shared between CreateThread and ResumeThread.
    static void StartThread(ThreadInfo * info)
    {
        auto workItemHandler = ref new WorkItemHandler([=](IAsyncAction^)
        {
            TlsSetValue(my_tls(), info);
            // Run the user callback.
            try
            {
                info->lpStartAddress(info->lpParameter);
            }
            catch (...) { }

            // Clean up any TLS allocations made by this thread.
            TlsShutdown();

            // Signal that the thread has completed.
            SetEvent(info->completionEvent);
            CloseHandle(info->completionEvent);

            {
                lock_guard<mutex> lock(thread_data().threadsLock);
                thread_data().threads.erase(info->threadHandle);
            }
        }, CallbackContext::Any);

        ThreadPool::RunAsync(workItemHandler, GetWorkItemPriority(info->nPriority), WorkItemOptions::TimeSliced);
    }

    HANDLE WINAPI_DECL CreateThread(
        LPSECURITY_ATTRIBUTES unusedThreadAttributes, 
        SIZE_T unusedStackSize, 
        LPTHREAD_START_ROUTINE lpStartAddress, 
        LPVOID lpParameter, 
        DWORD dwCreationFlags, 
        LPDWORD pdwThreadId)
    {
        // Validate parameters.
        assert(unusedThreadAttributes == nullptr);
        assert(unusedStackSize == 0);
        assert((dwCreationFlags & ~CREATE_SUSPENDED) == 0);

        // Create a handle that will be signalled when the thread has completed.
        HANDLE threadHandle = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);

        if (!threadHandle)
            return nullptr;

        // Make a copy of the handle for internal use. This is necessary because
        // the caller is responsible for closing the handle returned by CreateThread,
        // and they may do that before or after the thread has finished running.
        HANDLE completionEvent;
        
        if (!DuplicateHandle(GetCurrentProcess(), threadHandle, GetCurrentProcess(), &completionEvent, 0, false, DUPLICATE_SAME_ACCESS))
        {
            CloseHandle(threadHandle);
            return nullptr;
        }

        ThreadInfo * info = new ThreadInfo(lpStartAddress, lpParameter, dwCreationFlags, threadHandle, completionEvent);
        {
            lock_guard<mutex> lock(thread_data().threadsLock);
            thread_data().threads[threadHandle] = info;
        }

        try
        {
            if (dwCreationFlags & CREATE_SUSPENDED)
            {
            }
            else
            {
                // Start the thread immediately.
                StartThread(info);
            }
    
            if (pdwThreadId) {
                *pdwThreadId = info->dwThreadId;
            }
            return threadHandle;
        }
        catch (...)
        {
            // Clean up if thread creation fails.
            CloseHandle(threadHandle);
            CloseHandle(completionEvent);

            return nullptr;
        }
    }

    DWORD WINAPI_DECL ResumeThread(
        HANDLE hThread)
    {
        lock_guard<mutex> lock(thread_data().threadsLock);

        // Look up the requested thread.
        auto threadInfo = thread_data().threads.find(hThread);

        if (threadInfo == thread_data().threads.end())
        {
            // Can only resume threads while they are in CREATE_SUSPENDED state.
            assert(false);
            return (DWORD)-1;
        }

        // Start the thread.
        try
        {
            ThreadInfo * info = threadInfo->second;

            StartThread(info);
        }
        catch (...)
        {
            return (DWORD)-1;
        }

        return 0;
    }


    BOOL WINAPI_DECL SetThreadPriority(
        HANDLE hThread, 
        int nPriority)
    {
        lock_guard<mutex> lock(thread_data().threadsLock);

        // Look up the requested thread.
        auto threadInfo = thread_data().threads.find(hThread);

        if (threadInfo == thread_data().threads.end())
        {
            // Can only set priority on threads while they are in CREATE_SUSPENDED state.
            assert(false);
            return false;
        }

        // Store the new priority.
        threadInfo->second->nPriority = nPriority;

        return true;
    }

    BOOL WINAPI_DECL TerminateThread(
        _Inout_  HANDLE hThread,
        _In_     DWORD dwExitCode
        )
    {
        return FALSE;
    }

    _Use_decl_annotations_ VOID WINAPI_DECL Sleep(DWORD dwMilliseconds)
    {
        static HANDLE singletonEvent = nullptr;

        HANDLE sleepEvent = singletonEvent;

        // Demand create the event.
        if (!sleepEvent)
        {
            sleepEvent = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);

            if (!sleepEvent)
                return;

            HANDLE previousEvent = InterlockedCompareExchangePointerRelease(&singletonEvent, sleepEvent, nullptr);
            
            if (previousEvent)
            {
                // Back out if multiple threads try to demand create at the same time.
                CloseHandle(sleepEvent);
                sleepEvent = previousEvent;
            }
        }

        // Emulate sleep by waiting with timeout on an event that is never signalled.
        WaitForSingleObjectEx(sleepEvent, dwMilliseconds, false);
    }


    DWORD WINAPI_DECL TlsAlloc()
    {
        lock_guard<mutex> lock(tls_data().tlsAllocationLock);
        
        // Can we reuse a previously freed TLS slot?
        if (!tls_data().freeTlsIndices.empty())
        {
            DWORD result = tls_data().freeTlsIndices.back();
            tls_data().freeTlsIndices.pop_back();
            return result;
        }

        // Allocate a new TLS slot.
        return tls_data().nextTlsIndex++;
    }


    _Use_decl_annotations_ BOOL WINAPI_DECL TlsFree(
        DWORD dwTlsIndex)
    {
        lock_guard<mutex> lock(tls_data().tlsAllocationLock);

        assert(dwTlsIndex < tls_data().nextTlsIndex);
        assert(find(tls_data().freeTlsIndices.begin(), tls_data().freeTlsIndices.end(), dwTlsIndex) == tls_data().freeTlsIndices.end());

        // Store this slot for reuse by TlsAlloc.
        try
        {
            tls_data().freeTlsIndices.push_back(dwTlsIndex);
        }
        catch (...)
        {
            return false;
        }

        // Zero the value for all threads that might be using this now freed slot.
        for each (auto threadData in tls_data().allThreadData)
        {
            if (threadData->size() > dwTlsIndex)
            {
                threadData->at(dwTlsIndex) = nullptr;
            }
        }

        return true;
    }


    _Use_decl_annotations_ LPVOID WINAPI_DECL TlsGetValue(DWORD dwTlsIndex)
    {
        ThreadLocalData* threadData = currentThreadData;

        if (threadData && threadData->size() > dwTlsIndex)
        {
            // Return the value of an allocated TLS slot.
            return threadData->at(dwTlsIndex);
        }
        else
        {
            // Default value for unallocated slots.
            return nullptr;
        }
    }


    _Use_decl_annotations_ BOOL WINAPI_DECL TlsSetValue(
        DWORD dwTlsIndex, 
        LPVOID lpTlsValue)
    {
        ThreadLocalData* threadData = currentThreadData;

        if (!threadData)
        {
            // First time allocation of TLS data for this thread.
            try
            {
                threadData = new ThreadLocalData(dwTlsIndex + 1, nullptr);
                
                lock_guard<mutex> lock(tls_data().tlsAllocationLock);

                tls_data().allThreadData.insert(threadData);

                currentThreadData = threadData;
            }
            catch (...)
            {
                if (threadData)
                    delete threadData;

                return false;
            }
        }
        else if (threadData->size() <= dwTlsIndex)
        {
            // This thread already has a TLS data block, but it must be expanded to fit the specified slot.
            try
            {
                lock_guard<mutex> lock(tls_data().tlsAllocationLock);

                threadData->resize(dwTlsIndex + 1, nullptr);
            }
            catch (...)
            {
                return false;
            }
        }

        // Store the new value for this slot.
        threadData->at(dwTlsIndex) = lpTlsValue;

        return true;
    }


    // Called at thread exit to clean up TLS allocations.
    void WINAPI_DECL TlsShutdown()
    {
        ThreadLocalData* threadData = currentThreadData;

        if (threadData)
        {
            {
                lock_guard<mutex> lock(tls_data().tlsAllocationLock);

                tls_data().allThreadData.erase(threadData);
            }

            currentThreadData = nullptr;

            delete threadData;
        }
    }

    void WINAPI_DECL InitializeCriticalSection(
        _Out_  LPCRITICAL_SECTION lpCriticalSection
        )
    {
        InitializeCriticalSectionEx(
            lpCriticalSection, 
            1000, 
            0);
    }

    HANDLE WINAPI_DECL CreateEventA(
        _In_opt_  LPSECURITY_ATTRIBUTES lpEventAttributes,
        _In_      BOOL bManualReset,
        _In_      BOOL bInitialState,
        _In_opt_  LPCSTR lpName
        )
    {
        charset_t charset(lpName);
        if (charset.ec()) {
            return NULL;
        }
        HANDLE hEvent = CreateEventW(
            lpEventAttributes, 
            bManualReset, 
            bInitialState, 
            charset.wstr());
        return hEvent;
    }

    HANDLE WINAPI_DECL CreateEventW(
        _In_opt_  LPSECURITY_ATTRIBUTES lpEventAttributes,
        _In_      BOOL bManualReset,
        _In_      BOOL bInitialState,
        _In_opt_  LPCWSTR lpName
        )
    {
        DWORD dwFlags = 0;
        if (bManualReset)
            dwFlags |= CREATE_EVENT_MANUAL_RESET;
        if (bInitialState)
            dwFlags |= CREATE_EVENT_INITIAL_SET;
        HANDLE hEvent = CreateEventExW(
            lpEventAttributes, 
            lpName, 
            dwFlags, 
            EVENT_ALL_ACCESS);
        return hEvent;
    }

    HANDLE WINAPI_DECL CreateMutexA(
        _In_opt_  LPSECURITY_ATTRIBUTES lpMutexAttributes,
        _In_      BOOL bInitialOwner,
        _In_opt_  LPCSTR lpName
        )
    {
        charset_t charset(lpName);
        if (charset.ec()) {
            assert(false);
            return NULL;
        }
        HANDLE hMutex = CreateMutexW(
            lpMutexAttributes, 
            bInitialOwner, 
            charset.wstr());
        return hMutex;
    }

    HANDLE WINAPI_DECL CreateMutexW(
        _In_opt_  LPSECURITY_ATTRIBUTES lpMutexAttributes,
        _In_      BOOL bInitialOwner,
        _In_opt_  LPCWSTR lpName
        )
    {
        DWORD dwFlags = 0;
        if (bInitialOwner)
            dwFlags |= CREATE_MUTEX_INITIAL_OWNER;
        HANDLE hMutex = CreateMutexExW(
            lpMutexAttributes, 
            lpName, 
            dwFlags, 
            MUTEX_ALL_ACCESS);
        return hMutex;
    }

    HANDLE WINAPI_DECL OpenMutexA(
        _In_  DWORD dwDesiredAccess,
        _In_  BOOL bInheritHandle,
        _In_  LPCSTR lpName
        )
    {
        charset_t charset(lpName);
        if (charset.ec()) {
            return NULL;
        }
        HANDLE hMutex = OpenMutexW(
            dwDesiredAccess, 
            bInheritHandle, 
            charset.wstr());
        return hMutex;
    }

    HANDLE WINAPI_DECL CreateSemaphoreA(
        _In_opt_  LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
        _In_      LONG lInitialCount,
        _In_      LONG lMaximumCount,
        _In_opt_  LPCSTR lpName
        )
    {
        charset_t charset(lpName);
        if (charset.ec()) {
            return NULL;
        }
        HANDLE hSemaphore = CreateSemaphoreW(
            lpSemaphoreAttributes, 
            lInitialCount, 
            lMaximumCount, 
            charset.wstr());
        return hSemaphore;
    }

    HANDLE WINAPI_DECL CreateSemaphoreW(
        _In_opt_  LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
        _In_      LONG lInitialCount,
        _In_      LONG lMaximumCount,
        _In_opt_  LPCWSTR lpName
        )
    {
        DWORD dwFlags = 0;
        HANDLE hSemaphore = CreateSemaphoreExW(
            lpSemaphoreAttributes, 
            lInitialCount, 
            lMaximumCount, 
            lpName, 
            dwFlags, 
            SEMAPHORE_ALL_ACCESS);
        return hSemaphore;
    }

    HANDLE WINAPI_DECL OpenSemaphoreA(
        _In_  DWORD dwDesiredAccess,
        _In_  BOOL bInheritHandle,
        _In_  LPCSTR lpName
        )
    {
        charset_t charset(lpName);
        if (charset.ec()) {
            return NULL;
        }
        HANDLE hSemaphore = OpenSemaphoreW(
            dwDesiredAccess, 
            bInheritHandle, 
            charset.wstr());
        return hSemaphore;
    }

    DWORD WINAPI_DECL WaitForSingleObject(
        _In_  HANDLE hHandle,
        _In_  DWORD dwMilliseconds
        )
    {
        return WaitForSingleObjectEx(
            hHandle, 
            dwMilliseconds, 
            FALSE);
    }

    DWORD WINAPI_DECL WaitForMultipleObjects(
        _In_  DWORD nCount,
        _In_  const HANDLE *lpHandles,
        _In_  BOOL bWaitAll,
        _In_  DWORD dwMilliseconds
        )
    {
        return WaitForMultipleObjectsEx(
            nCount, 
            lpHandles, 
            bWaitAll, 
            dwMilliseconds, 
            FALSE);
    }

    struct ThreadProxyData
    {
        typedef unsigned (__stdcall* func)(void*);
        func start_address_;
        void* arglist_;
        ThreadProxyData(func start_address,void* arglist) : start_address_(start_address), arglist_(arglist) {}
    };

    static DWORD __stdcall ThreadProxy(LPVOID args)
    {
        ThreadProxyData* data=reinterpret_cast<ThreadProxyData*>(args);
        DWORD ret=data->start_address_(data->arglist_);
        delete data;
        return ret;
    }

    uintptr_t const WINAPI_DECL _beginthreadex(void* security, unsigned stack_size, unsigned (__stdcall* start_address)(void*),
        void* arglist, unsigned initflag, unsigned* thrdaddr)
    {
        DWORD threadID;
        HANDLE hthread=CreateThread(static_cast<LPSECURITY_ATTRIBUTES>(security),stack_size,ThreadProxy,
            new ThreadProxyData(start_address,arglist),initflag,&threadID);
        if (hthread!=0)
            *thrdaddr=threadID;
        return reinterpret_cast<uintptr_t const>(hthread);
    }

//}
