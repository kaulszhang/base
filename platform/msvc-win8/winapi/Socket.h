// Socket.h

#pragma once

#include "WsaContext.h"

#include <deque>
#include <mutex>
#include <map>
#include <condition_variable>

#include <assert.h>

namespace winapi
{

    struct overlap_task
    {
        overlap_task(
            _In_   LPWSABUF lpBuffers,
            _In_   DWORD dwBufferCount,
            _In_   LPWSAOVERLAPPED lpOverlapped)
            : dwBufferCount(dwBufferCount)
            , lpOverlapped(lpOverlapped)
            , lpFrom(NULL)
            , lpFromlen(NULL)
        {
            std::copy(lpBuffers, lpBuffers + dwBufferCount, buffers);
        }

        overlap_task(
            _In_   LPWSABUF lpBuffers,
            _In_   DWORD dwBufferCount,    
            _Out_    struct sockaddr *lpFrom,
            _Inout_  LPINT lpFromlen, 
            _In_   LPWSAOVERLAPPED lpOverlapped)
            : dwBufferCount(dwBufferCount)
            , lpFrom(lpFrom)
            , lpFromlen(lpFromlen)
            , lpOverlapped(lpOverlapped)
        {
            std::copy(lpBuffers, lpBuffers + dwBufferCount, buffers);
        }

        WSABUF buffers[16];
        DWORD dwBufferCount;
        struct sockaddr *lpFrom;
        LPINT lpFromlen;
        LPWSAOVERLAPPED lpOverlapped;
    };

    class iocp_t;
    class select_t;

