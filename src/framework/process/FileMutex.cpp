// FileMutex.cpp

#include "framework/Framework.h"
#include "framework/process/FileMutex.h"

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#else
#  include <stdlib.h>
#  include <signal.h>
#  include <fstream>
#  include <boost/interprocess/shared_memory_object.hpp>
#endif

namespace framework
{
    namespace process
    {

#ifdef BOOST_WINDOWS_API

#else
        void nullhander( int signo )
        {
        }

        FileLocks::FileLocks( char const * filename )
        {
            std::string file_name = framework::filesystem::framework_temp_path().string();
            filename ? file_name += "/", file_name += filename : file_name += "/process.filelock";
            m_fd_ = open( file_name.c_str(), O_CREAT | O_RDWR, 0666 );
            if ( -1 == m_fd_ )
            {
                const char* err_msg = "Filelocks create fail!";
                throw std::runtime_error( err_msg );
            }
        }

        FileLocks::~FileLocks()
        {
            for ( size_t i = 0; i < m_pmutexs_.size(); ++i )
            {
                if ( m_pmutexs_[i] )
                {
                    delete m_pmutexs_[i];
                }
            }
        }

        size_t FileLocks::alloc_lock( std::string const & keyname )
        {
            if ( keyname.empty() )
            {
                m_thread_mutex_.lock();

                writew_lock( m_fd_, 0, SEEK_SET, 4 );

                char buff[4 + 1] = { 0 };
                long seqno = 0;
                size_t readlen = 0;
                lseek( m_fd_, 0L, SEEK_SET );
                readlen = read( m_fd_, buff, 4 );
                buff[readlen] = '\0';
                readlen = sscanf( buff, "%ld", &seqno );
                snprintf( buff, sizeof( buff ), "%ld", seqno + 1 );
                lseek( m_fd_, 0L, SEEK_SET );
                write( m_fd_, buff, strlen( buff ) );

                un_lock( m_fd_, 0, SEEK_SET, 4 );

                m_pmutexs_.resize( seqno + 1 );
                m_pmutexs_[seqno] = new boost::mutex;
                //std::cout << alloc_lock id = "  << seqno << " lock addr = " << m_pmutexs_[seqno] << " this = " << this << std::endl;
                m_thread_mutex_.unlock();

                return seqno;
            }
            else/// 命名锁
            {
                m_thread_mutex_.lock();
                writew_lock( m_fd_, 0, SEEK_SET, 4 );

                /// 载入已有的锁
                std::ifstream ifs;
                ifs.open( namelock_cache( "process.filelock" ).string().c_str() );
                std::string line;
                std::getline(ifs, line);
                while (std::getline(ifs, line)) 
                {
                    if (line.empty() || line[0] == '#')
                    {
                        continue;
                    }
                    std::string::size_type p = line.find('\t');
                    if (p == std::string::npos)
                    {
                        continue;
                    }

                    std::string name = line.substr(0, p);
                    long value = atol( line.substr(p + 1).c_str() );
                    if ( name == keyname )
                    {
                        ifs.close();
                        un_lock( m_fd_, 0, SEEK_SET, 4 );
                        m_thread_mutex_.unlock();
                        return value;
                    }
                }
                ifs.close();

                // 没有查到
                char buff[4 + 2] = { 0 };
                long seqno = 0;
                size_t readlen = 0;
                lseek( m_fd_, 0L, SEEK_SET );
                readlen = read( m_fd_, buff, 4 );
                buff[readlen] = '\0';
                readlen = sscanf( buff, "%ld", &seqno );
                snprintf( buff, sizeof( buff ), "%ld\n", seqno + 1 );
                lseek( m_fd_, 0L, SEEK_SET );
                write( m_fd_, buff, strlen( buff ) );

                /// 附加到尾部
                std::ofstream ofs;
                ofs.open( namelock_cache( "process.filelock" ).string().c_str(), std::ios_base::app );
                ofs << keyname << "\t" << seqno << std::endl;
                ofs.close();

                un_lock( m_fd_, 0, SEEK_SET, 4 );

                m_pmutexs_.resize( seqno + 1 );
                m_pmutexs_[seqno] = new boost::mutex;
                //std::cout << alloc_lock id = "  << seqno << " lock addr = " << m_pmutexs_[seqno] << " this = " << this << std::endl;

                m_thread_mutex_.unlock();

                return seqno;
            }
        }

