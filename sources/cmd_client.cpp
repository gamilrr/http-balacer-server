#include "protocol_cipher.hpp"
#include "cmd_client.hpp"
#include <stdexcept>
#include <boost/bind.hpp>
#include <sstream>
#include <algorithm>
#include <iostream>


namespace balancer_server_module{

    using namespace rapidjson;
    
   cmd_client::cmd_client()
   :msg_db{}, 
   mInBuffer{},
   mpIOSth{nullptr}, 
   mpIO{std::make_unique<boost::asio::io_service>()}{
       mpSocket = std::make_unique<boost::asio::ip::tcp::socket>(*mpIO);
   }
   
   cmd_client::~cmd_client(){
       stop();
   }
    
   int cmd_client::start(std::string host, int pPort){
       boost::system::error_code error;
       mpSocket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), pPort), error);
       if(error){ 
           stop();  std::cerr<<"Command Service is not running " << "ERROR: "<<error.message()<<"\n"; 
           return 0;
       }
       reload();
       mpIOSth = std::make_unique<boost::thread>([&]{mpIO->run();});
       return 1;
   }
   
   void cmd_client::reload(){
       boost::asio::async_read_until(*mpSocket,mInBuffer,'|',boost::bind(&cmd_client::cmd_read_handle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        
   }
   
   void cmd_client::stop(){
       mpSocket->close();
       mpIO->stop();
       if(mpIOSth != nullptr)
          mpIOSth->join();     
   }
   
   void cmd_client::cmd_read_handle(const boost::system::error_code& error, std::size_t size){
       if(error){
           //do something if an error issued
       }
       std::ostringstream ss;
       ss << (&mInBuffer);
       std::string cmdData{(ss.str()).substr(0,size - 1)}; 
       
       cmd_analyzer(cmdData);
       reload(); //reload async call
   }
   
   std::string cmd_client::get_msg(const std::vector<hex2>& cardID, uint nodeID, hex2 msgType){
       
       auto cardIt = msg_db.cardIdMsg.find(cardID);
       if(cardIt != msg_db.cardIdMsg.end()){ //look for cardID
           auto typeIt =  cardIt->second.find(msgType); //look for msgType
           if(typeIt != cardIt->second.end()){
               return typeIt->second;
           }
       }
       
       auto cardIdRangeIt = std::find_if(msg_db.cardIdRangeMsg.begin(), msg_db.cardIdRangeMsg.end(),
                            [&cardID](const std::pair<msg_coll::CardIdRange, msg_coll::Msg>& item){
                                return (item.first.first < cardID && cardID < item.first.second);
                            });
                            
       if(cardIdRangeIt != msg_db.cardIdRangeMsg.end()){
           auto typeIt =  cardIdRangeIt->second.find(msgType); //look for msgType
           if(typeIt != cardIdRangeIt->second.end()){
               return typeIt->second;
           }
       }
       
       auto nodeIt = msg_db.nodeIdMsg.find(nodeID);
       if(nodeIt != msg_db.nodeIdMsg.end()){
           auto typeIt =  nodeIt->second.find(msgType); //look for msgType
           if(typeIt != nodeIt->second.end()){
               return typeIt->second;
           }
       }
       
       nodeIt = msg_db.nodeIdMsg.find(0);
       if(nodeIt != msg_db.nodeIdMsg.end()){
           auto typeIt =  nodeIt->second.find(msgType); //look for msgType
           if(typeIt != nodeIt->second.end()){
               return typeIt->second;
           }
       }
       
       return "n";
   }
   
   
   //command analyzer==============================================================
   void cmd_client::cmd_analyzer(std::string& cmdData){
       msg_db.clear();
       Document dom;
       
       dom.Parse(cmdData.data());
       
       if(!dom.IsObject()) return;
       
       //validation-------------------- 
       if(!dom.HasMember("cmd")) return;
       if(!(dom["cmd"].IsString())) return;
       
       Value& cmd = dom["cmd"];
       std::string cmdType = cmd.GetString();
       
       //insert here news cmd
       if(cmdType == "msg"){
           return cmd_msg(dom);
       }
   }
   
   void cmd_client::cmd_msg(Document& dom){
       
       if(!dom.HasMember("payload")) return;
       
       const Value& payload = dom["payload"];
       
       if(!payload.IsArray()) return;
       
        for(auto& item : payload.GetArray()){
            if(!item.HasMember("nodeId") ||!item.HasMember("cardIds")|| !item.HasMember("cardIde") || !item.HasMember("title") || !item.HasMember("content") || !item.HasMember("type") || !item.HasMember("ttl")) continue;
           
            if(!item["title"].IsString() || !item["content"].IsString() || !item["type"].IsNumber() || !item["ttl"].IsNumber()) continue;
           
           
            //make the message
            std::string msg(std::string(item["title"].GetString()) + '|' + std::string(item["content"].GetString()) + '|' + std::to_string(item["ttl"].GetInt()));
            hex2 msgType = item["type"].GetInt();
           
            if(!(item["cardIde"].IsNull())){ //that means cardId range type of message
                std::string sCardIds = item["cardIds"].GetString();
                std::vector<hex2> vCardIds = protocol_cipher::hex2str_to_bin(sCardIds);
                if(vCardIds.size() != 8) continue; //minimal validation, get better here after
                
                std::string sCardIde = item["cardIde"].GetString();
                std::vector<hex2> vCardIde = protocol_cipher::hex2str_to_bin(sCardIde);
                if(vCardIde.size() != 8) continue; //minimal validation, get better here after
                
                if(vCardIde < vCardIds) continue;
                
                if(vCardIde == vCardIds){ 
                    msg_db.cardIdMsg[vCardIds][msgType] = msg;
                }
                
                if(vCardIde > vCardIds){
                    msg_db.cardIdRangeMsg[std::make_pair(vCardIds, vCardIde)][msgType] = msg;
                }
                
            }else if(!(item["cardIds"].IsNull())){
               std::string sCardId = item["cardIds"].GetString();
               std::vector<hex2> vCardId = protocol_cipher::hex2str_to_bin(sCardId);
               if(vCardId.size() != 8) continue; //minimal validation, get better here after
               
               msg_db.cardIdMsg[vCardId][msgType] = msg;
       
            }else if(!(item["nodeId"].IsNull())){
               std::string sNodeId = item["nodeId"].GetString();
               std::vector<hex2> vNodeId = protocol_cipher::hex2str_to_bin(sNodeId);
               if(vNodeId.size() != 3) continue; //means bad format in nodeID string
                       
               uint nodeID = (((static_cast<uint>(vNodeId[0]) << 16) & 0xFF0000 ) | ((static_cast<uint>(vNodeId[1]) << 8) & 0xFF00) | (static_cast<uint>(vNodeId[2]) & 0xFF ));
                
               msg_db.nodeIdMsg[nodeID][msgType] = msg;
               
            }
               
        }
    }
}
