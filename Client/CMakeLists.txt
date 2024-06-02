cmake_minimum_required(VERSION 3.28)
project(Client)

set(CMAKE_CXX_STANDARD 17)

add_library(client SHARED Client.cpp Helpers/Utilities.cpp)
