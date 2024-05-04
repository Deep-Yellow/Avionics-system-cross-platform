//
// Created by 黄迪 on 2024/4/18.
//


#include "Client.h"
#include "../common/Args.h"

//    uint64_t seq;
//    {
//        std::lock_guard<std::mutex> lock(mtx);
//        seq = seq++;  // 获取当前调用号并递增
//    }
//    RequestHeader header = {method, seq, ""};
//
//    Request req = {header, body};
//
//    // 发送请求
//
//    std::unique_lock<std::mutex> lock(mtx);
//    if (!cv.wait_for(lock, std::chrono::seconds(10), [this, seq] { return last_response.seq == seq; })) {
//        return false; // 超时
//    }
//
//    *reply = last_response;  // 设置响应
//    return true;

void sendByDDS(const std::shared_ptr<Request> &request);

Client::Client() {
    // Launching the receiveFromDDS method in a separate thread
    std::thread(&Client::receiveFromDDS, this).detach();
}

Client::~Client() {
    // Set shutdown or closing flags to true to stop the thread
    shutdown = true;
}

bool Client::Call(const std::string &method, const GetRadarStatusRequest &args, Response *reply) {

// 创建请求头部
    RequestHeader header(method, seq++, "");

    // 创建请求对象
    auto request = std::make_shared<Request>(header, args);

    // 使用std::future来等待响应
    auto future = request->get_future();

    // 发送请求
    send(request);
    try {
        // 等待响应或超时
        if (future.wait_for(std::chrono::seconds(6)) != std::future_status::ready) {
            // 超时处理
            std::cerr << "Request timed out." << std::endl;
            removeRequest(request->header.seq); // 从pending中移除
            reply->error = "Request timed out.";
            return false;
        }
        // 获取响应
        *reply = future.get();  // 可能抛出异常，如果promise被设置为异常
        // 检查响应中的错误字段
        if (reply->status != Response::STATUS_SUCCESS) {
            std::cerr << "Error from server: " << reply->error << std::endl;
            return false;
        }
//        std::cout << "-------"<<reply->status<<std::endl;
        return true;  // 成功处理请求
    } catch (const std::exception &e) {
        // 处理可能的异常
        std::cerr << "Exception while waiting for message: " << e.what() << std::endl;
        return false;
    }
}

//void Client::sendDDS(const Request& req) {
//    // 实现发送请求的逻辑
//    // 调用DDS发送函数
//}
//Response Client::receiveResponse() {
//    // 实现接收响应的逻辑
//    // 调用DDS接收函数
//    // 模拟接收到的数据
//    // return {seq-1};
//    GetRadarStatusResponse getRadarStatus = {"test"};
//    Response message(getRadarStatus);
//    return message;
//}
//
//void Client::receiveDDS() {
//    // TODO
//    Response message = receiveResponse();  // 接收数据
//    std::lock_guard<std::mutex> lock(mtx);
//    last_response = message;
//    cv.notify_all();  // 通知等待的Call方法
//}

bool Client::IsAvailable() const {
    return !this->closing && !this->shutdown;
}

std::shared_ptr<Request> Client::removeRequest(uint64_t req_seq) {
    std::lock_guard<std::mutex> lock(mtx);  // 使用锁确保线程安全
    auto it = pending.find(req_seq);
    if (it != pending.end()) {
        auto request = it->second;
        pending.erase(it);  // 如果找到，从映射中删除
        return request;  // 返回找到的请求
    } else {
        // 如果没有找到对应的请求，打印日志消息
        std::cerr << "Request with Seq " << req_seq << " not found, might have timed out or been terminated already." << std::endl;
        return nullptr;  // 返回空指针
    }
}

uint64_t Client::registerRequest(const std::shared_ptr<Request> &request) {
    std::lock_guard<std::mutex> lock(mtx);
    if (closing || shutdown) {
        throw std::runtime_error("Client is shutting down");
    }
    request->header.seq = this->seq++;
    pending[request->header.seq] = request;
    return request->header.seq;
}

void Client::send(const std::shared_ptr<Request> &request) {
    std::thread([request, this]() {
        try {
            uint64_t seq = registerRequest(request);  // 注册这个请求并获得序列号
            // 省略实际的网络发送逻辑，仅模拟调用
            sendByDDS(request);  // 发送请求通过DDS
            std::cout<< "[send] DDS sent success" << std::endl;
            // 不再在这里处理响应
        } catch (const std::exception &e) {
            checkPendingRequests(); //检查是否正确register
            // 发生异常，设置异常信息并完成请求
            GetRadarStatusResponse errorResponse;  // 创建一个默认的响应对象来表示错误
            Response response(request->header.seq, 500, e.what(), errorResponse);  // 设置错误响应
            request->complete(response);  // 完成请求，设置错误响应
        }
    }).detach();
}

void Client::checkPendingRequests() {
    std::lock_guard<std::mutex> lock(mtx); // 保证线程安全
    std::cout << "Total pending requests: " << pending.size() << std::endl;

    for (const auto &pair: pending) {
        std::cout << "Request Seq: " << pair.first << std::endl;
        // 假设Request类有一个名为getDescription的方法来提供请求的详细描述
        // std::cout << "Request Detail: " << pair.second->getDescription() << std::endl;
    }
}

void Client::terminateRequests(const std::string &err) {

}

void Client::receiveFromDDS() {
    // 模拟从DDS接收数据
    // TODO
    // 根据实际的DDS接收逻辑来调整
    while (!shutdown) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));  // 模拟接收延迟
        Response response;
        if (!pending.empty()) {
            std::cout << "[Simulation] Receive response from dds" << std::endl;
            auto it = pending.begin();
            auto request = it->second;

            GetRadarStatusResponse mockResponse{200, "Operation successful"};
            response.seq = request->header.seq;
            response.status = mockResponse.status;
            response.responseBody = mockResponse;
            response.error = "";  // 没有错误

            request->complete(response);  // 完成请求，设置响应
            // pending.erase 在removeRequest中处理
        }
    }
}

void sendByDDS(const std::shared_ptr<Request> &request) {
    //TODO
    std::cout << "Sent By DDS" << std::endl;
}