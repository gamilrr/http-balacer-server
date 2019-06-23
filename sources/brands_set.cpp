#include "brands_set.hpp"
#include "protocol_cipher.hpp"
#include <string>
#include <exception>

namespace balancer_server_module{
    
    brands_set::brands_set(std::string pFilePath):file_read_once(pFilePath),  vBrands{}, brandsSet{}{
        set_parameters();
    }
    
    void brands_set::set_parameters(){
        std::string hexString;
        std::string hex;
        while(mFileData >> hex){
            hexString += hex;
        }
       
        vBrands = protocol_cipher::hex2str_to_bin(hexString);
        if(vBrands.size() == 0){
            throw std::runtime_error("ERROR bad format in brands.conf");
        }
        
        brandsSet.insert(vBrands.begin(), vBrands.end());
    }
    
    
    
}

