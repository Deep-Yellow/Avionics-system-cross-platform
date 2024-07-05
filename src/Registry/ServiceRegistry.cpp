// ServiceRegistry.cpp

#include "ServiceRegistry.h"
#include <iostream>
#include <regex>
#include <limits>
#include <cmath>
#include <random>
#include <sstream>
#include <string>
#include <iomanip>
#include <utility>

bool isValidUUID(const std::string &uuid);

double calculateDistance(const LocationInfo &a, const LocationInfo &b);

void printServiceRegistry(const std::map<std::string, std::vector<Service>>& registry);

std::vector<uint8_t> mock_receive_message();
std::vector<uint8_t> serialize_services(const std::vector<Service>& services);
std::vector<Service> deserialize_services(const std::vector<uint8_t>& data);

// Initiation for testing
void ServiceRegistry::initialize(const std::vector<Service>& services) {
    // 添加节点
    Node node1("node1", 30.2741, 120.1551, "Hangzhou");  // 杭州的经纬度
    Node node2("node2", 31.2304, 121.4737, "Shanghai"); // 上海的经纬度
    nodeList[node1.nodeId] = node1;
    nodeList[node2.nodeId] = node2;

    // 将服务添加到注册表
    for (const auto& service : services) {
        registry[service.service_name].push_back(service);
    }

    // 构建Merkle树
    buildMerkleTree();
}


// ----- Business logic code -------

PerformanceMetrics findPerformanceMetrics(const std::string &instance_id);

void ServiceRegistry::registerNode(const Node &node) {
    nodeList[node.nodeId] = node;
}

// instanceId采用UUID，暂定方案是由服务自己生成
Response ServiceRegistry::registerService(const ServiceRegisterRequest &request) {
    if (!isValidUUID(request.instance_id)) {
        return Response(0, Response::STATUS_ERROR, "Illegal instanceId.", RespVariant{});
    } else {
        // 根据ServiceRegisterRequest构造Service对象
        Service newService;
        newService.service_name = request.service_name;
        newService.instance_id = request.instance_id;
        newService.nodeId = request.node_id; // 注意这里的node_id改为nodeId，取决于Service结构的定义
        newService.is_alive = request.is_alive;

        // 添加到注册表中
        registry[request.service_name].push_back(newService);
        return Response(0, Response::STATUS_SUCCESS, "Register Success.", RespVariant{});
    }
}

Response ServiceRegistry::deregisterService(const ServiceDeregisterRequest &request) {
    auto &instances = registry[request.service_name];
    auto originalSize = instances.size(); // 保存原始大小以判断是否有元素被删除

    instances.erase(
            std::remove_if(instances.begin(), instances.end(),
                           [&request](const Service &instance) {
                               return instance.instance_id == request.instance_id;
                           }),
            instances.end());

    if (instances.size() == originalSize) {
        return Response(0, Response::STATUS_NOT_FOUND, "Instance not found.", RespVariant{});
    }

    return Response(0, Response::STATUS_SUCCESS, "Deregister Success.", RespVariant{});
}

// TODO
// 技术点之一 服务的匹配
Response ServiceRegistry::findService(const FindServiceRequest &request) {
    // 解析 service_name 以获取 MethodId
    std::istringstream iss(request.service_name);
    std::string segment;
    std::vector<std::string> seglist;

    while (std::getline(iss, segment, '.')) {
        seglist.push_back(segment);
    }

    if (seglist.size() < 3) {
        return Response(0, Response::STATUS_ERROR, "Invalid service name format.", RespVariant{});
    }

    std::string methodId = seglist[2];  // 获取 MethodId

    // 查找具有相应 MethodId 的服务
    auto it = registry.find(methodId);
    if (it != registry.end() && !it->second.empty()) {
        Service *bestService = nullptr;
        double bestScore = std::numeric_limits<double>::max();

        for (Service &service : it->second) {
            if (!service.is_alive) continue;

            double score;
            if (request.descriptor.mode == 0) { // 地理位置最近
                LocationInfo serviceLocation = findServiceLocation(service.instance_id);
                score = calculateDistance(request.descriptor.location, serviceLocation);
            } else if (request.descriptor.mode == 1) { // 响应时间最短
                PerformanceMetrics metrics = findPerformanceMetrics(service.instance_id);
                score = metrics.responseTime;
            } else {
                continue; // 如果有其他模式，也可以在这里添加处理逻辑
            }

            if (score < bestScore) {
                bestScore = score;
                bestService = &service;
            }
        }

        if (bestService) {
            FindServiceResponse findServiceResponse{Response::STATUS_SUCCESS, *bestService};
            return Response(0, Response::STATUS_SUCCESS, "", findServiceResponse);
        }
    }

    return Response(0, Response::STATUS_NOT_FOUND, "Service not found or no suitable service", RespVariant{});
}

