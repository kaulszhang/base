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
//    ���峣�õĺ�
#define ERR_FAILURE 0xFFFFFFFF
#define ERR_SUCCESS 0x00000000

namespace framework
{
    namespace debuging
    {

#ifndef __MINGW32__

        //==============================================================================
        // ��������ı�־���ļ���һ������ļ������ؾ��
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
            // ���������Ϣ
            MINIDUMP_EXCEPTION_INFORMATION eInfo;
            eInfo.ThreadId = GetCurrentThreadId();
            eInfo.ExceptionPointers = ExceptionInfo;
            eInfo.ClientPointers = FALSE;

            // ׼��������ز���
            HANDLE hProc = GetCurrentProcess();
            DWORD nProc = GetCurrentProcessId();
            PMINIDUMP_EXCEPTION_INFORMATION pMDI = ExceptionInfo ? &eInfo : NULL;
            BOOL bDump = MiniDumpWriteDump(hProc,nProc,hFile,MiniDumpNormal,pMDI,NULL,NULL);
            return bDump ? ERR_SUCCESS : ERR_FAILURE ;
        }

        //==================================================================================================
        // ����ĺ���Ϊ�ڲ�һ���ĺ������ṩ���ⲿ�ӿڵ���
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
        // �ú���Ϊ�������쳣���������������ʹ��ȫ��δ�����쳣�������ô���Ǹú���
        static LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
        {
            // �����Ѿ����ڱ���ʱ������Ϣ�����
            TCHAR szTag[64] = { TEXT("") };
            KeGenerateUniversalSymbol(szTag, 64);
            return KeDumpDebugger(szTag, ExceptionInfo);
        }

        static LONG WINAPI ExceptionHandler(LPEXCEPTION_POINTERS lpExceptionInfo)
        {
            // ������ڵ�������ִ�еģ���ôֱ�ӷ��ظ�������
            //if(IsDebuggerPresent()) 
            {
                //return EXCEPTION_CONTINUE_SEARCH;
            }

            // �����쳣���񲢷�����������쳣����ֵ
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
