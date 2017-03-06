// SocketEmulation.h

#pragma once

typedef UINT_PTR SOCKET;

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#define SOMAXCONN       0x7fffffff

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef int socklen_t;

#define SOCK_STREAM     1               /* stream socket */
#define SOCK_DGRAM      2               /* datagram socket */
#define SOCK_RAW        3               /* raw-protocol interface */
#define SOCK_RDM        4               /* reliably-delivered message */
#define SOCK_SEQPACKET  5               /* sequenced packet stream */

#define AF_UNSPEC       0               // unspecified
#define AF_INET         2               // internetwork: UDP, TCP, etc.
#define AF_INET6        23              // Internetwork Version 6

#define PF_UNSPEC       AF_UNSPEC
#define PF_INET         AF_INET
#define PF_INET6        AF_INET6

#define IPPROTO_IP              0
enum
{
    IPPROTO_ICMP = 1,
    IPPROTO_TCP = 6, 
    IPPROTO_UDP = 17
};

#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

#define MSG_OOB         0x1             /* process out-of-band data */
#define MSG_PEEK        0x2             /* peek at incoming message */
#define MSG_DONTROUTE   0x4             /* send without using routing tables */

#define IOCPARM_MASK    0x7f            /* parameters must be < 128 bytes */
#define IOC_VOID        0x20000000      /* no parameters */
#define IOC_OUT         0x40000000      /* copy out parameters */
#define IOC_IN          0x80000000      /* copy in parameters */
#define IOC_INOUT       (IOC_IN|IOC_OUT)
/* 0x20000000 distinguishes new &
old ioctl's */
#define _IO(x,y)        (IOC_VOID|((x)<<8)|(y))

#define _IOR(x,y,t)     (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define _IOW(x,y,t)     (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define FIONREAD    _IOR('f', 127, u_long) /* get # bytes to read */
#define FIONBIO     _IOW('f', 126, u_long) /* set/clear non-blocking i/o */
#define FIOASYNC    _IOW('f', 125, u_long) /* set/clear async i/o */

#define IP_OPTIONS                 1 // Set/get IP options.
#define IP_HDRINCL                 2 // Header is included with data.
#define IP_TOS                     3 // IP type of service.
#define IP_TTL                     4 // IP TTL (hop limit).
#define IP_MULTICAST_IF            9 // IP multicast interface.
#define IP_MULTICAST_TTL          10 // IP multicast TTL (hop limit).
#define IP_MULTICAST_LOOP         11 // IP multicast loopback.
#define IP_ADD_MEMBERSHIP         12 // Add an IP group membership.
#define IP_DROP_MEMBERSHIP        13 // Drop an IP group membership.
#define IP_DONTFRAGMENT           14 // Don't fragment IP datagrams.
#define IP_ADD_SOURCE_MEMBERSHIP  15 // Join IP group/source.
#define IP_DROP_SOURCE_MEMBERSHIP 16 // Leave IP group/source.
#define IP_BLOCK_SOURCE           17 // Block IP group/source.
#define IP_UNBLOCK_SOURCE         18 // Unblock IP group/source.
#define IP_PKTINFO                19 // Receive packet information.
#define IP_HOPLIMIT               21 // Receive packet hop limit.
#define IP_RECEIVE_BROADCAST      22 // Allow/block broadcast reception.
#define IP_RECVIF                 24 // Receive arrival interface.
#define IP_RECVDSTADDR            25 // Receive destination address.
#define IP_IFLIST                 28 // Enable/Disable an interface list.
#define IP_ADD_IFLIST             29 // Add an interface list entry.
#define IP_DEL_IFLIST             30 // Delete an interface list entry.
#define IP_UNICAST_IF             31 // IP unicast interface.
#define IP_RTHDR                  32 // Set/get IPv6 routing header.
#define IP_RECVRTHDR              38 // Receive the routing header.

#define SOL_SOCKET      0xffff          /* options for socket level */

#define SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SO_LINGER       0x0080          /* linger on close if data present */
#define SO_OOBINLINE    0x0100          /* leave received OOB data in line */

#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */

#define SO_UPDATE_ACCEPT_CONTEXT    0x700B
#define SO_CONNECT_TIME             0x700C

#define TCP_NODELAY         0x0001

struct sockaddr
{
    u_short sa_family;              /* address family */
    char    sa_data[14];            /* up to 14 bytes of direct address */
};

