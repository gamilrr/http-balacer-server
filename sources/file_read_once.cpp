#include "file_read_once.hpp"

namespace balancer_server_module{
    
file_read_once::file_read_once(std::string mFilePath):file_reader(mFilePath), mFileData{} {
    start_reading();
    stop_reading();
    if(was_cache_modified()){
        mFileData.str(get_file_data());
    }else{
        throw std::runtime_error("ERROR something went wrong opening the configuration file");
    }
}


}