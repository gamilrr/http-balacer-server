#include "epcs_balancer.hpp"
#include "protocol_cipher.hpp"
#include "bs_random.hpp"
#include <string>
#include <vector>
#include <iterator>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstdlib>


namespace balancer_server_module{
 

epcs_balancer::epcs_balancer(balancer_mode& balancer,std::string pConfigFilePath):mConfig{pConfigFilePath}, 
mCid{mConfig},
mConv{},
mGoodResponse{"[Msg:+][index1:" + mConfig.index1.substr(7) + "][index2:" + mConfig.index2.substr(7) + "][index3:" + mConfig.index3.substr(7) + "][index4:"},//is completed after depending of round robin algorithm
mBalancer{balancer},
mCmdClient{}
{
    mBalancer.set_param(mConfig.ind4Node);
    mBalancer.start(); //start balancer thread
    mCmdClient.start("127.0.0.1", 45643);
    key = {0xb6, 0x59, 0x9e, 0x37, 0xce, 0xfa, 0x0a, 0x58, 0x32, 0x50, 0xdc, 0xd1, 0xeb, 0x8d, 0xde, 0xc7};
   
}

epcs_balancer::epcs_balancer(balancer_mode& mode):epcs_balancer(mode,"./balancer_server.conf") {}

//optimize these calls for release version----------------------
std::string epcs_balancer::req_path_analysis(std::string pRequestPath){ 
    
    int pathLen = pRequestPath.size(); //filter the length of request path to identify the type of request
    
    if((pathLen != 52) && (pathLen != 72) && (pathLen != 54)) return "";  //52 balancer request, 72 sids request , 54 sms request
       
    //test hex right format data from request
    std::vector<hex2> pathHex2 = protocol_cipher::hex2str_to_bin(pRequestPath);
    if((pathHex2.size() != 26) && (pathHex2.size() != 36) && (pathHex2.size() != 27)) return ""; //check if there was a error trying to convert
    
    //extract data key and cardID
    std::vector<hex2> xorKey{pathHex2.begin() + 18,pathHex2.begin() + 26};
    std::vector<hex2> maskStbData{pathHex2.begin(), pathHex2.begin() + 12};  
    
    //unmask the data
    std::vector<hex2> encStbData = protocol_cipher::unmask_data(maskStbData, xorKey);
    
    
    //===========================================================================VALIDATION OF CLIENT ID==============================================================================================================
    
    uint nodeID = (((static_cast<uint>(encStbData[9]) << 16) & 0xFF0000 ) | ((static_cast<uint>(encStbData[10]) << 8) & 0xFF00) | (static_cast<uint>(encStbData[11]) & 0xFF ));
    
    //test nodeID
    if(nodeID != 0xFFFFFF){
        auto nodeIt = mConfig.ind4Node.find(nodeID);
        if(nodeIt == mConfig.ind4Node.end()) return "";
    }
    
    //test if the byte of the generation is right and extract the 3DES parameters
    hex2 genCard = encStbData[8];
    //decrypt the data
    std::vector<hex2> cardID;
    auto genIt = mCid.mUIDKeys.uidKeysMap.find(genCard);
    if(genIt == mCid.mUIDKeys.uidKeysMap.end()){
        return "";
    }else{
        cardID = protocol_cipher::custom_3des_decrypt({encStbData.begin(), encStbData.begin() + 8},genIt->second.key, genIt->second.iv);
    }
   
    //test batch key
    if(mCid.mBatchKeys.batchKeysSet.find(0xFFFF) == mCid.mBatchKeys.batchKeysSet.end()){  //if 0xFFFF batch key is the batch, it won't be tested 
        hex4 batchKey = ((static_cast<hex4>(cardID[3]) << 8) & 0xFF00 ) | (static_cast<hex4>(cardID[4]) & 0x00FF);
        if(mCid.mBatchKeys.batchKeysSet.find(batchKey) == mCid.mBatchKeys.batchKeysSet.end()) return "";
    }
    
    //test brand-----------------------------------------------
    hex2 brand = cardID[1];
    if(mCid.mBrands.brandsSet.find(brand) == mCid.mBrands.brandsSet.end()) return "";
    
    //test distributor,dicuss the data format here-----------------------------------
    hex2 distID = cardID[2];
    auto distIt = mCid.mDistributors.distRangeMap.find(distID);
    if(distIt == mCid.mDistributors.distRangeMap.end()) return "";
        
  
    mConv.str(std::string()); //improve performance here later
    mConv << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(cardID[0]); //from hex to decimal string
    uint batchNum = stoul(mConv.str()); //Batch # is decimal
    
    //test batch number
    if(batchNum <= distIt->second.batch_range[0] || batchNum >= distIt->second.batch_range[1]) return "";
    
    mConv.str(std::string());
    mConv << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(cardID[5])<< std::setw(2) << static_cast<int>(cardID[6]) << std::setw(2) << static_cast<int>(cardID[7]); //from hex to decimal string 
    uint number = stoul(mConv.str()); //Number is decimal
    
    if(number <= distIt->second.num_range[0] || number >= distIt->second.num_range[1]) return "";
    
    
    //============================================================HERE CLIENT IS A VALID ONE==================================================================================================
    hex2 command = 0x00;
    std::vector<hex2> timeStamp;
    if(pathLen == 54){
        timeStamp.insert(timeStamp.end(),pathHex2.begin() + 12, pathHex2.begin() + 26);
        command = pathHex2[26] ^ xorKey[0];
    }
    
    //get providers from request
    std::vector<hex4> providers;
    if(pathLen == 72){ 
        std::vector<hex2> provReq = protocol_cipher::unmask_data({pathHex2.begin() + 26, pathHex2.end()}, xorKey);
        if(provReq.size() != 10) return "";
        std::vector<hex2>::iterator it = provReq.begin();
        std::vector<hex2>::iterator nextIt;
        while(it != provReq.end()){
            nextIt = it + 1;
            if(*it != 0x00 || *nextIt != 0x00)
                providers.push_back(((static_cast<hex4>(*it) << 8) & 0xFF00 ) | (static_cast<hex4>(*nextIt) & 0x00FF));
            it += 2; 
        }
    }
    
    //request handlers-----------------------------------------------------------
    if(pathLen == 52){ //balancer request
        int index = mBalancer.get_item(nodeID);
        if(index == -1){
            return "n";
        }else{ 
            std::string sResult  = mGoodResponse + mConfig.ind4Node[nodeID].urls[index].substr(7) + '/' + protocol_cipher::bin_to_hex2str(bs_random::vhex2_random(16)) + ']';
            std::vector<unsigned char> vResult{sResult.begin(),sResult.end()};
            return protocol_cipher::bin_to_hex2str(protocol_cipher::unmask_data(vResult,xorKey));
        }
        
    }else if(pathLen == 72){ //sids request
        std::vector<hex2> utf = mBalancer.get_sids(providers,nodeID);
        if(utf.size() == 0) return "n";
        std::vector<hex2> maskedUtf = protocol_cipher::unmask_data(utf,xorKey);
        return protocol_cipher::bin_to_hex2str(maskedUtf);
        
    }else if(pathLen == 54){ //command request
        
        hex2 cmdType = (command >> 4) & 0x0F;
        
        if(cmdType == 0x0C){//message cmd
            std::string msg = mCmdClient.get_msg(cardID, nodeID, command);
            
            if(msg == "n") return "n";
            
            std::vector<hex2> msgChar{msg.begin(), msg.end()};
            std::vector<hex2> maskedMsg = protocol_cipher::unmask_data(msgChar, xorKey);
            std::vector<hex2> hmacSign = protocol_cipher::hmacSha256(key, timeStamp);
            hmacSign.insert(hmacSign.end(), maskedMsg.begin(), maskedMsg.end());
            return protocol_cipher::bin_to_hex2str(hmacSign);
        }
    }
    return "";
}

}