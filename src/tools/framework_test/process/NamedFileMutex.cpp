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


#define MAXLINE 1024


void * check_namefilelock( void * arg )
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
        FileMutex namefilemutex( "namekey" );
        namefilemutex.lock();
        lseek( fd, 0L, SEEK_SET );
        n = read( fd, line, MAXLINE );
        line[n] = '\0';
        n = sscanf( line, "%ld\n", &seqno );
        printf( "%s: pid = %ld, seq = %ld\n", "FileMutex", (long)pid, seqno );
        seqno++;
        snprintf( line, sizeof(line), "%ld\n", seqno );
        lseek( fd, 0L, SEEK_SET );
        write( fd, line, strlen( line ) );
        namefilemutex.unlock();
    }

    return NULL;
}

void * check_namefilelock1( void * arg )
{
    int fd;    
    long seqno = 0;
    pid_t pid;
    size_t n;
    char line[MAXLINE + 1];
    pid = getpid();

    

    /// 调用前手动创建此文件，用以测试
    fd = open( "locklock1", O_RDWR, 0666 );

    for ( int i = 0; i < 100; ++i )
    {
        FileMutex namefilemutex( "namekey1" );
        namefilemutex.lock();
        lseek( fd, 0L, SEEK_SET );
        n = read( fd, line, MAXLINE );
        line[n] = '\0';
        n = sscanf( line, "%ld\n", &seqno );
        printf( "%s: pid = %ld, seq = %ld\n", "FileMutex", (long)pid, seqno );
        seqno++;
        snprintf( line, sizeof(line), "%ld\n", seqno );
        lseek( fd, 0L, SEEK_SET );
        write( fd, line, strlen( line ) );
        namefilemutex.unlock();
    }

    return NULL;
}

#endif

/// 命名文件锁测试
static void test_namefilemutex(Config & conf)
{
#ifdef BOOST_WINDOWS_API

    std::cout << "Please test it in linux!" << std::endl;

#else

    int pid = 0;

    //FileMutex namefilemutex( "namekey" );
    //FileMutex namefilemutex1( "namekey1" );

    for ( int i = 0; i < 100; ++i )
    {
        pid = fork();

        if(pid == -1 )
        {
        } 
        else if ( pid == 0 ) 
        {
            if ( i % 2 )
            {
                //FileMutex namefilemutex( "namekey" );
                check_namefilelock1(  ( void * )0 );
            }
            else
            {
                //FileMutex namefilemutex( "namekey1" );
                check_namefilelock(  ( void * )0 );
            }
            _exit( 0 );
        }
        else
        {
            printf( "The parent process id = %d child process id = %d\n", getpid(), pid );
        }
    }

#endif // BOOST_WINDOWS_API
}

/// 测试多线程下的文件锁
static void test_namefilemutex_multithread(Config & conf)
{
#ifdef BOOST_WINDOWS_API

    std::cout << "Please test it in linux!" << std::endl;

#else

    FileMutex namefilemutex( "namekey" );
    pthread_t threads[100];
    for ( int i = 0; i < 100; ++i )
    {
        if ( i % 2 )
        {
            //FileMutex namefilemutex( "namekey" );
            if ( ! pthread_create( &threads[i], NULL, check_namefilelock, NULL ) ) continue;
        }
        else
        {
            //FileMutex namefilemutex( "namekey1" );
            if ( ! pthread_create( &threads[i], NULL, check_namefilelock1, NULL ) ) continue;
        }
    }

    for ( int i = 0; i < 100; ++i )
        pthread_join( threads[i], NULL );

#endif // BOOST_WINDOWS_API
}

static void test_namefilemutex_multi_process_thread(Config & conf)
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
            if ( i % 2 )
            {
                FileMutex namefilemutex( "namekey" );
                for ( int j = 0; j < 10; ++j )
                {
                    if ( ! pthread_create( &thread[j], NULL, check_namefilelock, NULL ) ) continue;
                }

                for ( int j = 0; j < 10; ++j )
                    pthread_join( thread[j], NULL );
            }
            else
            {
                FileMutex namefilemutex( "namekey1" );
                for ( int j = 0; j < 10; ++j )
                {
                    if ( ! pthread_create( &thread[j], NULL, check_namefilelock1, NULL ) ) continue;
                }

                for ( int j = 0; j < 10; ++j )
                    pthread_join( thread[j], NULL );
            }

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


static TestRegister namefilemutextest("namefilemutex", test_namefilemutex );

static TestRegister namefilemutextestmt("namefilemutexmt", test_namefilemutex_multithread );

static TestRegister namefilemutextestmpmt("namefilemutexmpmt", test_namefilemutex_multi_process_thread );
