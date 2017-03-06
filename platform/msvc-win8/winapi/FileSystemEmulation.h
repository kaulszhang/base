// FileSystemEmulation.h

#pragma once

#include <windows.h>

//#ifdef __cplusplus
//extern "C"
//{
//#endif

    /* wrong fileapi.h file, defined some functions but can't be use
     * CreateDirectoryA
     * RemoveDirectoryA
     * DeleteFileA
     * MoveFileExA
     */
    BOOL WINAPI_DECL CreateDirectory2A(
        _In_      LPCSTR lpPathName,
        _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
        );

#define CreateDirectoryA CreateDirectory2A

#ifdef UNICODE
//#define CreateDirectory  CreateDirectoryW
#else
#define CreateDirectory  CreateDirectoryA
#endif // !UNICODE

    BOOL WINAPI_DECL RemoveDirectory2A(
        _In_  LPCSTR lpPathName
        );

#define RemoveDirectoryA RemoveDirectory2A

#ifdef UNICODE
//#define RemoveDirectory  RemoveDirectoryW
#else
#define RemoveDirectory  RemoveDirectoryA
#endif // !UNICODE

    DWORD WINAPI_DECL GetFileAttributesA(
        _In_  LPCSTR lpFileName
        );

    DWORD WINAPI_DECL GetFileAttributesW(
        _In_  LPCWSTR lpFileName
        );

#ifdef UNICODE
#define GetFileAttributes  GetFileAttributesW
#else
#define GetFileAttributes  GetFileAttributesA
#endif // !UNICODE

    HANDLE WINAPI_DECL CreateFileA(
        _In_      LPCSTR lpFileName,
        _In_      DWORD dwDesiredAccess,
        _In_      DWORD dwShareMode,
        _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        _In_      DWORD dwCreationDisposition,
        _In_      DWORD dwFlagsAndAttributes,
        _In_opt_  HANDLE hTemplateFile
        );

    HANDLE WINAPI_DECL CreateFileW(
        _In_      LPCWSTR lpFileName,
        _In_      DWORD dwDesiredAccess,
        _In_      DWORD dwShareMode,
        _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        _In_      DWORD dwCreationDisposition,
        _In_      DWORD dwFlagsAndAttributes,
        _In_opt_  HANDLE hTemplateFile
        );

#ifdef UNICODE
#define CreateFile  CreateFileW
#else
#define CreateFile  CreateFileA
#endif // !UNICODE

    BOOL WINAPI_DECL DeleteFile2A(
        _In_  LPCSTR lpFileName
        );

#define DeleteFileA DeleteFile2A

    BOOL WINAPI_DECL MoveFileEx2A(
        _In_      LPCSTR lpExistingFileName,
        _In_opt_  LPCSTR lpNewFileName,
        _In_      DWORD dwFlags
        );

#define MoveFileExA MoveFileEx2A

    DWORD WINAPI_DECL SetFilePointer(
        _In_         HANDLE hFile,
        _In_         LONG lDistanceToMove,
        _Inout_opt_  PLONG lpDistanceToMoveHigh,
        _In_         DWORD dwMoveMethod
        );

    DWORD WINAPI_DECL GetFileSize(
        _In_       HANDLE hFile,
        _Out_opt_  LPDWORD lpFileSizeHigh
        );

    BOOL WINAPI_DECL GetFileSizeEx(
        _In_   HANDLE hFile,
        _Out_  PLARGE_INTEGER lpFileSize
        );

    /* fileapi.h file has following functions, but we need combine with winrt token files
     * SetFilePointerEx
     * ReadFile
     * WriteFile
     */

#define SetFilePointerEx SetFilePointerEx2

    BOOL WINAPI_DECL SetFilePointerEx2(
        _In_       HANDLE hFile,
        _In_       LARGE_INTEGER liDistanceToMove,
        _Out_opt_  PLARGE_INTEGER lpNewFilePointer,
        _In_       DWORD dwMoveMethod
        );

#define ReadFile ReadFile2

    BOOL WINAPI_DECL ReadFile2(
        _In_         HANDLE hFile,
        _Out_        LPVOID lpBuffer,
        _In_         DWORD nNumberOfBytesToRead,
        _Out_opt_    LPDWORD lpNumberOfBytesRead,
        _Inout_opt_  LPOVERLAPPED lpOverlapped
        );

#define WriteFile WriteFile2

    BOOL WINAPI_DECL WriteFile2(
        _In_         HANDLE hFile,
        _In_         LPCVOID lpBuffer,
        _In_         DWORD nNumberOfBytesToWrite,
        _Out_opt_    LPDWORD lpNumberOfBytesWritten,
        _Inout_opt_  LPOVERLAPPED lpOverlapped
        );

    DWORD WINAPI_DECL GetCurrentDirectoryA(
        _In_   DWORD nBufferLength,
        _Out_  LPSTR lpBuffer
        );

    DWORD WINAPI_DECL GetCurrentDirectoryW(
        _In_   DWORD nBufferLength,
        _Out_  LPWSTR lpBuffer
        );

#ifdef UNICODE
#define GetCurrentDirectory  GetCurrentDirectoryW
#else
#define GetCurrentDirectory  GetCurrentDirectoryA
#endif // !UNICODE

    BOOL WINAPI_DECL SetCurrentDirectoryA(
        _In_  LPCSTR lpPathName
        );

    BOOL WINAPI_DECL SetCurrentDirectoryW(
        _In_  LPCWSTR lpPathName
        );

