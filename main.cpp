#include <iostream>
#include "src/Client/Client.h"
#include "src/Server/Server.h"
#include "src/common/Args.h"
#include "src/common/Request.h"
#include "src/Registry/ServiceRegistry.h"

int testClient();
void testServer();
void testFindBestPerformanceService(ServiceRegistry& registry);
void testFindNearestService(ServiceRegistry& registry);

int main() {
    ServiceRegistry registry;
//    testFindBestPerformanceService(registry);
    testFindNearestService(registry);
    return 0;
}

int testClient(){
    Client client;  // 实例化Client对象

    // 创建请求参数对象并设置相关参数
    GetRadarStatusRequest args;
    args.from_service = "RadarService_Consumer1";  // 假设的服务名
    args.to_service = "RadarService2";  // 假设的目标服务名

    Response reply;  // 用于接收响应的对象

    // 调用Client的Call方法，发送请求并等待响应
    bool result = client.Call("GetRadarStatus", args, &reply);

    // 检查调用结果
    if (result) {
        // 调用成功，处理响应数据
        std::cout << "Call succeeded.\n";
        if (std::holds_alternative<GetRadarStatusResponse>(reply.responseBody)) {
            auto response = std::get<GetRadarStatusResponse>(reply.responseBody);  // 提取响应数据
            std::cout << "Status: " << reply.status << "\n";
            std::cout << "Message: " << response.message << "\n";
        } else {
            std::cout << "Received an unexpected type of message.\n";
        }
    } else {
        // 调用失败，输出错误信息
        std::cout << "Call failed: " << reply.error << std::endl;
    }

    return 0;
}

void testServer(){
    Server server;
    while (true){

    }
}

// Expected result: service2 on node1
void testFindBestPerformanceService(ServiceRegistry& registry) {
    // 构造位置信息（示例位置）
    LocationInfo location{31.2304, 121.4737, "Shanghai"};
    // 构造性能指标，具体值在这里不重要，假定findPerformanceMetrics会提供真实数据
    PerformanceMetrics performance{0.0, 100.0, 0};
    // 构造服务描述符，寻找性能最好的DataService
    ServiceDescriptor descriptor(1, location, performance);
    // 创建FindService请求
    FindServiceRequest findRequest("DataService", descriptor);
    // 包装成ServiceRegistryRequestContainer
    ServiceRegistryRequestContainer requestContainer(findRequest);
    // 调用handleRequest处理请求
    Response response = registry.handleRequest(requestContainer);

    // 输出测试结果
    if (response.status == Response::STATUS_SUCCESS) {
        auto& service = std::get<FindServiceResponse>(response.responseBody);
        std::cout << "Found best performance DataService: " << service.service.instance_id << std::endl;
    } else {
        std::cout << "Service not found or no suitable service: " << response.error << std::endl;
    }
}

void testFindNearestService(ServiceRegistry& registry) {
    // 构造位置信息（假设这是客户当前所在的位置）
    LocationInfo clientLocation{30.2672, 120.1528, "Hangzhou"};

    // 构造服务描述符，寻找地理位置最近的DataService
    ServiceDescriptor descriptor(0, clientLocation, PerformanceMetrics{0, 0, 0}); // mode 0 表示基于地理位置寻找

    // 创建FindService请求
    FindServiceRequest findRequest("DataService", descriptor);

    // 包装成ServiceRegistryRequestContainer
    ServiceRegistryRequestContainer requestContainer(findRequest);

    // 调用handleRequest处理请求
    Response response = registry.handleRequest(requestContainer);

    // 输出测试结果
    if (response.status == Response::STATUS_SUCCESS) {
        auto& service = std::get<FindServiceResponse>(response.responseBody).service;
        std::cout << "Found nearest DataService: " << service.instance_id << " at node " << service.nodeId << std::endl;
    } else {
        std::cout << "Service not found or no suitable service: " << response.error << std::endl;
    }
}
