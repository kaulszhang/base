// boost_config.h

// sp conunt
#define BOOST_SP_USE_PTHREADS

// atomic count
#define BOOST_AC_USE_PTHREADS

// asio disable epoll
#define BOOST_ASIO_DISABLE_EPOLL

#define unix 1
#include <sys/limits.h>
#include <unistd.h>
#undef _POSIX_SHARED_MEMORY_OBJECTS

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

#define IN6_IS_ADDR_MC_NODELOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a)                                             \
         && ((((__const uint8_t *) (a))[1] & 0xf) == 0x1))

#define IN6_IS_ADDR_MC_GLOBAL(a) \
        (IN6_IS_ADDR_MULTICAST(a)                                             \
         && ((((__const uint8_t *) (a))[1] & 0xf) == 0xe))

#define get_nprocs() 1
