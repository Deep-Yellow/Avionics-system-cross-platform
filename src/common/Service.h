//
// Created by 黄迪 on 2024/4/16.
//
#include <string>
#include <utility>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdint>

#ifndef REGISTRYCPP_SERVICEINSTANCE_H
#define REGISTRYCPP_SERVICEINSTANCE_H

/**
 * 一些嵌套的定义
 */

enum class ServiceRequestType {
    FindService,
    RegisterService,
    DeregisterService,
    HeartBeat
};

/**
 * 服务注册、发现相关
 */
struct Node {
    std::string nodeId;  // 节点ID
    double latitude;     // 纬度
    double longitude;    // 经度
    std::string region;  // 区域

    Node() : latitude(0.0), longitude(0.0) {}

    Node(std::string id, double lat, double lon, std::string reg) : nodeId(std::move(id)), latitude(lat),
                                                                    longitude(lon), region(std::move(reg)) {}
};


struct Service {
    std::string service_name; // 服务名称
    std::string instance_id;  // 实例ID unique
    std::string nodeId;       // 服务所在节点ID
    bool is_alive;            // 是否健康活跃

    void serialize(std::vector<uint8_t>& out) const {
        auto serialize_string = [&out](const std::string& str) {
            uint32_t length = str.size();
            out.insert(out.end(), reinterpret_cast<const uint8_t*>(&length), reinterpret_cast<const uint8_t*>(&length) + sizeof(length));
            out.insert(out.end(), str.begin(), str.end());
        };

        serialize_string(service_name);
        serialize_string(instance_id);
        serialize_string(nodeId);
        out.push_back(static_cast<uint8_t>(is_alive));
    }
    static Service deserialize(const std::vector<uint8_t>& in, size_t& offset) {
        auto deserialize_string = [&in, &offset]() {
            uint32_t length;
            std::memcpy(&length, &in[offset], sizeof(length));
            offset += sizeof(length);
            std::string str(in.begin() + offset, in.begin() + offset + length);
            offset += length;
            return str;
        };

        Service service;
        service.service_name = deserialize_string();
        service.instance_id = deserialize_string();
        service.nodeId = deserialize_string();
        service.is_alive = static_cast<bool>(in[offset]);
        offset += sizeof(uint8_t);

        return service;
    }
};

// 定义一个结构体来表示服务的位置信息
struct LocationInfo {
    double latitude;  // 纬度
    double longitude; // 经度
    std::string region; // 区域描述，例如机场代码或城市名
};

// 定义一个结构体来表示服务的性能指标
struct PerformanceMetrics {
    double responseTime; // 响应时间，单位为毫秒
    double uptime;       // 正常运行时间百分比
    int throughput;      // 吞吐量，单位为请求/秒
};

struct ServiceDescriptor {
    int mode;                       // 匹配模式 0-地理位置最相近；1-性能最好
    LocationInfo location;          // 服务的地理位置信息
    PerformanceMetrics performance; // 性能指标

    // 构造函数，初始化服务描述
    ServiceDescriptor(int mode, LocationInfo loc, PerformanceMetrics perf)
            : mode(mode), location(std::move(loc)), performance(perf) {}
};

// 服务描述结构体
struct FindServiceRequest {
    std::string service_name;
    ServiceDescriptor descriptor;

    FindServiceRequest(std::string name, ServiceDescriptor desc)
            : service_name(std::move(name)), descriptor(std::move(desc)) {}
};

struct ServiceRegisterRequest {
    std::string service_name;
    std::string instance_id;
    std::string node_id;
    bool is_alive;
};

struct ServiceDeregisterRequest {
    std::string service_name;
    std::string instance_id;
};

struct HeartBeatRequest {
    std::string instance_id;
};

struct ServiceRegistryRequestContainer {
    ServiceRequestType requestType;  // 用于标识请求的类型
    std::variant<FindServiceRequest, ServiceRegisterRequest, ServiceDeregisterRequest, HeartBeatRequest> request;

    explicit ServiceRegistryRequestContainer(const FindServiceRequest &req)
            : requestType(ServiceRequestType::FindService), request(req) {}

    explicit ServiceRegistryRequestContainer(const ServiceRegisterRequest &req)
            : requestType(ServiceRequestType::RegisterService), request(req) {}

    explicit ServiceRegistryRequestContainer(const ServiceDeregisterRequest &req)
            : requestType(ServiceRequestType::DeregisterService), request(req) {}

    explicit ServiceRegistryRequestContainer(const HeartBeatRequest &req)
            : requestType(ServiceRequestType::HeartBeat), request(req) {}
};

#endif //REGISTRYCPP_SERVICEINSTANCE_H
