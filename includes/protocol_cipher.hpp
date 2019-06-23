#ifndef CIPHER_HPP
#define CIPHER_HPP

#include <string>
#include <vector>
#include <array>
#include "bs_types.hpp"

namespace balancer_server_module{

struct protocol_cipher {
    
    
    
    /*calculate hmac sha256 digest*/
    static std::vector<hex2> hmacSha256(const std::vector<hex2>& pKey, const std::vector<hex2>& data);
    
    
    /*become string to two digits hex data vector*/
    static std::vector<hex2> hex2str_to_bin(const std::string& pStringData);
    
    /*become vector of two digit hex to string, return a empty vector if the string has a bad format*/
    static std::string bin_to_hex2str(const std::vector<hex2>& pVectorData);
    
    /*unmask the data coming from client*/
    static std::vector<byte> unmask_data(const std::vector<byte>& pDataMasked, const std::vector<byte>& pMaskKey);
    
    
    static std::vector<byte> custom_3des_decrypt(const std::vector<byte>& pData,const std::vector<byte>& pKey, const std::vector<byte>& pIVector);   
   
};   


}
#endif /* CIPHER_HPP */

