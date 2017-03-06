// FileSystemEmulation.cpp

#include <Windows.h>

#define WINAPI_DECL     __declspec(dllexport)

#include "FileSystemEmulation.h"
#include "Charset.h"
using namespace SystemEmulation;

#include <memoryapi.h>

#include <assert.h>

namespace FileSystemEmulation
{

    BOOL WINAPI_DECL CreateDirectory2A(
        _In_      LPCSTR lpPathName,
        _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
        )
    {
        charset_t charset(lpPathName);
        if (charset.wstr() == NULL) {
            return FALSE;
        }
        BOOL b = CreateDirectoryW(
            charset.wstr(), 
            lpSecurityAttributes);
        return b;
    }

    BOOL WINAPI_DECL RemoveDirectory2A(
        _In_  LPCSTR lpPathName
        )
    {
        charset_t charset(lpPathName);
        if (charset.wstr() == NULL) {
            return FALSE;
        }
        BOOL b = RemoveDirectoryW(
            charset.wstr());
        return b;
    }

    DWORD WINAPI_DECL GetFileAttributesA(
        _In_  LPCSTR lpFileName
        )
    {
        charset_t charset(lpFileName);
        if (charset.wstr() == NULL) {
            return INVALID_FILE_ATTRIBUTES;
        }
        WIN32_FILE_ATTRIBUTE_DATA fileInformation;
        BOOL b = GetFileAttributesExW(
            charset.wstr(), 
            GetFileExInfoStandard, 
            &fileInformation);
        return b ? fileInformation.dwFileAttributes : INVALID_FILE_ATTRIBUTES;
    }

    DWORD WINAPI_DECL GetFileAttributesW(
        _In_  LPCWSTR lpFileName
        )
    {
        WIN32_FILE_ATTRIBUTE_DATA fileInformation;
        BOOL b = GetFileAttributesExW(
            lpFileName, 
            GetFileExInfoStandard, 
            &fileInformation);
        return b ? fileInformation.dwFileAttributes : INVALID_FILE_ATTRIBUTES;
    }

    DWORD WINAPI_DECL GetFileAttributesEx2A(
        _In_ LPCSTR lpFileName,
        _In_ GET_FILEEX_INFO_LEVELS fInfoLevelId,
        _Out_writes_bytes_(sizeof(WIN32_FILE_ATTRIBUTE_DATA)) LPVOID lpFileInformation
        )
    {
        charset_t charset(lpFileName);
        if (charset.wstr() == NULL) {
            return INVALID_FILE_ATTRIBUTES;
        }
        WIN32_FILE_ATTRIBUTE_DATA fileInformation;
        BOOL b = GetFileAttributesExW(
            charset.wstr(),
            GetFileExInfoStandard,
            &fileInformation);
        return b ? fileInformation.dwFileAttributes : INVALID_FILE_ATTRIBUTES;
    }

    HANDLE WINAPI_DECL CreateFileA(
        _In_      LPCSTR lpFileName,
        _In_      DWORD dwDesiredAccess,
        _In_      DWORD dwShareMode,
        _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        _In_      DWORD dwCreationDisposition,
        _In_      DWORD dwFlagsAndAttributes,
        _In_opt_  HANDLE hTemplateFile
        )
    {
        charset_t charset(lpFileName);
        if (charset.wstr() == NULL) {
            return INVALID_HANDLE_VALUE;
        }
        HANDLE hFile = CreateFileW(
            charset.wstr(), 
            dwDesiredAccess, 
            dwShareMode, 
            lpSecurityAttributes, 
            dwCreationDisposition, 
            dwFlagsAndAttributes, 
            hTemplateFile);
        return hFile;
    }

    HANDLE WINAPI_DECL CreateFileW(
        _In_      LPCWSTR lpFileName,
        _In_      DWORD dwDesiredAccess,
        _In_      DWORD dwShareMode,
        _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        _In_      DWORD dwCreationDisposition,
        _In_      DWORD dwFlagsAndAttributes,
        _In_opt_  HANDLE hTemplateFile
        )
    {
        CREATEFILE2_EXTENDED_PARAMETERS createExParams;
        createExParams.dwSize = sizeof(createExParams);
        createExParams.dwFileAttributes = dwFlagsAndAttributes;
        createExParams.dwFileFlags = 0;
        createExParams.dwSecurityQosFlags = 0;
        createExParams.lpSecurityAttributes = lpSecurityAttributes;
        createExParams.hTemplateFile = hTemplateFile;
        HANDLE hFile = CreateFile2(
            lpFileName, 
            dwDesiredAccess, 
            dwShareMode, 
            dwCreationDisposition, 
            &createExParams);
        return hFile;
    }

