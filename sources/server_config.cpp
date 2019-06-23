#include "server_config.hpp"
#include <stdexcept>
#include <string>
#include <iostream>
#include <limits>
#include <algorithm>

namespace balancer_server_module{
    
    
    server_config::server_config(std::string pFilePath):file_read_once(pFilePath), index1{}, index2{}, index3{}, ind4Node{}, epcsPath{}{set_parameters();}
    
    server_config::server_config():server_config("./balancer_server.conf"){}
    
    void server_config::set_parameters(){
        std::string data;
        char commentChar = '#';
        std::string protocol("http://");
        std::vector<bool> parFlag(5, false);
        
        while(mFileData >> data){ //extraction of string from file
            if(data[0] == commentChar){
                mFileData.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
                
            if(data == "index1"){
                mFileData >> index1;
                if(index1.substr(0,protocol.size()) == protocol){
                    parFlag[0] = true;
                    continue;
                }
                /*throw by default if some data was wrong*/
                throw std::runtime_error("ERROR wrong configuration file format");
            }else if(data == "index2"){
                mFileData >> index2;
                if(index2.substr(0,protocol.size()) == protocol){
                    parFlag[1] = true;
                    continue;
                }
                /*throw by default if some data was wrong*/
                throw std::runtime_error("ERROR wrong configuration file format");
            }else if(data == "index3"){
                mFileData >> index3;
                if(index3.substr(0,protocol.size()) == protocol){
                     parFlag[2] = true;
                    continue;
                }    
                /*throw by default if some data was wrong*/
                throw std::runtime_error("ERROR wrong configuration file format");
            }else if(data == "index4"){
                bool serversFlag = false;
                bool nodeIdFlag = false;
                uint nodeId = 0;
                
                std::getline(mFileData, data);
                std::transform(data.begin(), data.end(), data.begin(), [](auto ch){
                    if(ch == '|'){
                        return ' ';
                    }
                       return ch;});
                       
                std::istringstream mFileDataLine(data);
                
                while(mFileDataLine >> data){
                    if(data[0] == commentChar){ 
                         mFileData.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                         break;
                    }
                    
                    if(nodeIdFlag == false){
                        nodeId = std::stoul(data, 0, 16);
                        nodeIdFlag = true;
                        continue;
                    }
                    
                    if(data.substr(0,protocol.size()) == protocol && serversFlag == false && nodeIdFlag == true){
                        //index4Url.push_back(data);
                        ind4Node[nodeId].urls.push_back(data);
                    }else if(serversFlag == true){
                        //index4Ip.push_back(data);
                        ind4Node[nodeId].ips.push_back(data);
                        parFlag[3] = true;
                    }else{
                        /*throw by default if some data was wrong*/
                       throw std::runtime_error("ERROR wrong configuration file format");
                    }
                    serversFlag = !serversFlag;
                }
                continue;
            }else if(data == "EPCS_Path"){
                mFileData >> epcsPath;
                std::string typePath = epcsPath.substr(0,1);
                if(typePath == "/" || typePath == "."){
                    parFlag[4] = true;
                    continue;
                }
                /*throw by default if some data was wrong*/
                throw std::runtime_error("ERROR wrong configuration file format");
            }else
                 throw std::runtime_error("ERROR wrong configuration file format");
            
            
        }
        
        for(bool i : parFlag){
            if(i == false){
                 throw std::runtime_error("ERROR missing parameter in configuration file");
            }
        }
        
    }

}

