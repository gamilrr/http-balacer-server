#include "uid_keys_set.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <exception>
#include "protocol_cipher.hpp"

namespace balancer_server_module{

uid_keys_set::uid_keys_set(std::string pFilePath):file_read_once(pFilePath), uidKeysMap{} {
    set_parameters();
}

void uid_keys_set::set_parameters(){
    char delem = ':';
    char comment[] = "//";
    std::string data;
    std::string sTemp;
    std::vector<hex2> vTemp;
    std::istringstream ss;
            
    while(std::getline(mFileData, data)){
        if(data.substr(0,2) == std::string(comment) || data.substr(0,2) == "\r" || data.substr(0, 2) == "\n\r" || data.substr(0, 2) == "\n" || data.substr(0, 2) == ""){
                mFileData >> std::ws;
				sTemp.clear();
                continue;
        }
        
        std::replace(data.begin(), data.end(),delem, ' ');
        ss.str(data);
		ss.seekg(0);
        gen_card_key gen;
        
        try{
            ss >> sTemp;
            sTemp = sTemp.substr(1);
            gen.id = stoi(sTemp);
            ss >> sTemp; 
            if(sTemp != "KEY") throw;
            ss >> sTemp;
            vTemp = protocol_cipher::hex2str_to_bin(sTemp);
            if(vTemp.size() == 0 || vTemp.size() != 16) throw;
            gen.key = vTemp;
            ss >> sTemp;
            if(sTemp != "IV") throw;
            ss >> sTemp;
            vTemp = protocol_cipher::hex2str_to_bin(sTemp);
            if(vTemp.size() == 0 || vTemp.size() != 8) throw;
            gen.iv = vTemp;
            
            uidKeysMap[gen.id] = gen;
            
        }catch(...){
            throw std::runtime_error("ERROR bad format in uid_keys.conf file");
        }
    }
  
}

}