// SystemEmulation.h

#pragma once

#define FORMAT_MESSAGE_ALLOCATE_BUFFER    0x00000100

//#ifdef __cplusplus
//extern "C"
//{
//#endif

    DWORD WINAPI_DECL FormatMessage2A(
        _In_      DWORD dwFlags,
        _In_opt_  LPCVOID lpSource,
        _In_      DWORD dwMessageId,
        _In_      DWORD dwLanguageId,
        _Out_     LPSTR lpBuffer,
        _In_      DWORD nSize,
        _In_opt_  va_list *Arguments
        );

#define FormatMessageA FormatMessage2A

#ifdef UNICODE
//#define FormatMessage  FormatMessageW
#else
#define FormatMessage  FormatMessageA
#endif // !UNICODE

    void WINAPI_DECL OutputDebugString2A(
        _In_opt_  LPCSTR lpOutputString
        );

#define OutputDebugStringA OutputDebugString2A

#ifdef UNICODE
//#define OutputDebugString  OutputDebugStringW
#else
#define OutputDebugString  OutputDebugStringA
#endif // !UNICODE

    DWORD WINAPI_DECL GetTickCount(void);

    void WINAPI_DECL GetSystemInfo(
        _Out_  LPSYSTEM_INFO lpSystemInfo
        );

    LPCH WINAPI_DECL GetEnvironmentStringsA(void);

    LPWCH WINAPI_DECL GetEnvironmentStringsW(void);

#ifdef UNICODE
#define GetEnvironmentStrings GetEnvironmentStringsW
#else
#define GetEnvironmentStrings GetEnvironmentStringsA
#endif // !UNICODE

    DWORD WINAPI_DECL GetEnvironmentVariableA(
        _In_opt_   LPCSTR lpName,
        _Out_opt_  LPSTR lpBuffer,
        _In_       DWORD nSize
        );

    DWORD WINAPI_DECL GetEnvironmentVariableW(
        _In_opt_   LPCWSTR lpName,
        _Out_opt_  LPWSTR lpBuffer,
        _In_       DWORD nSize
        );

#ifdef UNICODE
#define GetEnvironmentVariable GetEnvironmentVariableW
#else
#define GetEnvironmentVariable GetEnvironmentVariableA
#endif // !UNICODE

    BOOL WINAPI_DECL SetEnvironmentVariableA(
        _In_      LPCSTR lpName,
        _In_opt_  LPCSTR lpValue
        );

    BOOL WINAPI_DECL SetEnvironmentVariableW(
        _In_      LPCWSTR lpName,
        _In_opt_  LPCWSTR lpValue
        );

#ifdef UNICODE
#define SetEnvironmentVariable SetEnvironmentVariableW
#else
#define SetEnvironmentVariable SetEnvironmentVariableA
#endif // !UNICODE

    LCID WINAPI_DECL GetUserDefaultLCID(void);

    int WINAPI_DECL LCMapStringA(
        _In_       LCID Locale,
        _In_       DWORD dwMapFlags,
        _In_       LPCSTR lpSrcStr,
        _In_       int cchSrc,
        _Out_opt_  LPSTR lpDestStr,
        _In_       int cchDest
        );

    int WINAPI_DECL LCMapStringW(
        _In_       LCID Locale,
        _In_       DWORD dwMapFlags,
        _In_       LPCWSTR lpSrcStr,
        _In_       int cchSrc,
        _Out_opt_  LPWSTR lpDestStr,
        _In_       int cchDest
        );

#ifdef UNICODE
#define LCMapString  LCMapStringW
#else
#define LCMapString  LCMapStringA
#endif // !UNICODE

    BOOL WINAPI_DECL GetStringTypeExA(
        _In_   LCID Locale,
        _In_   DWORD dwInfoType,
        _In_   LPCSTR lpSrcStr,
        _In_   int cchSrc,
        _Out_  LPWORD lpCharType
        );

    //BOOL WINAPI_DECL GetStringTypeExW(
    //    _In_   LCID Locale,
    //    _In_   DWORD dwInfoType,
    //    _In_   LPCWSTR lpWSrcStr,
    //    _In_   int cchSrc,
    //    _Out_  LPWORD lpCharType
    //    );

#ifdef UNICODE
//#define GetStringTypeEx  GetStringTypeExW
#else
#define GetStringTypeEx  GetStringTypeExA
#endif // !UNICODE

    int WINAPI_DECL LoadStringA(
        _In_opt_  HINSTANCE hInstance,
        _In_      UINT uID,
        _Out_     LPSTR lpBuffer,
        _In_      int nBufferMax
        );

    int WINAPI_DECL LoadStringW(
        _In_opt_  HINSTANCE hInstance,
        _In_      UINT uID,
        _Out_     LPWSTR lpBuffer,
        _In_      int nBufferMax
        );

#ifdef UNICODE
#define LoadString  LoadStringW
#else
#define LoadString  LoadStringA
#endif // !UNICODE

    HANDLE WINAPI_DECL GetStdHandle(
        _In_  DWORD nStdHandle
        );

    BOOL WINAPI_DECL SetConsoleTextAttribute(
        _In_  HANDLE hConsoleOutput,
        _In_  WORD wAttributes
        );

    typedef struct _SMALL_RECT {
        SHORT Left;
        SHORT Top;
        SHORT Right;
        SHORT Bottom;
    } SMALL_RECT, *PSMALL_RECT;

    typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
        COORD dwSize;
        COORD dwCursorPosition;
        WORD  wAttributes;
        SMALL_RECT srWindow;
        COORD dwMaximumWindowSize;
    } CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;

    BOOL WINAPI_DECL GetConsoleScreenBufferInfo(
        _In_ HANDLE hConsoleOutput,
        _Out_ PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo
        );

    BOOL WINAPI_DECL FileTimeToLocalFileTime(
        _In_   const FILETIME *lpFileTime,
        _Out_  LPFILETIME lpLocalFileTime
        );

    HMODULE WINAPI_DECL GetModuleHandleA(
        _In_opt_  LPCSTR lpModuleName
        );

    HMODULE WINAPI_DECL GetModuleHandleW(
        _In_opt_  LPCWSTR lpModuleName
        );

#ifdef UNICODE
#define GetModuleHandle  GetModuleHandleW
#else
#define GetModuleHandle  GetModuleHandleA
#endif // !UNICODE

    HMODULE WINAPI_DECL LoadLibraryA(
        _In_  LPCSTR lpFileName
        );

    HMODULE WINAPI_DECL LoadLibraryW(
        _In_  LPCWSTR lpFileName
        );

#ifdef UNICODE
#define LoadLibrary  LoadLibraryW
#else
#define LoadLibrary  LoadLibraryA
#endif // !UNICODE

    DWORD WINAPI_DECL GetModuleFileNameA(
      _In_opt_  HMODULE hModule,
      _Out_     LPSTR lpFilename,
      _In_      DWORD nSize
    );

    DWORD WINAPI_DECL GetModuleFileNameW(
      _In_opt_  HMODULE hModule,
      _Out_     LPWSTR lpFilename,
      _In_      DWORD nSize
    );

#ifdef UNICODE
#define GetModuleFileName  GetModuleFileNameW
#else
#define GetModuleFileName  GetModuleFileNameA
#endif // !UNICODE

//#ifdef __cplusplus
//}
//#endif
