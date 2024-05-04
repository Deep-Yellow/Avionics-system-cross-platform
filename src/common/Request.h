//
// Created by 黄迪 on 2024/4/15.
//

#ifndef REGISTRYCPP_REQUEST_H
#define REGISTRYCPP_REQUEST_H

#include "Request.h"
#include "Args.h"
#include <string>
#include <map>
#include <cstdint>
#include <typeindex>
#include <typeinfo>
#include <memory>
#include <functional>
#include <iostream>
#include <utility>
#include <future>

/**
 * TODO
 * ResponseHeader的设计
 */

class MethodType {
public:
    std::type_index argType;
    std::type_index replyType;
    uint64_t numCalls = 0;

    // 构造函数初始化类型索引
    MethodType(std::type_index aType, std::type_index rType)
            : argType(aType), replyType(rType) {}

    // 调用计数增加函数
    void incrementCalls() { ++numCalls; }
};

// 服务类
//class Service {
//public:
//    std::string name;
//    std::map<std::string, std::shared_ptr<MethodType>> methods;
//
//    // 添加方法
//    void addMethod(const std::string &methodName, std::shared_ptr<MethodType> methodType) {
//        methods[methodName] = std::move(methodType);
//    }
//
//    // 获取方法
//    std::shared_ptr<MethodType> getMethod(const std::string &methodName) {
//        auto it = methods.find(methodName);
//        if (it != methods.end()) {
//            return it->second;
//        }
//        return nullptr;  // 或抛出一个异常
//    }
//};

// 请求头
struct RequestHeader {
    std::string serviceMethod;  // 服务和方法 "Service.Method"
    uint64_t seq{};               // 客户端选择的序列号
    std::string error;          // 错误信息

    RequestHeader() = default;

    RequestHeader(std::string sMethod, uint64_t s, std::string err = "")
            : serviceMethod(std::move(sMethod)), seq(s), error(std::move(err)) {}
};

// RequestBody
class RequestBody {
public:

    RequestBody() = default;  // 添加默认构造函数，利用std::variant的默认行为

    explicit RequestBody(const GetRadarStatusRequest &stringDemo) : param(stringDemo) {}

    explicit RequestBody(const SetRadarStatusRequest &intDemo) : param(intDemo) {}

    ArgVariant param;
};

class Response {
public:
    static const int STATUS_SUCCESS = 200;       // 操作成功
    static const int STATUS_NOT_FOUND = 404;     // 资源未找到
    static const int STATUS_ERROR = 500;         // 服务器内部错误
    static const int STATUS_UNAUTHORIZED = 401;  // 需要认证


    uint64_t seq{};               // 请求的序列号
    int status{};                 // 增加状态字段，用于详细的状态码
    std::string error;
    RespVariant responseBody;

public:

    // 无参构造，用于测试
    Response() : seq(0), status(200), error("Uninitialized"), responseBody(GetRadarStatusResponse{}) {}
    // 用GetRadarStatusResponse初始化Response，无错误信息

    // 全参数构造函数
    Response(uint64_t s, int st, std::string err, RespVariant resp)
            : seq(s), status(st), error(std::move(err)), responseBody(std::move(resp)) {}


    // GetRadarStatus接口的返回值
    explicit Response(const GetRadarStatusResponse &response)
            : seq(0), status(response.status), responseBody(response) {}
    // SetRadarStatus接口的返回值
    explicit Response(const SetRadarStatusResponse &response)
            : seq(0), status(response.status), responseBody(response) {}


};

class Request {
public:


    RequestHeader header;
    RequestBody body;
    std::promise<Response> promise;

    Request() = default;

    // Constructor for creating a Request with a GetRadarStatusRequest object
    Request(RequestHeader hdr, const GetRadarStatusRequest &stringDemo)
            : header(std::move(hdr)), body(stringDemo) {}

    // Constructor for creating a Request with a SetRadarStatusRequest object
    Request(RequestHeader hdr, const SetRadarStatusRequest &intDemo)
            : header(std::move(hdr)), body(intDemo) {}

    void complete(const Response &response) {
        promise.set_value(response);
    }

    std::future<Response> get_future() {
        return promise.get_future();
    }
};


#endif //REGISTRYCPP_REQUEST_H
