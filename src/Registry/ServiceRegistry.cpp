// ServiceRegistry.cpp

#include "ServiceRegistry.h"
#include <iostream>
#include <regex>
#include <limits>
#include <cmath>
#include <random>

bool isValidUUID(const std::string &uuid);

double calculateDistance(const LocationInfo &a, const LocationInfo &b);

// Initiation for testing
void ServiceRegistry::initialize() {
    // 添加节点
    Node node1("node1", 30.2741, 120.1551, "Hangzhou");  // 杭州的经纬度
    Node node2("node2", 31.2304, 121.4737, "Shanghai"); // 上海的经纬度
    nodeList[node1.nodeId] = node1;
    nodeList[node2.nodeId] = node2;

    // 添加服务
    Service service1;
    service1.service_name = "DataService";
    service1.instance_id = "service1";
    service1.nodeId = "node2";
    service1.is_alive = true;

    Service service2;
    service2.service_name = "DataService";
    service2.instance_id = "service2";
    service2.nodeId = "node1";
    service2.is_alive = true;

    Service service3;
    service3.service_name = "AuthenticationService";
    service3.instance_id = "service3";
    service3.nodeId = "node2";
    service3.is_alive = true;

    Service service4;
    service4.service_name = "LoggingService";
    service4.instance_id = "service4";
    service4.nodeId = "node2";
    service4.is_alive = true;

    Service service5;
    service5.service_name = "LoggingService";
    service5.instance_id = "service5";
    service5.nodeId = "node1";
    service5.is_alive = true;

    // 将服务添加到注册表
    registry[service1.service_name].push_back(service1);
    registry[service2.service_name].push_back(service2);
    registry[service3.service_name].push_back(service3);
    registry[service4.service_name].push_back(service4);
    registry[service5.service_name].push_back(service5);
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

ServiceRegistry::ServiceRegistry() {
    initialize();
}


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