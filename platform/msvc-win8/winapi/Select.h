// SocketEmulation.cpp

#include "SocketEmulation.h"

#include <atomic>
#include <mutex>
#include <condition_variable>

#include <assert.h>

namespace winapi
{

    class select_t
    {
    public:
        select_t();

    public:
        void set(
            int t, 
            int s);

        int select(
            _In_     int nfds,
            _Inout_  fd_set *readfds,
            _Inout_  fd_set *writefds,
            _Inout_  fd_set *exceptfds,
            _In_     const struct timeval *timeout
        );

    private:
        void attach(
            _In_  int t, 
            _In_  fd_set *readfds, 
            _Out_ int * ec);

        void detach(
            _In_  int t, 
            _In_  fd_set *readfds);

    private:
        fd_set sets_[3];
        std::atomic_uint32_t count_;
        HANDLE hEvent_;
    };

}
