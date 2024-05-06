## 4月总结

### 开发内容

1. 实现了RPC调用中所用到的Client和Server部分，调用方式为Client.Call("ServiceName.MethodName", args, &reply)
2. 部分实现了服务注册发现中心，目前已完成服务注册、服务注销、发现、心跳保活这几个接口

### TODO

1. 超过30s没有心跳的将服务设置为不可用 超过60s的删除该服务
2. 设置一个入口函数，根据请求参数的类型（ServiceRegisterRequest、ServiceDeregisterRequest等）调用对应的的方法



## 5.6

1. 增加注册中心的初始化方法
2. 实现“最优性能匹配”模式的测试代码