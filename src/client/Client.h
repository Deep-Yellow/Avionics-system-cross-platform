//
// Created by 黄迪 on 2024/4/18.
//

#ifndef REGISTRYCPP_CLIENT_H
#define REGISTRYCPP_CLIENT_H


#include <cstdint>
#include <condition_variable>
#include "../common/Args.h"
#include "../common/Request.h"

/**
 * 1. 服务消费者调用Client.Call("NodeName.ServiceName.ServiceName", body, &reply)进行远程调用
 * 2. Client.Call方法中需要封装出Request对象
 * 3. 上锁，通过调用DDS Send方法发给Gateway
 * 4. 记录本次调用号，阻塞当前进程
 * 5. DDS的回调中通过cv.notify_all唤醒进程
 * 6. 释放锁
 */

/**
 * TODO
 * 1. 需要考虑多个相同服务吗？ --- 锁改成静态变量
 * 2. Client到Gateway的调用应该有调用方的ServiceID
 * 3. 异步调用的支持，设置"pending" map 存储为处理完的请求
 */

class Client {
private:
    uint64_t seq = 0;
    std::mutex mtx;
    std::condition_variable cv;
    Response last_response;
    std::atomic<bool> closing{false};  // 客户端是否正在关闭
    std::atomic<bool> shutdown{false}; // 客户端是否已经关闭
    std::map<uint64_t, std::shared_ptr<Request>> pending; // 存储所有待处理的调用

    void receiveFromDDS();
//    void sendDDS(const Request& req);
//    Response receiveResponse();

public:
    Client();
    ~Client();

    bool Call(const std::string& method, const GetRadarStatusRequest& args, Response* reply);
    bool IsAvailable() const;
    uint64_t registerRequest(const std::shared_ptr<Request>& request);
    std::shared_ptr<Request> removeRequest(uint64_t req_seq);
    void terminateRequests(const std::string& err);
    void send(const std::shared_ptr<Request>& call);
    void checkPendingRequests();
};

#endif //REGISTRYCPP_CLIENT_H
