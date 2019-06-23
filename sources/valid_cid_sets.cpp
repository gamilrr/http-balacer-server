#include "valid_cid_sets.hpp"

namespace balancer_server_module{

valid_cid_sets::valid_cid_sets(const server_config& pConfig): 
mUIDKeys{pConfig.epcsPath + "/uid_keys.conf"},
mBatchKeys{pConfig.epcsPath + "/batch_keys.conf"},
mDistributors{pConfig.epcsPath + "/distributors.conf"},
mBrands{pConfig.epcsPath + "/brands.conf"} 
{}

}

