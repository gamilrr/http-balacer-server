#ifndef CARDID_SETS_HPP
#define CARDID_SETS_HPP

#include "server_config.hpp"
#include "batch_keys_set.hpp"
#include "brands_set.hpp"
#include "distributors_set.hpp"
#include "uid_keys_set.hpp"

namespace balancer_server_module{
    
struct valid_cid_sets {
    
    explicit valid_cid_sets(const server_config& pConfig);
    
    uid_keys_set mUIDKeys;
    batch_keys_set mBatchKeys;
    distributors_set mDistributors;
    brands_set mBrands;


};
}
#endif /* CARDID_SETS_HPP */

