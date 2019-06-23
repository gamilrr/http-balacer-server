#ifndef BS_RANDOM_HPP
#define BS_RANDOM_HPP

#include "bs_types.hpp"

namespace balancer_server_module{

struct bs_random {
    
    static hex2 hex2_random();
    static std::vector<hex2> vhex2_random(int len);
    
};

}
#endif /* BS_RANDOM_HPP */

