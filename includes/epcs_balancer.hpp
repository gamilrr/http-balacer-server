#ifndef EPCS_BALANCER_HPP
#define EPCS_BALANCER_HPP

#include <string>
#include <sstream>
#include <memory>
#include <boost/thread.hpp>
#include "server_files_header.hpp"
#include "balancer_mode.hpp"
#include "cmd_client.hpp"

namespace balancer_server_module{

class epcs_balancer {
public:
    explicit epcs_balancer(balancer_mode& mode);
    epcs_balancer(balancer_mode& mode ,std::string pConfigFilePath);
    
    /*this function analyzes the request path and return the correct response*/
    std::string req_path_analysis(std::string pPathReq);
   
private:
    server_config mConfig; //here the order is important in order to improve performance
    valid_cid_sets mCid;
    
    balancer_mode& mBalancer; //shared_ptr will allow me exchange balancer resources 
    std::vector<hex2> key;
    cmd_client mCmdClient;
    
    std::string mGoodResponse;
    std::ostringstream mConv; //here because speed reasons
    
};
}
#endif /* EPCS_BALANCER_HPP */

