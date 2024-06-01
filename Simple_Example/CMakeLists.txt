cmake_minimum_required(VERSION 3.25.1)
project(Simple_Example)

set(CMAKE_CXX_STANDARD 17)

# Add the include directory for Client.h
include_directories(${CMAKE_SOURCE_DIR}/Client)

# Add the directory containing libclient.so to the library search path
link_directories(${CMAKE_SOURCE_DIR}/Client)

# Add your executable
add_executable(Simple_Example main.cpp)

# Link your executable with libclient
target_link_libraries(Simple_Example client)