    class socket_t
        : public wsa_handle_t<socket_t, 1>
    {
    public:
        socket_t();

        ~socket_t();

    public:
        void create(
            _In_  int af,
            _In_  int type,
            _In_  int protocol);

        int bind(
            _In_  const struct sockaddr *name,
            _In_  int namelen);

        int connect(
            _In_  const struct sockaddr *name,
            _In_  int namelen);

        int listen(
        _In_  int backlog);

        int accept(
            _In_     socket_t * sock,
            _Out_    struct sockaddr *addr,
            _Inout_  int *addrlen);

        BOOL connect_ex(
            _In_  const struct sockaddr *name,
            _In_  int namelen,
            _In_   LPOVERLAPPED lpOverlapped);

        BOOL accept_ex(
            _In_   socket_t * sock,
            _In_   PVOID lpOutputBuffer,
            _In_   DWORD dwReceiveDataLength,
            _In_   DWORD dwLocalAddressLength,
            _In_   DWORD dwRemoteAddressLength,
            _Out_  LPDWORD lpdwBytesReceived,
            _In_   LPOVERLAPPED lpOverlapped);

        static void get_accept_ex_addrs(
            _In_   PVOID lpOutputBuffer,
            _In_   DWORD dwReceiveDataLength,
            _In_   DWORD dwLocalAddressLength,
            _In_   DWORD dwRemoteAddressLength,
            _Out_  LPSOCKADDR *LocalSockaddr,
            _Out_  LPINT LocalSockaddrLength,
            _Out_  LPSOCKADDR *RemoteSockaddr,
            _Out_  LPINT RemoteSockaddrLength);

        int recv_ex(
            _Inout_  LPWSABUF lpBuffers,
            _In_     DWORD dwBufferCount,
            _Out_    LPDWORD lpNumberOfBytesRecvd,
            _Inout_  LPDWORD lpFlags,
            _In_     LPWSAOVERLAPPED lpOverlapped,
            _In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
            );

        int recv_from_ex(
            _Inout_  LPWSABUF lpBuffers,
            _In_     DWORD dwBufferCount,
            _Out_    LPDWORD lpNumberOfBytesRecvd,
            _Inout_  LPDWORD lpFlags,
            _Out_    struct sockaddr *lpFrom,
            _Inout_  LPINT lpFromlen,
            _In_     LPWSAOVERLAPPED lpOverlapped,
            _In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

        int send_ex(
            _In_   LPWSABUF lpBuffers,
            _In_   DWORD dwBufferCount,
            _Out_  LPDWORD lpNumberOfBytesSent,
            _In_   DWORD dwFlags,
            _In_   LPWSAOVERLAPPED lpOverlapped,
            _In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

        int send_to_ex(
            _In_   LPWSABUF lpBuffers,
            _In_   DWORD dwBufferCount,
            _Out_  LPDWORD lpNumberOfBytesSent,
            _In_   DWORD dwFlags,
            _In_   const struct sockaddr *lpTo,
            _In_   int iToLen,
            _In_   LPWSAOVERLAPPED lpOverlapped,
            _In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

        BOOL cancel_io(
            _In_   LPWSAOVERLAPPED lpOverlapped);

        int getsockname(
            _Out_    struct sockaddr *name,
            _Inout_  int *namelen);

        int getpeername(
            _Out_    struct sockaddr *name,
            _Inout_  int *namelen);

        int shutdown(
            _In_  int how);

        int setsockopt(
            _In_  int level,
            _In_  int optname,
            _In_  const char *optval,
            _In_  int optlen);

        int getsockopt(
            _In_     int level,
            _In_     int optname,
            _Out_    char *optval,
            _Inout_  int *optlen);

        int ioctlsocket(
            _In_     long cmd,
            _Inout_  u_long *argp);

        int close();

    public:
        void attach_iocp(
            _In_  iocp_t * iocp, 
            _In_  ULONG_PTR CompletionKey);

    public:
        void select_attach(
            _In_  int t,
            _In_  select_t * select);

        void select_detach(
            _In_  int t,
            _In_  select_t * select);

    public:
        static Windows::Networking::HostName ^ sockaddr_to_host_name(
            _In_  const struct sockaddr *name);

        static Platform::String ^ sockaddr_to_svc_name(
            _In_  const struct sockaddr *name);

        static int host_name_port_to_sockaddr(
            _In_  int af,
            _Out_    struct sockaddr *name,
            _Inout_  int *namelen, 
            Windows::Networking::HostName ^ host_name, 
            Platform::String ^ svc);

    private:
        static DWORD read_buffer(
            Windows::Storage::Streams::IBuffer ^ buffer, 
            _Inout_  LPWSABUF lpBuffers,
            _In_     DWORD dwBufferCount);

        static DWORD write_buffer(
            Windows::Storage::Streams::IBuffer ^ buffer, 
            _Inout_  LPWSABUF lpBuffers,
            _In_     DWORD dwBufferCount);

    private:
        void on_connect(
            int ec);

        void tcp_recv_some();

        void tcp_send_some();

        void tcp_on_recv(
            int ec, 
            Windows::Storage::Streams::IBuffer ^ buffer, 
            size_t size);

        void tcp_on_send(
            int ec, 
            Windows::Storage::Streams::IBuffer ^ buffer, 
            size_t size);

        void udp_send();

        void udp_on_recv(
            int ec, 
            Windows::Storage::Streams::IBuffer ^ buffer, 
            Windows::Networking::HostName ^ host_name, 
            Platform::String ^ svc, 
            size_t size);

        void udp_on_send(
            int ec, 
            size_t size);

        void udp_reset();

        void handle_overlap_connect();

        void handle_overlap_accept();

        void handle_overlap_read();

        void handle_overlap_write();

        void handle_select_read();

        void handle_select_write();

        void handle_select_except();

        void handle_select(
            _In_  int t);

        void accept_conn(
            _In_   socket_t * sock,
            _In_   PVOID lpOutputBuffer,
            _In_   DWORD dwReceiveDataLength,
            _In_   DWORD dwLocalAddressLength,
            _In_   DWORD dwRemoteAddressLength,
            _Out_  LPDWORD lpdwBytesReceived);

        bool read_data(
            _Inout_  LPWSABUF lpBuffers,
            _In_     DWORD dwBufferCount,
            _Out_    LPDWORD lpNumberOfBytesRecvd, 
            _Out_    struct sockaddr *lpFrom,
            _Inout_  LPINT iToLen,
            _In_     LPWSAOVERLAPPED lpOverlapped,
            _Out_    LPINT ErrorCode);

        bool write_data(
            _In_   LPWSABUF lpBuffers,
            _In_   DWORD dwBufferCount,
            _Out_  LPDWORD lpNumberOfBytesSent, 
            _In_   const struct sockaddr *lpTo,
            _In_   int iToLen,
            _In_   LPWSAOVERLAPPED lpOverlapped,
            _Out_  LPINT ErrorCode);

    private:
        enum StatusEnum
        {
            s_can_read = 1, 
            s_can_write = 2, 
            s_read_eof = 4, 
            s_write_eof = 8, 

            s_established = 16, 

            s_establish = s_established | s_can_read | s_can_write, 
        };

        enum FlagEnum
        {
            f_non_block = 1, 
        };

    private:
        int af;
        int type;
        int protocol;
        Windows::Networking::Sockets::StreamSocket ^ stream_socket_;
        Windows::Networking::Sockets::StreamSocketListener ^ stream_listener_;
        Windows::Networking::Sockets::DatagramSocket ^ datagram_socket_;
        std::deque<Windows::Networking::Sockets::StreamSocket ^> accept_sockets_;
        boost::shared_ptr<iocp_t> iocp_;
        ULONG_PTR lpCompletionKey_;
        std::deque<overlap_task> read_tasks_; // or accept tasks
        std::deque<overlap_task> write_tasks_;
        std::deque<select_t *> select_tasks_[3];
        std::deque<Windows::Storage::Streams::IBuffer ^> read_datas_;
        std::deque<Windows::Storage::Streams::IBuffer ^> write_datas_;
        std::deque<std::pair<Windows::Networking::HostName ^, Platform::String ^> > udp_read_addrs_;
        std::deque<Windows::Storage::Streams::IOutputStream ^> udp_write_addrs_;
        std::map<uint64_t, Windows::Storage::Streams::IOutputStream ^> udp_streams_;
        int flags_;
        int status_;
        size_t read_data_size_;
        size_t write_data_size_;
        size_t read_data_total_;
        size_t write_data_total_;
        size_t read_data_capacity_;
        size_t write_data_capacity_;
        unsigned long long last_recv_tick_;
        bool connecting_;
        bool reading_;
        bool writing_;
        int ec_;
        std::recursive_mutex mutex_;
        std::condition_variable_any cond_;
    };

}
