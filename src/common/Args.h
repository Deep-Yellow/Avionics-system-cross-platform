//
// Created by 黄迪 on 2024/4/16.
//

#ifndef REGISTRYCPP_ARGS_H
#define REGISTRYCPP_ARGS_H

#include <utility>

#include "string"
#include "Service.h"



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

/**
 * ResponseBody 部分
 */

struct GetRadarStatusResponse {
public:
    int status;
    std::string message;
};

struct SetRadarStatusResponse {
public:
    int status;
    std::string message;
};

struct FindServiceResponse {
    int status;   // 用于表示服务查找操作的状态码
    Service service;  // 存放找到的服务实例信息

    // 可以添加构造函数来方便地初始化这个结构体
    FindServiceResponse(int stat, Service srv) : status(stat), service(std::move(srv)) {}
};

using ArgVariant = std::variant<GetRadarStatusRequest, SetRadarStatusRequest, FindServiceRequest>;

using RespVariant = std::variant<GetRadarStatusResponse, SetRadarStatusResponse, FindServiceResponse>;


#endif //REGISTRYCPP_ARGS_H
