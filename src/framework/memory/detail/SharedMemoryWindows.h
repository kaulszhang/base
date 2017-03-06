// SharedMemoryWindows.h

#ifndef _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WINDOWS_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WINDOWS_H_

#include "framework/memory/detail/SharedMemoryImpl.h"

#ifdef BOOST_WINDOWS_API
#include <Windows.h>
#endif

namespace framework
{
    namespace memory
    {

        namespace detail
        {

            class SharedMemoryWindows
                : public SharedMemoryImpl
            {
            private:
                static std::string name_key(
                    boost::uint32_t iid, 
                    boost::uint32_t key)
                {
                    return std::string("Local\\") + SharedMemoryImpl::key_file(iid, key);
                }

                bool create(
                    void ** id, 
                    boost::uint32_t iid,
                    boost::uint32_t key, 
                    boost::uint32_t size,
                    boost::system::error_code & ec)
                {
                    ObjectWrapper ow;
                    ErrorCodeWrapper ecw(ec);
#ifdef WIN_PHONE
					HANDLE hFileMap = ::CreateFileMappingA(
#else
					HANDLE hFileMap = ::CreateFileMapping(
#endif
                        INVALID_HANDLE_VALUE, 
                        NULL, 
                        PAGE_READWRITE, 
                        0, 
                        size, 
                        name_key(iid, key).c_str());

                    if (hFileMap == NULL) {
                        return false;
                    }

                    ow.reset(hFileMap, ::CloseHandle);

                    if (GetLastError() == ERROR_ALREADY_EXISTS) {
                        return false;
                    }

                    *id = ow.release();

                    return true;
                }

                bool open( 
                    void ** id, 
                    boost::uint32_t iid,
                    boost::uint32_t key,
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);
#ifdef WIN_PHONE
					HANDLE hFileMap = ::OpenFileMappingA(
#else
                    HANDLE hFileMap = ::OpenFileMapping(
#endif
                        FILE_MAP_ALL_ACCESS, 
                        FALSE, 
                        name_key(iid, key).c_str());

                    if (hFileMap == NULL) {
                        return false;
                    }

                    *id = (void *)hFileMap;

                    return true;
                }

                void * map(
                    void * id, 
                    boost::uint32_t size,
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    HANDLE hFileMap = ObjectWrapper::cast_object<HANDLE>(id);

                    void * p = MapViewOfFile(
                        hFileMap, 
                        FILE_MAP_ALL_ACCESS, 
                        0, 
                        0, 
                        0);

                    if (p == NULL) {
                        return NULL;
                    }

                    return p;
                }

                bool unmap(
                    void * addr, 
                    boost::uint32_t size,
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    BOOL b = ::UnmapViewOfFile(
                        addr);

                    if (b == FALSE) {
                        return false;
                    }

                    return true;
                }

                bool close(
                    void * id,
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    HANDLE hFileMap = ObjectWrapper::cast_object<HANDLE>(id);

                    BOOL b = ::CloseHandle(
                        hFileMap);

                    if (b == FALSE) {
                        return false;
                    }

                    return true;
                }

                bool destory( 
                    boost::uint32_t iid, 
                    boost::uint32_t key,
                    boost::system::error_code & ec)
                {
#ifdef WIN_PHONE
					HANDLE hFileMap = ::OpenFileMappingA(
#else
					HANDLE hFileMap = ::OpenFileMapping(
#endif
                        FILE_MAP_ALL_ACCESS, 
                        FALSE, 
                        name_key(iid, key).c_str());

                    if (hFileMap == NULL) {
                        return true;
                    }

                    ec.clear();

                    ::CloseHandle(
                        hFileMap);

                    return false;
                }
            };

            static SharedMemoryWindows shared_memory_windows;

        } // namespace detail

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WINDOWS_H_
