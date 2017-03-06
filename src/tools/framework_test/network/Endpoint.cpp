// Interface.cpp

#include "tools/framework_test/Common.h"
#include <framework/network/Endpoint.h>
using namespace framework::system;
using namespace framework::configure;
using namespace framework::network;

static void test_network_endpoint(Config & conf)
{
    Endpoint ep("(tcp)127.0.0.1:80");
}

static TestRegister test("network_endpoint", test_network_endpoint);
