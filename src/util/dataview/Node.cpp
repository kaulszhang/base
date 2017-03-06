// Node.cpp

#include "util/Util.h"
#include "util/dataview/Node.h"
#include "util/dataview/DataViewError.h"
using namespace util::dataview::error;

#include <framework/string/StringToken.h>
using namespace framework::string;

using namespace boost::system;

namespace util
{

    namespace dataview
    {

        std::string Path::to_string(
            std::string const & spliter) const
        {
            std::string result;
            for (size_t i = 0; i < size(); ++i) {
                result += spliter;
                result += (*this)[i];
            }
            if (result.empty())
                result = spliter;
            return result;
        }

        error_code Path::from_string(
            std::string const & path_str, 
            std::string const & spliter)
        {
            StringToken st(path_str, spliter, true);
            std::string token;
            clear();
            boost::system::error_code ec;
            while (st.next_token(token, ec) == error_succeed) {
                push_back(token);
            }
            return error_succeed;
        }

    }
}
