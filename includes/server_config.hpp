/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   server_config.hpp
 * Author: root
 *
 * Created on January 10, 2018, 2:35 PM
 */
#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include "file_read_once.hpp"
#include "bs_types.hpp"

namespace balancer_server_module{
    
   
    
class server_config: public file_read_once
{

public:
    server_config(std::string pFilePath);
    server_config(); //default constructor will search the file in the current directory
    
    std::string index1;
    std::string index2;
    std::string index3;
    //std::vector<std::string>index4Url;
    //std::vector<std::string>index4Ip;
    std::map<uint,node_id> ind4Node; 
    std::string epcsPath;

private:
    void set_parameters() override;
    
};


}
#endif /* SERVER_CONFIG_HPP */

