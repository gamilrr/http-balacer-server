#ifndef BALANCER_MODE_HPP
#define BALANCER_MODE_HPP

#include "bs_types.hpp"
#include <string>
#include <vector>
#include <map>

namespace balancer_server_module{
    

class balancer_mode { //interface for balancing mode 
public:
    
     virtual void start() = 0;
    virtual void stop() = 0;
    
    virtual int get_item(uint) = 0; 
    
    virtual void set_param(const std::map<uint,node_id>& ) = 0;
    
    virtual std::vector<hex2> get_sids(const std::vector<hex4>& provID, uint nodeID) = 0;
    
    virtual ~balancer_mode() = default;
    
};

}

#endif /* BALANCER_MODE_HPP */

