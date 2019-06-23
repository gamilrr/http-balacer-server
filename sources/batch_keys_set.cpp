#include "batch_keys_set.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <exception>

namespace balancer_server_module{

    batch_keys_set::batch_keys_set(std::string pFilePath) : file_read_once(pFilePath), batchKeysSet{}, batchKeysVec{} {
      set_parameters();
    }


    void batch_keys_set::set_parameters(){
       
        std::string data;
        std::istringstream ss;
        
        while(std::getline(mFileData, data)){
            if(data.substr(0,2) == "//" || data.substr(0,2) == "\r"){
                mFileData >> std::ws;
                continue;
            }    
            
            ss.str(data);
            try{
                ss >> data;
                batchKeysVec.push_back(static_cast<hex4>(std::stoi(data, 0, 16)));
            
            }catch(...){
                throw std::runtime_error("ERROR bad format in batch_keys.conf file");
            }
        }
        
        batchKeysSet.insert(batchKeysVec.begin(), batchKeysVec.end());
    }

}


