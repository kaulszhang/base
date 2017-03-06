//  error_code support implementation file  ----------------------------------//

//  Copyright Beman Dawes 2002, 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See library home page at http://www.boost.org/libs/system

//----------------------------------------------------------------------------//

// define BOOST_SYSTEM_SOURCE so that <boost/system/config.hpp> knows
// the library is being built (possibly exporting rather than importing code)
#define BOOST_SYSTEM_SOURCE

#include <algorithm>

#include <boost/system/error_code.hpp>

#ifndef BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/system/detail/error_code.ipp>
#endif

namespace boost
{
    namespace system
    {
        std::vector<error_category const *> & error_category::get_categories()
        {
            static std::vector<error_category const *> categories;
            return categories;
        }

        std::vector<error_category const *> & error_category::categories_ = error_category::get_categories();

        BOOST_SYSTEM_DECL void error_category::register_category(error_category const & cat)
        {
            get_categories().push_back(&cat);
        }

        BOOST_SYSTEM_DECL void error_category::unregister_category(error_category const & cat)
        {
            std::vector<error_category const *> & categories = get_categories();
            categories.erase(std::remove(categories.begin(), categories.end(), &cat), categories.end());
            if (categories.empty()) {
                // ÊÍ·ÅÄÚ²¿´æ´¢
                std::vector<error_category const *>().swap(categories);
            }
        }

        BOOST_SYSTEM_DECL error_category const & error_category::find_category(char const * name)
        {
            std::vector<error_category const *>::const_iterator iter = get_categories().begin();
            std::vector<error_category const *>::const_iterator end = get_categories().end();
            for (; iter != end; ++iter)
                if (strcmp((*iter)->name(), name) == 0)
                    return **iter;
            assert(false);
            return get_generic_category();
        }

    }
}
