cmake_minimum_required(VERSION 3.13)

project(fireServer)

#set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++17")

include_directories(${PROJECT_SOURCE_DIR}/inc ${PROJECT_SOURCE_DIR}/apps)

add_executable(fireServer main.cpp inc/eventLoop.hpp src/eventLoop.cpp inc/Channel.hpp src/Channel.cpp inc/eventMonitor.hpp src/eventMonitor.cpp inc/Acceptor.hpp src/Acceptor.cpp inc/TcpServer.hpp src/TcpServer.cpp inc/Buffer.hpp src/Buffer.cpp inc/eventThreadPool.hpp src/eventThreadPool.cpp inc/timerQueue.hpp src/timerQueue.cpp apps/HttpServer.cpp apps/HttpServer.hpp apps/HttpData.cpp apps/HttpData.hpp)

target_link_libraries(fireServer -lpthread)