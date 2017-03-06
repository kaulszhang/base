// FileSystemEmulation.cpp

#include <Windows.h>

#define WINAPI_DECL     __declspec(dllexport)

#include "FileSystemEmulation.h"
#undef WriteFile
#undef ReadFile
#undef SetFilePointerEx
#undef CloseHandle
#include "AsyncHelper.h"
#include "Charset.h"
using namespace winapi;

#include <mutex>
#include <vector>
#include <algorithm>

#include <memoryapi.h>

#include <assert.h>

//extern "C"
//{

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

    struct WinrtFileHandle
    {
        Windows::Storage::Streams::IRandomAccessStream ^ stream;
        HANDLE hEvent;

        struct find_by_event
        {
            find_by_event(
                HANDLE hEvent)
                : hEvent_(hEvent)
            {
            }

            bool operator()(
                WinrtFileHandle * m)
            {
                return hEvent_ == m->hEvent;
            }

            HANDLE hEvent_;
        };
    };

    static std::mutex mutex_file_;
    static std::vector<WinrtFileHandle *> files_;

    HANDLE CreateWinrtFile(
        _In_      LPCWSTR lpFileName, 
        _In_      DWORD dwDesiredAccess,
        _In_      DWORD dwCreationDisposition)
    {
        LPCWCHAR p = wcschr(lpFileName, '\\');
        Windows::Storage::StorageFile ^ file;
        Windows::Storage::Streams::IRandomAccessStream ^ stream;
        int ec = 0;
        if (p == NULL) {
            Platform::String ^ token = ref new Platform::String(lpFileName);
            ec = wait_operation(
                Windows::Storage::AccessCache::StorageApplicationPermissions::FutureAccessList->GetFileAsync(token), 
                file);
        } else {
            Platform::String ^ token = ref new Platform::String(lpFileName, p - lpFileName);
            Windows::Storage::StorageFolder ^ folder;
            int ec = wait_operation(
                Windows::Storage::AccessCache::StorageApplicationPermissions::FutureAccessList->GetFolderAsync(token), 
                folder);
            if (ec == 0) {
                if (dwCreationDisposition == OPEN_EXISTING || dwCreationDisposition == TRUNCATE_EXISTING) {
                    ec = wait_operation(
                        folder->GetFileAsync(ref new Platform::String(p + 1)), 
                        file);
                } else {
                    Windows::Storage::CreationCollisionOption option;
                    switch (dwCreationDisposition) {
                    case CREATE_ALWAYS:
                        option = Windows::Storage::CreationCollisionOption::ReplaceExisting;
                        break;
                    case CREATE_NEW:
                        option = Windows::Storage::CreationCollisionOption::FailIfExists;
                        break;
                    case OPEN_ALWAYS:
                        option = Windows::Storage::CreationCollisionOption::OpenIfExists;
                        break;
                    }
                    ec = wait_operation(
                        folder->CreateFileAsync(ref new Platform::String(p + 1), option), 
                        file);
                }
            }
        }
        if (ec == 0) {
            Windows::Storage::FileAccessMode mode = 
                dwDesiredAccess == GENERIC_READ ? Windows::Storage::FileAccessMode::Read : Windows::Storage::FileAccessMode::ReadWrite;
            ec = wait_operation(
                file->OpenAsync(mode), 
                stream);
        }
        if (ec == 0) {
            WinrtFileHandle * file = new WinrtFileHandle;
            file->stream = stream;
            file->hEvent = CreateEventExW(
                NULL, 
                NULL, 
                0, 
                EVENT_ALL_ACCESS);
            std::unique_lock<std::mutex> lc(mutex_file_);
            files_.push_back(file);
            return file->hEvent;
        } else {
            SetLastError(ec);
            return INVALID_HANDLE_VALUE;
        }
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
        if (wcsncmp(lpFileName, L"\\winrt:\\", 8) == 0) {
            return CreateWinrtFile(lpFileName + 8, dwDesiredAccess, dwCreationDisposition);
        }
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

    BOOL WINAPI_DECL SetFilePointerEx2(
        _In_       HANDLE hFile,
        _In_       LARGE_INTEGER liDistanceToMove,
        _Out_opt_  PLARGE_INTEGER lpNewFilePointer,
        _In_       DWORD dwMoveMethod
        )
    {
        std::unique_lock<std::mutex> lc(mutex_file_);
        std::vector<WinrtFileHandle *>::iterator iter = 
            std::find_if(files_.begin(), files_.end(), WinrtFileHandle::find_by_event(hFile));
        if (iter == files_.end()) {
            return SetFilePointerEx(
                hFile, 
                liDistanceToMove, 
                lpNewFilePointer, 
                dwMoveMethod);
        } else {
            LARGE_INTEGER FilePointer;
            switch (dwMoveMethod)
            {
            case FILE_BEGIN:
                FilePointer = liDistanceToMove;
                break;
            case FILE_CURRENT:
                FilePointer.QuadPart = (*iter)->stream->Position + liDistanceToMove.QuadPart;
                break;
            case FILE_END:
                FilePointer.QuadPart = (*iter)->stream->Size + liDistanceToMove.QuadPart;
                break;
            }
            (*iter)->stream->Seek(FilePointer.QuadPart);
            if (lpNewFilePointer)
                *lpNewFilePointer = FilePointer;
            return TRUE;
        }
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
        BOOL b = SetFilePointerEx2(
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
        LARGE_INTEGER FileSize;
        BOOL b = GetFileSizeEx(
                hFile, 
                &FileSize);
        if (b == FALSE)
            return INVALID_FILE_SIZE;
        if (lpFileSizeHigh)
            *lpFileSizeHigh = FileSize.HighPart;
        return FileSize.LowPart;
    }

    BOOL WINAPI_DECL GetFileSizeEx(
        _In_   HANDLE hFile,
        _Out_  PLARGE_INTEGER lpFileSize
        )
    {
        std::unique_lock<std::mutex> lc(mutex_file_);
        std::vector<WinrtFileHandle *>::iterator iter = 
            std::find_if(files_.begin(), files_.end(), WinrtFileHandle::find_by_event(hFile));
        if (iter == files_.end()) {
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
        } else {
            lpFileSize->QuadPart = (*iter)->stream->Size;
            return TRUE;
        }
    }

    BOOL WINAPI_DECL ReadFile2(
        _In_         HANDLE hFile,
        _Out_        LPVOID lpBuffer,
        _In_         DWORD nNumberOfBytesToRead,
        _Out_opt_    LPDWORD lpNumberOfBytesRead,
        _Inout_opt_  LPOVERLAPPED lpOverlapped
        )
    {
        std::unique_lock<std::mutex> lc(mutex_file_);
        std::vector<WinrtFileHandle *>::iterator iter = 
            std::find_if(files_.begin(), files_.end(), WinrtFileHandle::find_by_event(hFile));
        if (iter == files_.end()) {
            return ::ReadFile(
                hFile, 
                lpBuffer, 
                nNumberOfBytesToRead, 
                lpNumberOfBytesRead, 
                lpOverlapped);
        } else {
            assert(lpOverlapped == NULL);
            Windows::Storage::Streams::IBuffer ^ buffer = 
                ref new Windows::Storage::Streams::Buffer(nNumberOfBytesToRead);
            int ec = wait_operation2(
                (*iter)->stream->ReadAsync(buffer, nNumberOfBytesToRead, Windows::Storage::Streams::InputStreamOptions::Partial), 
                buffer);
            if (ec) {
                SetLastError(ec);
                return FALSE;
            } else {
                Windows::Storage::Streams::DataReader ^ reader = 
                    Windows::Storage::Streams::DataReader::FromBuffer(buffer);
                reader->ReadBytes(Platform::ArrayReference<uint8_t>((uint8_t*)lpBuffer, buffer->Length));
                *lpNumberOfBytesRead = buffer->Length;
                return TRUE;
            }
        }
    }

    BOOL WINAPI_DECL WriteFile2(
        _In_         HANDLE hFile,
        _In_         LPCVOID lpBuffer,
        _In_         DWORD nNumberOfBytesToWrite,
        _Out_opt_    LPDWORD lpNumberOfBytesWritten,
        _Inout_opt_  LPOVERLAPPED lpOverlapped
        )
    {
        std::unique_lock<std::mutex> lc(mutex_file_);
        std::vector<WinrtFileHandle *>::iterator iter = 
            std::find_if(files_.begin(), files_.end(), WinrtFileHandle::find_by_event(hFile));
        if (iter == files_.end()) {
            return ::WriteFile(
                hFile, 
                lpBuffer, 
                nNumberOfBytesToWrite, 
                lpNumberOfBytesWritten, 
                lpOverlapped);
        } else {
            assert(lpOverlapped == NULL);
            Windows::Storage::Streams::DataWriter ^ writer = 
                ref new Windows::Storage::Streams::DataWriter;
            writer->WriteBytes(Platform::ArrayReference<uint8_t>((uint8_t *)lpBuffer, nNumberOfBytesToWrite));
            uint32_t size = 0;
            int ec = wait_operation2(
                (*iter)->stream->WriteAsync(writer->DetachBuffer()), 
                size);
            if (ec) {
                SetLastError(ec);
                return FALSE;
            } else {
                *lpNumberOfBytesWritten = size;
                return TRUE;
            }
        }
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
        Platform::String ^ temp = Windows::Storage::ApplicationData::Current->TemporaryFolder->Path;
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

    BOOL WINAPI_DECL CloseHandle2(
        _In_  HANDLE hObject
        )
    {
        std::unique_lock<std::mutex> lc(mutex_file_);
        std::vector<WinrtFileHandle *>::iterator iter = 
            std::find_if(files_.begin(), files_.end(), WinrtFileHandle::find_by_event(hObject));
        if (iter != files_.end()) {
            WinrtFileHandle * file = *iter;
            files_.erase(iter);
            ::CloseHandle(file->hEvent);
            delete file;
            return TRUE;
        }
        return ::CloseHandle(hObject);
    }

//}
