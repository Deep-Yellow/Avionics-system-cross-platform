#include <iostream>
#include "src/Client/Client.h"
#include "src/Server/Server.h"
#include "src/common/Args.h"
#include "src/common/Request.h"
#include "src/Registry/ServiceRegistry.h"

int testClient();

void testServer();

void testFindBestPerformanceService(ServiceRegistry &registry);

void testFindNearestService(ServiceRegistry &registry);

void testHashService(ServiceRegistry &registry);

void test_compareAndSyncTree();

void test_compareAndSyncTree_with_changes();

void test_compareAndSyncTree_with_changes2();

int main() {
//    ServiceRegistry registry("RegistryA");
//    testFindBestPerformanceService(registry);
//    testFindNearestService(registry);
//    testHashService(registry);

    test_compareAndSyncTree_with_changes2();

    return 0;
}

int testClient() {
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

void testServer() {
    Server server;
    while (true) {

    }
}

// Expected result: service2 on node1
void testFindBestPerformanceService(ServiceRegistry &registry) {
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
        auto &service = std::get<FindServiceResponse>(response.responseBody);
        std::cout << "Found best performance DataService: " << service.service.instance_id << std::endl;
    } else {
        std::cout << "Service not found or no suitable service: " << response.error << std::endl;
    }
}

void testFindNearestService(ServiceRegistry &registry) {
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
        auto &service = std::get<FindServiceResponse>(response.responseBody).service;
        std::cout << "Found nearest DataService: " << service.instance_id << " at node " << service.nodeId << std::endl;
    } else {
        std::cout << "Service not found or no suitable service: " << response.error << std::endl;
    }
}

void testHashService(ServiceRegistry &registry) {
    // 构造位置信息（示例位置）
    LocationInfo location{31.2304, 121.4737, "Shanghai"};
    // 构造性能指标，具体值在这里不重要，假定findPerformanceMetrics会提供真实数据
    PerformanceMetrics performance{0.0, 100.0, 0};
    // 构造服务描述符，寻找性能最好的DataService
    ServiceDescriptor descriptor(1, location, performance);
    // 创建FindService请求
    FindServiceRequest findRequest("Aircraft_Formation_1.UAV1.DataService", descriptor);
    // 包装成ServiceRegistryRequestContainer
    ServiceRegistryRequestContainer requestContainer(findRequest);
    // 调用handleRequest处理请求
    Response response = registry.handleRequest(requestContainer);

    // 输出测试结果
    if (response.status == Response::STATUS_SUCCESS) {
        auto &service = std::get<FindServiceResponse>(response.responseBody);
        std::cout << "[hashService] Use Service: " << service.service.instance_id << " to test, Hash value: "
                  << registry.hashService(service.service) << std::endl;
        // s1 22af04edd52983e24ca684ecc6aa896541bdb67adbb918d07000000000000000
        // s2 22af04edd52983e24ca684ecc6aa896541bd377adbb918517000000000000000
    } else {
        std::cout << "Service not found or no suitable service: " << response.error << std::endl;
    }
}

void test_compareAndSyncTree() {
    // 创建两个ServiceRegistry实例
    ServiceRegistry registry1("Registry1");
    ServiceRegistry registry2("Registry2");

    // 创建服务列表1
    std::vector<Service> services1 = {
            {"DataService",           "service1", "node1", true},
            {"AuthenticationService", "service2", "node1", true},
            {"LoggingService",        "service3", "node2", true}
    };

    // 创建服务列表2，修改其中一个服务以制造不一致
    std::vector<Service> services2 = {
            {"DataService",           "service1", "node1", true},
            {"AuthenticationService", "service2", "node1", true},
            {"LoggingService",        "service4", "node2", true}  // 这里instance_id不同
    };

    // 初始化两个Registry
    registry1.initialize(services1);
    registry2.initialize(services2);

    // 序列化Registry2的树
    std::vector<uint8_t> serializedTree;
    registry2.tree.serialise(serializedTree);

    // 比较并同步树
    registry1.compareAndSyncTree(serializedTree);
}

void test_compareAndSyncTree_with_changes() {
    // 创建两个ServiceRegistry实例
    ServiceRegistry registry1("node1");
    ServiceRegistry registry2("node2");

    // 创建初始服务列表
    std::vector<Service> initialServices1 = {
            {"DataService",           "service1", "node1", true},
            {"AuthenticationService", "service2", "node1", true},
            {"LoggingService",        "service3", "node2", true}
    };

    std::vector<Service> initialServices2 = {
            {"DataService",           "service1", "node1", true},
            {"AuthenticationService", "service2", "node1", true},
            {"LoggingService",        "service3", "node2", true}
    };

    // 初始化两个Registry
    registry1.initialize(initialServices1);
    registry2.initialize(initialServices2);

    // 随机数生成器
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 无限循环比较并同步树，每隔4秒执行一次
    int loopCounter = 0;
    while (true) {
        // 序列化Registry1的树
        std::vector<uint8_t> serializedTree1;
        registry1.tree.serialise(serializedTree1);

        // 序列化Registry2的树
        std::vector<uint8_t> serializedTree2;
        registry2.tree.serialise(serializedTree2);

        std::cout << "Loop count: " << loopCounter << "-----------" << std::endl;

        // 比较并同步树
        registry1.compareAndSyncTree(serializedTree2);

        std::this_thread::sleep_for(std::chrono::seconds(4));

        registry2.compareAndSyncTree(serializedTree1);



        // 服务状态变更
        std::this_thread::sleep_for(std::chrono::seconds(4));
        // 每隔8秒有50%概率改变一个服务的可用状态
        if (loopCounter % 2 == 0) { // 每4秒执行一次
            if (std::rand() % 2 == 0) {
                // 在Registry1中随机选择一个node为"node1"的服务并改变其状态
                auto it1 = std::find_if(initialServices1.begin(), initialServices1.end(),
                                        [](const Service& service) { return service.nodeId == "node1"; });
                if (it1 != initialServices1.end()) {
                    it1->is_alive = !it1->is_alive;
                    registry1.setServiceList(initialServices1);
                    std::cout << "Changed availability of service in Registry1: " << it1->service_name << std::endl;
                }
            }

            if (std::rand() % 2 == 0) {
                // 在Registry2中随机选择一个node为"node2"的服务并改变其状态
                auto it2 = std::find_if(initialServices2.begin(), initialServices2.end(),
                                        [](const Service& service) { return service.nodeId == "node2"; });
                if (it2 != initialServices2.end()) {
                    it2->is_alive = !it2->is_alive;
                    registry2.setServiceList(initialServices2);
                    std::cout << "Changed availability of service in Registry2: " << it2->service_name << std::endl;
                }
            }
        }

        // 等待4秒
        std::this_thread::sleep_for(std::chrono::seconds(4));
        ++loopCounter;
    }
}