struct in_addr
{
    union {
        struct { UCHAR s_b1,s_b2,s_b3,s_b4; } S_un_b;
        struct { USHORT s_w1,s_w2; } S_un_w;
        ULONG S_addr;
    } S_un;
#define s_addr  S_un.S_addr /* can be used for most tcp & ip code */
#define s_host  S_un.S_un_b.s_b2    // host on imp
#define s_net   S_un.S_un_b.s_b1    // network
#define s_imp   S_un.S_un_w.s_w2    // imp
#define s_impno S_un.S_un_b.s_b4    // imp #
#define s_lh    S_un.S_un_b.s_b3    // logical host
};

#define IN_CLASSA(i)            (((long)(i) & 0x80000000) == 0)
#define IN_CLASSA_NET           0xff000000
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          0x00ffffff
#define IN_CLASSA_MAX           128

#define IN_CLASSB(i)            (((long)(i) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          0x0000ffff
#define IN_CLASSB_MAX           65536

#define IN_CLASSC(i)            (((long)(i) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET           0xffffff00
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          0x000000ff

#define IN_CLASSD(i)            (((long)(i) & 0xf0000000) == 0xe0000000)
#define IN_CLASSD_NET           0xf0000000       /* These ones aren't really */
#define IN_CLASSD_NSHIFT        28               /* net and host fields, but */
#define IN_CLASSD_HOST          0x0fffffff       /* routing needn't know.    */
#define IN_MULTICAST(i)         IN_CLASSD(i)

#define INADDR_ANY              (u_long)0x00000000
#define INADDR_LOOPBACK         0x7f000001
#define INADDR_BROADCAST        (u_long)0xffffffff
#define INADDR_NONE             0xffffffff

struct sockaddr_in {
    short   sin_family;
    u_short sin_port;
    struct  in_addr sin_addr;
    char    sin_zero[8];
};

struct ip_mreq
{
    struct in_addr  imr_multiaddr;  /* IP multicast address of group */
    struct in_addr  imr_interface;  /* local IP address of interface */
};

struct timeval {
    long    tv_sec;         /* seconds */
    long    tv_usec;        /* and microseconds */
};

struct  linger {
    u_short l_onoff;                /* option on/off */
    u_short l_linger;               /* linger time */
};

typedef long int __fd_mask;

#define __NFDBITS       (8 * (int) sizeof (__fd_mask))
#define __FDELT(d)      ((d) / __NFDBITS)
#define __FDMASK(d)     ((__fd_mask) 1 << ((d) % __NFDBITS))

#define FD_SETSIZE      1024

typedef struct fd_set {
    __fd_mask fds_bits[FD_SETSIZE / __NFDBITS];
} fd_set;

# define __FDS_BITS(set) ((set)->fds_bits)

typedef struct fd_set FD_SET;

#define FD_ZERO(set) (memset(__FDS_BITS (set), 0, sizeof(__FDS_BITS (set))))

#define FD_SET(d, set)    (__FDS_BITS (set)[__FDELT (d)] |= __FDMASK (d))
#define FD_CLR(d, set)    (__FDS_BITS (set)[__FDELT (d)] &= ~__FDMASK (d))

#define FD_ISSET(d, set) \
    ((__FDS_BITS (set)[__FDELT (d)] & __FDMASK (d)) != 0)

#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND
#define TRY_AGAIN               WSATRY_AGAIN
#define NO_RECOVERY             WSANO_RECOVERY
#define NO_DATA                 WSANO_DATA

struct adapter {
    int        len;
    int        lan_type;
    GUID    id;
    //char    addr[];
};

struct  hostent {
    char    FAR * h_name;           /* official name of host */
    char    FAR * FAR * h_aliases;  /* alias list */
    short   h_addrtype;             /* host address type */
    short   h_length;               /* length of address */
    char    FAR * FAR * h_addr_list; /* list of addresses */
#define h_addr  h_addr_list[0]          /* address, for backward compat */
};

struct  servent {
    char    FAR * s_name;           /* official service name */
    char    FAR * FAR * s_aliases;  /* alias list */
#ifdef _WIN64
    char    FAR * s_proto;          /* protocol to use */
    short   s_port;                 /* port # */
#else
    short   s_port;                 /* port # */
    char    FAR * s_proto;          /* protocol to use */
#endif
};