#ifdef UNICODE
#define SetCurrentDirectory  SetCurrentDirectoryW
#else
#define SetCurrentDirectory  SetCurrentDirectoryA
#endif // !UNICODE

    BOOL WINAPI_DECL CreateHardLinkA(
        _In_        LPCSTR lpFileName,
        _In_        LPCSTR lpExistingFileName,
        _Reserved_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
        );

    BOOL WINAPI_DECL CreateHardLinkW(
        _In_        LPCWSTR lpFileName,
        _In_        LPCWSTR lpExistingFileName,
        _Reserved_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
        );

#ifdef UNICODE
#define CreateHardLink  CreateHardLinkW
#else
#define CreateHardLink  CreateHardLinkA
#endif // !UNICODE

    DWORD WINAPI_DECL GetFullPathNameA(
        _In_   LPCSTR lpFileName,
        _In_   DWORD nBufferLength,
        _Out_  LPSTR lpBuffer,
        _Out_  LPSTR *lpFilePart
        );

    DWORD WINAPI_DECL GetFullPathNameW(
        _In_   LPCWSTR lpFileName,
        _In_   DWORD nBufferLength,
        _Out_  LPWSTR lpBuffer,
        _Out_  LPWSTR *lpFilePart
        );

#ifdef UNICODE
#define GetFullPathName  GetFullPathNameW
#else
#define GetFullPathName  GetFullPathNameA
#endif // !UNICODE

    DWORD WINAPI_DECL GetShortPathNameA(
        _In_   LPCSTR lpszLongPath,
        _Out_  LPSTR lpszShortPath,
        _In_   DWORD cchBuffer
        );

    DWORD WINAPI_DECL GetShortPathNameW(
        _In_   LPCWSTR lpszLongPath,
        _Out_  LPWSTR lpszShortPath,
        _In_   DWORD cchBuffer
        );

#ifdef UNICODE
#define GetShortPathName  GetShortPathNameW
#else
#define GetShortPathName  GetShortPathNameA
#endif // !UNICODE

    BOOL WINAPI_DECL MoveFileA(
        _In_  LPCSTR lpExistingFileName,
        _In_  LPCSTR lpNewFileName
        );

    BOOL WINAPI_DECL MoveFileW(
        _In_  LPCWSTR lpExistingFileName,
        _In_  LPCWSTR lpNewFileName
        );

#ifdef UNICODE
#define MoveFile  MoveFileW
#else
#define MoveFile  MoveFileA
#endif // !UNICODE

    BOOL WINAPI_DECL CopyFileA(
        _In_  LPCSTR lpExistingFileName,
        _In_  LPCSTR lpNewFileName,
        _In_  BOOL bFailIfExists
        );

    BOOL WINAPI_DECL CopyFileW(
        _In_  LPCWSTR lpExistingFileName,
        _In_  LPCWSTR lpNewFileName,
        _In_  BOOL bFailIfExists
        );

#ifdef UNICODE
#define CopyFile  CopyFileW
#else
#define CopyFile  CopyFileA
#endif // !UNICODE

    HANDLE WINAPI_DECL FindFirstFileA(
        _In_   LPCSTR lpFileName,
        _Out_  LPWIN32_FIND_DATAA lpFindFileData
        );

    HANDLE WINAPI_DECL FindFirstFileW(
        _In_   LPCWSTR lpFileName,
        _Out_  LPWIN32_FIND_DATAW lpFindFileData
        );

#ifdef UNICODE
#define FindFirstFile  FindFirstFileW
#else
#define FindFirstFile  FindFirstFileA
#endif // !UNICODE

    typedef struct _BY_HANDLE_FILE_INFORMATION {
        DWORD    dwFileAttributes;
        FILETIME ftCreationTime;
        FILETIME ftLastAccessTime;
        FILETIME ftLastWriteTime;
        DWORD    dwVolumeSerialNumber;
        DWORD    nFileSizeHigh;
        DWORD    nFileSizeLow;
        DWORD    nNumberOfLinks;
        DWORD    nFileIndexHigh;
        DWORD    nFileIndexLow;
    } BY_HANDLE_FILE_INFORMATION, *LPBY_HANDLE_FILE_INFORMATION;

    BOOL WINAPI_DECL GetFileInformationByHandle(
        _In_   HANDLE hFile,
        _Out_  LPBY_HANDLE_FILE_INFORMATION lpFileInformation
        );

    BOOL WINAPI_DECL GetFileTime(
        _In_       HANDLE hFile,
        _Out_opt_  LPFILETIME lpCreationTime,
        _Out_opt_  LPFILETIME lpLastAccessTime,
        _Out_opt_  LPFILETIME lpLastWriteTime
        );

    BOOL WINAPI_DECL SetFileTime(
        _In_      HANDLE hFile,
        _In_opt_  const FILETIME *lpCreationTime,
        _In_opt_  const FILETIME *lpLastAccessTime,
        _In_opt_  const FILETIME *lpLastWriteTime
        );

    DWORD WINAPI_DECL GetTempPathA(
        _In_   DWORD nBufferLength,
        _Out_  LPSTR lpBuffer
        );

    DWORD WINAPI_DECL GetTempPathW(
        _In_   DWORD nBufferLength,
        _Out_  LPWSTR lpBuffer
        );

#ifdef UNICODE
#define GetTempPath  GetTempPathW
#else
#define GetTempPath  GetTempPathA
#endif // !UNICODE

#define CloseHandle CloseHandle2

    BOOL WINAPI_DECL CloseHandle2(
        _In_  HANDLE hObject
        );

//#ifdef __cplusplus
//}
//#endif