void ServiceRegistry::heartbeat(const HeartBeatRequest &request) {
    for (auto &entry: registry) {
        for (Service &service: entry.second) {
            if (service.instance_id == request.instance_id) {
                service.is_alive = true;  // 更新服务状态为活跃
                break;
            }
        }
    }
}

LocationInfo ServiceRegistry::findServiceLocation(const std::string &instance_id) {
    for (const auto &pair: registry) {
        for (const Service &service: pair.second) {
            if (service.instance_id == instance_id) {
                const Node &node = nodeList[service.nodeId];
                return LocationInfo{node.latitude, node.longitude, ""}; // 使用实际的地区描述
            }
        }
    }
    // 如果找不到服务，返回一个空的 LocationInfo 结构体
    return LocationInfo{0.0, 0.0, "Service not found"};
}

Response ServiceRegistry::handleRequest(const ServiceRegistryRequestContainer &requestContainer) {
    switch (requestContainer.requestType) {
        case ServiceRequestType::RegisterService: {
            auto &req = std::get<ServiceRegisterRequest>(requestContainer.request);
            return registerService(req);
        }
        case ServiceRequestType::DeregisterService: {
            auto &req = std::get<ServiceDeregisterRequest>(requestContainer.request);
            return deregisterService(req);
        }
        case ServiceRequestType::FindService: {
            auto &req = std::get<FindServiceRequest>(requestContainer.request);
            return findService(req);
        }
        case ServiceRequestType::HeartBeat: {
            auto &req = std::get<HeartBeatRequest>(requestContainer.request);
            heartbeat(req);
            return Response(0, Response::STATUS_SUCCESS, "Heartbeat processed.", RespVariant{});
        }
        default:
            return Response(0, Response::STATUS_ERROR, "Unsupported request type.", RespVariant{});
    }
}

ServiceRegistry::ServiceRegistry(std::string  name) : registryName(std::move(name)) {
//    // 创建服务
//    Service service1;
//    service1.service_name = "DataService";
//    service1.instance_id = "service1";
//    service1.nodeId = "node2";
//    service1.is_alive = true;
//
//    Service service2;
//    service2.service_name = "DataService";
//    service2.instance_id = "service2";
//    service2.nodeId = "node1";
//    service2.is_alive = true;
//
//    Service service3;
//    service3.service_name = "AuthenticationService";
//    service3.instance_id = "service3";
//    service3.nodeId = "node2";
//    service3.is_alive = true;
//
//    Service service4;
//    service4.service_name = "LoggingService";
//    service4.instance_id = "service4";
//    service4.nodeId = "node2";
//    service4.is_alive = true;
//
//    Service service5;
//    service5.service_name = "LoggingService";
//    service5.instance_id = "service5";
//    service5.nodeId = "node1";
//    service5.is_alive = true;
//
//    // 将服务添加到向量中
//    std::vector<Service> services = {service1, service2, service3, service4, service5};
//
//    // 使用服务向量初始化注册表
//    initialize(services);
//    syncServiceListOnInit();
}

// ----- 服务状态信息同步

void ServiceRegistry::syncServiceListOnInit() {
    // 从 registry 成员变量中获取服务列表并序列化
    std::vector<Service> my_services;
    for (const auto& [service_name, services] : registry) {
        my_services.insert(my_services.end(), services.begin(), services.end());
    }
    std::vector<uint8_t> serialized_services = serialize_services(my_services);
    sendSerializedServices(serialized_services);
    receiveAndDeserializeServices();
}

void ServiceRegistry::sendSerializedServices(const std::vector<uint8_t> &serialized_services) {
    // 模拟发送序列化后的服务列表
    std::cout << "[" << registryName << "] Sending serialized services..." << std::endl;
}

void ServiceRegistry::receiveAndDeserializeServices() {
    std::vector<uint8_t> received_data = mock_receive_message();
    std::vector<Service> services = deserialize_services(received_data);

    for (const auto& service : services) {
        registry[service.service_name].push_back(service);
    }
}

