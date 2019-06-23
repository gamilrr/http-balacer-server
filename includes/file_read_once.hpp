#ifndef FILE_READ_ONCE_HPP
#define FILE_READ_ONCE_HPP

#include "file_reader.hpp"
#include <string>

namespace balancer_server_module{

class file_read_once : public file_reader 
{
    
public:
    file_read_once(std::string mFilePath);
     
protected:
    std::stringstream mFileData;
    
private:
    virtual void set_parameters() = 0;
};

}
#endif /* FILE_READ_ONCE_HPP */

