// FileMutex.h

#ifndef _FRAMEWORK_PROCESS_FILE_MUTEX_H_
#define _FRAMEWORK_PROCESS_FILE_MUTEX_H_

#ifdef BOOST_WINDOWS_API
#  include <boost/interprocess/sync/interprocess_mutex.hpp>
#else
#  include "framework/process/detail/FileLock.h"
#  include "framework/filesystem/Path.h"
#  include <boost/filesystem/path.hpp>
#  include <boost/thread.hpp>
#endif

namespace framework
{
    namespace process
    {

#ifdef BOOST_WINDOWS_API

        class FileMutex
            : public boost::interprocess::interprocess_mutex
        {
        };

#else
  
        class FileLocks
        {
        public:
            FileLocks( char const * filename = NULL );

            ~FileLocks();

            size_t alloc_lock( std::string const & keyname );

            void lock( size_t lock_pos );

            bool try_lock( size_t lock_pos );

            bool timed_lock( size_t lock_pos, int wait_sec );

            void unlock( size_t unlock_pos );

            void free_lock();

        private:
            bool check_thread_lock( size_t lock_pos );

            boost::filesystem::path namelock_cache( std::string const & filename );

        private:
            int             m_fd_;
            boost::mutex    m_thread_mutex_;
            std::vector < boost::mutex * > m_pmutexs_;
        };

        inline FileLocks & global_file_lock()
        {
            static FileLocks g_fileblocks( "process.filelock" );
            return g_fileblocks;
        }

        /// ÎÄ¼þ¼ÇÂ¼Ëø
        class FileMutex
        {
        public:
            FileMutex( std::string const keyname = "" )
                : m_lockbyte_( global_file_lock().alloc_lock( keyname ) )
            {
                //std::cout << "FileMutex constructor lockbyte = " << m_lockbyte_ << std::endl;
            }

            ~FileMutex()
            {
            }

            void lock()
            {
                //std::cout << "FileMutex lock lockbyte = " << m_lockbyte_ << std::endl;
                global_file_lock().lock( m_lockbyte_ );
            }

            bool timed_lock( int wait_sec )
            {
                //std::cout << "FileMutex timed_lock lockbyte = " << m_lockbyte_ << " wait_sec = " << wait_sec << std::endl;
                return global_file_lock().timed_lock( m_lockbyte_, wait_sec );
            }

            bool try_lock()
            {
                //std::cout << "FileMutex try_lock lockbyte = " << m_lockbyte_ << std::endl;
                return global_file_lock().try_lock( m_lockbyte_ );
            }

            void unlock()
            {
                //std::cout << "FileMutex unlock lockbyte = " << m_lockbyte_ << std::endl;
                global_file_lock().unlock( m_lockbyte_ );
            }

        private:
            size_t m_lockbyte_;
        };

#endif

    } // namespace process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_FILE_MUTEX_H_
