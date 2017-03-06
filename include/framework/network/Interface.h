// Interface.h

#ifndef _FRAMEWORK_NETWORK_INTERFACE_H_
#define _FRAMEWORK_NETWORK_INTERFACE_H_

#include <boost/asio/ip/address.hpp>

namespace framework
{
    namespace network
    {

        struct Interface
        {
            enum FlagEnum
            {
                up              = 0x0001, 
                broadcast       = 0x0002, 
                debug           = 0x0004, 
                loopback        = 0x0008, 
                pointtopoint    = 0x0010, 
                notrailers      = 0x0020, 
                running         = 0x0040, 
                noarp           = 0x0080, 
                promisc         = 0x0100, 
                allmulti        = 0x0200, 

                master          = 0x0400, 
                slave           = 0x0800, 

                multicast       = 0x1000, 

                portsel         = 0x2000, 
                automedia       = 0x4000, 
                dynamic         = 0x8000, 
            };

            Interface()
                : flags(0)
                , metric(0)
                , mtu(0)
            {
                name[0] = '\0';
            }

            char name[64];
            int index;
            int flags;
            int metric;
            int mtu;
            boost::uint8_t hwaddr[6];
            boost::asio::ip::address addr;
            boost::asio::ip::address netmask;

            std::string hard_addr_string(
                char splitter = '-', 
                bool up_cast = true) const;
        };

        boost::system::error_code enum_interface(
            std::vector<Interface> & interfaces);

    } // namespace network
} // namespace framework

#endif//_FRAMEWORK_NETWORK_INTERFACE_H_
