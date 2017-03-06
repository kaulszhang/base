// windows.h

#pragma once

#ifdef _WIN32_WINNT
#  undef _WIN32_WINNT
#endif

#define WINRT    1

#include <..\..\Include\um\windows.h> // orignal windows.h

#define WINAPI_DECL     __declspec(dllimport)

#include <..\winapi\MemoryEmulation.h>
#include <..\winapi\SystemEmulation.h>
#include <..\winapi\FileSystemEmulation.h>
#include <..\winapi\ThreadEmulation.h>
#include <..\winapi\SocketEmulation.h>
