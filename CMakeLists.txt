cmake_minimum_required(VERSION 3.13)

project(fireServer)

#set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++17")

include_directories(
        ${PROJECT_SOURCE_DIR}/net
        ${PROJECT_SOURCE_DIR}/apps
        ${PROJECT_SOURCE_DIR}/utils
)
set(SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/net/Acceptor.cpp
        ${PROJECT_SOURCE_DIR}/net/Buffer.cpp
        ${PROJECT_SOURCE_DIR}/net/Channel.cpp
        ${PROJECT_SOURCE_DIR}/net/Connector.cpp
        ${PROJECT_SOURCE_DIR}/net/eventLoop.cpp
        ${PROJECT_SOURCE_DIR}/net/eventMonitor.cpp
        ${PROJECT_SOURCE_DIR}/net/eventThreadPool.cpp
        ${PROJECT_SOURCE_DIR}/net/TcpClient.cpp
        ${PROJECT_SOURCE_DIR}/net/TcpServer.cpp
        ${PROJECT_SOURCE_DIR}/net/timerQueue.cpp
        ${PROJECT_SOURCE_DIR}/utils/asyncLogger.cpp
        ${PROJECT_SOURCE_DIR}/utils/LogFile.cpp
        ${PROJECT_SOURCE_DIR}/apps/HttpData.cpp
        ${PROJECT_SOURCE_DIR}/apps/HttpServer.cpp
)
add_executable(fireServer main.cpp ${SOURCE_FILES})

target_link_libraries(fireServer -lpthread)