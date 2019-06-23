#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <memory>
#include "bs_types.hpp"

namespace balancer_server_module{
    
/*this object will read and refresh any file into ram, just use it for small files*/
class file_reader {
    
public:
    
    file_reader(std::string pFilePath, stime pRefreshTime = 0);
    
    file_reader(const file_reader& ) = delete;         //non-copyable policy objects
    file_reader& operator=(const file_reader& ) = delete;
    
    file_reader(file_reader &&) = default;
    file_reader& operator= (file_reader&&) = default; //can be moved
    
    /*start the reading process*/
    void start_reading();
  
    /*stop the reading process*/
    void stop_reading();
    
    
    /*this function return the value of the parameter using the format "parameter = value" inside config file*/
    std::string get_file_data(); //access by lambda notation or functor
    
    bool was_cache_modified(){return mCacheModified;}
    
    virtual ~file_reader();


private:
    
    std::unique_ptr<boost::thread> mpRefreshThread; //thread handler
    boost::shared_mutex mSharedMutex;
    boost::condition_variable_any mReadIt;
    
    bool mReadingFlag; //flag to stop or start the config file reading thread
    bool mCacheModified; //flag to indicate that the file was modified
    stime mRefreshTime; //time in second to test if the file was modified
    
    const std::string mFilePath;//config file path
    std::ifstream mFileHandler;//file handler 
    std::stringstream mFileCache; //store the configuration file
    
    /*thread to access file*/
    void refresh_config_thread(); 
};

}

#endif /* CONFIG_RULER_HPP */

