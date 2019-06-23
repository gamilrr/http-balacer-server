#include "file_reader.hpp"
#include <exception>
#include <memory>
#include <fstream>
#include <iterator>
#include <boost/filesystem/operations.hpp>
#include <boost/chrono.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include <iostream>
//#include <unistd.h>
#include <cstdlib>

namespace balancer_server_module{
          
file_reader::file_reader(std::string pFilePath, stime pRefreshTime)
:mFilePath{pFilePath},
mFileHandler{},
mpRefreshThread{}, //thread handler
mFileCache{},      //store file in ram
mReadingFlag{false},
mCacheModified{false},
mRefreshTime{pRefreshTime},
mSharedMutex{},
mReadIt{}
{}


file_reader::~file_reader(){
    stop_reading();
}

void file_reader::start_reading(){
     boost::unique_lock<boost::shared_mutex> lock{mSharedMutex};
    if(mReadingFlag == false){ //avoid undefined behavior by calling this function several time 
        mpRefreshThread = std::make_unique<boost::thread>(&file_reader::refresh_config_thread,this);
        mReadIt.wait(mSharedMutex); //wait for one read
    }
}

void file_reader::stop_reading(){
    
    if(mReadingFlag == true){
        mReadingFlag = false;
    
        if(mpRefreshThread->joinable())
            mpRefreshThread->join(); //wait until the thread end and clean it up
    }
}


std::string file_reader::get_file_data(){
    
    mCacheModified = false;
    boost::shared_lock<boost::shared_mutex> lock{mSharedMutex};
    return mFileCache.str();
}

void file_reader::refresh_config_thread(){
    
    static std::time_t sLastFileMod = 0;
    mReadingFlag = true;
    
    while(mReadingFlag){
         
        if(boost::filesystem::exists(mFilePath)){
            mFileHandler.open(mFilePath);
      
            //test if there was not any error opening the file
            if ( (mFileHandler.rdstate() & std::ifstream::failbit ) != 0 ){
                std::cerr << "ERROR opening 'balancer_server.conf'\n";
                mFileHandler.clear();
                if(mFileHandler.is_open())mFileHandler.close();
                boost::this_thread::sleep_for(boost::chrono::seconds{mRefreshTime});
                continue;
            } 
        }else{
            std::cout<<"ERROR balancer_server.conf does not exist"<<std::endl;
        }
        
       
        if(sLastFileMod == boost::filesystem::last_write_time(mFilePath)){ //if has not been modified don't do anything
            if(mFileHandler.is_open())mFileHandler.close();
            boost::this_thread::sleep_for(boost::chrono::seconds{mRefreshTime}); 
            continue;
        }
        
       
        sLastFileMod = boost::filesystem::last_write_time(mFilePath);
        
        
        boost::unique_lock<boost::shared_mutex> lock{mSharedMutex};
        mFileCache.str(std::string());
        mFileCache << mFileHandler.rdbuf();
        lock.unlock();
        
        
        
        mFileHandler.close();
        
        mCacheModified = true;
        
        
		mReadIt.notify_one();

        boost::this_thread::sleep_for(boost::chrono::seconds{mRefreshTime});
        
    }
    
    if(mFileHandler.is_open()) //just to be sure
        mFileHandler.close();
    
}

}