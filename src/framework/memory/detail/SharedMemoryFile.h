// SharedMemoryFile.h

#ifndef _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_FILE_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_FILE_H_

#include "framework/process/detail/FileLock.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

namespace framework
{
    namespace memory
    {

        namespace detail
        {

            class SharedMemoryFile
                : public SharedMemoryImpl
            {
            private:
                bool create( 
                    void ** id, 
                    boost::uint32_t iid, 
                    boost::uint32_t key, 
                    boost::uint32_t size, 
                    boost::system::error_code & ec)
                {
                    std::string name = key_path(iid, key);

                    ObjectWrapper ow_destroy;
                    ObjectWrapper ow;
                    ErrorCodeWrapper ecw(ec);

                    int fd = ::open( 
                        name.c_str(),
                        O_CREAT | O_RDWR | O_EXCL, 
                        00666);

                    if (fd == -1) {
                        return false;
                    }

                    ow_destroy.reset(name.c_str(), ::unlink);
                    ow.reset(fd, ::close);

                    int r = ::ftruncate(
                        fd, 
                        size);

                    if (r == -1) {
                        return false;
                    }

                    r = framework::process::read_lock(
                        fd, 
                        0, 
                        SEEK_SET, 
                        0);

                    if (r == -1) {
                        return false;
                    }

                    *id = ow.release();
                    ow_destroy.release();

                    return true;
                }

                bool open( 
                    void ** id, 
                    boost::uint32_t iid,
                    boost::uint32_t key,
                    boost::system::error_code & ec)
                {
                    ObjectWrapper ow;
                    ErrorCodeWrapper ecw(ec);

                    int fd = ::open( 
                        key_path(iid, key).c_str(),
                        O_RDWR);

                    if (fd == -1) {
                        return false;
                    }

                    ow.reset(fd, ::close);

                    int r = framework::process::read_lock(
                        fd, 
                        0, 
                        SEEK_SET, 
                        0);

                    if (r == -1) {
                        return false;
                    }

                    *id = ow.release();

                    return true;
                }

                void * map(
                    void * id,
                    boost::uint32_t size,
                    boost::system::error_code & ec )
                {
                    ErrorCodeWrapper ecw(ec);

                    int fd = ObjectWrapper::cast_object<int>(id);

                    void * p = ::mmap(
                        NULL, 
                        size, 
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        fd,
                        0);

                    if (p == MAP_FAILED) {
                        return NULL;
                    }

                    return p;
                }

                bool unmap(
                    void * addr, 
                    boost::uint32_t size,
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    int r = ::munmap(
                        addr, 
                        size);

                    if (r == -1) {
                        return false;
                    }

                    return true;
                }

                bool close(
                    void * id, 
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    int fd = ObjectWrapper::cast_object<int>(id);

                    int r = ::close(
                        fd);

                    if (r == -1) {
                        return false;
                    }

                    return true;
                }

                bool destory( 
                    boost::uint32_t iid, 
                    boost::uint32_t key,
                    boost::system::error_code & ec)
                {  
                    ObjectWrapper ow;
                    ErrorCodeWrapper ecw(ec);

                    int fd = ::open( 
                        key_path(iid, key).c_str(),
                        O_RDWR);

                    if (fd == -1) {
                        return false;
                    }

                    ow.reset(fd, ::close);

                    /// 直接加非阻塞的写锁，成功则删除
                    int r = framework::process::write_lock(
                        fd, 
                        0, 
                        SEEK_SET, 
                        0);

                    if (r == -1) {
                        return false;
                    }

                    ow.reset();

                    r = ::unlink(
                        key_path( iid, key ).c_str());

                    if (r == -1) {
                        return false;
                    }

                    return true;
                }
            };

            static SharedMemoryFile shared_memory_file;

        } // namespace detail

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_FILE_H_
