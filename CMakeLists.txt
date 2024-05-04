cmake_minimum_required(VERSION 3.26)
project(RegistryCPP)

set(CMAKE_CXX_STANDARD 17)

add_executable(RegistryCPP main.cpp
        src/common/Request.h
        src/common/Service.h
        src/Registry/ServiceRegistry.cpp
        src/Registry/ServiceRegistry.h
        src/common/Args.h
        src/server/Server.cpp
        src/server/Server.h
        src/client/Client.cpp
        src/client/Client.h
        src/Gateway/Gateway.cpp
        src/Gateway/Gateway.h)
