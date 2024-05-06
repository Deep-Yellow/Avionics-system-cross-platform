// ServiceRegistry.h

#ifndef SERVICEREGISTRY_H
#define SERVICEREGISTRY_H

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include "../common/Service.h"
#include "../common/Request.h"


// TODO list
// 1. 超过30s没有心跳的将服务设置为不可用 超过60s的删除该服务
// 2. 设置一个入口函数，根据请求参数的类型（ServiceRegisterRequest、ServiceDeregisterRequest等）调用对应的的方法
// 3.

class ServiceRegistry {
private:
    std::map<std::string, std::vector<Service>> registry;
    std::map<std::string, Node> nodeList;

public:
    ServiceRegistry();

    void registerNode(const Node &node);

    Response registerService(const ServiceRegisterRequest &request);

    Response deregisterService(const ServiceDeregisterRequest &request);

    Response findService(const FindServiceRequest &request);

    // 入口函数，在DDS的回调中被调用
    Response handleRequest(const ServiceRegistryRequestContainer &requestContainer);

    void heartbeat(const HeartBeatRequest &request);

    LocationInfo findServiceLocation(const std::string &instance_id);


    void initialize();
};


#endif // SERVICEREGISTRY_H
