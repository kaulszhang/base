// function.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/system/system_error.hpp>

using namespace boost::interprocess;

void test_shared_memory_object(Config & conf)
{
    std::string name = "test_shared_memory_object";

    try {
        shared_memory_object shm(open_only, name.c_str(), read_write);
    } catch (std::exception const & err) {
        std::cout << "open shm: " << err.what() << std::endl;
    } catch (...) {
        std::cout << "open shm: unknown error" << std::endl;
    }

    try {
        shared_memory_object shm(create_only, name.c_str(), read_write);
        shm.truncate(1024);
        mapped_region mr(shm, read_only);
    } catch (std::exception const & err) {
        std::cout << "create shm: " << err.what() << std::endl;
    } catch (...) {
        std::cout << "open shm: unknown error" << std::endl;
    }

    shared_memory_object::remove(name.c_str());
}

static TestRegister test("shared_memory_object", test_shared_memory_object);
