// SharedMemorySystemV.h

#ifndef _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_SYSTEM_V_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_SYSTEM_V_H_

#include "framework/memory/detail/SharedMemoryImpl.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace framework
{
    namespace memory
    {

        namespace detail
        {

            class SharedMemorySystemV
                : public SharedMemoryImpl
            {
            private:
                static std::string tmp_file_name(
                    boost::uint32_t iid)
                {
                    std::string name = key_path(iid, 0);

                    int fd = ::open(
                        name.c_str(), 
                        O_CREAT, 
                        S_IRWXG | S_IRWXO | S_IRWXU);

                    if (-1 == fd) {
                        const char* err_msg = "SystemV share memory create fail!";
                        throw std::runtime_error( err_msg );
                    }

                    ::close(
                        fd);

                    return name;
                }

                key_t name_key(
                    boost::uint32_t iid, 
                    boost::uint32_t key)
                {
                    static std::string file_name = tmp_file_name(iid);
                    return ftok(file_name.c_str(), key);
                }

                bool create( 
                    void ** id, 
                    boost::uint32_t iid,
                    boost::uint32_t key, 
                    boost::uint32_t size, 
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    int shmid = ::shmget(
                        name_key(iid, key), 
                        size, 
                        IPC_CREAT | IPC_EXCL | 0666);

                    if (shmid == -1) {
                        return false;
                    }

                    *id = (void *)shmid;

                    return true;
                }

                bool open( 
                    void ** id, 
                    boost::uint32_t iid,
                    boost::uint32_t key,
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    int shmid = ::shmget(
                        name_key(iid, key), 
                        0, 
                        0666);

                    if (shmid == -1) {
                        return false;
                    }

                    *id = (void *)shmid;

                    return true;
                }

                void * map(
                    void * id,
                    boost::uint32_t size,
                    boost::system::error_code & ec )
                {
                    ErrorCodeWrapper ecw(ec);

                    int shmid = ObjectWrapper::cast_object<int>(id);

                    void * p = ::shmat(
                        shmid, 
                        NULL, 
                        0);

                    if (p == (void *)-1) {
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

                    int r = ::shmdt(
                        addr);

                    if (r == -1) {
                        return false;
                    }

                    return true;
                }

                bool close(
                    void * id, 
                    boost::system::error_code & ec)
                {
                    ec.clear();
                    return true;
                }

                bool destory( 
                    boost::uint32_t iid, 
                    boost::uint32_t key,
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    int shmid = ::shmget(
                        name_key(iid, key), 0, 0666);

                    if (shmid == -1) {
                        return false;
                    }

                    shmid_ds sds;
                    int r = ::shmctl(
                        shmid, 
                        IPC_STAT, 
                        &sds);

                    if (r == -1) {
                        return false;
                    }
                    
                    if (sds.shm_nattch != 0) {
                        return false;
                    }

                    r = ::shmctl(
                        shmid, 
                        IPC_RMID, 
                        NULL);

                    if (r == -1) {
                        return false;
                    }

                    return true;
                }
            };

            static SharedMemorySystemV shared_memory_systemv;

        } // namespace detail

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_SYSTEM_V_H_
