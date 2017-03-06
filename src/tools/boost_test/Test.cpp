// main.cpp

#include "tools/boost_test/Common.h"

#include <framework/configure/Config.h>
#include <framework/logger/Logger.h>

void test_shared_memory_object(framework::configure::Config & conf);

int main(int argc, char * argv[])
{
    framework::configure::Config conf("boost_test.conf");
    framework::logger::glog.load_config(conf);

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

