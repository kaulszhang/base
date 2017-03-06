// stdlib.h

#pragma once

#include <..\..\VC\include\stdlib.h> // orignal stdlib.h

#ifdef __cplusplus
extern "C"
{
#endif

    __declspec(dllimport) char ** environ;

    char * __cdecl getenv(
        _In_z_ const char * _VarName
        );

#ifdef __cplusplus
}
#endif