//#ifdef __cplusplus
//extern "C"
//{
//#endif

    SOCKET WINAPI_DECL socket(
        _In_  int af,
        _In_  int type,
        _In_  int protocol
        );

    int WINAPI_DECL bind(
        _In_  SOCKET s,
        _In_  const struct sockaddr *name,
        _In_  int namelen
        );

    int WINAPI_DECL connect(
        _In_  SOCKET s,
        _In_  const struct sockaddr *name,
        _In_  int namelen
        );

    int WINAPI_DECL listen(
        _In_  SOCKET s,
        _In_  int backlog
        );

    SOCKET WINAPI_DECL accept(
        _In_     SOCKET s,
        _Out_    struct sockaddr *addr,
        _Inout_  int *addrlen
        );

    int WINAPI_DECL getsockname(
        _In_     SOCKET s,
        _Out_    struct sockaddr *name,
        _Inout_  int *namelen
        );

    int WINAPI_DECL getpeername(
        _In_     SOCKET s,
        _Out_    struct sockaddr *name,
        _Inout_  int *namelen
        );

    int WINAPI_DECL recv(
        _In_   SOCKET s,
        _Out_  char *buf,
        _In_   int len,
        _In_   int flags
        );

    int WINAPI_DECL recvfrom(
        _In_         SOCKET s,
        _Out_        char *buf,
        _In_         int len,
        _In_         int flags,
        _Out_        struct sockaddr *from,
        _Inout_opt_  int *fromlen
        );

    int WINAPI_DECL send(
        _In_  SOCKET s,
        _In_  const char *buf,
        _In_  int len,
        _In_  int flags
        );

    int WINAPI_DECL sendto(
        _In_  SOCKET s,
        _In_  const char *buf,
        _In_  int len,
        _In_  int flags,
        _In_  const struct sockaddr *to,
        _In_  int tolen
        );

    int WINAPI_DECL shutdown(
        _In_  SOCKET s,
        _In_  int how
        );

    int WINAPI_DECL setsockopt(
        _In_  SOCKET s,
        _In_  int level,
        _In_  int optname,
        _In_  const char *optval,
        _In_  int optlen
        );

    int WINAPI_DECL getsockopt(
        _In_     SOCKET s,
        _In_     int level,
        _In_     int optname,
        _Out_    char *optval,
        _Inout_  int *optlen
        );

    int WINAPI_DECL ioctlsocket(
        _In_     SOCKET s,
        _In_     long cmd,
        _Inout_  u_long *argp
        );

    int WINAPI_DECL closesocket(
        _In_  SOCKET s
        );

    int WINAPI_DECL select(
        _In_     int nfds,
        _Inout_  fd_set *readfds,
        _Inout_  fd_set *writefds,
        _Inout_  fd_set *exceptfds,
        _In_     const struct timeval *timeout
        );

    int WINAPI_DECL gethostname(
        _Out_  char *name,
        _In_   int namelen
        );

    int WINAPI_DECL getadapters(
        _Out_  char *adapters,
        _In_   int len
        );

    WINAPI_DECL struct hostent* gethostbyname(
        _In_  const char *name
        );

    WINAPI_DECL struct hostent* gethostbyaddr(
        _In_  const char *addr,
        _In_  int len,
        _In_  int type
        );

    WINAPI_DECL struct servent* getservbyname(
        _In_  const char *name,
        _In_  const char *proto
        );

    WINAPI_DECL struct servent* getservbyport(
        _In_  int port,
        _In_  const char *proto
        );

    unsigned long WINAPI_DECL inet_addr(
        _In_  const char *cp
        );

    WINAPI_DECL char* inet_ntoa(
        _In_  struct in_addr in
        );

//#ifdef __cplusplus
//}
//#endif

#define WSA_IO_PENDING          (ERROR_IO_PENDING)
#define WSA_IO_INCOMPLETE       (ERROR_IO_INCOMPLETE)
#define WSA_INVALID_HANDLE      (ERROR_INVALID_HANDLE)
#define WSA_INVALID_PARAMETER   (ERROR_INVALID_PARAMETER)
#define WSA_NOT_ENOUGH_MEMORY   (ERROR_NOT_ENOUGH_MEMORY)
#define WSA_OPERATION_ABORTED   (ERROR_OPERATION_ABORTED)

typedef struct _WSABUF {
    ULONG len;     /* the length of the buffer */
    _Field_size_bytes_(len) CHAR FAR *buf; /* the pointer to the buffer */
} WSABUF, FAR * LPWSABUF;

#define WSAOVERLAPPED           OVERLAPPED

typedef struct _WSAPROTOCOL_INFO
{
    DWORD dw;
} WSAPROTOCOL_INFO, FAR * LPWSAPROTOCOL_INFO;