        void FileLocks::lock( size_t lock_pos )
        {
            check_thread_lock( lock_pos );
            //std::cout << lock sizeof m_pmutexs_ = " << m_pmutexs_.size() << std::endl;
            m_pmutexs_[lock_pos]->lock();
            writew_lock( m_fd_, lock_pos + 4, SEEK_SET, 1 );
        }

        bool FileLocks::try_lock( size_t lock_pos )
        {
            check_thread_lock( lock_pos );
            if ( m_pmutexs_[lock_pos]->try_lock() )
            {
                if ( write_lock( m_fd_, lock_pos + 4, SEEK_SET, 1 ) != -1 )
                {
                    return true;
                }
                else
                {
                    m_pmutexs_[lock_pos]->unlock();
                    return false;
                }
            }
            else
            {
                 return false;
            }
        }

        bool FileLocks::timed_lock( size_t lock_pos, int wait_sec )
        {
            check_thread_lock( lock_pos );
            int loops = wait_sec * 50;
            while ( loops-- )
            {
                if ( m_pmutexs_[lock_pos]->try_lock() )
                {
                    while ( loops-- )
                    {
                        if ( write_lock( m_fd_, lock_pos + 4, SEEK_SET, 1 ) != -1 ) return true;
                        boost::this_thread::sleep( boost::posix_time::milliseconds( 20 ) );
                    }
                    m_pmutexs_[lock_pos]->unlock();
                    return false;
                }
                boost::this_thread::sleep( boost::posix_time::seconds( 20 ) );
            }

            return false;

#if 0
            if ( m_thread_mutex_.try_lock() )
            {
                lock( lock_pos );
                return true;
            }
            else
            {
                bool ret = false;
                struct sigaction act, oact;

                act.sa_handler = nullhander;
                sigemptyset( &act.sa_mask );
                act.sa_flags = 0;
                sigaction( SIGALRM, &act, &oact );
                int sec = alarm( wait_sec );
                if ( writew_lock( m_fd_, lock_pos + 4, SEEK_SET, 1 ) == 0 )
                {
                    alarm( sec );
                    sigaction( SIGALRM, &oact, NULL );
                    ret = true;
                }
                else
                {
                    alarm( sec );
                    sigaction( SIGALRM, &oact, NULL );
                    ret = false;
                }

                return ret;
            }
#endif

        }

        void FileLocks::unlock( size_t unlock_pos )
        {
            un_lock( m_fd_, unlock_pos + 4, SEEK_SET, 1 );
            m_pmutexs_[unlock_pos]->unlock();/// 确保线程锁解锁后，文件锁是释放状态，方便timed_lock处理。
        }

        void FileLocks::free_lock()
        {

        }

        bool FileLocks::check_thread_lock( size_t lock_pos )
        {
            bool ret = false;
            if ( m_pmutexs_.size() <= lock_pos || m_pmutexs_[lock_pos] == NULL )
            {
                m_thread_mutex_.lock();
                if ( m_pmutexs_.size() <= lock_pos )
                {
                    m_pmutexs_.resize( lock_pos + 1 );
                    ret = true;
                }
                //std::cout << check_thread_lock before new object the size of m_pmutexs_ is " << m_pmutexs_.size() << " address = " << m_pmutexs_[lock_pos] << std::endl;
                //std::cout << pid " << getpid() << " fill boost lock++++++++++++++++++" << std::endl;
                m_pmutexs_[lock_pos] = new boost::mutex;
                //std::cout << check_thread_lock id = " << lock_pos << "lock address = " << m_pmutexs_[lock_pos] << std::endl;
                m_thread_mutex_.unlock();
            }

            return ret;
        }

        boost::filesystem::path FileLocks::namelock_cache( std::string const & filename )
        {
            return (framework::filesystem::temp_path() / filename );
        }
#endif

    } // namespace process
} // namespace framework
