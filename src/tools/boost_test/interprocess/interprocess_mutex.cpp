// function.cpp

#include "tools/boost_test/Common.h"

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::configure;
using namespace framework::logger;

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/thread/thread.hpp>

using namespace boost::interprocess;
using namespace boost;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("test_interprocess_mutex", 0)

struct my_mutex
    : public interprocess_mutex
{
    my_mutex()
        : is_construct(true)
    {
    }

    bool is_construct;
};

void test_interprocess_mutex(Config & conf)
{
    std::string name = "test_interprocess_mutex";

    try {
        shared_memory_object shm(open_or_create, name.c_str(), read_write);
        mapped_region mr(shm, read_write);
        my_mutex * mtx = (my_mutex *)mr.get_address();
        if (!mtx->is_construct)
            new (mtx) my_mutex;
        mtx->lock();
        LOG_S(Logger::kLevelDebug, "lock");
        this_thread::sleep(boost::posix_time::seconds(5));
        mtx->unlock();
        LOG_S(Logger::kLevelDebug, "unlock");
    } catch (std::exception const & err) {
        std::cout << "open shm: " << err.what() << std::endl;
    } catch (...) {
        std::cout << "open shm: unknown error" << std::endl;
    }

    shared_memory_object::remove(name.c_str());
}

static TestRegister test("interprocess_mutex", test_interprocess_mutex);
