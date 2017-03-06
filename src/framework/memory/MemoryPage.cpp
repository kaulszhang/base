// MemoryPage.cpp

#include "framework/Framework.h"
#include "framework/memory/MemoryPage.h"

#ifdef BOOST_WINDOWS_API
#include <windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

namespace framework
{

	namespace memory
	{
		size_t MemoryPage::page_size()
		{
#ifdef _WIN32
			SYSTEM_INFO sSysInfo;
			GetSystemInfo(&sSysInfo);
			return sSysInfo.dwPageSize;
#else
			return getpagesize();
#endif
		}

	}
}
