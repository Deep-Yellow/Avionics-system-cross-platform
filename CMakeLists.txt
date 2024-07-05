cmake_minimum_required(VERSION 3.26)
project(RegistryCPP)

set(CMAKE_CXX_STANDARD 17)

add_executable(RegistryCPP main.cpp
        src/common/Request.h
        src/common/Service.h
        src/Registry/ServiceRegistry.cpp
        src/Registry/ServiceRegistry.h
        src/common/Args.h
        src/Server/Server.cpp
        src/Server/Server.h
        src/Client/Client.cpp
        src/Client/Client.h
        src/Gateway/Gateway.cpp
        src/Gateway/Gateway.h
        src/test.cpp
        src/test.h)
