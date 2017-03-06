// Iocp.cpp

#include <Windows.h>

#define WINAPI_DECL     __declspec(dllexport)

#include "Iocp.h"

namespace winapi
{

    class set_last_error
    {
    public:
        set_last_error()
            : ec_(0)
        {
        }
        
        void set(
            int ec)
        {
            ec_ = ec;
        }

        ~set_last_error()
        {
            SetLastError(ec_);
        }

    private:
        int ec_;
    };

    iocp_t::iocp_t()
    {
        event_ = CreateEventExW(NULL, NULL, 0, EVENT_ALL_ACCESS);
        assert(event_ != NULL);
    }

    iocp_t::~iocp_t()
    {
        CloseHandle(event_);
    }

    void iocp_t::push(
        ULONG_PTR lpCompletionKey, 
        LPOVERLAPPED lpOverlapped, 
        ULONG_PTR Internal, 
        DWORD dwNumberOfBytesTransferred)
    {
        std::unique_lock<std::mutex> lc(mutex_);
        OVERLAPPED_ENTRY entry = {lpCompletionKey, lpOverlapped, Internal, dwNumberOfBytesTransferred};
        overlaps_.push_back(entry);
        if (overlaps_.size() == 1) {
            SetEvent(event_);
        }
    }

    BOOL iocp_t::pop(
        _Out_  LPDWORD lpNumberOfBytes,
        _Out_  PULONG_PTR lpCompletionKey,
        _Out_  LPOVERLAPPED *lpOverlapped,
        _In_   DWORD dwMilliseconds)
    {
        set_last_error le;
        std::unique_lock<std::mutex> lc(mutex_);

        while (overlaps_.empty()) {
            lc.unlock();
            DWORD dw = WaitForSingleObjectEx(event_, dwMilliseconds, FALSE);
            if (dw == WAIT_OBJECT_0) {
                lc.lock();
                continue;
            }
            if (dw == WAIT_FAILED) {
                le.set(GetLastError());
            } else {
                le.set((int)dw);
            }
            *lpOverlapped = NULL;
            return FALSE;
        }

        OVERLAPPED_ENTRY entry = overlaps_.front();
        overlaps_.pop_front();
        lc.unlock();
        *lpNumberOfBytes = entry.dwNumberOfBytesTransferred;
        *lpCompletionKey = entry.Internal ? entry.Internal : entry.lpCompletionKey;
        *lpOverlapped = entry.lpOverlapped;
        return entry.Internal ? FALSE : TRUE;
    }

    BOOL iocp_t::close()
    {
        std::unique_lock<std::mutex> lc(mutex_);
        overlaps_.clear();
        return TRUE;
    }

}