typedef WSAOVERLAPPED * LPWSAOVERLAPPED;
typedef void * LPWSAOVERLAPPED_COMPLETION_ROUTINE;
typedef unsigned int GROUP;

#define WSAAPI                  FAR PASCAL

typedef int  INT;

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr FAR *LPSOCKADDR;

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;
typedef struct sockaddr_in FAR *LPSOCKADDR_IN;

typedef struct linger LINGER;
typedef struct linger *PLINGER;
typedef struct linger FAR *LPLINGER;

typedef struct fd_set FD_SET;
typedef struct fd_set *PFD_SET;
typedef struct fd_set FAR *LPFD_SET;

typedef struct hostent HOSTENT;
typedef struct hostent *PHOSTENT;
typedef struct hostent FAR *LPHOSTENT;

typedef struct servent SERVENT;
typedef struct servent *PSERVENT;
typedef struct servent FAR *LPSERVENT;

typedef struct protoent PROTOENT;
typedef struct protoent *PPROTOENT;
typedef struct protoent FAR *LPPROTOENT;

typedef struct timeval TIMEVAL;
typedef struct timeval *PTIMEVAL;
typedef struct timeval FAR *LPTIMEVAL;

#define WSA_FLAG_OVERLAPPED           0x01

typedef struct WSAData {
    WORD           wVersion;
    WORD           wHighVersion;
    //        char           szDescription[WSADESCRIPTION_LEN+1];
    //        char           szSystemStatus[WSASYS_STATUS_LEN+1];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char FAR       *lpVendorInfo;
} WSADATA, *LPWSADATA;

//#ifdef __cplusplus
//extern "C"
//{
//#endif

    int WINAPI_DECL __WSAFDIsSet(SOCKET fd, fd_set FAR *);

    int WINAPI_DECL WSAStartup(
        _In_   WORD wVersionRequested,
        _Out_  LPWSADATA lpWSAData
        );

    int WINAPI_DECL WSACleanup(void);

    u_short WINAPI_DECL htons(
        _In_  u_short hostshort
        );

    u_long WINAPI_DECL htonl(
        _In_  u_long hostlong
        );

    u_short WINAPI_DECL ntohs(
        _In_  u_short netshort
        );

    u_long WINAPI_DECL ntohl(
        _In_  u_long netlong
        );

    SOCKET WINAPI_DECL WSASocket(
        _In_  int af,
        _In_  int type,
        _In_  int protocol,
        _In_  LPWSAPROTOCOL_INFO lpProtocolInfo,
        _In_  GROUP g,
        _In_  DWORD dwFlags
        );

#define HAS_ConnectEx

    BOOL WINAPI_DECL ConnectEx(
        _In_   SOCKET s,
        _In_   const struct sockaddr *lpTo,
        _In_   int iToLen,
        _In_   LPOVERLAPPED lpOverlapped
        );

    BOOL WINAPI_DECL AcceptEx(
        _In_   SOCKET sListenSocket,
        _In_   SOCKET sAcceptSocket,
        _In_   PVOID lpOutputBuffer,
        _In_   DWORD dwReceiveDataLength,
        _In_   DWORD dwLocalAddressLength,
        _In_   DWORD dwRemoteAddressLength,
        _Out_  LPDWORD lpdwBytesReceived,
        _In_   LPOVERLAPPED lpOverlapped
        );

    void WINAPI_DECL GetAcceptExSockaddrs(
        _In_   PVOID lpOutputBuffer,
        _In_   DWORD dwReceiveDataLength,
        _In_   DWORD dwLocalAddressLength,
        _In_   DWORD dwRemoteAddressLength,
        _Out_  LPSOCKADDR *LocalSockaddr,
        _Out_  LPINT LocalSockaddrLength,
        _Out_  LPSOCKADDR *RemoteSockaddr,
        _Out_  LPINT RemoteSockaddrLength
        );

    int WINAPI_DECL WSARecv(
        _In_     SOCKET s,
        _Inout_  LPWSABUF lpBuffers,
        _In_     DWORD dwBufferCount,
        _Out_    LPDWORD lpNumberOfBytesRecvd,
        _Inout_  LPDWORD lpFlags,
        _In_     LPWSAOVERLAPPED lpOverlapped,
        _In_     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
        );

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
        );

    int WINAPI_DECL WSASend(
        _In_   SOCKET s,
        _In_   LPWSABUF lpBuffers,
        _In_   DWORD dwBufferCount,
        _Out_  LPDWORD lpNumberOfBytesSent,
        _In_   DWORD dwFlags,
        _In_   LPWSAOVERLAPPED lpOverlapped,
        _In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
        );

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
        );

    INT WINAPI_DECL WSAAddressToStringA(
        _In_      LPSOCKADDR lpsaAddress,
        _In_      DWORD dwAddressLength,
        _In_opt_  LPWSAPROTOCOL_INFO lpProtocolInfo,
        _Inout_   LPSTR lpszAddressString,
        _Inout_   LPDWORD lpdwAddressStringLength
        );

    INT WINAPI_DECL WSAAddressToStringW(
        _In_      LPSOCKADDR lpsaAddress,
        _In_      DWORD dwAddressLength,
        _In_opt_  LPWSAPROTOCOL_INFO lpProtocolInfo,
        _Inout_   LPWSTR lpszAddressString,
        _Inout_   LPDWORD lpdwAddressStringLength
        );

