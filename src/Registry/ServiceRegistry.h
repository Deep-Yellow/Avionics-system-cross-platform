// ServiceRegistry.h

#ifndef SERVICEREGISTRY_H
#define SERVICEREGISTRY_H

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <sstream>
#include "../common/Service.h"
#include "../common/Request.h"


/// description
/// 1. 初始化服务列表 2. 两个编队的有人机交换服务列表 3. 构建哈希树 4. 服务调用 5. 服务状态更新 6. 无人机增加、退出，导致树结构变化

class ServiceRegistry {
private:
    std::map<std::string, std::vector<Service>> registry;
    std::map<std::string, Node> nodeList;
    void syncServiceListOnInit();
    void sendSerializedServices(const std::vector<uint8_t>& serialized_services);
    void receiveAndDeserializeServices();

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
