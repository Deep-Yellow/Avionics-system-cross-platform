//
// Created by 黄迪 on 2024/4/17.
//

#include "Server.h"
#include <thread>
#include <chrono>


Server::Server() {
    // Register methods
    methods["Service.getRadarStatus"] = [this](Request& req) { return handle_getRadarStatus(req); };
    methods["Service.setRadarStatus"] = [this](Request& req) { return handle_setRadarStatus(req); };

    std::thread([this]() {
        this->receiveFromDDS();
    }).detach();  // 启动线程并立即分离
}

Response Server::handle_getRadarStatus(Request& req) {
    if (auto* stringArgs = std::get_if<GetRadarStatusRequest>(&req.body.param)) {
        // 参数类型正确，处理请求
        GetRadarStatusResponse getResponse;
        getResponse.status = Response::STATUS_SUCCESS;
        getResponse.message = "Receive request from " + stringArgs->from_service + " to " + stringArgs->to_service;

        Response response(req.header.seq, getResponse.status, "----", getResponse);

        return response;
    } else {
        // 参数类型不匹配，返回错误信息
        GetRadarStatusResponse getResponse;
        getResponse.status = Response::STATUS_ERROR;
        getResponse.message = "错误: 参数类型不是GetRadarStatusArgs";

        Response response(req.header.seq, getResponse.status, "参数类型错误", getResponse);
        return response;
    }
}

Response Server::handle_setRadarStatus(Request& req) {
    if (auto* intArgs = std::get_if<SetRadarStatusRequest>(&req.body.param)) {
        // 成功转换，创建响应对象
        SetRadarStatusResponse setResponse;
        setResponse.status = Response::STATUS_SUCCESS;
        setResponse.message = "处理成功: a = " + std::to_string(intArgs->a) + ", b = " + std::to_string(intArgs->b);

        Response response(req.header.seq, setResponse.status, "====", setResponse);
        return response;
    } else {
        // 转换失败，创建错误响应
        SetRadarStatusResponse setResponse;
        setResponse.status = Response::STATUS_ERROR;
        setResponse.message = "错误: 参数类型不是MyArgsIntDemo";

        Response response(req.header.seq, setResponse.status, "参数类型错误", setResponse);
        return response;
    }
}

Response Server::dispatch(Request& req) {
    auto it = methods.find(req.header.serviceMethod);
    if (it != methods.end()) {
        std::promise<Response> responsePromise;
        std::future<Response> responseFuture = responsePromise.get_future();
        std::thread t([this, it, &req, &responsePromise]() {
            Response response = it->second(req);
            responsePromise.set_value(response); // 将结果存入promise
        });
        t.detach();
        return responseFuture.get(); // 获取并返回异步执行的结果
    } else {
        Response errorResponse;
        errorResponse.status = Response::STATUS_NOT_FOUND;
        errorResponse.error = "Error: Method not found";
        return errorResponse;
    }
}



Response Server::processRequest(Request& req) {
    Response result = dispatch(req);
    // add additional logic to send the message
    return result;
}

//    std::string method = "Service.getRadarStatus";  // 选择一个默认方法
//    std::string from_service = "ServiceA";
//    std::string to_service = "ServiceB";
//    int a = 10;  // 对于setRadarStatus的测试值
//    int b = 20;
//
//    // 创建RequestHeader和Args对象，模拟从DDS接收到的数据
//    RequestHeader header(method, 0);  // 假设seq为0，实际应用中可能不同
//
//    // TODO
//    // Mock
//    Request req;
//    if (method == "Service.getRadarStatus") {
//        GetRadarStatusRequest getArgs{};
//        getArgs.from_service = from_service;
//        getArgs.to_service = to_service;
//        req = Request(header, getArgs);
//
//    } else if (method == "Service.setRadarStatus") {
//        SetRadarStatusRequest setArgs{};
//        setArgs.a = a;
//        setArgs.b = b;
//        req = Request(header, setArgs);
//    }
//
//    // 触发请求处理
//    Response response = processRequest(req);
//
//    // 可以根据实际情况来处理这个响应，比如打印信息，或者触发其他的操作
//    std::cout << "Response received: " << response.error << std::endl;

[[noreturn]] void Server::receiveFromDDS() {
    while (true) {
        // Prepare the first request for Service.getRadarStatus
        RequestHeader header1("Service.getRadarStatus", 0);
        GetRadarStatusRequest getArgs{};
        getArgs.from_service = "ServiceA";
        getArgs.to_service = "ServiceB";
        Request req1(header1, getArgs);

        // Prepare the second request for Service.setRadarStatus
        RequestHeader header2("Service.setRadarStatus", 1);
        SetRadarStatusRequest setArgs{};
        setArgs.a = 10;
        setArgs.b = 20;
        Request req2(header2, setArgs);

        // Launch both requests asynchronously in parallel
        std::future<Response> future1 = std::async(std::launch::async, &Server::processRequest, this, std::ref(req1));
        std::future<Response> future2 = std::async(std::launch::async, &Server::processRequest, this, std::ref(req2));

        // Wait for both async tasks to complete
        Response response1 = future1.get();
        Response response2 = future2.get();

        // Output the responses
        sendByDDS(response1);
        sendByDDS(response2);

        // Wait six seconds before repeating the loop
        std::this_thread::sleep_for(std::chrono::seconds(6));
    }
}

void Server::sendByDDS(Response& response){
    std::cout<<"Process finished, response sent by dds"<< response.error << std::endl;
}



