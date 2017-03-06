// MemoryEmulation.h

#pragma once

#include <windows.h>

namespace MemoryEmulation
{

    HLOCAL WINAPI_DECL LocalAlloc(
        _In_  UINT uFlags,
        _In_  SIZE_T uBytes
        );

    HLOCAL WINAPI_DECL LocalFree(
        _In_  HLOCAL hMem
        );

    LPVOID WINAPI_DECL VirtualAlloc(
        _In_opt_  LPVOID lpAddress,
        _In_      SIZE_T dwSize,
        _In_      DWORD flAllocationType,
        _In_      DWORD flProtect
        );

    BOOL WINAPI_DECL VirtualFree(
        _In_  LPVOID lpAddress,
        _In_  SIZE_T dwSize,
        _In_  DWORD dwFreeType
        );

    HANDLE WINAPI_DECL CreateFileMappingA(
        _In_      HANDLE hFile,
        _In_opt_  LPSECURITY_ATTRIBUTES lpAttributes,
        _In_      DWORD flProtect,
        _In_      DWORD dwMaximumSizeHigh,
        _In_      DWORD dwMaximumSizeLow,
        _In_opt_  LPCSTR lpName
        );

    HANDLE WINAPI_DECL OpenFileMappingA(
        _In_ DWORD dwDesiredAccess,
        _In_ BOOL bInheritHandle,
        _In_ LPCSTR lpName
        );

    LPVOID WINAPI_DECL MapViewOfFile(
        _In_  HANDLE hFileMappingObject,
        _In_  DWORD dwDesiredAccess,
        _In_  DWORD dwFileOffsetHigh,
        _In_  DWORD dwFileOffsetLow,
        _In_  SIZE_T dwNumberOfBytesToMap
        );

    LPVOID WINAPI_DECL MapViewOfFileEx(
        _In_      HANDLE hFileMappingObject,
        _In_      DWORD dwDesiredAccess,
        _In_      DWORD dwFileOffsetHigh,
        _In_      DWORD dwFileOffsetLow,
        _In_      SIZE_T dwNumberOfBytesToMap,
        _In_opt_  LPVOID lpBaseAddress
		);

	BOOL WINAPI_DECL FlushViewOfFile(
		_In_  LPCVOID lpBaseAddress,
		_In_  SIZE_T dwNumberOfBytesToFlush
		);

	BOOL WINAPI_DECL UnmapViewOfFile(
		_In_  LPCVOID lpBaseAddress
		);

#define CloseHandle CloseHandle2

	BOOL WINAPI_DECL CloseHandle2(
		_In_  HANDLE hObject
		);

}
