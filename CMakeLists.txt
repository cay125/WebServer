cmake_minimum_required(VERSION 3.13)

project(fireServer)

set(CMAKE_CXX_STANDARD 14)

include_directories(${PROJECT_SOURCE_DIR}/inc)

add_executable(fireServer main.cpp inc/eventLoop.hpp src/eventLoop.cpp inc/Channel.hpp src/Channel.cpp inc/eventMonitor.hpp src/eventMonitor.cpp inc/Acceptor.hpp src/Acceptor.cpp inc/TcpServer.hpp src/TcpServer.cpp inc/Buffer.hpp src/Buffer.cpp inc/eventThreadPool.hpp src/eventThreadPool.cpp)

target_link_libraries(fireServer -lpthread)