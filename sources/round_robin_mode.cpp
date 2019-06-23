#include "protocol_cipher.hpp"
#include "round_robin_mode.hpp"
#include <boost/chrono.hpp>
#include <iterator>
#include <sstream>
#include <map>
#include <string>
#include <set>
#include <algorithm>
#include <iostream>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPSession.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPBasicCredentials.h>
#include <Poco/StreamCopier.h>
#include <Poco/Exception.h>
#include <Poco/URI.h>


namespace balancer_server_module {

    round_robin_mode::round_robin_mode(stime pRefreshTime) : mMapItem{}, mItem{}, mRefreshTime{pRefreshTime}, mRRThread{}, mRunFlag{false}, mThreadFlag{false}, mMutex{}, providers{}, mReadIt{}
    {};

    round_robin_mode::~round_robin_mode() {
        stop();
    }

    
    void round_robin_mode::set_param(const std::map<uint,node_id>& param ){
        mMapItem = param;
        for(auto it = param.begin(); it != param.end(); it++){
            mItem[it->first] = -1;
        }
    }
    
    
    void round_robin_mode::start() {
        boost::unique_lock<boost::mutex> lock{mMutex};
        if (mRunFlag == false) {
            mThreadFlag = true;
            mRRThread = std::make_unique<boost::thread>(&round_robin_mode::round_robin_thread, this);
              mReadIt.wait(mMutex); //wait for one read
        }

    }

    void round_robin_mode::stop() {
        if (mRunFlag == true) {
            mRunFlag = false;
            mThreadFlag = false;
            if (mRRThread->joinable())
               mRRThread->join(); //wait until the thread end and clean it up

        }
    }

    int round_robin_mode::get_item(uint nodeID) {
        return mItem[nodeID];
    }

    std::vector<hex2> round_robin_mode::get_sids(const std::vector<hex4>& provID, uint nodeID) {
       
        std::vector<hex2> result;
        std::map<hex4, std::vector<char>>::iterator it;
     
        for(auto& prov : provID){
            mMutex.lock(); //optimize here later 
             it = providers[nodeID].find(prov);
             if(it != providers[nodeID].end()){
                 result.insert(result.end(), it->second.begin(),it->second.end());
             }
            mMutex.unlock();
        }
        
        
        return result; //move
    }