    BOOL WINAPI_DECL DeleteFile2A(
        _In_  LPCSTR lpFileName
        )
    {
        charset_t charset(lpFileName);
        if (charset.wstr() == NULL) {
            return FALSE;
        }
        BOOL b = DeleteFileW(
            charset.wstr());
        return b;
    }

    BOOL WINAPI_DECL MoveFileEx2A(
        _In_      LPCSTR lpExistingFileName,
        _In_opt_  LPCSTR lpNewFileName,
        _In_      DWORD dwFlags
        )
    {
        charset_t charset1(lpExistingFileName);
        if (charset1.wstr() == NULL) {
            return FALSE;
        }
        charset_t charset2(lpNewFileName);
        if (charset2.wstr() == NULL) {
            return FALSE;
        }
        BOOL b = MoveFileExW(
            charset1.wstr(), 
            charset2.wstr(), 
            dwFlags);
        return b;
    }

    DWORD WINAPI_DECL SetFilePointer(
        _In_         HANDLE hFile,
        _In_         LONG lDistanceToMove,
        _Inout_opt_  PLONG lpDistanceToMoveHigh,
        _In_         DWORD dwMoveMethod
        )
    {
        LARGE_INTEGER liDistanceToMove;
        LARGE_INTEGER liNewFilePointer;
        liDistanceToMove.LowPart = lDistanceToMove;
        if (lpDistanceToMoveHigh)
            liDistanceToMove.HighPart = *lpDistanceToMoveHigh;
        else
            liDistanceToMove.HighPart = 0;
        BOOL b = SetFilePointerEx(
            hFile, 
            liDistanceToMove, 
            &liNewFilePointer, 
            dwMoveMethod);
        if (b == FALSE) {
            return  INVALID_SET_FILE_POINTER;
        }
        if (lpDistanceToMoveHigh)
            *lpDistanceToMoveHigh = liNewFilePointer.HighPart;
        return liNewFilePointer.LowPart;
    }

    DWORD WINAPI_DECL GetFileSize(
        _In_       HANDLE hFile,
        _Out_opt_  LPDWORD lpFileSizeHigh
        )
    {
        FILE_STANDARD_INFO standardInfo;
        BOOL b = GetFileInformationByHandleEx(
            hFile, 
            FileStandardInfo, 
            &standardInfo, 
            sizeof(standardInfo));
        if (b == FALSE)
            return INVALID_FILE_SIZE;
        return standardInfo.EndOfFile.LowPart;    
    }

    BOOL WINAPI_DECL GetFileSizeEx(
        _In_   HANDLE hFile,
        _Out_  PLARGE_INTEGER lpFileSize
        )
    {
        FILE_STANDARD_INFO standardInfo;
        BOOL b = GetFileInformationByHandleEx(
            hFile, 
            FileStandardInfo, 
            &standardInfo, 
            sizeof(standardInfo));
        if (b == FALSE)
            return FALSE;
        *lpFileSize = standardInfo.EndOfFile;
        return TRUE;
    }

    DWORD WINAPI_DECL GetCurrentDirectoryA(
        _In_   DWORD nBufferLength,
        _Out_  LPSTR lpBuffer
        )
    {
        return 0;
    }

    DWORD WINAPI_DECL GetCurrentDirectoryW(
        _In_   DWORD nBufferLength,
        _Out_  LPWSTR lpBuffer
        )
    {
        assert(false);
        return 0;
    }

    BOOL WINAPI_DECL SetCurrentDirectoryA(
        _In_  LPCSTR lpPathName
        )
    {
        assert(false);
        return FALSE;
    }

    BOOL WINAPI_DECL SetCurrentDirectoryW(
        _In_  LPCWSTR lpPathName
        )
    {
        assert(false);
        return FALSE;
    }

