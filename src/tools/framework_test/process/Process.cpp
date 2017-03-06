// Process.cpp

#include "tools/framework_test/Common.h"

#include <framework/process/Process.h>
#include <framework/system/ErrorCode.h>
using namespace framework::process;
using namespace framework::system;
using namespace framework::configure;

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
using namespace boost::system;

void thread_create_process(
                           Process & process, 
                           std::string const & cmd, 
                           error_code & ec)
{
    process.create(cmd, ec);
}

static void test_process(Config & conf)
{
    std::vector<ProcessInfo> processes;
    std::string cmd("");
    error_code ec = enum_process(cmd, processes);
    for (size_t i = 0; i < processes.size(); ++i) {
        std::cout << processes[i].pid << " " << processes[i].bin_file << std::endl;
    }
/*
    Process process;
    process.open(cmd, ec);
    if (ec) {
        std::cout << "open failed: " << ec.message() << std::endl;
        boost::thread th(boost::bind(thread_create_process, 
            boost::ref(process), 
            boost::cref(cmd), 
            boost::ref(ec)));
        th.join();
        if (ec)
            std::cout << "start failed: " << ec.message() << std::endl;
    }
    if (!ec) {
        process.is_alive(ec);
        if (ec)
            std::cout << "is_alive failed: " << ec.message() << std::endl;
    }
    if (!ec) {
        process.join(ec);
        if (ec)
            std::cout << "join failed: " << ec.message() << std::endl;
        else
            std::cout << "process exit with code " << process.exit_code(ec) << std::endl;
    }
    */
}

static TestRegister test("process", test_process);
