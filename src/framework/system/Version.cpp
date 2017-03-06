// Version.cpp

#include "framework/Framework.h"
#include "framework/system/ErrorCode.h"
#include "framework/system/Version.h"
#include "framework/system/FileTag.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"
#include "framework/string/StringToken.h"

using namespace framework::string;

namespace framework
{
    namespace system
    {

        Version::Version(
            boost::uint8_t major, 
            boost::uint8_t minor, 
            boost::uint16_t revision, 
            boost::uint32_t build)
        {
            boost::uint8_t * ch = (boost::uint8_t *)&value;
            ch[0] = major;
            ch[1] = minor;
            ch[2] = (boost::uint8_t)((revision >> 8) & 0x00ff);
            ch[3] = (boost::uint8_t)((revision      ) & 0x00ff);
            ch[4] = (boost::uint8_t)((build >> 24) & 0x000000ff);
            ch[5] = (boost::uint8_t)((build >> 16) & 0x000000ff);
            ch[6] = (boost::uint8_t)((build >>  8) & 0x000000ff);
            ch[7] = (boost::uint8_t)((build      ) & 0x0000000ff);
        }

        Version::Version(
            std::string const & str)
        {
            from_string(str);
        }

        boost::system::error_code Version::from_string(
            std::string const & str)
        {
            StringToken st(str, ".");
            std::string major_str;
            std::string minor_str;
            std::string revision_str;
            std::string build_str;
            boost::uint8_t major = 0;
            boost::uint8_t minor = 0;
            boost::uint16_t revision = 0;
            boost::uint16_t build = 0;
            boost::system::error_code ec;
            if (st.next_token(major_str, ec) || 
                st.next_token(minor_str, ec) || 
                (ec = parse2(major_str, major)) || 
                (ec = parse2(minor_str, minor))) {
                    return ec;
            }
            if (!st.next_token(revision_str, ec)) {
                ec = parse2(revision_str, revision);
            }
            if (!ec) {
                ec = parse2(st.remain(), build);
            }
            if (ec == framework::system::logic_error::no_more_item) {
                ec = boost::system::error_code();
            }
            if (!ec) {
                *this = Version(major, minor, revision, build);
            }
            return ec;
        }

        std::string Version::to_string() const
        {
            boost::uint8_t const * ch = (boost::uint8_t const *)&value;
            return format(ch[0]) 
                + "." + format(ch[1]) 
                + "." + format(
                    (((boost::uint16_t)ch[2]) << 8) 
                    | (boost::uint16_t)ch[3])
                + "." + format(
                    (((boost::uint32_t)ch[4]) << 24) 
                    | ((boost::uint32_t)ch[5] << 16)
                    | ((boost::uint32_t)ch[6] << 8)
                    | ((boost::uint32_t)ch[7]));
        }

        std::string Version::to_simple_string() const
        {
            boost::uint8_t const * ch = (boost::uint8_t const *)&value;
            return format(ch[0]) 
                + "." + format(ch[1]) 
                + "." + format(
                    (((boost::uint16_t)ch[2]) << 8) 
                    | (boost::uint16_t)ch[3]);
        }

        boost::system::error_code Version::get_version(
            std::string const & file, 
            std::string const & module, 
            std::string & version)
        {
            FileTag ft(file, "version");
            return ft.get(module, version);
        }

        boost::system::error_code Version::set_version(
            std::string const & file, 
            std::string const & module, 
            std::string const & version)
        {
            FileTag ft(file, "version");
            return ft.set(module, version);
        }

        boost::system::error_code Version::get_version(
            std::string const & file, 
            std::map<std::string, std::string> & module_versions)
        {
            FileTag ft(file, "version");
            return ft.get_all(module_versions);
        }

    } // namespace system
} // namespace boost
