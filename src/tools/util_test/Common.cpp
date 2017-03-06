// Common.cpp

#include "tools/util_test/Common.h"

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
    return ret.second ? succeed : invalid_argument;
}

error_code start_test(
    std::string const & name, 
    Config & conf)
{
    std::map<std::string, FUNC_TEST>::iterator i = tests_->find(name);
    if (i == tests_->end()) {
        std::cerr << "test instance not found" << std::endl;
        return invalid_argument;
    } else {
        i->second(conf);
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
        std::cout << "start test " << i->first << std::endl;
        i->second(conf);
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
