cmake_minimum_required(VERSION 3.24)
project(server_boost)

set(CMAKE_CXX_STANDARD 20)
find_package(Boost COMPONENTS coroutine system thread context REQUIRED)

add_executable(server_boost main.cpp)
target_link_libraries(server_boost Boost::boost Boost::context Boost::thread)