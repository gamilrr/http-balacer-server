#include "round_robin_mode.hpp"
#include "epcs_balancer.hpp"
#include <exception>
#include <iostream>
#include <crow.h>

using namespace std;
using namespace balancer_server_module;

int main(int argc, char** argv) {
    
    try{
        round_robin_mode rr{30}; //refresh each 45s
    
        epcs_balancer bal{rr, std::string{"./balancer_server.conf"}}; //balancer
    
        crow::SimpleApp app;
        app.loglevel(crow::LogLevel::Warning);
    
        int port = 12345; //default port
    
        if(argc == 2){
            port = std::stoul(argv[1]);
        }
    
        std::cout <<"balancer process started, running at port: " << port << "\n";
        int y = 0;
        

        CROW_ROUTE(app, "/<string>")
        ([&bal](string path) {
        
            int pathLen = path.size();
            if((pathLen != 72) && (pathLen != 54)) return crow::response(400);
        
            std::ostringstream os;
            os << "<html>\n"
               << "<head>\n"
               << "<title></title>\n"
               << "</head>\n"
               << "<body>\n";
             
            string resp{bal.req_path_analysis(path)};
            if(resp == "") return crow::response(400);
            os << "<p>:::"<< resp <<":::</p>\n"
               << "</body>\n"
               << "</html>";
        
        
            return crow::response(os.str());
        });

        app.bindaddr("127.0.0.1").port(port).multithreaded().run();
        
    }catch(const std::exception& e){
        std::cerr << "\nThis is a global message error: " << e.what() << "\n"; 
    }
    
    return 0;
}


