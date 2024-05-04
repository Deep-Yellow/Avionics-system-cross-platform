//
// Created by 黄迪 on 2024/4/16.
//

#ifndef REGISTRYCPP_ARGS_H
#define REGISTRYCPP_ARGS_H

#include <utility>

#include "string"
#include "Service.h"

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
 * RequestBody部分
 */

struct GetRadarStatusRequest {
    std::string from_service;
    std::string to_service;
};

struct SetRadarStatusRequest {
    int a;
    int b;
};

// 服务描述结构体
struct FindServiceRequest {
    std::string service_name;
    ServiceDescriptor descriptor;
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

    ServiceRegistryRequestContainer() = default;

    explicit ServiceRegistryRequestContainer(const FindServiceRequest& req)
            : requestType(ServiceRequestType::FindService), request(req) {}

    explicit ServiceRegistryRequestContainer(const ServiceRegisterRequest& req)
            : requestType(ServiceRequestType::RegisterService), request(req) {}

    explicit ServiceRegistryRequestContainer(const ServiceDeregisterRequest& req)
            : requestType(ServiceRequestType::DeregisterService), request(req) {}

    explicit ServiceRegistryRequestContainer(const HeartBeatRequest& req)
            : requestType(ServiceRequestType::HeartBeat), request(req) {}
};

/**
 * ResponseBody 部分
 */

struct GetRadarStatusResponse{
public:
    int status;
    std::string message;
};

struct SetRadarStatusResponse{
public:
    int status;
    std::string message;
};

struct FindServiceResponse {
    int status;   // 用于表示服务查找操作的状态码
    Service service;  // 存放找到的服务实例信息

    // 可以添加构造函数来方便地初始化这个结构体
    FindServiceResponse(int stat, Service  srv) : status(stat), service(std::move(srv)) {}
};

using ArgVariant = std::variant<GetRadarStatusRequest, SetRadarStatusRequest, FindServiceRequest>;

using RespVariant = std::variant<GetRadarStatusResponse, SetRadarStatusResponse, FindServiceResponse>;






#endif //REGISTRYCPP_ARGS_H
