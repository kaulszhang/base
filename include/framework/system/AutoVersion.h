// AutoVersion.h

#ifndef _FRAMEWORK_SYSTEM_AUTO_VERSION_H_
#define _FRAMEWORK_SYSTEM_AUTO_VERSION_H_

#include "framework/system/Version.h"
#include "framework/system/FileTag.h"

#include <boost/preprocessor/stringize.hpp>

namespace framework
{
    namespace system
    {

        inline std::map<char const *, char const *> & version_collection()
        {
            static std::map<char const *, char const *> version_collection_;
            return version_collection_;
        }

    } // namespace system
} // namespace boost

#endif // _FRAMEWORK_SYSTEM_AUTO_VERSION_H_

BEGIN_NAME_SPACE

#ifdef VERSION_SOURCE
#undef VERSION_SOURCE

char const * version_string()
{
    char const * version = FILE_TAG_VALUE(version, NAME, BOOST_PP_STRINGIZE(VERSION));
    framework::system::version_collection().insert(std::make_pair(BOOST_PP_STRINGIZE(NAME), version));
    return version;
}

framework::system::Version const & version()
{
    static framework::system::Version my_ver(version_string());
    return my_ver;
}

#else

static inline char const * simple_version_string()
{
    return BOOST_PP_STRINGIZE(VERSION);
}

char const * version_string();

framework::system::Version const & version();

static framework::system::Version const & _version = version(); 

#endif

END_NAME_SPACE

#undef END_NAME_SPACE

#undef BEGIN_NAME_SPACE

#undef VERSION

#undef NAME

#undef LIB
