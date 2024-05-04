// ServiceRegistry.h

#ifndef SERVICEREGISTRY_H
#define SERVICEREGISTRY_H

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include "../common/Service.h"
#include "../common/Request.h"


class ServiceRegistry {
private:
    std::map<std::string, std::vector<Service>> registry;
    std::map<std::string, Node> nodeList;


public:
    void registerNode(const Node& node);
    Response registerService(const ServiceRegisterRequest& request);
    Response deregisterService(const ServiceDeregisterRequest& request);
    Response findService(const FindServiceRequest& request);

    void heartbeat(const HeartBeatRequest& request);

    LocationInfo findServiceLocation(const std::string &instance_id);
};




#endif // SERVICEREGISTRY_H
