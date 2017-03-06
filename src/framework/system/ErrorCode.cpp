//  error_code support implementation file  ----------------------------------//

//  Copyright Beman Dawes 2002, 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See library home page at http://www.boost.org/libs/system

//----------------------------------------------------------------------------//

#include "framework/Framework.h"
#include "framework/system/ErrorCode.h"

# if defined( BOOST_WINDOWS_API )
#   include <windows.h>
# endif

namespace framework
{
  namespace system
  {

      boost::system::error_code last_system_error()
      {
#if !defined( BOOST_WINDOWS_API )
          return boost::system::error_code(errno, boost::system::get_system_category());
#else
          return boost::system::error_code(::GetLastError(), boost::system::get_system_category());
#endif
      }

  } // namespace system
} // namespace boost
