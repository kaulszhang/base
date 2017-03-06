// GlobalSemaphore.h

#ifndef _FRAMEWORK_PROCESS_GLOBAL_SEMAPHORE_H_
#define _FRAMEWORK_PROCESS_GLOBAL_SEMAPHORE_H_

#include "framework/filesystem/Path.h"
#include "framework/Version.h"
#include <boost/thread/locks.hpp>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace framework
{
    namespace process
    {

        namespace detail
        {

            static inline key_t global_sem_key()
            {
                static int const key_ = 555555;
                std::string file_name = framework::filesystem::temp_path().file_string();
                file_name += "/GLOBAL_SEMAPHORE_";
                file_name += simple_version_string();
                int fd = ::open(file_name.c_str(), O_CREAT, S_IRWXG | S_IRWXO | S_IRWXU);
                ::close(fd);
                return ftok(file_name.c_str(), key_);
            }

            static inline int make_global_sem()
            {
                key_t key = global_sem_key();
                int sem = ::semget(key, 1, IPC_CREAT | IPC_EXCL | 0660);
                if (errno == EEXIST) {
                    sem = ::semget(key, 1, 0660);
                } else {
                    ::semctl(sem, 0, SETVAL, 1);
                }
                return sem;
            }

            inline int global_sem()
            {
                static int gs = make_global_sem();
                return gs;
            }

            static inline void global_lock()
            {
                struct sembuf sb;
                sb.sem_num = 0;
                sb.sem_op = -1;
                sb.sem_flg = SEM_UNDO;
                ::semop(global_sem(), &sb, 1);
            }

            static inline void global_unlock()
            {
                struct sembuf sb;
                sb.sem_num = 0;
                sb.sem_op = 1;
                sb.sem_flg = SEM_UNDO;
                ::semop(global_sem(), &sb, 1);
            }

        }

    } // namespace this_process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_GLOBAL_SEMAPHORE_H_