#ifdef UNICODE
#define WSAAddressToString  WSAAddressToStringW
#else
#define WSAAddressToString  WSAAddressToStringA
#endif // !UNICODE

    INT WINAPI_DECL WSAStringToAddressA(
        _In_      LPSTR AddressString,
        _In_      INT AddressFamily,
        _In_opt_  LPWSAPROTOCOL_INFO lpProtocolInfo,
        _Out_     LPSOCKADDR lpAddress,
        _Inout_   LPINT lpAddressLength
        );

    INT WINAPI_DECL WSAStringToAddressW(
        _In_      LPWSTR AddressString,
        _In_      INT AddressFamily,
        _In_opt_  LPWSAPROTOCOL_INFO lpProtocolInfo,
        _Out_     LPSOCKADDR lpAddress,
        _Inout_   LPINT lpAddressLength
        );

#ifdef UNICODE
#define WSAStringToAddress  WSAStringToAddressW
#else
#define WSAStringToAddress  WSAStringToAddressA
#endif // !UNICODE

    void WINAPI_DECL WSASetLastError(
        _In_  int iError
        );

    int WINAPI_DECL WSAGetLastError(void);

    HANDLE WINAPI_DECL CreateIoCompletionPort(
        _In_      HANDLE FileHandle,
        _In_opt_  HANDLE ExistingCompletionPort,
        _In_      ULONG_PTR CompletionKey,
        _In_      DWORD NumberOfConcurrentThreads
        );

    BOOL WINAPI_DECL GetQueuedCompletionStatus(
        _In_   HANDLE CompletionPort,
        _Out_  LPDWORD lpNumberOfBytes,
        _Out_  PULONG_PTR lpCompletionKey,
        _Out_  LPOVERLAPPED *lpOverlapped,
        _In_   DWORD dwMilliseconds
        );

    BOOL WINAPI_DECL CancelIo(
        _In_   HANDLE hFile
        );

    BOOL WINAPI_DECL CancelIoEx(
        _In_      HANDLE hFile,
        _In_opt_  LPOVERLAPPED lpOverlapped
        );

    BOOL WINAPI_DECL PostQueuedCompletionStatus(
        _In_      HANDLE CompletionPort,
        _In_      DWORD dwNumberOfBytesTransferred,
        _In_      ULONG_PTR dwCompletionKey,
        _In_opt_  LPOVERLAPPED lpOverlapped
        );

    BOOL WINAPI_DECL GetOverlappedResult(
        _In_   HANDLE hFile,
        _In_   LPOVERLAPPED lpOverlapped,
        _Out_  LPDWORD lpNumberOfBytesTransferred,
        _In_   BOOL bWait
        );

    BOOL WINAPI_DECL CloseIoCompletionPort(
        _In_  HANDLE hObject
        );

    BOOL WINAPI_DECL GetCommState(
        _In_     HANDLE hFile,
        _Inout_  LPDCB lpDCB
        );

    BOOL WINAPI_DECL SetCommState(
        _In_  HANDLE hFile,
        _In_  LPDCB lpDCB
        );

    BOOL WINAPI_DECL GetCommTimeouts(
        _In_   HANDLE hFile,
        _Out_  LPCOMMTIMEOUTS lpCommTimeouts
        );

    BOOL WINAPI_DECL SetCommTimeouts(
        _In_  HANDLE hFile,
        _In_  LPCOMMTIMEOUTS lpCommTimeouts
        );

//#ifdef __cplusplus
//}
//#endif
