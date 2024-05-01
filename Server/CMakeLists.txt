cmake_minimum_required(VERSION 3.27)
project(Server)

set(CMAKE_CXX_STANDARD 17)

find_library(LIBURING_LIBRARY NAMES uring)

if(NOT LIBURING_LIBRARY)
    message(FATAL_ERROR "liburing library not found")
endif()

# Use FetchContent to download yaml-cpp
include(FetchContent)
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG yaml-cpp-0.7.0
)
FetchContent_MakeAvailable(yaml-cpp)

add_executable(Server main.cpp
        Server.cpp
        Server.h
        ServerConfiguration/ServerConfiguration.cpp
        ServerConfiguration/ServerConfiguration.h
        Socket/SocketManager.cpp
        Socket/SocketManager.h
        Structures.h
        Client/Client.cpp
        Client/Client.h)

target_link_libraries(Server ${LIBURING_LIBRARY} yaml-cpp)