    BOOL WINAPI_DECL CreateHardLinkA(
        _In_        LPCSTR lpFileName,
        _In_        LPCSTR lpExistingFileName,
        _Reserved_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
        )
    {
        assert(false);
        return FALSE;
    }

    BOOL WINAPI_DECL CreateHardLinkW(
        _In_        LPCWSTR lpFileName,
        _In_        LPCWSTR lpExistingFileName,
        _Reserved_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
        )
    {
        assert(false);
        return FALSE;
    }

    DWORD WINAPI_DECL GetFullPathNameA(
        _In_   LPCSTR lpFileName,
        _In_   DWORD nBufferLength,
        _Out_  LPSTR lpBuffer,
        _Out_  LPSTR *lpFilePart
        )
    {
        assert(false);
        return 0;
    }

    DWORD WINAPI_DECL GetFullPathNameW(
        _In_   LPCWSTR lpFileName,
        _In_   DWORD nBufferLength,
        _Out_  LPWSTR lpBuffer,
        _Out_  LPWSTR *lpFilePart
        )
    {
        assert(false);
        return 0;
    }

    DWORD WINAPI_DECL GetShortPathNameA(
        _In_   LPCSTR lpszLongPath,
        _Out_  LPSTR lpszShortPath,
        _In_   DWORD cchBuffer
        )
    {
        assert(false);
        return 0;
    }

    DWORD WINAPI_DECL GetShortPathNameW(
        _In_   LPCWSTR lpszLongPath,
        _Out_  LPWSTR lpszShortPath,
        _In_   DWORD cchBuffer
        )
    {
        assert(false);
        return 0;
    }

    BOOL WINAPI_DECL MoveFileA(
        _In_  LPCSTR lpExistingFileName,
        _In_  LPCSTR lpNewFileName
        )
    {
        return MoveFileExA(
            lpExistingFileName, 
            lpNewFileName, 
            0);
    }

    BOOL WINAPI_DECL MoveFileW(
        _In_  LPCWSTR lpExistingFileName,
        _In_  LPCWSTR lpNewFileName
        )
    {
        return MoveFileExW(
            lpExistingFileName, 
            lpNewFileName, 
            0);
    }

    BOOL WINAPI_DECL CopyFileA(
        _In_  LPCSTR lpExistingFileName,
        _In_  LPCSTR lpNewFileName,
        _In_  BOOL bFailIfExists
        )
    {
        charset_t charset1(lpExistingFileName);
        if (charset1.wstr() == NULL) {
            return FALSE;
        }
        charset_t charset2(lpNewFileName);
        if (charset2.wstr() == NULL) {
            return FALSE;
        }
        BOOL b = CopyFileW(
            charset1.wstr(), 
            charset2.wstr(), 
            bFailIfExists);
        return b;
    }

    BOOL WINAPI_DECL CopyFileW(
        _In_  LPCWSTR lpExistingFileName,
        _In_  LPCWSTR lpNewFileName,
        _In_  BOOL bFailIfExists
        )
    {
        COPYFILE2_EXTENDED_PARAMETERS extendedParameters;
        extendedParameters.dwSize = sizeof(extendedParameters);
        extendedParameters.dwCopyFlags = bFailIfExists ? COPY_FILE_FAIL_IF_EXISTS : 0;
        extendedParameters.pfCancel = FALSE;
        extendedParameters.pProgressRoutine = NULL;
        extendedParameters.pvCallbackContext = NULL;
        HRESULT hr = CopyFile2(
            lpExistingFileName, 
            lpNewFileName, 
            &extendedParameters);
        return SUCCEEDED(hr);
    }

    HANDLE WINAPI_DECL FindFirstFileA(
        _In_   LPCSTR lpFileName,
        _Out_  LPWIN32_FIND_DATAA lpFindFileData
        )
    {
        charset_t charset(lpFileName);
        if (charset.wstr() == NULL) {
            return FALSE;
        }
        WIN32_FIND_DATAW FindFileData;
        memset(&FindFileData, 0, sizeof(FindFileData));
        HANDLE hFindFile = FindFirstFileW(
            charset.wstr(), 
            &FindFileData);
        if (hFindFile != INVALID_HANDLE_VALUE) {
#define COPY_FindFileData(x) lpFindFileData->x = FindFileData.x
            COPY_FindFileData(dwFileAttributes);
            COPY_FindFileData(ftCreationTime);
            COPY_FindFileData(ftLastAccessTime);
            COPY_FindFileData(ftLastWriteTime);
            COPY_FindFileData(nFileSizeHigh);
            COPY_FindFileData(nFileSizeLow);
            COPY_FindFileData(dwReserved0);
            COPY_FindFileData(dwReserved1);
            charset.w2a(FindFileData.cFileName, -1, lpFindFileData->cFileName, sizeof(lpFindFileData->cFileName));
            charset.w2a(FindFileData.cAlternateFileName, -1, lpFindFileData->cAlternateFileName, sizeof(lpFindFileData->cAlternateFileName));
        }
        return hFindFile;
    }

