// FullName.h

#ifndef _FRAMEWORK_SYSTEM_AUTO_NAME_H_
#define _FRAMEWORK_SYSTEM_AUTO_NAME_H_

#include <boost/preprocessor/stringize.hpp>

#if (defined LIB)
#  if (defined _STATIC)
#    define FRAMEWORK_STATIC
#  elif (defined _DYNAMIC)
#    define FRAMEWORK_DYNAMIC
#  else
#    ifndef DEFAULT_LIB_STATIC
#      undef DEFAULT_LIB_DYNAMIC
#      define FRAMEWORK_LIB_DYNAMIC
#    else
#      undef DEFAULT_LIB_STATIC
#      define FRAMEWORK_LIB_STATIC
#    endif
#  endif
#endif

#if (defined _SINGLE)
#  define FRAMEWORK_SINGLE
#elif (defined _MULTI)
#  define FRAMEWORK_MULTI
#else
#  define FRAMEWORK_MULTI
#endif

#if (defined _DEBUG)
#  define FRAMEWORK_DEBUG
#elif (defined _NDEBUG)
#  define FRAMEWORK_RELEASE
#else
#  define FRAMEWORK_RELEASE
#endif

#ifdef LIB
#if (defined _WIN32) || (defined WIN32)
#    if (defined WINRT) || (defined WIN_PHONE)
#      define PREFIX
#    else
#      define PREFIX lib
#    endif
#    ifdef FRAMEWORK_LIB_STATIC
#      define SUFFIX .lib
#    else
#      define SUFFIX .dll
#    endif
#  else
#    define PREFIX lib
#    ifdef FRAMEWORK_LIB_STATIC
#      define SUFFIX .a
#    else
#      define SUFFIX .so
#    endif
#  endif
#else
#if (defined _WIN32) || (defined WIN32)
#    define PREFIX
#    define SUFFIX .exe
#  else
#    define PREFIX
#    define SUFFIX
#  endif
#endif

#ifdef FRAMEWORK_LIB_STATIC
#  ifdef FRAMEWORK_MULTI
#    ifdef FRAMEWORK_DEBUG
#      define CONFIG_NAME -mt-sgd
#    else
#      define CONFIG_NAME -mt-s
#    endif
#  else
#    ifdef FRAMEWORK_DEBUG
#      define CONFIG_NAME -sgd
#    else
#      define CONFIG_NAME
#    endif
#  endif
#else
#  ifdef FRAMEWORK_MULTI
#    ifdef FRAMEWORK_DEBUG
#      define CONFIG_NAME -mt-gd
#    else
#      define CONFIG_NAME -mt
#    endif
#  else
#    ifdef FRAMEWORK_DEBUG
#      define CONFIG_NAME -gd
#    else
#      define CONFIG_NAME
#    endif
#  endif
#endif

BEGIN_NAME_SPACE

static inline char const * name_string()
{
#if (!defined(PLATFORM_NAME) || !defined(TOOL_NAME))
    return BOOST_PP_STRINGIZE(PREFIX) \
        BOOST_PP_STRINGIZE(NAME) \
        BOOST_PP_STRINGIZE(SUFFIX);
#else
#  undef linux
#  undef win32
#  undef mips
#  undef arm
    return BOOST_PP_STRINGIZE(PREFIX) \
        BOOST_PP_STRINGIZE(NAME) "-" \
        BOOST_PP_STRINGIZE(PLATFORM_NAME) "-" \
        BOOST_PP_STRINGIZE(TOOL_NAME) \
        BOOST_PP_STRINGIZE(CONFIG_NAME) "-" \
        BOOST_PP_STRINGIZE(VERSION) \
        BOOST_PP_STRINGIZE(SUFFIX);
#endif
}

END_NAME_SPACE

#undef PREFIX

#undef SUFFIX

#undef FRAMEWORK_STATIC

#undef FRAMEWORK_DYNAMIC

#undef FRAMEWORK_MULTI

#undef FRAMEWORK_SINGLE

#undef FRAMEWORK_DEBUG

#undef FRAMEWORK_RELEASE

#undef CONFIG_NAME

#undef END_NAME_SPACE

#undef BEGIN_NAME_SPACE

#undef VERSION

#undef NAME

#undef LIB

#endif // _FRAMEWORK_SYSTEM_AUTO_NAME_H_
