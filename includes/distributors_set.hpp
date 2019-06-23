#ifndef DISTRIBUTORS_SET_HPP
#define DISTRIBUTORS_SET_HPP

#include "bs_types.hpp"
#include <array>
#include <cstdint>
#include "file_read_once.hpp"

namespace balancer_server_module{

    
    
class distributors_set : file_read_once {
    struct distributor{
        hex2 id;
        std::array<uint,2> num_range;
        std::array<uint,2> batch_range;
    };
    
    
public:
    distributors_set(std::string pFilePath);
    
    std::map<hex2, distributor> distRangeMap;
    
private:
    void set_parameters() override;
};

}
#endif /* DISTRIBUTORS_SET_HPP */

