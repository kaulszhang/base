// SocketEmulation.cpp

#include "WsaContext.h"

#include <deque>
#include <mutex>
#include <condition_variable>

#include <assert.h>

namespace winapi
{

    class iocp_t
        : public wsa_handle_t<iocp_t, 2>
    {
    public:
        iocp_t();

        ~iocp_t();

    public:
        void push(
            ULONG_PTR lpCompletionKey, 
            LPOVERLAPPED lpOverlapped, 
            ULONG_PTR Internal, 
            DWORD dwNumberOfBytesTransferred);

        BOOL pop(
            _Out_  LPDWORD lpNumberOfBytes,
            _Out_  PULONG_PTR lpCompletionKey,
            _Out_  LPOVERLAPPED *lpOverlapped,
            _In_   DWORD dwMilliseconds);

        BOOL close();

    private:
        std::mutex mutex_;
        // std::condition_variable cond_; // there is a bug on wait_for
        HANDLE event_;
        std::deque<OVERLAPPED_ENTRY> overlaps_;
    };

}
