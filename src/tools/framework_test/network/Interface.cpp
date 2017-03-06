// Interface.cpp

#include "tools/framework_test/Common.h"

#include <framework/network/Interface.h>
using namespace framework::system;
using namespace framework::configure;
using namespace framework::network;

char const * const flag_str[] = {
   "UP", 
   "BROADCAST", 
   "DEBUG", 
   "LOOPBACK", 
   "POINTTOPOINT", 
   "NOTRAILERS", 
   "RUNNING", 
   "NOARP", 
   "PROMISC", 
   "ALLMULTI", 

   "MASTER", 
   "SLAVE", 

   "MULTICAST", 

   "PORTSEL", 
   "AUTOMEDIA", 
   "DYNAMIC", 

   "LOWER_UP", 
   "DORMANT", 
};

static void test_network_interface(Config & conf)
{
    std::vector<Interface> infs;
    enum_interface(infs);
    for (size_t i = 0; i < infs.size(); ++i) {
        Interface const & inf = infs[i];
        std::cout << "Interface " << inf.name << std::endl;
        std::cout << "  Index: " << inf.index << std::endl;
        std::cout << "  Metric: " << inf.metric << std::endl;
        std::cout << "  Mtu: " << inf.mtu << std::endl;
        std::cout << "  Flags:" ;
        size_t idx = 0;
        for (boost::uint16_t f = 1; f; ++idx, f <<= 1) {
            if (inf.flags & f)
                std::cout << " " << flag_str[idx];
        }
        std::cout << std::endl;
        std::cout << "  HARD Address: " << inf.hard_addr_string() << std::endl;
        std::cout << "  IP Address: " << inf.addr.to_string() << std::endl;
        std::cout << "  IP Mask: " << inf.netmask.to_string() << std::endl;
    }
}

static TestRegister test("network_interface", test_network_interface);
