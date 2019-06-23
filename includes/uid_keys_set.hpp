#ifndef UID_KEYS_SET_HPP
#define UID_KEYS_SET_HPP

#include <string>
#include <map>
#include <vector>
#include "file_read_once.hpp"
#include "bs_types.hpp"

namespace balancer_server_module{

class uid_keys_set : public file_read_once  {
    struct gen_card_key{
        int id;
        std::vector<hex2> key;
        std::vector<hex2> iv;
    };
   
public:
    explicit uid_keys_set(std::string pFilePath);
    
    std::map<int,gen_card_key> uidKeysMap;
               
private:
    void set_parameters() override;
};



}
#endif /* UID_KEYS_SET_HPP */

