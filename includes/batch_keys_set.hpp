#ifndef BATCH_KEYS_SET_HPP
#define BATCH_KEYS_SET_HPP

#include "file_read_once.hpp"
#include "bs_types.hpp"
#include <string>
#include <vector>
#include <set>

namespace balancer_server_module{

class batch_keys_set : file_read_once{
public:
    batch_keys_set(std::string pFilePath);
   
    //get rid of slower one
    std::set<hex4> batchKeysSet;
    std::vector<hex4> batchKeysVec;
    
private:
    void set_parameters() override;
};
}

#endif /* BATCH_KEYS_SET_HPP */

