// Convert.cpp

#include "framework/Framework.h"
#include "framework/string/Convert.h"
using namespace framework::system::logic_error;

#include <sstream>

#include <errno.h>

#ifdef FRAMEWORK_WITH_LIB_ICONV
#  ifndef BOOST_WINDOWS_API
#    include <iconv.h>
#  else
#    include <windows.h>
#  endif
#endif

namespace framework
{
    namespace string
    {

        Convert::Convert(
            std::string const & t1, 
            std::string const & t2)
            : t1(t1)
            , t2(t2)
        {
        }

        Convert::~Convert()
        {
        }

        boost::system::error_code Convert::convert(
            std::string const & s1, 
            std::string & s2)
        {
            boost::system::error_code retval = succeed;
#ifdef FRAMEWORK_WITH_LIB_ICONV
#  ifndef BOOST_WINDOWS_API
            iconv_t cd = iconv_open(t1.c_str(), t2.c_str());
            if (cd == (iconv_t)-1)
                return item_not_exist;
            std::ostringstream oss;
            char buf[1024];
            char *inbuf = (char*)s1.c_str();
            size_t inbytesleft = s1.size();
            char *outbuf = buf;
            size_t outbytesleft = sizeof(buf);
            size_t ret = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            while (ret == (size_t)-1) {
                if (errno != E2BIG) {
                    if (errno == EILSEQ) {
                        err_pos_ = inbuf - s1.c_str();
                        retval = invalid_argument;
                    } else if (errno == EINVAL) {
                        err_pos_ = inbuf - s1.c_str();
                        retval = invalid_argument;
                    } else
                        retval = unknown_error;
                    break;
                }
                oss.write(buf, outbuf - buf);
                outbuf = buf;
                outbytesleft = sizeof(buf);
                ret = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            }
            if (ret != (size_t)-1) {
                oss.write(buf, outbuf - buf);
                s2 = oss.str();
            }
            iconv_close(cd);
#  else // BOOST_WINDOWS_API
            static std::map<std::string, unsigned int> name_cp;
            if (name_cp.empty()) {
                name_cp["unicode"] = 0;
                name_cp["acp"] = CP_ACP;
                name_cp["tacp"] = CP_THREAD_ACP;
                name_cp["utf8"] = CP_UTF8;
                name_cp["utf-8"] = CP_UTF8;
                name_cp["gbk"] = 936;
            }
            retval = invalid_argument;
            std::basic_string<WCHAR> unicode;
            int n1 = MultiByteToWideChar(name_cp[t2], 0, &s1[0], s1.length(), NULL, 0);
            if (n1 > 0) {
                unicode.reserve(n1);
                MultiByteToWideChar(name_cp[t2], 0, &s1[0], s1.length(), &unicode[0], n1);
                int n2 = WideCharToMultiByte(name_cp[t1], 0, &unicode[0], n1, NULL, 0, NULL, NULL);
                if (n2 > 0) {
                    s2.resize(n2);
                    WideCharToMultiByte(name_cp[t1], 0, &unicode[0], n1, &s2[0], n2, NULL, NULL);
                    retval = succeed;
                }
            }
#  endif // BOOST_WINDOWS_API
#else // FRAMEWORK_NO_ICONV
            retval = not_supported;
#endif // FRAMEWORK_NO_ICONV
            return retval;
        }

    } // namespace string
} // namespace framework
