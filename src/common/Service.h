//
// Created by 黄迪 on 2024/4/16.
//
#include <string>
#include <utility>

#ifndef REGISTRYCPP_SERVICEINSTANCE_H
#define REGISTRYCPP_SERVICEINSTANCE_H

/**
 * 服务注册、发现相关
 */
struct Node {
    std::string nodeId;  // 节点ID
    double latitude;     // 纬度
    double longitude;    // 经度
    std::string region;  // 区域

    Node() : latitude(0.0), longitude(0.0) {}
    Node(std::string id, double lat, double lon) : nodeId(std::move(id)), latitude(lat), longitude(lon) {}
};


struct Service {
    std::string service_name; // 服务名称
    std::string instance_id;  // 实例ID unique
    std::string nodeId;       // 服务所在节点ID
    bool is_alive;            // 是否健康活跃
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




#endif //REGISTRYCPP_SERVICEINSTANCE_H
