// Library.cpp

#include "framework/Framework.h"
#include "framework/library/Library.h"
#include "framework/system/ErrorCode.h"
#include "framework/logger/LoggerFormatRecord.h"
using namespace framework::system;
using namespace framework::logger;

using namespace boost::system;

#include <iostream>

#ifdef BOOST_WINDOWS_API
#include <windows.h>
#else
#include <dlfcn.h>
#endif

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Library", 2)

namespace framework
{
    namespace library
    {
   
        Library::Library()
            : handle_(NULL)
            , need_close_(false)
        {
        }

        Library::Library(
            Library const & r)
            : handle_(r.handle_)
            , need_close_(false)
        {
        }

        Library::Library(
            std::string const & name)
            : handle_(NULL)
            , need_close_(false)
        {
            open(name);
        }

        Library::Library(
            void * handle)
            : handle_(handle)
            , need_close_(false)
        {
        }

        Library::~Library()
        {
            if (handle_ && need_close_)
                close();
        }

        error_code Library::open(
            std::string const & name)
        {
#ifdef BOOST_WINDOWS_API
#ifdef WIN_PHONE
			handle_ = ::LoadLibraryA(name.c_str());
#else
            handle_ = ::LoadLibrary(name.c_str());
#endif
#else
            if (name.find('.') == std::string::npos)
                handle_ = ::dlopen((std::string("lib") + name + ".so").c_str(), RTLD_LAZY | RTLD_LOCAL);
            else
                handle_ = ::dlopen(name.c_str(), RTLD_LAZY | RTLD_LOCAL);
            if (handle_ == NULL)
                LOG_F(Logger::kLevelAlarm, "[open] dlopen: %1%" % ::dlerror());
#endif
            if (handle_)
                need_close_ = true;
            return handle_ ? error_code() : last_system_error();
        }

        error_code Library::close()
        {
#ifdef BOOST_WINDOWS_API
            ::FreeLibrary((HMODULE)handle_);
#else
            ::dlclose(handle_);
#endif
            handle_ = NULL;
            return last_system_error();
        }

        void * Library::symbol(
            std::string const & name) const
        {
#ifdef BOOST_WINDOWS_API
            return (void *)::GetProcAddress((HMODULE)handle_, name.c_str());
#else
            return ::dlsym(handle_, name.c_str());
#endif
        }

        static size_t const MAX_PATH_SIZE = 1024;

        std::string Library::path() const
        {
#ifdef BOOST_WINDOWS_API
            char path[MAX_PATH_SIZE] = {0};
#ifdef WIN_PHONE
			DWORD len = ::GetModuleFileNameA((HMODULE)handle_, path, MAX_PATH_SIZE);
#else
            DWORD len = ::GetModuleFileName((HMODULE)handle_, path, MAX_PATH_SIZE);
#endif
            if (0 != len) {
                return path;
            }
#elif __USE_GNU
            Dl_info info;
            int ret = ::dladdr(handle_, &info);
            if (ret != 0) {
                return info.dli_fname;
            }
#endif
            return std::string();
        }

        Library Library::self()
        {
            return from_address((void *)&Library::self);
        }

        Library Library::from_address(
            void * addr)
        {
#ifdef BOOST_WINDOWS_API
            MEMORY_BASIC_INFORMATION info = {0};
            if (sizeof(info) != ::VirtualQuery(addr, &info, sizeof(info))) {
                return info.AllocationBase;
            }
#elif __USE_GNU
            Dl_info info;
            int ret = ::dladdr(addr, &info);
            if (ret != 0) {
                return info.dli_fbase;
            }
#endif
            return NULL;
        }

    } // namespace library
} // namespace framework
