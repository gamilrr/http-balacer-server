#ifndef BS_TYPES_H
#define BS_TYPES_H

#include <cstdint>
#include <string>
#include <vector>
namespace balancer_server_module{
    
    
    using byte = uint8_t; //type alias 
    using hex2 = byte; //hex of two digits
    using hex4 = uint16_t ;//hex of four digits
    using stime = unsigned int ;//time in second
    using uint = unsigned int; 
	
     struct node_id{
        std::vector<std::string> urls;
        std::vector<std::string> ips;
    };
    
    
    //using size = std::size_t; //size type depending on platform
}

#endif /* BS_TYPES_H */

