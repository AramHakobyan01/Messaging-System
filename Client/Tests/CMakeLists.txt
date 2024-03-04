cmake_minimum_required(VERSION 3.27)
project(Tests)

set(CMAKE_CXX_STANDARD 17)

add_executable(Tests main.cpp ../Client.cpp ../Client.h)