    HANDLE WINAPI_DECL FindFirstFileW(
        _In_   LPCWSTR lpFileName,
        _Out_  LPWIN32_FIND_DATAW lpFindFileData
        )
    {
        HANDLE hFindFile = FindFirstFileExW(
            lpFileName, 
            FindExInfoStandard, 
            lpFindFileData, 
            FindExSearchNameMatch, 
            NULL, 
            0);
        return hFindFile;
    }

    BOOL WINAPI_DECL GetFileInformationByHandle(
        _In_   HANDLE hFile,
        _Out_  LPBY_HANDLE_FILE_INFORMATION lpFileInformation
        )
    {
        FILE_BASIC_INFO basicInfo;
        FILE_STANDARD_INFO standardInfo;
        BOOL b = GetFileInformationByHandleEx(
            hFile, 
            FileBasicInfo, 
            &basicInfo, 
            sizeof(basicInfo));
        if (b == FALSE)
            return FALSE;
        b = GetFileInformationByHandleEx(
            hFile, 
            FileStandardInfo, 
            &standardInfo, 
            sizeof(standardInfo));
        if (b == FALSE)
            return FALSE;
        lpFileInformation->dwFileAttributes = basicInfo.FileAttributes;
        lpFileInformation->ftCreationTime = (FILETIME const &)basicInfo.CreationTime;
        lpFileInformation->ftLastAccessTime = (FILETIME const &)basicInfo.LastAccessTime;
        lpFileInformation->ftLastWriteTime = (FILETIME const &)basicInfo.LastWriteTime;
        lpFileInformation->dwVolumeSerialNumber = 0;
        lpFileInformation->nFileSizeHigh = standardInfo.EndOfFile.HighPart;
        lpFileInformation->nFileSizeLow = standardInfo.EndOfFile.LowPart;
        lpFileInformation->nNumberOfLinks = standardInfo.NumberOfLinks;
        lpFileInformation->nFileIndexHigh = 0;
        lpFileInformation->nFileIndexLow = 0;
        return TRUE;
    }

    BOOL WINAPI_DECL GetFileTime(
        _In_       HANDLE hFile,
        _Out_opt_  LPFILETIME lpCreationTime,
        _Out_opt_  LPFILETIME lpLastAccessTime,
        _Out_opt_  LPFILETIME lpLastWriteTime
        )
    {
        FILE_BASIC_INFO basicInfo;
        BOOL b = GetFileInformationByHandleEx(
            hFile, 
            FileBasicInfo, 
            &basicInfo, 
            sizeof(basicInfo));
        if (b == FALSE)
            return FALSE;
        if (lpCreationTime)
            *lpCreationTime = (FILETIME const &)basicInfo.CreationTime;
        if (lpLastAccessTime)
            *lpLastAccessTime = (FILETIME const &)basicInfo.LastAccessTime;
        if (lpLastWriteTime)
            *lpLastWriteTime = (FILETIME const &)basicInfo.LastWriteTime;
        return TRUE;
    }

    BOOL WINAPI_DECL SetFileTime(
        _In_      HANDLE hFile,
        _In_opt_  const FILETIME *lpCreationTime,
        _In_opt_  const FILETIME *lpLastAccessTime,
        _In_opt_  const FILETIME *lpLastWriteTime
        )
    {
        FILE_BASIC_INFO basicInfo;
        BOOL b = GetFileInformationByHandleEx(
            hFile, 
            FileBasicInfo, 
            &basicInfo, 
            sizeof(basicInfo));
        if (b == FALSE)
            return FALSE;
        if (lpCreationTime)
            basicInfo.CreationTime = *(LARGE_INTEGER const *)lpCreationTime;
        if (lpLastAccessTime)
            basicInfo.LastAccessTime = *(LARGE_INTEGER const *)lpLastAccessTime;
        if (lpLastWriteTime)
            basicInfo.LastWriteTime = *(LARGE_INTEGER const *)lpLastAccessTime;
        b = SetFileInformationByHandle(
            hFile, 
            FileBasicInfo, 
            &basicInfo, 
            sizeof(basicInfo));
        return b;
    }

