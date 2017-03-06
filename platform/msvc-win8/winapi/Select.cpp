// Iocp.cpp

#include <Windows.h>

#define WINAPI_DECL     __declspec(dllexport)

#include "Select.h"
#include "WsaContext.h"
#include "Socket.h"

namespace winapi
{

    select_t::select_t()
    {
        FD_ZERO(&sets_[0]);
        FD_ZERO(&sets_[1]);
        FD_ZERO(&sets_[2]);
    }

    void select_t::set(
        int t, 
        int s)
    {
        FD_SET(s, &sets_[t]);
        ++count_;
        SetEvent(hEvent_);
    }

    int select_t::select(
        _In_     int nfds,
        _Inout_  fd_set *readfds,
        _Inout_  fd_set *writefds,
        _Inout_  fd_set *exceptfds,
        _In_     const struct timeval *timeout)
    {
        hEvent_ = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);

        int ec = 0;

        if (readfds)
            attach(0, readfds, &ec);
        if (ec == 0 && writefds)
            attach(1, writefds, &ec);
        if (ec == 0 && exceptfds)
            attach(2, exceptfds, &ec);

        DWORD ret = 0;
        
        if (ec == 0) {
            DWORD time = timeout ? (timeout->tv_sec * 1000 + timeout->tv_usec / 1000) : INFINITE;
            WaitForSingleObjectEx(hEvent_, time, FALSE);
        }

        if (readfds)
            detach(0, readfds);
        if (writefds)
            detach(1, writefds);
        if (exceptfds)
            detach(2, exceptfds);

        CloseHandle(hEvent_);

        if (ec != 0) {
            SetLastError(ec);
            return -1;
        } else if (ret == WAIT_OBJECT_0 || ret == WAIT_TIMEOUT) {
            if (readfds)
                *readfds = sets_[0];
            if (writefds)
                *writefds = sets_[1];
            if (exceptfds)
                *exceptfds = sets_[2];
            return count_;
        } else {
            return -1;
        }
    }

    void select_t::attach(
        _In_ int t, 
        _In_  fd_set *fds, 
        _Out_ int * ec)
    {
        wsa_context * context = &g_wsa_context();
        for (int i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(i, fds)) {
                socket_t::pointer_t socket = context->get<socket_t>(i);
                if (socket == NULL) {
                    *ec = WSAENOTSOCK;
                    break;
                }
                socket->select_attach(t, this);
            }
        }
    }

    void select_t::detach(
        _In_  int t, 
        _In_  fd_set *fds)
    {
        wsa_context * context = &g_wsa_context();
        for (int i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(i, fds)) {
                socket_t::pointer_t socket = context->get<socket_t>(i);
                if (socket != NULL) {
                    socket->select_detach(t, this);
                }
            }
        }
    }

}
