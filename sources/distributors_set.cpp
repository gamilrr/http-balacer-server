#include "distributors_set.hpp"
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <exception>
#include <array>

namespace balancer_server_module{

    distributors_set::distributors_set(std::string pFilePath) : file_read_once(pFilePath), distRangeMap{}{
        set_parameters();
    }
    
    void distributors_set::set_parameters(){
        std::array<char,3> delm = {':', '|', '-'};
        std::string data;
        std::string sTemp;
        hex2 id;
        uint rangeL, rangeH;
        std::istringstream ss;
        
        while(std::getline(mFileData, data)){
            if(data.substr(0,2) == "//" || data.substr(0,2) == "\r"){
                mFileData >> std::ws;
                continue;
            }    
            
            std::transform(data.begin(), data.end(), data.begin(), [&delm](auto ch){
                for(char c : delm)
                if(c == ch){
                    return ' ';
                }
                   return ch;});
                
            ss.str(data);
            try{
                ss >> sTemp;
                id = static_cast<hex2>(std::stoi(sTemp,0,16));
                if(id > 0xFF) throw;
                
                ss >> rangeL;
                ss >> rangeH;
                if(rangeL > rangeH) throw;
                distRangeMap[id].num_range[0] = rangeL;
                distRangeMap[id].num_range[1] = rangeH;
                
                ss >> rangeL;
                ss >> rangeH;
                if(rangeL > rangeH) throw;
                distRangeMap[id].batch_range[0] = rangeL;
                distRangeMap[id].batch_range[1] = rangeH;
                
                  
           
            }catch(...){
                throw std::runtime_error("ERROR bad format in distributors.conf file");
            }
            
            
            
            
        }
        
 
    }



}

