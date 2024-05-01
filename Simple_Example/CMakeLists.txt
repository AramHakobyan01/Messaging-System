cmake_minimum_required(VERSION 3.27)
project(Simple_Example)

set(CMAKE_CXX_STANDARD 17)

add_executable(Simple_Example main.cpp ../Client/Client.cpp ../Client/Client.h)