// FileMutex.cpp

#include "tools/framework_test/Common.h"

#include <framework/process/FileMutex.h>
#include <framework/timer/TickCounter.h>
#include <framework/system/ErrorCode.h>
using namespace framework::process;
using namespace framework::system;
using namespace framework::configure;

#ifdef BOOST_WINDOWS_API

#include <iostream>

#else

#include <pthread.h>
#include <sys/wait.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
using namespace boost::system;

#define USE_FILELOCK
//#define USE_TICKCOUNT
//#define TIMEDLOCK_TEST
#define LOCK_TEST

#define MAXLINE 1024
FileMutex filemutex;

void * printinfo( void * arg )
{
    int fd;    
    long seqno = 0;
    pid_t pid;
    size_t n;
    char line[MAXLINE + 1];
    pid = getpid();

    /// 调用前手动创建此文件，用以测试
    fd = open( "locklock", O_RDWR, 0666 );

    for ( int i = 0; i < 100; ++i )
    {
#ifdef USE_FILELOCK
        filemutex.lock();
#endif
        lseek( fd, 0L, SEEK_SET );
        n = read( fd, line, MAXLINE );
        line[n] = '\0';
        n = sscanf( line, "%ld\n", &seqno );
        printf( "%s: pid = %ld, seq# = %ld\n", "FileMutex", (long)pid, seqno );
        seqno++;
        snprintf( line, sizeof(line), "%ld\n", seqno );
        lseek( fd, 0L, SEEK_SET );
        write( fd, line, strlen( line ) );
#ifdef USE_FILELOCK
        filemutex.unlock();
#endif
    }

    return NULL;
}

#endif

/// 文件锁测试
static void test_filemutex(Config & conf)
{
#ifdef BOOST_WINDOWS_API

    std::cout << "Please test it in linux!" << std::endl;

#else

    int pid = 0;

#ifdef LOCK_TEST

#ifdef USE_TICKCOUNT
    boost::uint32_t begtick = framework::timer::TickCounter::tick_count();
#endif // USE_TICKCOUNT

    for ( int i = 0; i < 100; ++i )
    {
        pid = fork();

        if(pid == -1 )
        {
        } 
        else if ( pid == 0 ) 
        {
#ifndef USE_TICKCOUNT
            printinfo(  ( void * )0 );
#else
            filemutex.lock();
            filemutex.unlock();
#endif // USE_TICKCOUNT
            _exit( 0 );
        }
        else
        {
            printf( "The parent process id = %d child process id = %d\n", getpid(), pid );
        }
    }
#ifdef USE_TICKCOUNT
    wait( NULL );
    boost::uint32_t endtick = framework::timer::TickCounter::tick_count();
    printf( "Total process:%d, Total time:%dms\n", 100, endtick - begtick );
#endif // USE_TICKCOUNT

#endif // LOCK_TEST
    
#ifdef TIMEDLOCK_TEST

    pid = fork();

    if(pid == -1 )
    {
    } 
    else if ( pid == 0 ) 
    {
        bool bRet = filemutex.timed_lock( 5 );
        if ( bRet )
        {
            printf( "Child process lock success!\n" );
            filemutex.unlock();
        }
        else
        {
            printf( "Child process lock fail!\n" );
        }
        _exit( 0 );
    }
    else
    {
        printf( "Child process ID:%d\n", pid );
        wait( &pid );
        filemutex.lock();
    }
    printf( "Outer\n", pid );

    filemutex.unlock();
#endif // TIMELOCK_TEST
#endif // BOOST_WINDOWS_API
}

/// 测试多线程下的文件锁
static void test_filemutex_multithread(Config & conf)
{
#ifdef BOOST_WINDOWS_API

    std::cout << "Please test it in linux!" << std::endl;

#else

    for ( int i = 0; i < 100; ++i )
    {
        pthread_t thread;
        if ( ! pthread_create( &thread, NULL, printinfo, NULL ) ) continue;
        pthread_join( thread, NULL );
    }

#endif // BOOST_WINDOWS_API
}

/// 测试多进程下多线程的文件锁
static void test_filemutex_multi_process_thread(Config & conf)
{
#ifdef BOOST_WINDOWS_API

    std::cout << "Please test it in linux!" << std::endl;

#else

    int pid = 0;

    for ( int i = 0; i < 10; ++i )
    {
        pid = fork();

        if(pid < 0 )
        {
        } 
        else if ( pid == 0 ) 
        {
            pthread_t thread[10];
            for ( int j = 0; j < 10; ++j )
            {
                if ( ! pthread_create( &thread[j], NULL, printinfo, NULL ) ) continue;
            }

            for ( int j = 0; j < 10; ++j )
                pthread_join( thread[j], NULL );
            
            _exit( 0 );
        }
        else
        {
        }
    }
    int status;
    do 
    {
        int w = waitpid( -1, &status, WUNTRACED );
        if (w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            printf("exited, status=%d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("killed by signal %d\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("stopped by signal %d\n", WSTOPSIG(status));
        } 
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    printf( "The parent process id = %d child process id = %d\n", getpid(), pid );
#endif // BOOST_WINDOWS_API
}

static TestRegister test("filemutex", test_filemutex );

static TestRegister testt("filemutext", test_filemutex_multithread );

static TestRegister testpt("filemutexpt", test_filemutex_multi_process_thread );
