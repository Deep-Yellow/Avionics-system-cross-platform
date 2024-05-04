//
// Created by 黄迪 on 2024/4/17.
//

#ifndef REGISTRYCPP_SERVER_H
#define REGISTRYCPP_SERVER_H


#include <unordered_map>
#include "../common/Request.h"

class Server {
private:
    std::unordered_map<std::string, std::function<Response(Request&)>> methods;
    static void sendByDDS(Response& response);

public:
    Server();
    Response handle_getRadarStatus(Request& req);
    Response handle_setRadarStatus(Request& req);
    Response dispatch(Request& req);
    Response processRequest(Request& req);
    [[noreturn]] void receiveFromDDS();
};





#endif //REGISTRYCPP_SERVER_H
