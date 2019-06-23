#ifndef CMD_CLINET_HPP
#define CMD_CLINET_HPP

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "bs_types.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <boost/thread.hpp>
#include <vector>
#include <utility>
#include <map>

namespace balancer_server_module{
   
class cmd_client {
    
     struct msg_coll{ //structure to store message
        
        typedef std::map<hex2,std::string> Msg; //<type of msg, msg in string>
        typedef std::vector<hex2> CardId;
        typedef std::map<uint, Msg> ByNodeMsg;
        typedef std::map<CardId,Msg> ByCardIdMsg;
        typedef std::pair<CardId, CardId> CardIdRange;
        typedef std::map<CardIdRange,Msg> ByCardIdRangeMsg;
        
        ByNodeMsg nodeIdMsg; //node collection
        ByCardIdMsg cardIdMsg; //cardId collection
        ByCardIdRangeMsg cardIdRangeMsg; //cardId range collection
        
        void clear(){
            nodeIdMsg.clear();
            cardIdMsg.clear();
            cardIdRangeMsg.clear();
        }
    };
    
public:
    cmd_client();
    ~cmd_client();
    
    //this function connect to config service automatically, if return 0 something went wrong 
    int start(std::string host, int pPort);
    
    //this function will stop the socket and process to get cmd
    void stop();
    
    //get the command payload like a string depending of cmd
    std::string get_msg(const std::vector<hex2>& cardId, uint NodeId, hex2 type);
    
private:
    msg_coll msg_db; //collection to get message
    
    void cmd_analyzer(std::string& cmd);
    void cmd_msg(rapidjson::Document& dom);
    
   
    //connection part
    std::unique_ptr<boost::asio::io_service> mpIO;
    std::unique_ptr<boost::asio::ip::tcp::socket> mpSocket;
    std::unique_ptr<boost::thread> mpIOSth; 
    boost::asio::streambuf mInBuffer; //input buffer
    void cmd_read_handle(const boost::system::error_code& error, std::size_t size);
    void reload();
};
}
#endif /* CMD_CLINET_HPP */

