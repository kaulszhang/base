// Environments.h

#ifndef _FRAMEWORK_PROCESS_ENVIRONMENTS_H_
#define _FRAMEWORK_PROCESS_ENVIRONMENTS_H_

namespace framework
{
    namespace process
    {

        std::string get_environment(
            std::string const & key, 
            std::string const & def = "");

        void set_environment(
            std::string const & key, 
            std::string const & value);

        void clear_environment(
            std::string const & key);

    } // namespace process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_SIGNAL_HANDLER_H_
