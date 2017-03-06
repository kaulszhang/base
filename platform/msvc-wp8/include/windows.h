// windows.h

#pragma once

#ifdef _WIN32_WINNT
#  undef _WIN32_WINNT
#endif

#define WIN_PHONE    1

#include <..\windows.h> // orignal windows.h

#define WINAPI_DECL     __declspec(dllimport)

#include <..\winapi\MemoryEmulation.h>
#include <..\winapi\SystemEmulation.h>
#include <..\winapi\FileSystemEmulation.h>
#include <..\winapi\ThreadEmulation.h>
#include <..\winapi\NetworkEmulation.h>

using namespace MemoryEmulation;
using namespace SystemEmulation;
using namespace FileSystemEmulation;
using namespace ThreadEmulation;
using namespace NetworkEmulation;