void ServiceRegistry::buildMerkleTree() {
    // 清空现有的树
    tree = merkle::Tree();

    // 遍历 registry 中的每个服务列表，计算其哈希值并插入到 Merkle 树中
    for (const auto& entry : registry) {
        const std::vector<Service>& services = entry.second;
        if (!services.empty()) {
            // 计算整个 vector<Service> 的哈希值
            std::string hashValue = hashServices(services);
            merkle::Tree::Hash hash(hashValue);
            tree.insert(hash);
        }
    }

    // 计算根哈希
    auto rootHash = tree.root();
    std::cout << "[" << registryName << "] Merkle Tree Root Hash: " << rootHash.to_string() << std::endl;
}

void ServiceRegistry::setServiceList(const std::vector<Service>& services) {
    // 清空现有的注册表
    registry.clear();

    // 将服务添加到注册表
    for (const auto& service : services) {
        registry[service.service_name].push_back(service);
    }

    // 构建Merkle树
    buildMerkleTree();
}

std::vector<std::string> ServiceRegistry::compareAndSyncTree(const std::vector<uint8_t>& byteArray) {
    std::vector<std::string> changedServiceTypes;

    // 反序列化传入的树
    merkle::Tree remoteTree(byteArray);

    // 比较根哈希
    auto localRoot = tree.root();
    auto remoteRoot = remoteTree.root();

    if (localRoot != remoteRoot) {
        std::cout << "[" << registryName << "] Roots are not equal. Synchronizing trees..." << std::endl;

        // 找到不一致的节点索引
        auto inconsistentIndices = tree.findInconsistentLeaves(remoteTree);

        for (auto index : inconsistentIndices) {
            // 获取对应的 map 键值
            auto it = registry.begin();
            std::advance(it, index);
            const std::string& serviceName = it->first;

            // 输出服务名
            std::cout << "[" << registryName << "] Found inconsistent service: " << serviceName << std::endl;

            // 如果需要处理不一致服务的信息，这里可以加入请求服务信息的逻辑
            // const auto& service = it->second.front();
            // if (remoteTree.leaf(index).to_string() != localRoot.to_string()) {
            //     requestServiceInfo(service);
            // }

            // 记录变更的服务类型名字
            changedServiceTypes.push_back(serviceName);
        }
    } else {
        std::cout << "[" << registryName << "] Roots are equal. No synchronization needed." << std::endl;
    }

    return changedServiceTypes;
}

std::vector<uint8_t> ServiceRegistry::serializeServicesForNames(const std::vector<std::string> &serviceNames) {
    std::vector<Service> selectedServices;

    // 遍历传入的服务名列表
    for (const auto& serviceName : serviceNames) {
        // 查找服务名对应的服务列表
        auto it = registry.find(serviceName);
        if (it != registry.end()) {
            // 遍历服务列表，选择满足条件的服务（nodeid 和 registryName 相等）
            for (const auto& service : it->second) {
                if (service.nodeId == registryName) {
                    selectedServices.push_back(service);
                }
            }
        }
    }

    // 序列化选中的服务列表
    return serialize_services(selectedServices);
}

void ServiceRegistry::deserializeAndSetServices(const std::vector<uint8_t> &serializedServices) {
    // 反序列化服务列表
    std::vector<Service> deserializedServices = deserialize_services(serializedServices);

    if(!deserializedServices.empty()){

        // 更新本地的 registry
        for (const auto& service : deserializedServices) {
            // 删除原有相同服务名和节点名的服务
            auto& vec = registry[service.service_name];

            vec.erase(std::remove_if(vec.begin(), vec.end(),
                                     [&service](const Service& s) {
                                         return s.nodeId == service.nodeId;
                                     }),
                      vec.end());

            // 添加新的服务
            registry[service.service_name].push_back(service);
//            printServiceRegistry(registry);
        }

        buildMerkleTree();
        std::cout << "[" << registryName << "] Deserialized and updated local registry." << std::endl;
    }


}

//-----------辅助方法 未来调整一下文件结构
bool isValidUUID(const std::string &uuid) {
    std::regex uuidRegex("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[1-5][0-9a-fA-F]{3}-[89abAB][0-9a-fA-F]{3}-[0-9a-fA-F]{12}$");
    return std::regex_match(uuid, uuidRegex);
}

