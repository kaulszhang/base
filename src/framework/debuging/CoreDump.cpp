// CoreDump.cpp

#include "framework/Framework.h"
#include "framework/debuging/CoreDump.h"

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#  include <time.h>
#  include <shlwapi.h>
#  pragma comment(lib, "shlwapi")
#  ifndef __MINGW32__
#    include <DbgHelp.h>
#    pragma comment(lib, "dbghelp")
#  endif
#else
#  include <sys/time.h>
#  include <sys/resource.h>
#endif

#ifdef BOOST_WINDOWS_API

//==============================================================================
//    定义常用的宏
#define ERR_FAILURE 0xFFFFFFFF
#define ERR_SUCCESS 0x00000000

namespace framework
{
    namespace debuging
    {

#ifndef __MINGW32__

        //==============================================================================
        // 根据输入的标志、文件打开一个输出文件并返回句柄
        static HANDLE WINAPI GetDumpFile(LPCTSTR lpszTag, LPCTSTR lpszFile, DWORD dwAccess)
        {
            TCHAR szModule[MAX_PATH] = { TEXT("") };
            GetModuleFileName(NULL,szModule,MAX_PATH);
            LPTSTR lpszPath = PathFindExtension(szModule);
            wsprintf(lpszPath, TEXT(" %s %s"), lpszTag, lpszFile);
            DWORD dwAttr = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH;
            return CreateFile(szModule,dwAccess,0,0,CREATE_ALWAYS,dwAttr,0);
        }

        static LONG WINAPI DumpMini(HANDLE hFile, PEXCEPTION_POINTERS ExceptionInfo)
        {
            // 设置输出信息
            MINIDUMP_EXCEPTION_INFORMATION eInfo;
            eInfo.ThreadId = GetCurrentThreadId();
            eInfo.ExceptionPointers = ExceptionInfo;
            eInfo.ClientPointers = FALSE;

            // 准备进程相关参数
            HANDLE hProc = GetCurrentProcess();
            DWORD nProc = GetCurrentProcessId();
            PMINIDUMP_EXCEPTION_INFORMATION pMDI = ExceptionInfo ? &eInfo : NULL;
            BOOL bDump = MiniDumpWriteDump(hProc,nProc,hFile,MiniDumpNormal,pMDI,NULL,NULL);
            return bDump ? ERR_SUCCESS : ERR_FAILURE ;
        }

        //==================================================================================================
        // 下面的函数为内部一级的函数，提供给外部接口调用
        static LONG WINAPI KeGenerateUniversalSymbol(LPTSTR lpszTag, UINT nCount)
        {
            SYSTEMTIME st; GetLocalTime(&st); 
            DWORD nThread = GetCurrentThreadId();
            DWORD nProcess = GetCurrentProcessId();
            DWORD tClock = (DWORD)time(NULL); srand( GetTickCount() );
            DWORD nTime = st.wHour * 10000 + st.wMinute * 100 + st.wSecond;
            DWORD nDate = (st.wYear % 100) * 10000 + st.wMonth * 100 + st.wDay;
            static const TCHAR szFormat[] = { TEXT("{%08X-%04X-%04X-%04X-%06d%06d}") };
            return wsprintf(lpszTag, szFormat, tClock, nProcess, nThread, rand(), nDate, nTime);
        }

        static LONG WINAPI KeDumpDebugger(LPTSTR lpszTag, PEXCEPTION_POINTERS ExceptionInfo)
        { 
            const TCHAR szMiniDump[] = { TEXT("Crash.DMP") };
            HANDLE hFile = GetDumpFile(lpszTag, szMiniDump, GENERIC_WRITE);
            if(hFile == INVALID_HANDLE_VALUE) return 0;
            DumpMini( hFile, ExceptionInfo );
            CloseHandle( hFile );
            return NO_ERROR;
        }

        //==================================================================================================
        // 该函数为真正的异常过滤器函数，如果使用全局未处理异常句柄，那么就是该函数
        static LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
        {
            // 进程已经处于崩溃时错误信息输出中
            TCHAR szTag[64] = { TEXT("") };
            KeGenerateUniversalSymbol(szTag, 64);
            return KeDumpDebugger(szTag, ExceptionInfo);
        }

        static LONG WINAPI ExceptionHandler(LPEXCEPTION_POINTERS lpExceptionInfo)
        {
            // 如果是在调试器下执行的，那么直接返回给调试器
            //if(IsDebuggerPresent()) 
            {
                //return EXCEPTION_CONTINUE_SEARCH;
            }

            // 进行异常捕获并分析情况返回异常处理值
            return ExceptionFilter(lpExceptionInfo);
        }

#endif

        bool core_dump_install()
        {
#ifndef __MINGW32__
            SetUnhandledExceptionFilter(ExceptionFilter);
            return true;
#else
            return false;
#endif
        }

    }
}

#else

namespace framework
{
    namespace debuging
    {

        bool core_dump_install()
        {
            struct rlimit rlimit;
            rlimit.rlim_cur = 0x80000000;
            rlimit.rlim_max = 0x80000000;
            if (setrlimit(RLIMIT_CORE, &rlimit))
                return false;

            return true;
        }

    }
}

#endif
