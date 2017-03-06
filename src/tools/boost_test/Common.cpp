// Common.cpp

#include "tools/boost_test/Common.h"

using namespace framework::system::logic_error;
using namespace framework::configure;

using namespace boost::system;

std::map<std::string, FUNC_TEST> * tests_ = NULL;

error_code reg_test(
    std::string const & name, 
    FUNC_TEST test)
{
    if (!tests_) {
        tests_ = new std::map<std::string, FUNC_TEST>;
    }
    std::pair<std::map<std::string, FUNC_TEST>::iterator, bool> ret = 
        tests_->insert(std::make_pair(name, test));
    return ret.second ? succeed : item_already_exist;
}

void run_test(
    std::map<std::string, FUNC_TEST>::iterator i, 
    Config & conf)
{
    if (i->second) {
        std::cout << "start test " << i->first << std::endl;
        i->second(conf);
        i->second = NULL;
    }
};


error_code start_test(
    std::string const & name, 
    Config & conf)
{
    std::map<std::string, FUNC_TEST>::iterator i = tests_->find(name);
    if (i == tests_->end()) {
        std::cout << "test instance not found" << std::endl;
        return item_not_exist;
    } else {
        run_test(i, conf);
        return succeed;
    }
}

error_code start_test_all(
    Config & conf)
{
	if (!tests_)
		return succeed;
    std::map<std::string, FUNC_TEST>::iterator i = tests_->begin();
    for (; i != tests_->end(); ++i) {
        run_test(i, conf);
    }
    return succeed;
}

error_code stop_test()
{
    if (tests_) {
        delete tests_;
        tests_ = NULL;
    }
    return succeed;
}
