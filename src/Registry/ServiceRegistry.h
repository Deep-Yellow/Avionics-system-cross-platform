// ServiceRegistry.h

#ifndef SERVICEREGISTRY_H
#define SERVICEREGISTRY_H

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <sstream>
#include "merklecpp.h"
#include "../common/Service.h"
#include "../common/Request.h"


/// description
/// 1. 初始化服务列表 2. 两个编队的有人机交换服务列表 3. 构建哈希树 4. 服务调用 5. 服务状态更新 6. 无人机增加、退出，导致树结构变化

class ServiceRegistry {
private:
    std::string registryName; // 新增的成员变量，用于存储注册表的名字
    std::map<std::string, std::vector<Service>> registry;
    std::map<std::string, Node> nodeList;

    void syncServiceListOnInit();
    void receiveAndDeserializeServices();
    void buildMerkleTree(); // 新增的构建Merkle树的方法

public:

    merkle::Tree tree;

    ServiceRegistry(std::string  name);

    void registerNode(const Node &node);

    Response registerService(const ServiceRegisterRequest &request);

    Response deregisterService(const ServiceDeregisterRequest &request);

    Response findService(const FindServiceRequest &request);


    // 入口函数，在DDS的回调中被调用
    Response handleRequest(const ServiceRegistryRequestContainer &requestContainer);

    void heartbeat(const HeartBeatRequest &request);

    void sendSerializedServices(const std::vector<uint8_t>& serialized_services);

    LocationInfo findServiceLocation(const std::string &instance_id);

    std:: string hashService(const Service& service);

    std::string hashServices(const std::vector<Service>& services); // 新增hashServices方法

    void setServiceList(const std::vector<Service>& services); // 新增设置服务列表的方法

    std::vector<std::string> compareAndSyncTree(const std::vector<uint8_t>& byteArray); // 新增比较并同步树的方法

    std::vector<uint8_t> serializeServicesForNames(const std::vector<std::string>& serviceNames);

    void deserializeAndSetServices(const std::vector<uint8_t>& serializedServices);

    void initialize(const std::vector<Service>& services);

    std::vector<Service> getServiceList() const;
};


#endif // SERVICEREGISTRY_H
