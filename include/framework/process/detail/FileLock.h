// GlobalFileSemaphore.h

#ifndef _FRAMEWORK_PROCESS_GLOBAL_FILE_SEMAPHORE_H_
#define _FRAMEWORK_PROCESS_GLOBAL_FILE_SEMAPHORE_H_

#include <unistd.h>
#include <fcntl.h>

namespace framework
{
    namespace process
    {

        namespace detail
        {

            inline int lock_reg( int fd, int cmd, int type, off_t offset, int whence, off_t len )
            {
                struct flock lock;

                lock.l_type     = type;
                lock.l_start    = offset;
                lock.l_whence   = whence;
                lock.l_len      = len;

                return ( fcntl( fd, cmd, &lock ) );
            }

            inline pid_t lock_test( int fd, int type, off_t offset, int whence, off_t len )
            {
                struct flock lock;

                lock.l_type     = type;
                lock.l_start    = offset;
                lock.l_whence   = whence;
                lock.l_len      = len;

                if ( fcntl( fd, F_GETLK, &lock ) < 0 )
                    return ( 0 );

                if ( lock.l_type == F_UNLCK )
                    return ( 0 );

                return  ( lock.l_pid );
            }

        }// namespace detail

    } // namespace this_process
} // namespace framework

#define read_lock( fd, offset, whence, len ) \
    detail::lock_reg( (fd), F_SETLK, F_RDLCK, (offset), (whence), (len) )
#define readw_lock( fd, offset, whence, len ) \
    detail::lock_reg( (fd), F_SETLKW, F_RDLCK, (offset), (whence), (len) )
#define write_lock( fd, offset, whence, len ) \
    detail::lock_reg( (fd), F_SETLK, F_WRLCK, (offset), (whence), (len) )
#define writew_lock( fd, offset, whence, len ) \
    detail::lock_reg( (fd), F_SETLKW, F_WRLCK, (offset), (whence), (len) )
#define un_lock( fd, offset, whence, len ) \
    detail::lock_reg( (fd), F_SETLK, F_UNLCK, (offset), (whence), (len) )
#define is_read_lockable( fd, offset, whence, len ) \
    detail::lock_test( (fd), F_RDLCK, (offset), (whence), (len) )
#define is_write_lockable( fd, offset, whence, len ) \
    detail::lock_test( (fd), F_WRLCK, (offset), (whence), (len) )

#endif // _FRAMEWORK_PROCESS_GLOBAL_FILE_SEMAPHORE_H_