    DWORD WINAPI_DECL GetTempPathA(
        _In_   DWORD nBufferLength,
        _Out_  LPSTR lpBuffer
        )
    {
        charset_t charset(lpBuffer, (int)nBufferLength);
        if (charset.wstr() == NULL) {
            return 0;
        }
        charset.wlen(GetTempPathW(
            charset.wlen(), 
            charset.wstr()));
        charset.w2a();
        nBufferLength = charset.len();
        lpBuffer[nBufferLength] = '\0';
        return nBufferLength;
    }

    DWORD WINAPI_DECL GetTempPathW(
        _In_   DWORD nBufferLength,
        _Out_  LPWSTR lpBuffer
        )
    {
        Platform::String ^ temp = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
        wcsncpy_s(lpBuffer, nBufferLength, temp->Data(), temp->Length());
        return temp->Length();
    }

    DWORD WINAPI_DECL GetLocalPathW(
        _In_   DWORD nBufferLength,
        _Out_  LPWSTR lpBuffer
        )
    {
        Platform::String ^ temp = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
        wcsncpy_s(lpBuffer, nBufferLength, temp->Data(), temp->Length());
        return temp->Length();
    }

    DWORD WINAPI_DECL GetLocalPathA(
        _In_   DWORD nBufferLength,
        _Out_  LPSTR lpBuffer
        )
    {
        charset_t charset(lpBuffer, (int)nBufferLength);
        if (charset.wstr() == NULL) {
            return 0;
        }
        charset.wlen(GetLocalPathW(
            charset.wlen(), 
            charset.wstr()));
        charset.w2a();
        nBufferLength = charset.len();
        lpBuffer[nBufferLength] = '\0';
        return nBufferLength;
    }

    BOOL WINAPI_DECL GetDiskFreeSpaceEx2A(
        _In_opt_ LPCSTR lpDirectoryName,
        _Out_opt_ PULARGE_INTEGER lpFreeBytesAvailableToCaller,
        _Out_opt_ PULARGE_INTEGER lpTotalNumberOfBytes,
        _Out_opt_ PULARGE_INTEGER lpTotalNumberOfFreeBytes
        )
    {
        charset_t charset(lpDirectoryName);
        if (charset.wstr() == NULL) {
            return INVALID_FILE_ATTRIBUTES;
        }
        return GetDiskFreeSpaceExW(charset.wstr(),
            lpFreeBytesAvailableToCaller,
            lpTotalNumberOfBytes,
            lpTotalNumberOfFreeBytes);
    }

    BOOL WINAPI_DECL FindNextFile2A(
        _In_ HANDLE hFindFile,
        _Out_ LPWIN32_FIND_DATAA lpFindFileData
        )
    {
        WIN32_FIND_DATAW tmpData;
        if (FindNextFileW(hFindFile, &tmpData))
        {
            lpFindFileData->dwFileAttributes = tmpData.dwFileAttributes;
            lpFindFileData->ftCreationTime = tmpData.ftCreationTime;
            lpFindFileData->ftLastAccessTime = tmpData.ftLastAccessTime;
            lpFindFileData->ftLastWriteTime = tmpData.ftLastWriteTime;
            lpFindFileData->nFileSizeHigh = tmpData.nFileSizeHigh;
            lpFindFileData->nFileSizeLow = tmpData.nFileSizeLow;
            lpFindFileData->dwReserved0 = tmpData.dwReserved0;
            lpFindFileData->dwReserved1 = tmpData.dwReserved1;
            charset_t::w2a(tmpData.cFileName, MAX_PATH, lpFindFileData->cFileName, MAX_PATH);
            charset_t::w2a(tmpData.cAlternateFileName, 14, lpFindFileData->cAlternateFileName, 14);
            return TRUE;
        } 
        return FALSE;
    }
}
