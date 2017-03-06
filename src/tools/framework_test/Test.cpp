// main.cpp

#include "tools/framework_test/Common.h"

#include <framework/Version.h>
#include <framework/configure/Config.h>
#include <framework/logger/Logger.h>

#include <boost/thread/thread.hpp>

int main(int argc, char * argv[])
{
    //std::cout << "framework version: " << framework::version().to_string() << std::endl;

    framework::configure::Config conf("framework_test.conf");
    framework::logger::global_logger().load_config(conf);

    for (int i = 2; i < argc; ++i) {
        conf.profile().pre_set(argv[i]);
    }

    if (argc == 1 || std::string(argv[1]) == "all")
        start_test_all(conf);
    else
        start_test(argv[1], conf);

    stop_test();

    std::cout << "press any key to continue..." << std::flush;
    std::cin.get();

    return 0;
}
