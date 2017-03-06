// ErrorCode.h

#ifndef _FRAMEWORK_SYSTEM_ERROR_CODE_H_
#define _FRAMEWORK_SYSTEM_ERROR_CODE_H_

#include <boost/system/error_code.hpp>

namespace framework
{
  namespace system
  {

      boost::system::error_code last_system_error();

  } // namespace system
} // namespace boost

#endif // _FRAMEWORK_SYSTEM_ERROR_CODE_H_