double calculateDistance(const LocationInfo &a, const LocationInfo &b) {
    // Hypothetical function to calculate geographic distance
    // In real-world scenarios, use appropriate geographic distance calculation
    return std::sqrt(std::pow(a.latitude - b.latitude, 2) + std::pow(a.longitude - b.longitude, 2));
}

// TODO
// 关于 监控 服务运行状态信息需要再设计
// 当前做法：直接生成随机数
PerformanceMetrics findPerformanceMetrics(const std::string &instance_id) {
    // 创建一个随机数生成器
    std::random_device rd;  // 用于获取种子
    std::mt19937 gen(rd()); // 标准 mersenne_twister_engine
    std::uniform_real_distribution<> dis(50.0, 150.0); // 定义在 50.0 到 150.0 毫秒之间的均匀分布

    // 生成随机的响应时间
    double randomResponseTime = dis(gen);

    // 假设其他性能指标，这些也可以是随机生成的或根据实际情况计算得出
    double uptime = 99.9; // 假设正常运行时间百分比
    int throughput = 1000; // 假设吞吐量

    return PerformanceMetrics{randomResponseTime, uptime, throughput};
}

// 自定义哈希函数
std::string to_hex(unsigned char* data, size_t length) {
    std::ostringstream oss;
    for (size_t i = 0; i < length; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return oss.str();
}

std::string improved_hash(const std::string& input) {
    unsigned char hash[32] = {0}; // 使用32字节（256位）的哈希值

    for (size_t i = 0; i < input.size(); ++i) {
        hash[i % 32] ^= input[i];
        hash[i % 32] = (hash[i % 32] >> 1) | (hash[i % 32] << 7); // 右旋转操作
        hash[i % 32] ^= (i * 31); // 增加一个与索引相关的扰动
    }

    return to_hex(hash, 32);
}

std::string ServiceRegistry::hashService(const Service& service) {
    std::ostringstream oss;
    oss << service.service_name
        << service.instance_id
        << service.nodeId
        << service.is_alive;

    std::string input = oss.str();
    return improved_hash(input);
}

std::string ServiceRegistry::hashServices(const std::vector<Service>& services) {
    std::ostringstream combinedHashes;

    for (const auto& service : services) {
        combinedHashes << hashService(service);
    }

    return improved_hash(combinedHashes.str());
}

std::vector<Service> ServiceRegistry::getServiceList() const {
    std::vector<Service> services;
    for (const auto& entry : registry) {
        const std::vector<Service>& serviceList = entry.second;
        for (const Service& service : serviceList) {
            services.push_back(service);
        }
    }
    return services;
}


std::vector<uint8_t> serialize_services(const std::vector<Service>& services) {
    std::vector<uint8_t> result;

    // 序列化服务数量
    uint32_t num_services = services.size();
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&num_services), reinterpret_cast<const uint8_t*>(&num_services) + sizeof(num_services));

    for (const auto& service : services) {
        service.serialize(result);
    }

    return result;
}

// 反序列化 std::vector<Service>
std::vector<Service> deserialize_services(const std::vector<uint8_t>& data) {
    size_t offset = 0;

    uint32_t num_services;
    std::memcpy(&num_services, &data[offset], sizeof(num_services));
    offset += sizeof(num_services);

    std::vector<Service> services;
    services.reserve(num_services);

    for (uint32_t i = 0; i < num_services; ++i) {
        services.push_back(Service::deserialize(data, offset));
    }

    return services;
}

// 模拟接收消息的方法，返回包含服务信息的 std::vector<uint8_t>
std::vector<uint8_t> mock_receive_message() {
    std::vector<Service> services = {
            {"ExampleService1", "12345", "node1", true},
            {"ExampleService2", "67890", "node2", false},
            {"ExampleService3", "54321", "node3", true}
    };

    return serialize_services(services);
}


void printServiceRegistry(const std::map<std::string, std::vector<Service>>& registry) {
    std::cout << "Service Registry Contents:" << std::endl;
    for (const auto& entry : registry) {
        const std::string& serviceType = entry.first;
        const std::vector<Service>& services = entry.second;

        std::cout << "Service Type: " << serviceType << std::endl;
        for (const auto& service : services) {
            std::cout << "  Service Name: " << service.service_name
                      << ", Instance ID: " << service.instance_id
                      << ", Node ID: " << service.nodeId
                      << ", Is Alive: " << (service.is_alive ? "Yes" : "No") << std::endl;
        }
    }
}