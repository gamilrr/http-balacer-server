#include "protocol_cipher.hpp"
#include "s3_des.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <boost/algorithm/hex.hpp>
#include <algorithm>
#include <openssl/hmac.h>
#include <openssl/evp.h>

namespace balancer_server_module{

std::vector<hex2> protocol_cipher::hmacSha256(const std::vector<hex2>& pKey, const std::vector<hex2>& data){
    
     //calculate hmac_256
    unsigned char* sign = new unsigned char[EVP_MAX_MD_SIZE];
    unsigned int signLen;
    
    unsigned int keyLen = pKey.size();
    unsigned char* key = new unsigned char[keyLen];
    memcpy(key, pKey.data(), keyLen);
    
    unsigned int cdataLen = data.size();
    unsigned char* cdata = new unsigned char[cdataLen];
    memcpy(cdata, data.data(), cdataLen);
    
    
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);

    HMAC_Init_ex(&ctx, key, keyLen, EVP_sha256(), NULL);
    HMAC_Update(&ctx, cdata, cdataLen);
    HMAC_Final(&ctx, sign, &signLen);
    HMAC_CTX_cleanup(&ctx);
    
    
    std::vector<hex2> hmac{sign, sign + signLen};
    
    delete [] sign;
    delete [] cdata;
    delete [] key;
    
    return hmac;
}
    
    
std::vector<hex2> protocol_cipher:: hex2str_to_bin(const std::string& pStringData){
    
    std::vector<hex2> out;
    
    try{
        boost::algorithm::unhex(pStringData.begin(), pStringData.end(), std::back_inserter(out));
    }catch(std::exception& error){
        return std::vector<hex2>{};
    }
    
    return out;
}    


std::string protocol_cipher::bin_to_hex2str(const std::vector<hex2>& pVectorData){
    
    std::ostringstream stringData; //make this faster if possible....................................
    
    for(hex2 i : pVectorData){ //to make it faster try to use ostringstream instead strinstream
        stringData << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(i);
    }
    
    return stringData.str();
    
}

std::vector<hex2> protocol_cipher::unmask_data(const std::vector<hex2>& pDataMasked, const std::vector<hex2>& pMaskKey){
 
    int dataSize = pDataMasked.size();
    int keyIndex = 0;
    
    std::vector<hex2> result;
   
    for(int i = 0; i != dataSize; i++){
        result.push_back(pDataMasked[i] ^ pMaskKey[keyIndex++]);
        if(keyIndex == pMaskKey.size()){
            keyIndex = 0;
        }
                
    }
    
    return result;
    
}

std::vector<byte> protocol_cipher::custom_3des_decrypt(const std::vector<byte>& pData,const std::vector<byte>& pKey,const std::vector<byte>& pIVector){
    
    std::vector<byte> decData{pData};
    i_DES_key_schedule ks1, ks2;
    i_DES_key_sched((i_DES_cblock *)(pKey.data() + 0), &ks1);
    i_DES_key_sched((i_DES_cblock *)(pKey.data() + 8), &ks2);
    
    unsigned char *iv = new unsigned char[pIVector.size()];
    
    memcpy(iv, pIVector.data(), pIVector.size());
    
    i_DES_ede2_cbc_encrypt(pData.data(),decData.data(), pData.size(), &ks1, &ks2, (i_DES_cblock *)iv, i_DES_DECRYPT); //this function will modify the IV parameter be aware about that
    
    delete [] iv;
    
    return decData; 
}

}