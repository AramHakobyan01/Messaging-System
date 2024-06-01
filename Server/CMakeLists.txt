cmake_minimum_required(VERSION 3.25.1)
project(Server)

set(CMAKE_CXX_STANDARD 17)

# Copy config.yaml to build directory
configure_file(${CMAKE_SOURCE_DIR}/config.yaml ${CMAKE_BINARY_DIR}/config.yaml COPYONLY)

# Find liburing
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

# Fetch and build SQLite
FetchContent_Declare(
        sqlite3
        URL https://www.sqlite.org/2024/sqlite-amalgamation-3450300.zip
)
FetchContent_GetProperties(sqlite3)
if(NOT sqlite3_POPULATED)
    FetchContent_Populate(sqlite3)
    add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
    target_include_directories(sqlite3 INTERFACE ${sqlite3_SOURCE_DIR})
endif()

if(NOT sqlite3_POPULATED)
    FetchContent_Populate(sqlite3)
    add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
    target_include_directories(sqlite3 INTERFACE ${sqlite3_SOURCE_DIR})
endif()
message(STATUS "SQLite Download Status: ${sqlite3_POPULATED}")

# Add executable
add_executable(Server
        main.cpp
        Server.cpp
        Server.h
        ServerConfiguration/ServerConfiguration.cpp
        ServerConfiguration/ServerConfiguration.h
        SocketManager/SocketManager.cpp
        SocketManager/SocketManager.h
        Helpers/Structures.h
        Database/Database.cpp
        Database/Database.h
        Helpers/Utilities.cpp
        Helpers/Utilities.h
)

# Link with libraries
target_link_libraries(Server
        PRIVATE
        ${LIBURING_LIBRARY}
        yaml-cpp
        sqlite3
)
