cmake_minimum_required(VERSION 3.27)
project(Server)

set(CMAKE_CXX_STANDARD 17)

add_executable(Server main.cpp
        Server.cpp
        Server.h
        ServerConfiguration/ServerConfiguration.cpp
        ServerConfiguration/ServerConfiguration.h
        ClientConfig/ClientConfig.cpp
        ClientConfig/ClientConfig.h
        Message/Message.cpp
        Message/Message.h
        Socket/Socket.cpp
        Socket/Socket.h
        ClientGroup/ClientGroup.cpp
        ClientGroup/ClientGroup.h)