void test_compareAndSyncTree_with_changes2() {
    // 创建两个ServiceRegistry实例
    ServiceRegistry registry1("node1");
    ServiceRegistry registry2("node2");

    // 创建初始服务列表
    std::vector<Service> initialServices1 = {
            {"DataService",           "service1", "node1", true},
            {"AuthenticationService", "service2", "node1", true},
            {"LoggingService",        "service3", "node2", true}
    };

    std::vector<Service> initialServices2 = {
            {"DataService",           "service1", "node1", true},
            {"AuthenticationService", "service2", "node1", true},
            {"LoggingService",        "service3", "node2", true}
    };

    // 初始化两个Registry
    registry1.initialize(initialServices1);
    registry2.initialize(initialServices2);

    // 随机数生成器
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 无限循环比较并同步树，每隔4秒执行一次
    int loopCounter = 0;
    while (true) {
        // 序列化Registry1的服务列表
        std::vector<uint8_t> serializedTree1;
        registry1.tree.serialise(serializedTree1);

        // 序列化Registry2的树
        std::vector<uint8_t> serializedTree2;
        registry2.tree.serialise(serializedTree2);

        std::cout << "---------------------------" << std::endl;

        std::cout << "Loop count: " << loopCounter << " start" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "【1】比较并同步树 "<< std::endl;
        // 比较并同步树
        std::vector<std::string> changedServiceTypes1 = registry1.compareAndSyncTree(serializedTree2);

        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::vector<std::string> changedServiceTypes2 = registry2.compareAndSyncTree(serializedTree1);


        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "【2】序列化并发送服务信息给对方 "<< std::endl;

        // Registry1接收并反序列化Registry2的服务列表
        std::vector<uint8_t> serializedServices2ForRegistry1 = registry1.serializeServicesForNames(changedServiceTypes1);
        if (serializedServices2ForRegistry1.size()>4) {
            // 模拟发送序列化的服务列表
            registry1.sendSerializedServices(serializedServices2ForRegistry1);
        }

        // Registry2接收并反序列化Registry1的服务列表
        std::vector<uint8_t> serializedServices1ForRegistry2 = registry2.serializeServicesForNames(changedServiceTypes2);
        if (serializedServices1ForRegistry2.size()>4) {
            // 模拟发送序列化的服务列表
            registry2.sendSerializedServices(serializedServices1ForRegistry2);
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "【3】反序列化并更新 "<< std::endl;

        // 反序列化对方的服务列表
        if (!serializedServices1ForRegistry2.empty()) {
            registry1.deserializeAndSetServices(serializedServices1ForRegistry2);
        }
        if (!serializedServices2ForRegistry1.empty()) {
            registry2.deserializeAndSetServices(serializedServices2ForRegistry1);
        }


        // 模拟服务状态变更
        // 每4个周期 有50%概率改变一个服务的可用状态
        if (loopCounter % 2 == 0) {
            if (std::rand() % 2 == 0) {
                // 在Registry1中随机选择一个node为"node1"的服务并改变其状态
                std::vector<Service> services1 = registry1.getServiceList(); // 假设这个方法返回当前注册的服务列表
                auto it1 = std::find_if(services1.begin(), services1.end(),
                                        [](const Service& service) { return service.nodeId == "node1"; });
                if (it1 != services1.end()) {
                    it1->is_alive = !it1->is_alive;
                    registry1.setServiceList(services1); // 更新服务状态后重新设置服务列表
                    std::cout << "Changed availability of service in Registry1: " << it1->service_name << std::endl;
                }
            }

            if (std::rand() % 2 == 0) {
                // 在Registry2中随机选择一个node为"node2"的服务并改变其状态
                std::vector<Service> services2 = registry2.getServiceList(); // 假设这个方法返回当前注册的服务列表
                auto it2 = std::find_if(services2.begin(), services2.end(),
                                        [](const Service& service) { return service.nodeId == "node2"; });
                if (it2 != services2.end()) {
                    it2->is_alive = !it2->is_alive;
                    registry2.setServiceList(services2); // 更新服务状态后重新设置服务列表
                    std::cout << "Changed availability of service in Registry2: " << it2->service_name << std::endl;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++loopCounter;
    }
}


