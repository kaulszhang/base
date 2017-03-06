// SocketEmulation.cpp

#include <Windows.h>

#define WINAPI_DECL     __declspec(dllexport)

#include "SocketEmulation.h"
#include "WsaContext.h"
#include "Socket.h"
#include "Iocp.h"
#include "Select.h"
#include "AsyncHelper.h"
#include "TlsPointer.h"
using namespace winapi;

#include <assert.h>

namespace winapi
{
    static inline u_short rotate(
        u_short v)
    {
        return v >> 8 | v << 8;
    }

    static inline u_long rotate(
        u_long v)
    {
        // 1 2 3 4
        // 2 3 4 1
        // 4 1 2 3
        return (((v >> 24 | v << 8) & 0x00ff00ff) 
            | ((v >> 8 | v << 24) & 0xff00ff00));
    }
}

//extern "C"
//{

    SOCKET WINAPI_DECL socket(
        _In_  int af,
        _In_  int type,
        _In_  int protocol
        )
    {
        return WSASocket(af, type, protocol, NULL, 0, 0);
    }

    int WINAPI_DECL bind(
        _In_  SOCKET s,
        _In_  const struct sockaddr *name,
        _In_  int namelen
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->bind(name, namelen);
    }

    int WINAPI_DECL connect(
        _In_  SOCKET s,
        _In_  const struct sockaddr *name,
        _In_  int namelen
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->connect(name, namelen);
    }

    int WINAPI_DECL listen(
        _In_  SOCKET s,
        _In_  int backlog
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->listen(backlog);
    }

    SOCKET WINAPI_DECL accept(
        _In_     SOCKET s,
        _Out_    struct sockaddr *addr,
        _Inout_  int *addrlen
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        socket_t::pointer_t socket2 = context->alloc<socket_t>();
        BOOL ret = socket->accept(socket2.get(), addr, addrlen);
        if (ret == TRUE) {
            return socket2->index;
        } else {
            context->free(socket2);
            return SOCKET_ERROR;
        }
    }

    int WINAPI_DECL getsockname(
        _In_     SOCKET s,
        _Out_    struct sockaddr *name,
        _Inout_  int *namelen
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->getsockname(name, namelen);
    }

    int WINAPI_DECL getpeername(
        _In_     SOCKET s,
        _Out_    struct sockaddr *name,
        _Inout_  int *namelen
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->getpeername(name, namelen);
    }

    int WINAPI_DECL recv(
        _In_   SOCKET s,
        _Out_  char *buf,
        _In_   int len,
        _In_   int flags
        )
    {
        WSABUF bufs = {len, buf};
        DWORD dwRecv = 0;
        DWORD dwFlags = flags;
        int ret = WSARecv(s, &bufs, 1, &dwRecv, &dwFlags, NULL, NULL);
        return ret == 0 ? dwRecv : ret;
    }

    int WINAPI_DECL recvfrom(
        _In_         SOCKET s,
        _Out_        char *buf,
        _In_         int len,
        _In_         int flags,
        _Out_        struct sockaddr *from,
        _Inout_opt_  int *fromlen
        )
    {
        WSABUF bufs = {len, buf};
        DWORD dwRecv = 0;
        DWORD dwFlags = flags;
        int ret = WSARecvFrom(s, &bufs, 1, &dwRecv, &dwFlags, from, fromlen, NULL, NULL);
        return ret == 0 ? dwRecv : ret;
    }

    int WINAPI_DECL send(
        _In_  SOCKET s,
        _In_  const char *buf,
        _In_  int len,
        _In_  int flags
        )
    {
        WSABUF bufs = {len, (char *)buf};
        DWORD dwRecv = 0;
        DWORD dwFlags = flags;
        int ret = WSASend(s, &bufs, 1, &dwRecv, dwFlags, NULL, NULL);
        return ret == 0 ? dwRecv : ret;
    }

    int WINAPI_DECL sendto(
        _In_  SOCKET s,
        _In_  const char *buf,
        _In_  int len,
        _In_  int flags,
        _In_  const struct sockaddr *to,
        _In_  int tolen
        )
    {
        WSABUF bufs = {len, (char *)buf};
        DWORD dwRecv = 0;
        DWORD dwFlags = flags;
        int ret = WSASendTo(s, &bufs, 1, &dwRecv, dwFlags, to, tolen, NULL, NULL);
        return ret == 0 ? dwRecv : ret;
    }

    int WINAPI_DECL shutdown(
        _In_  SOCKET s,
        _In_  int how
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->shutdown(how);
    }

    int WINAPI_DECL setsockopt(
        _In_  SOCKET s,
        _In_  int level,
        _In_  int optname,
        _In_  const char *optval,
        _In_  int optlen
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->setsockopt(level, optname, optval, optlen);
    }

    int WINAPI_DECL getsockopt(
        _In_     SOCKET s,
        _In_     int level,
        _In_     int optname,
        _Out_    char *optval,
        _Inout_  int *optlen
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->getsockopt(level, optname, optval, optlen);
    }

    int WINAPI_DECL ioctlsocket(
        _In_     SOCKET s,
        _In_     long cmd,
        _Inout_  u_long *argp
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->ioctlsocket(cmd, argp);
    }

    int WINAPI_DECL closesocket(
        _In_  SOCKET s
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
		int ret = WSAENOTSOCK;
		if (socket) {
			ret = socket->close();
			context->free(socket);
		}
        return ret;
    }

    int WINAPI_DECL select(
        _In_     int nfds,
        _Inout_  fd_set *readfds,
        _Inout_  fd_set *writefds,
        _Inout_  fd_set *exceptfds,
        _In_     const struct timeval *timeout
        )
    {
        select_t selector;
        return selector.select(
            nfds, 
            readfds, 
            writefds, 
            exceptfds, 
            timeout);
    }

    int WINAPI_DECL gethostname(
        _Out_  char *name,
        _In_   int namelen
        )
    {
        return 0;
    }

    int WINAPI_DECL getadapters(
        _Out_  char *adapters,
        _In_   int len
        )
    {
        char *buf = adapters;
        Windows::Foundation::Collections::IVectorView<Windows::Networking::HostName ^> ^ vec = 
            Windows::Networking::Connectivity::NetworkInformation::GetHostNames();
        Platform::String ^ svc = ref new Platform::String(L"0");
        for (unsigned int i = 0; i < vec->Size; i++) {
            Windows::Networking::HostName ^ host = vec->GetAt(i);
            int addrlen = host->RawName->Length() + 1;
            if ((int)sizeof(adapter) + addrlen <= len) {
                adapter * a = (adapter *)buf;
                a->len = sizeof(adapter) + addrlen;
                if (host->IPInformation && host->IPInformation->NetworkAdapter) {
                    a->lan_type = host->IPInformation->NetworkAdapter->IanaInterfaceType;
                    a->id = host->IPInformation->NetworkAdapter->NetworkAdapterId;
                }
                char * addr = (char *)(a + 1);
                WideCharToMultiByte(CP_ACP, 0, host->RawName->Data(), -1, addr, addrlen, NULL, FALSE);
                buf += a->len;
                len -= a->len;
            }
        }
        return buf - adapters;
    }

    struct tls_hostent_t
    {
        tls_hostent_t()
        {
            hostent = new struct hostent;
            hostent->h_name = NULL;
            hostent->h_aliases = NULL;
            hostent->h_addr_list = hostent_addr_list;
            memset(hostent_addr_list, 0, sizeof(hostent_addr_list));
        }

        ~tls_hostent_t()
        {
            delete hostent;
        }

        struct hostent * hostent;
        char * hostent_addr_list[4];
        char hostent_addr_chars[256];
    };

    static tls_pointer<tls_hostent_t> tls_hostent;

    WINAPI_DECL struct hostent* gethostbyname(
        _In_  const char *name
        )
    {
        tls_hostent_t * tls_data = tls_hostent.get();
        struct hostent* hostent = tls_data->hostent;
        hostent->h_addrtype = AF_INET;
        hostent->h_length = 4;
        hostent->h_addr = tls_data->hostent_addr_chars;
        *(unsigned long *)hostent->h_addr = inet_addr(name);
        if (*(unsigned long *)hostent->h_addr != INADDR_NONE) {
            hostent->h_addr_list[1] = NULL;
        } else {
            WCHAR wstr[256];
            MultiByteToWideChar(CP_ACP, 0, name, -1, wstr, 256);
            Windows::Foundation::Collections::IVectorView<Windows::Networking::EndpointPair ^> ^ vec;
            int ec = wait_operation(Windows::Networking::Sockets::DatagramSocket::GetEndpointPairsAsync(
                ref new Windows::Networking::HostName(ref new Platform::String(wstr)), ref new Platform::String(L"80")), vec);
            if (ec) {
                WSASetLastError(ec);
                return NULL;
            }
            unsigned int j = 0;
            unsigned long * in_addr = (unsigned long *)tls_data->hostent_addr_chars;
            for (unsigned int i = 0; i < vec->Size && j < 3; ++i) {
                Windows::Networking::EndpointPair ^ ep = vec->GetAt(i);
                sockaddr_in addr;
                int len = sizeof(addr);
                if (socket_t::host_name_port_to_sockaddr(AF_INET, (sockaddr *)&addr, &len, ep->RemoteHostName, ep->RemoteServiceName) == 0) {
                    hostent->h_addr_list[j] = (char *)in_addr;
                    *in_addr = addr.sin_addr.s_addr;
                    ++j;
                    ++in_addr;
                }
            }
        }
        return hostent;
    }

    WINAPI_DECL struct hostent* gethostbyaddr(
        _In_  const char *addr,
        _In_  int len,
        _In_  int type
        )
    {
        return NULL;
    }

    WINAPI_DECL struct servent* getservbyname(
        _In_  const char *name,
        _In_  const char *proto
        )
    {
        return NULL;
    }

    WINAPI_DECL struct servent* getservbyport(
        _In_  int port,
        _In_  const char *proto
        )
    {
        return NULL;
    }

    unsigned long WINAPI_DECL inet_addr(
        _In_  const char *cp
        )
    {
        sockaddr_in addr;
        int len = sizeof(addr);
        return WSAStringToAddressA((char *)cp, AF_INET, NULL, (sockaddr *)&addr, &len) == 0 ? addr.sin_addr.s_addr : INADDR_NONE;
    }

    WINAPI_DECL char* inet_ntoa(
        _In_  struct   in_addr in
        )
    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = 0;
        addr.sin_addr = in;
        static char str[256];
        DWORD len = sizeof(str);
        return WSAAddressToStringA((sockaddr *)&addr, sizeof(addr), NULL, str, &len) == 0 ? str : NULL;
    }

    int WINAPI_DECL __WSAFDIsSet(
        SOCKET fd, 
        fd_set FAR * set)
    {
        return FD_ISSET(fd, set);
    }

    int WINAPI_DECL WSAStartup(
        _In_   WORD wVersionRequested,
        _Out_  LPWSADATA lpWSAData
        )
    {
        g_wsa_context();
        return 0;
    }

    int WINAPI_DECL WSACleanup(void)
    {
        return 0;
    }

    u_short WINAPI_DECL htons(
        _In_  u_short hostshort
        )
    {
        return rotate(hostshort);
    }

    u_long WINAPI_DECL htonl(
        _In_  u_long hostlong
        )
    {
        return rotate(hostlong);
    }

    u_short WINAPI_DECL ntohs(
        _In_  u_short netshort
        )
    {
        return rotate(netshort);
    }

    u_long WINAPI_DECL ntohl(
        _In_  u_long netlong
        )
    {
        return rotate(netlong);
    }

    SOCKET WINAPI_DECL WSASocket(
        _In_  int af,
        _In_  int type,
        _In_  int protocol,
        _In_  LPWSAPROTOCOL_INFO lpProtocolInfo,
        _In_  GROUP g,
        _In_  DWORD dwFlags
        )
    {
        wsa_context & context = g_wsa_context();
        socket_t::pointer_t socket = context.alloc<socket_t>();
        socket->create(af, type, protocol);
        return socket->index;
    }

    BOOL WINAPI_DECL ConnectEx(
        _In_   SOCKET s,
        _In_   const struct sockaddr *lpTo,
        _In_   int iToLen,
        _In_   LPOVERLAPPED lpOverlapped
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->connect_ex(lpTo, iToLen, lpOverlapped);
    }

    BOOL WINAPI_DECL AcceptEx(
        _In_   SOCKET sListenSocket,
        _In_   SOCKET sAcceptSocket,
        _In_   PVOID lpOutputBuffer,
        _In_   DWORD dwReceiveDataLength,
        _In_   DWORD dwLocalAddressLength,
        _In_   DWORD dwRemoteAddressLength,
        _Out_  LPDWORD lpdwBytesReceived,
        _In_   LPOVERLAPPED lpOverlapped
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(sListenSocket);
        socket_t::pointer_t socket2 = context->get<socket_t>(sAcceptSocket);
        return socket->accept_ex(socket2.get(), 
            lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived, 
            lpOverlapped);
    }

    void WINAPI_DECL GetAcceptExSockaddrs(
        _In_   PVOID lpOutputBuffer,
        _In_   DWORD dwReceiveDataLength,
        _In_   DWORD dwLocalAddressLength,
        _In_   DWORD dwRemoteAddressLength,
        _Out_  LPSOCKADDR *LocalSockaddr,
        _Out_  LPINT LocalSockaddrLength,
        _Out_  LPSOCKADDR *RemoteSockaddr,
        _Out_  LPINT RemoteSockaddrLength
        )
    {
        socket_t::get_accept_ex_addrs(
            lpOutputBuffer,
            dwReceiveDataLength,
            dwLocalAddressLength,
            dwRemoteAddressLength,
            LocalSockaddr,
            LocalSockaddrLength,
            RemoteSockaddr,
            RemoteSockaddrLength);
    }

    int WINAPI_DECL WSARecv(
        _In_     SOCKET s,
        _Inout_  LPWSABUF lpBuffers,
        _In_     DWORD dwBufferCount,
        _Out_    LPDWORD lpNumberOfBytesRecvd,
        _Inout_  LPDWORD lpFlags,
        _In_     LPWSAOVERLAPPED lpOverlapped,
        _In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->recv_ex(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);
    }

    int WINAPI_DECL WSARecvFrom(
        _In_     SOCKET s,
        _Inout_  LPWSABUF lpBuffers,
        _In_     DWORD dwBufferCount,
        _Out_    LPDWORD lpNumberOfBytesRecvd,
        _Inout_  LPDWORD lpFlags,
        _Out_    struct sockaddr *lpFrom,
        _Inout_  LPINT lpFromlen,
        _In_     LPWSAOVERLAPPED lpOverlapped,
        _In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->recv_from_ex(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, 
            lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine);
    }

    int WINAPI_DECL WSASend(
        _In_   SOCKET s,
        _In_   LPWSABUF lpBuffers,
        _In_   DWORD dwBufferCount,
        _Out_  LPDWORD lpNumberOfBytesSent,
        _In_   DWORD dwFlags,
        _In_   LPWSAOVERLAPPED lpOverlapped,
        _In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->send_ex(lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
    }

    int WINAPI_DECL WSASendTo(
        _In_   SOCKET s,
        _In_   LPWSABUF lpBuffers,
        _In_   DWORD dwBufferCount,
        _Out_  LPDWORD lpNumberOfBytesSent,
        _In_   DWORD dwFlags,
        _In_   const struct sockaddr *lpTo,
        _In_   int iToLen,
        _In_   LPWSAOVERLAPPED lpOverlapped,
        _In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>(s);
        return socket->send_to_ex(lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, 
            lpTo, iToLen, lpOverlapped, lpCompletionRoutine);
    }

    INT WINAPI_DECL WSAAddressToStringA(
        _In_      LPSOCKADDR lpsaAddress,
        _In_      DWORD dwAddressLength,
        _In_opt_  LPWSAPROTOCOL_INFO lpProtocolInfo,
        _Inout_   LPSTR lpszAddressString,
        _Inout_   LPDWORD lpdwAddressStringLength
        )
    {
        if (lpsaAddress->sa_family == AF_INET) {
            LPSTR p = lpszAddressString;
            sockaddr_in * addr = (sockaddr_in *)lpsaAddress;
            for (int i = 0; i < 4; ++i) {
                UCHAR c = ((UCHAR *)(&addr->sin_addr))[i];
                if (c < 10) {
                    *p++ = '0' + c;
                } else if (c < 100) {
                    *p++ = '0' + c / 10;
                    *p++ = '0' + c % 10;
                } else {
                    *p++ = '0' + c / 100;
                    *p++ = '0' + (c % 100) / 10;
                    *p++ = '0' + c % 10;
                }
                *p++ = '.';
            }
            *(p - 1) = '\0';
            return p - lpszAddressString;
        } else {
            WSASetLastError(WSAEINVAL);
            return SOCKET_ERROR;
        }
    }

    INT WINAPI_DECL WSAAddressToStringW(
        _In_      LPSOCKADDR lpsaAddress,
        _In_      DWORD dwAddressLength,
        _In_opt_  LPWSAPROTOCOL_INFO lpProtocolInfo,
        _Inout_   LPWSTR lpszAddressString,
        _Inout_   LPDWORD lpdwAddressStringLength
        )
    {
        if (lpsaAddress->sa_family == AF_INET) {
            LPWSTR p = lpszAddressString;
            sockaddr_in * addr = (sockaddr_in *)lpsaAddress;
            for (int i = 0; i < 4; ++i) {
                UCHAR c = ((UCHAR *)(&addr->sin_addr))[i];
                if (c < 10) {
                    *p++ = L'0' + c;
                } else if (c < 100) {
                    *p++ = L'0' + c / 10;
                    *p++ = L'0' + c % 10;
                } else {
                    *p++ = L'0' + c / 100;
                    *p++ = L'0' + (c % 100) / 10;
                    *p++ = L'0' + c % 10;
                }
                *p++ = L'.';
            }
            *(p - 1) = L'\0';
            return p - lpszAddressString;
        } else {
            WSASetLastError(WSAEINVAL);
            return SOCKET_ERROR;
        }
    }

    INT WINAPI_DECL WSAStringToAddressA(
        _In_      LPSTR AddressString,
        _In_      INT AddressFamily,
        _In_opt_  LPWSAPROTOCOL_INFO lpProtocolInfo,
        _Out_     LPSOCKADDR lpAddress,
        _Inout_   LPINT lpAddressLength
        )
    {
        if (AddressFamily == AF_INET) {
            LPSTR p = AddressString;
            sockaddr_in * addr = (sockaddr_in *)lpAddress;
            addr->sin_family = AF_INET;
            for (int i = 0; i < 4; ++i) {
                UCHAR & c = ((UCHAR *)(&addr->sin_addr))[i];
                c = 0;
                while (*p && *p != '.') {
                    if (*p < '0' || *p > '9') {
                        WSASetLastError(WSAEINVAL);
                        return SOCKET_ERROR;
                    }
                    c = c * 10 + (*p - '0');
                    ++p;
                }
                if (*p == '.') {
                    ++p;
                } else if (i < 3) {
                    WSASetLastError(WSAEINVAL);
                    return SOCKET_ERROR;
                }
            }
            if (*p != 0) {
                WSASetLastError(WSAEINVAL);
                return SOCKET_ERROR;
            }
            if (lpAddressLength)
                *lpAddressLength = sizeof(sockaddr_in);
            return 0;
        } else {
            WSASetLastError(WSAEINVAL);
            return SOCKET_ERROR;
        }
    }

    INT WINAPI_DECL WSAStringToAddressW(
        _In_      LPWSTR AddressString,
        _In_      INT AddressFamily,
        _In_opt_  LPWSAPROTOCOL_INFO lpProtocolInfo,
        _Out_     LPSOCKADDR lpAddress,
        _Inout_   LPINT lpAddressLength
        )
    {
        if (AddressFamily == AF_INET) {
            if (*lpAddressLength < sizeof(sockaddr_in)) {
                WSASetLastError(WSAEFAULT);
                return SOCKET_ERROR;
            }
            *lpAddressLength = sizeof(sockaddr_in);
            LPWSTR p = AddressString;
            sockaddr_in * addr = (sockaddr_in *)lpAddress;
            addr->sin_family = AF_INET;
            for (int i = 0; i < 4; ++i) {
                UCHAR & c = ((UCHAR *)(&addr->sin_addr))[i];
                c = 0;
                while (*p && *p != L'.') {
                    if (*p < L'0' || *p > L'9') {
                        WSASetLastError(WSAEINVAL);
                        return SOCKET_ERROR;
                    }
                    c = c * 10 + (*p - L'0');
                    ++p;
                }
                if (*p == L'.')
                    ++p;
            }
            return 0;
        } else {
            WSASetLastError(WSAEINVAL);
            return SOCKET_ERROR;
        }
    }

    void WINAPI_DECL WSASetLastError(
        _In_  int iError
        )
    {
        SetLastError(iError);
    }

    int WINAPI_DECL WSAGetLastError(void)
    {
        return GetLastError();
    }

    HANDLE WINAPI_DECL CreateIoCompletionPort(
        _In_      HANDLE FileHandle,
        _In_opt_  HANDLE ExistingCompletionPort,
        _In_      ULONG_PTR CompletionKey,
        _In_      DWORD NumberOfConcurrentThreads
        )
    {
        if (FileHandle == INVALID_HANDLE_VALUE) {
            assert(ExistingCompletionPort == NULL);
            wsa_context * context = &g_wsa_context();
            iocp_t::pointer_t iocp = context->alloc<iocp_t>();
            (void)NumberOfConcurrentThreads; // no use
            return (HANDLE)iocp->index;
        } else {
            wsa_context * context = &g_wsa_context();
            iocp_t::pointer_t iocp = context->get<iocp_t>((size_t)ExistingCompletionPort);
            socket_t::pointer_t socket = context->get<socket_t>((size_t)FileHandle);
            socket->attach_iocp(iocp.get(), CompletionKey);
            return ExistingCompletionPort;
        }
    }

    BOOL WINAPI_DECL GetQueuedCompletionStatus(
        _In_   HANDLE CompletionPort,
        _Out_  LPDWORD lpNumberOfBytes,
        _Out_  PULONG_PTR lpCompletionKey,
        _Out_  LPOVERLAPPED *lpOverlapped,
        _In_   DWORD dwMilliseconds
        )
    {
        wsa_context * context = &g_wsa_context();
        iocp_t::pointer_t iocp = context->get<iocp_t>((size_t)CompletionPort);
        return iocp->pop(lpNumberOfBytes, lpCompletionKey, lpOverlapped, dwMilliseconds);
    }

    BOOL WINAPI_DECL CancelIo(
        _In_   HANDLE hFile
        )
    {
        return CancelIoEx(hFile, NULL);
    }

    BOOL WINAPI_DECL CancelIoEx(
        _In_      HANDLE hFile,
        _In_opt_  LPOVERLAPPED lpOverlapped
        )
    {
        wsa_context * context = &g_wsa_context();
        socket_t::pointer_t socket = context->get<socket_t>((size_t)hFile);
        return socket->cancel_io(lpOverlapped);
    }

    BOOL WINAPI_DECL PostQueuedCompletionStatus(
        _In_      HANDLE CompletionPort,
        _In_      DWORD dwNumberOfBytesTransferred,
        _In_      ULONG_PTR dwCompletionKey,
        _In_opt_  LPOVERLAPPED lpOverlapped
        )
    {
        wsa_context * context = &g_wsa_context();
        iocp_t::pointer_t iocp = context->get<iocp_t>((size_t)CompletionPort);
        if (lpOverlapped)
            lpOverlapped->Internal = 1;
        iocp->push(dwCompletionKey, lpOverlapped, 0, dwNumberOfBytesTransferred);
        return TRUE;
    }

    BOOL WINAPI_DECL GetOverlappedResult(
        _In_   HANDLE hFile,
        _In_   LPOVERLAPPED lpOverlapped,
        _Out_  LPDWORD lpNumberOfBytesTransferred,
        _In_   BOOL bWait
        )
    {
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    BOOL WINAPI_DECL CloseIoCompletionPort(
      _In_  HANDLE hObject
    )
    {
        wsa_context * context = &g_wsa_context();
        iocp_t::pointer_t iocp = context->get<iocp_t>((size_t)hObject);
        BOOL ret = iocp->close();
        context->free(iocp);
        return ret;
    }

//}
