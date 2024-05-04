#include <iostream>
#include "src/client/Client.h"
#include "src/server/Server.h"
#include "src/common/Args.h"
#include "src/common/Request.h"

int testClient();
void testServer();

int main() {
    std::cout << "Hello, World!" << std::endl;
//    testClient();
    testServer();
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
