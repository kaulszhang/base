// MemoryEmulation.cpp

#include <Windows.h>

#define WINAPI_DECL     __declspec(dllexport)

#include "MemoryEmulation.h"
#include "Charset.h"
using namespace winapi;

#include <assert.h>

//extern "C"
//{

    HLOCAL WINAPI_DECL LocalAlloc(
        _In_  UINT uFlags,
        _In_  SIZE_T uBytes
        )
    {
        assert(uFlags == LMEM_FIXED);
        return new char [uBytes];
    }

    HLOCAL WINAPI_DECL LocalFree(
        _In_  HLOCAL hMem
        )
    {
        delete [] (char *)hMem;
        return NULL;
    }

    LPVOID WINAPI_DECL VirtualAlloc(
        _In_opt_  LPVOID lpAddress,
        _In_      SIZE_T dwSize,
        _In_      DWORD flAllocationType,
        _In_      DWORD flProtect
        )
    {
        assert(lpAddress == NULL);
        assert(flAllocationType == MEM_COMMIT);
        HANDLE hFileMapping = CreateFileMappingFromApp(
            INVALID_HANDLE_VALUE, 
            NULL, 
            flProtect, 
            dwSize, 
            NULL);
        if (hFileMapping == NULL)
            return NULL;
        PVOID addr = MapViewOfFileFromApp(
            hFileMapping, 
            FILE_MAP_ALL_ACCESS, 
            0, 
            dwSize);
        CloseHandle(hFileMapping);
        return addr;
    }

    BOOL WINAPI_DECL VirtualFree(
        _In_  LPVOID lpAddress,
        _In_  SIZE_T dwSize,
        _In_  DWORD dwFreeType
        )
    {
        assert(dwSize == 0);
        assert(dwFreeType == MEM_RELEASE);
        return UnmapViewOfFile(
            lpAddress);
    }

    HANDLE WINAPI_DECL CreateFileMappingA(
        _In_      HANDLE hFile,
        _In_opt_  LPSECURITY_ATTRIBUTES lpAttributes,
        _In_      DWORD flProtect,
        _In_      DWORD dwMaximumSizeHigh,
        _In_      DWORD dwMaximumSizeLow,
        _In_opt_  LPCSTR lpName
        )
    {
        charset_t charset(lpName);
        if (charset.ec()) {
            return NULL;
        }
        HANDLE hFileMapping = CreateFileMappingW(
            hFile, 
            lpAttributes, 
            flProtect, 
            dwMaximumSizeHigh, 
            dwMaximumSizeLow, 
            charset.wstr());
        return hFileMapping;
    }

    HANDLE WINAPI_DECL CreateFileMappingW(
        _In_      HANDLE hFile,
        _In_opt_  LPSECURITY_ATTRIBUTES lpAttributes,
        _In_      DWORD flProtect,
        _In_      DWORD dwMaximumSizeHigh,
        _In_      DWORD dwMaximumSizeLow,
        _In_opt_  LPCWSTR lpName
        )
    {
        ULONG64 ulMaximumSize = (ULONG64)dwMaximumSizeHigh << 32 | dwMaximumSizeLow;
        HANDLE hFileMapping = CreateFileMappingFromApp(
            hFile, 
            lpAttributes, 
            flProtect, 
            ulMaximumSize , 
            lpName);
        return hFileMapping;
    }

    HANDLE WINAPI_DECL OpenFileMappingA(
        _In_ DWORD dwDesiredAccess,
        _In_ BOOL bInheritHandle,
        _In_ LPCSTR lpName
        )
    {
        HANDLE hFileMapping = CreateFileMappingA(
            INVALID_HANDLE_VALUE, 
            NULL, 
            dwDesiredAccess, 
            0, 
            0, 
            lpName);
        if (hFileMapping != NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
            SetLastError(0);
        }
        return hFileMapping;
    }

    HANDLE WINAPI_DECL OpenFileMappingW(
        _In_ DWORD dwDesiredAccess,
        _In_ BOOL bInheritHandle,
        _In_ LPCWSTR lpName
        )
    {
        HANDLE hFileMapping = CreateFileMappingW(
            INVALID_HANDLE_VALUE, 
            NULL, 
            dwDesiredAccess, 
            0, 
            0, 
            lpName);
        if (hFileMapping != NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
            SetLastError(0);
        }
        return hFileMapping;
    }

    LPVOID WINAPI_DECL MapViewOfFile(
        _In_  HANDLE hFileMappingObject,
        _In_  DWORD dwDesiredAccess,
        _In_  DWORD dwFileOffsetHigh,
        _In_  DWORD dwFileOffsetLow,
        _In_  SIZE_T dwNumberOfBytesToMap
        )
    {
        ULONG64 ulFileOffset = (ULONG64)dwFileOffsetHigh << 32 | dwFileOffsetLow;
        return MapViewOfFileFromApp(
            hFileMappingObject, 
            dwDesiredAccess, 
            ulFileOffset, 
            dwNumberOfBytesToMap);
    }

    LPVOID WINAPI_DECL MapViewOfFileEx(
        _In_      HANDLE hFileMappingObject,
        _In_      DWORD dwDesiredAccess,
        _In_      DWORD dwFileOffsetHigh,
        _In_      DWORD dwFileOffsetLow,
        _In_      SIZE_T dwNumberOfBytesToMap,
        _In_opt_  LPVOID lpBaseAddress
        )
    {
        assert(lpBaseAddress == NULL);
        ULONG64 ulFileOffset = (ULONG64)dwFileOffsetHigh << 32 | dwFileOffsetLow;
        return MapViewOfFileFromApp(
            hFileMappingObject, 
            dwDesiredAccess, 
            ulFileOffset, 
            dwNumberOfBytesToMap);
    }

//}
