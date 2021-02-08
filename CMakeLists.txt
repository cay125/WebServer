cmake_minimum_required(VERSION 3.10)

project(fireServer)

#set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++17")

find_package(glog REQUIRED)
find_package(gflags REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})
set(SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/net/Acceptor.cpp
        ${PROJECT_SOURCE_DIR}/net/Buffer.cpp
        ${PROJECT_SOURCE_DIR}/net/Channel.cpp
        ${PROJECT_SOURCE_DIR}/net/Connector.cpp
        ${PROJECT_SOURCE_DIR}/net/EventLoop.cpp
        ${PROJECT_SOURCE_DIR}/net/EventMonitor.cpp
        ${PROJECT_SOURCE_DIR}/net/EventThreadPool.cpp
        ${PROJECT_SOURCE_DIR}/net/TcpClient.cpp
        ${PROJECT_SOURCE_DIR}/net/TcpServer.cpp
        ${PROJECT_SOURCE_DIR}/net/TimerQueue.cpp
        ${PROJECT_SOURCE_DIR}/utils/AsyncLogger.cpp
        ${PROJECT_SOURCE_DIR}/utils/LogFile.cpp
        ${PROJECT_SOURCE_DIR}/apps/HttpData.cpp
        ${PROJECT_SOURCE_DIR}/apps/HttpServer.cpp
)
add_executable(fireServer main.cpp ${SOURCE_FILES})
target_link_libraries(fireServer -lpthread gflags glog::glog)

add_executable(httpserver samples/httpserver.cpp ${SOURCE_FILES})
target_link_libraries(httpserver -lpthread gflags glog::glog)