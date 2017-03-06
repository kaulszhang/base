// MemoryEmulation.cpp

#include <Windows.h>

#define WINAPI_DECL     __declspec(dllexport)

#include "Charset.h"
using namespace SystemEmulation;
#include "FileSystemEmulation.h"
using namespace FileSystemEmulation;

#include <mutex>
#include <vector>
#include <algorithm>

#include <assert.h>

namespace MemoryEmulation
{

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
        return new char [dwSize];
    }

    BOOL WINAPI_DECL VirtualFree(
        _In_  LPVOID lpAddress,
        _In_  SIZE_T dwSize,
        _In_  DWORD dwFreeType
        )
    {
        assert(dwSize == 0);
        assert(dwFreeType == MEM_RELEASE);
		delete [] (char *)lpAddress;
		return TRUE;
    }

	struct FileMap
	{
		CHAR lpName[MAX_PATH];
		CHAR lpFileName[MAX_PATH];
		LPVOID lpAddress;
		HANDLE hEvent;
		DWORD dwRefCount;

		struct find_by_name
		{
			find_by_name(
				LPCSTR lpName)
				: lpName_(lpName)
			{
			}

			bool operator()(
				FileMap * m)
			{
				return strcmp(lpName_, m->lpName) == 0;
			}

			LPCSTR lpName_;
		};

		struct find_by_file_name
		{
			find_by_file_name(
				LPCSTR lpFileName)
				: lpFileName_(lpFileName)
			{
			}

			bool operator()(
				FileMap * m)
			{
				return strcmp(lpFileName_, m->lpFileName) == 0;
			}

			LPCSTR lpFileName_;
		};

		struct find_by_addr
		{
			find_by_addr(
				LPCVOID lpAddr)
				: lpAddr_(lpAddr)
			{
			}

			bool operator()(
				FileMap * m)
			{
				return lpAddr_ == m->lpAddress;
			}

			LPCVOID lpAddr_;
		};

		struct find_by_event
		{
			find_by_event(
				HANDLE hEvent)
				: hEvent_(hEvent)
			{
			}

			bool operator()(
				FileMap * m)
			{
				return hEvent_ == m->hEvent;
			}

			HANDLE hEvent_;
		};
	};

	static std::mutex mutex_file_map_;
	static std::vector<FileMap *> file_maps_;

    HANDLE WINAPI_DECL CreateFileMappingA(
        _In_      HANDLE hFile,
        _In_opt_  LPSECURITY_ATTRIBUTES lpAttributes,
        _In_      DWORD flProtect,
        _In_      DWORD dwMaximumSizeHigh,
        _In_      DWORD dwMaximumSizeLow,
        _In_opt_  LPCSTR lpName
        )
    {
        std::vector<FileMap *>::iterator iter = file_maps_.end();
        LPCSTR pFileName = NULL;
        if (lpName) {
			iter = std::find_if(file_maps_.begin(), file_maps_.end(), FileMap::find_by_name(lpName));
        } else if (hFile != INVALID_HANDLE_VALUE) {
            CHAR * buf = new CHAR[MAX_PATH + 2];
            charset_t charset(buf, MAX_PATH + 2);
            FILE_NAME_INFO * pFileNameInfo = (FILE_NAME_INFO *)charset.wstr();
            GetFileInformationByHandleEx(
                hFile, 
                FILE_INFO_BY_HANDLE_CLASS::FileNameInfo, 
                pFileNameInfo, 
                charset.wlen() * sizeof(WCHAR));
            pFileNameInfo->FileNameLength /= 2;
            pFileNameInfo->FileName[pFileNameInfo->FileNameLength++] = 0;
            charset.wlen(pFileNameInfo->FileNameLength + 2);
            pFileNameInfo->FileNameLength = 0;
            charset.w2a();
			iter = std::find_if(file_maps_.begin(), file_maps_.end(), FileMap::find_by_file_name(buf + 2));
            pFileName = buf + 2;
        }
		FileMap * map = NULL;
		if (iter == file_maps_.end()) {
			DWORD dwSize = dwMaximumSizeLow;
			if (hFile != INVALID_HANDLE_VALUE) {
				dwSize = GetFileSize(hFile, NULL);
				if (dwMaximumSizeLow > 0 && dwMaximumSizeLow < dwSize) {
					dwSize = dwMaximumSizeLow;
				}
			}
			map = new FileMap;
            if (lpName) {
                strcpy_s(map->lpName, lpName);
            } else {
                map->lpName[0] = 0;
            }
            if (pFileName) {
                strcpy_s(map->lpFileName, pFileName);
            } else {
                map->lpFileName[0] = 0;
            }
			map->lpAddress = new char[dwSize];
			map->hEvent = CreateEventExW(
				NULL, 
				NULL, 
				0, 
				EVENT_ALL_ACCESS);
			map->dwRefCount = 1;
			file_maps_.insert(file_maps_.end(), map);
		} else {
			map = *iter;
			++map->dwRefCount;
			SetLastError(ERROR_ALREADY_EXISTS);
		}
        if (pFileName) {
            delete [] (pFileName - 2);
        }
		return map->hEvent;
    }

    HANDLE WINAPI_DECL OpenFileMappingA(
        _In_ DWORD dwDesiredAccess,
        _In_ BOOL bInheritHandle,
        _In_ LPCSTR lpName
        )
    {
        std::vector<FileMap *>::iterator iter = 
			std::find_if(file_maps_.begin(), file_maps_.end(), FileMap::find_by_name(lpName));
		if (iter == file_maps_.end()) {
            SetLastError(ERROR_NOT_FOUND);
			return NULL;
		}
		FileMap * map = *iter;
		++map->dwRefCount;
		return map->hEvent;
    }

    LPVOID WINAPI_DECL MapViewOfFile(
        _In_  HANDLE hFileMappingObject,
        _In_  DWORD dwDesiredAccess,
        _In_  DWORD dwFileOffsetHigh,
        _In_  DWORD dwFileOffsetLow,
        _In_  SIZE_T dwNumberOfBytesToMap
        )
    {
        std::vector<FileMap *>::iterator iter = 
			std::find_if(file_maps_.begin(), file_maps_.end(), FileMap::find_by_event(hFileMappingObject));
		if (iter == file_maps_.end()) {
            SetLastError(ERROR_INVALID_HANDLE);
			return NULL;
		}
		FileMap * map = *iter;
		++map->dwRefCount;
        return map->lpAddress;
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
        return MapViewOfFile(
            hFileMappingObject, 
            dwDesiredAccess, 
            dwFileOffsetHigh, 
			dwFileOffsetLow, 
            dwNumberOfBytesToMap);
    }

	BOOL WINAPI_DECL FlushViewOfFile(
		_In_  LPCVOID lpBaseAddress,
		_In_  SIZE_T dwNumberOfBytesToFlush
		)
	{
		return TRUE;
	}

	BOOL WINAPI_DECL UnmapViewOfFile(
		_In_  LPCVOID lpBaseAddress
		)
	{
        std::vector<FileMap *>::iterator iter = 
			std::find_if(file_maps_.begin(), file_maps_.end(), FileMap::find_by_addr(lpBaseAddress));
		if (iter == file_maps_.end()) {
            SetLastError(ERROR_INVALID_HANDLE);
			return FALSE;
		}
		FileMap * map = *iter;
		if (--map->dwRefCount == 0) {
			delete [] (char *)map->lpAddress;
			file_maps_.erase(iter);
			delete map;
		}
        return TRUE;
	}

#undef CloseHandle

	BOOL WINAPI_DECL CloseHandle2(
		_In_  HANDLE hObject
		)
	{
        std::vector<FileMap *>::iterator iter = 
			std::find_if(file_maps_.begin(), file_maps_.end(), FileMap::find_by_event(hObject));
		if (iter != file_maps_.end()) {
			FileMap * map = *iter;
			if (--map->dwRefCount == 0) {
				delete [] (char *)map->lpAddress;
				file_maps_.erase(iter);
                ::CloseHandle(map->hEvent);
				delete map;
			}
            return TRUE;
		}
		return ::CloseHandle(hObject);
	}

}
