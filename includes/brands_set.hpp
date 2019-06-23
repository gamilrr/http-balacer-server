#ifndef BRAND_CONFIG_HPP
#define BRAND_CONFIG_HPP

#include "bs_types.hpp"
#include "file_read_once.hpp"
#include <vector>
#include <set>

namespace balancer_server_module{

class brands_set : public file_read_once
{
public:
    
    explicit brands_set(std::string pFilePath);
    
    //get rid of slower one
    std::vector<hex2> vBrands;
    std::set<hex2> brandsSet;
    
private:
    void set_parameters() override;
};
}

#endif /* BRAND_CONFIG_HPP */

