#ifndef ROUND_ROBIN_MODE_H
#define ROUND_ROBIN_MODE_H

#include "balancer_mode.hpp"
#include "bs_types.hpp"
#include <boost/thread.hpp>
#include <vector>
#include <string>
#include <memory>
#include <atomic>

namespace balancer_server_module{

class round_robin_mode : public balancer_mode{

public:
    
    explicit round_robin_mode(stime pRefreshTime);
    
    virtual void start() override;
    virtual void stop() override;
    
    virtual int get_item(uint nodeID) override; 
    
    virtual void set_param(const std::map<uint,node_id>& param) override ;
    
    virtual std::vector<hex2> get_sids(const std::vector<hex4>& provID, uint nodeID) override;
    
    
    bool was_runned(){return mThreadFlag;}
    
    virtual ~round_robin_mode();
    
private:
    std::map<uint,node_id> mMapItem;
    std::map<uint, int> mItem;  //assuming that primitive int is atomic without using atomic_int
    stime mRefreshTime;
    
    //threads parameters
    std::unique_ptr<boost::thread>  mRRThread;
    boost::mutex mMutex;
    boost::condition_variable_any mReadIt;
    bool mRunFlag;
    bool mThreadFlag;
    void round_robin_thread();
    
   std::map<uint,std::map<hex4, std::vector<char>>>  providers;
    
   
    
    std::string make_request(const std::string& server, const std::string& uri, const std::string& credentials,int timeout);
};
}
#endif /* ROUND_ROBIN_MODE_H */

