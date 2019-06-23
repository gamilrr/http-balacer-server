#include "bs_random.hpp"
#include <random>
namespace balancer_server_module{
    
 hex2 bs_random::hex2_random(){
    static std::random_device rd;
    static std::mt19937 gen{rd()};
    std::uniform_int_distribution<> dis(0, 0xFF);
    return dis(gen);
}
 
 
  std::vector<hex2> bs_random::vhex2_random(int len){
      std::vector<hex2> result;
      for(int i = 0; i != len;i++)
            result.push_back(hex2_random());
      
      return result;
  }
 

}