    void round_robin_mode::round_robin_thread() { //url:50.7.65.50:16384/cache-listening.fssp?refresh=none&server=sv1"

        
        const std::string credentials = "root:root@tncam";
        const std::string uriPath = "/cache-listening.fssp?refresh=none&server=sv1";
        std::string rep{};
        const int timeout = 5; //timeout in seconds
        
        
        int percent = 0;
        int percentCheck = 0;
        int indexChecker = -1;
        const int upperUmbral = 85;
        
        //iterator of position to run over html string
        std::string::size_type posItb = 0;
        std::string::size_type posIte = 0;
        std::string::size_type posItn = 0;
        
        bool sidFlag = false; //this flag will be true when at least is got a provider
        std::map<uint,std::map<hex4, std::set<uint>>> provCache{}; //map for load the provider and its sids
        
        std::map<uint,std::map<hex4, std::vector<char>>> provOut{};
        
        const uint maxRefresh = 45; //second max to admit one sids
         
        mRunFlag = true; //change to atomic type to avoid compiler optimization
        while (mRunFlag) {
            mReadIt.notify_one();
            
            for(auto it = mMapItem.begin(); it != mMapItem.end(); it++){
                percent = upperUmbral;
                indexChecker = -1;
            for (int index = 0; index != (it->second.ips).size(); index++) {
                
                rep = make_request(it->second.ips[index], uriPath, credentials, timeout);
                int size = rep.size();
                if (size == 0) continue;

                //process the cpu % in response 
                posItb = rep.find("CPU Usage:");
                if (posItb == rep.npos) continue;

                posIte = rep.find("%", posItb += 10);
                if (posIte > posItb + 5) continue;

                percentCheck = std::stoi(rep.substr(posItb, posIte - posItb));
                    
                if(percentCheck < percent){
                    percent = percentCheck;
                    indexChecker = index;
                }
                //cpuMap[percent] = index;
               
                //get providers sid
                if (sidFlag == false) {
                    int cacheData = 0;
                    hex4 provID = 0;
                    posItn = 0;
                    posIte = 0;
                    posItb = 0;
                    int sid = 0;
                    std::string sidstr;
                    std::string sids;
                    std::string refresh;
                    
                    posItb = rep.find("</html>"); //check if the page was downloaded completely
                    if(posItb != rep.npos)
                        provCache[it->first].clear();
                    
                    try{ //highly sensitive code
                    while((posItb = rep.find("Cache [", posIte)) != rep.npos){
                        provID = 0;
                        
                        //check cache
                        posIte = rep.find("]", posItb += 7);
                        if (posIte > (posItb + 6)) break;
                        cacheData = std::stoi(rep.substr(posItb,posIte - posItb));
                       
                        //check provider ID
                        if(posItb < 15) break; 
                        posIte = posItb - 7;
                        posItb -= 14;
                        provID = std::stoi(rep.substr(posItb, posIte - posItb),0,16); //get provider ID in hex of 4 digit(2 byte)
                        
                        posItn = rep.find("Cache [", posIte + 7);
                        
                        if(cacheData == 0){ 
                            posIte = posItn;
                            provCache[it->first][provID].insert(0);
                            //continue; //the provider is not supported check the next
                        }
                        
                        //get sids
                        if(cacheData != 0) //if cacheData is 0 set unsupported provider flag "0:|"
                        while((posItb = rep.find("<b>", posIte)) < posItn){
                            posIte = rep.find("</b>", posItb += 3);
                            if (posIte > (posItb + 7)) break;
                            sidstr = rep.substr(posItb,posIte - posItb);
                            sid = std::stoi(sidstr);
                            
                            //check refresh
                            posItb = rep.find("<td width=\"40\">",posIte);
                            posIte = rep.find("</td>", posItb += 15);
                            if(posIte > posItb + 15) break;
                            if(posIte == posItb) continue;
                            refresh = rep.substr(posItb,posIte - posItb);
                            std::replace(refresh.begin(), refresh.end(), ':', '0');
                            int ref = std::stoul(refresh);
                            
                            
                            //delete the sid if refresh is greater than 45s

							auto sidIt = provCache[it->first][provID].find(sid);
                            
                            if(sidIt != provCache[it->first][provID].end()){
                                if(ref > maxRefresh) 
                                    provCache[it->first][provID].erase(sid);//if(std::stoul(refresh) > maxRefresh)
                            }else{
                                if(ref < maxRefresh)
                                    provCache[it->first][provID].insert(sid);
                            }
                                
                            sidFlag = true; //was loaded at least one sid
                        }
                        
                        if(provCache[it->first][provID].size() != 0){
                            std::string sidString = "|";
                            for(auto& p : provCache[it->first][provID]){
                                sidString += (std::to_string(p) + ":");
                            }      
                            
                            provOut[it->first][provID] = std::vector<char>{sidString.begin(),sidString.end()};
                        }   
                    }
                    }catch(...){ sidFlag = false; }//just to be sure
                }

                
                }
                if (indexChecker != -1) {
                     mItem[it->first] = indexChecker;
                } else {
                     mItem[it->first] = -1;
                }
                
                
                sidFlag = false;
               
            }
            
            mMutex.lock();
            //if(provOut.size() != 0){
            providers = std::move(provOut); //move
            //}
            mMutex.unlock();
            
          
            
            provOut.clear(); //just to be sure
            
            
            mThreadFlag = true;
            boost::this_thread::sleep_for(boost::chrono::seconds{mRefreshTime});
        }



    }

    std::string round_robin_mode::make_request(const std::string& server,const std::string& uri, const  std::string& credent, int timeout) {
        /*
        std::ostringstream os{};

        try {
            curlpp::Cleanup myCleanup;
            curlpp::Easy req;

            req.setOpt(curlpp::options::Url(url));
            req.setOpt(curlpp::options::Timeout(timeout));
            req.setOpt(new curlpp::options::UserPwd(credent));
            //req.setOpt(curlpp::options::WriteStream(&os));
            os << req;

        } catch (curlpp::RuntimeError& e) {
            std::cerr << e.what() << std::endl;
            return "";
        }

        return os.str(); */
		try {

			/*
			std::string::size_type portPost = server.find(':');

			std::string serverIP = server.substr(0, portPost);
			std::string serverPort = server.substr(portPost + 1, server.length());
			*/
			
			Poco::URI url{"http://" + server + uri };

			Poco::Net::HTTPClientSession session( url.getHost(), url.getPort() );
			std::string path{ url.getPathAndQuery() };
			session.setKeepAlive(true);

			Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
			
			//const std::string credentials = "root:root@tncam";
			
			Poco::Net::HTTPBasicCredentials creds("root", "root@tncam");
			creds.authenticate(req);

			req.setContentType("application/x-www-form-urlencoded");
			req.setKeepAlive(true); // notice setKeepAlive is also called on session (above)

			session.setTimeout(Poco::Timespan(timeout, 0));
			session.sendRequest(req); // sends request, returns open stream

			Poco::Net::HTTPResponse res;
			std::istream& iStr = session.receiveResponse(res);  // get the response from server

			std::string body;
			Poco::StreamCopier::copyToString(iStr, body);

			std::cout << body;

			return body;

		}
		catch (Poco::Exception& e) {
			std::cout << e.displayText() << '\n';
			return {};
		}
		
    }


}

