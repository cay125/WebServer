#include <iostream>
#include <sys/timerfd.h>
#include "eventLoop.hpp"
#include "Channel.hpp"
#include "string.h"
#include "Acceptor.hpp"
#include "TcpServer.hpp"
#include "HttpServer.hpp"
#include <unistd.h>
#include "timerQueue.hpp"

int main(int argc, char **argv)
{
    //for timer test
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Fire::eventLoop event_loop;
    Fire::Channel c(&event_loop, fd);
    c.setReadCallback(std::function<void()>([fd]()
                                            {
                                                std::cout << "timer event\n";
                                                uint64_t v;
                                                read(fd, &v, sizeof(v));
                                            }), true);
    itimerspec time_long;
    memset(&time_long, 0, sizeof(time_long));
    time_long.it_interval.tv_sec = 0;
    time_long.it_value.tv_sec = 1;
    timerfd_settime(fd, 0, &time_long, nullptr);

    //for acceptor class test
    Fire::netAddr listen_addr(Fire::netAddr::ANY_ADDR, 8080);
    Fire::Acceptor ac(&event_loop, listen_addr);
    ac.setNewConnCallback([](int fd, Fire::netAddr addr)
                          {
                              std::cout << "client: " << addr.GetPort() << " Ip: " << addr.GetAddr() << "\n";
                              write(fd, "echo\n", 5);
                              Fire::Socket::closeSock(fd);
                          });
    ac.listening();

    //for echo server test
    std::cout << "max hardware thread: " << std::thread::hardware_concurrency() << "\n";
    Fire::TcpServer server(&event_loop, 8079, 0);
    server.setMessageCallback([](std::shared_ptr<Fire::TcpConnection> p, const char *buf, ssize_t len)
                              {
                                  fwrite(buf, len, 1, stdout);
                                  p->send(buf);
                              });
    server.setConnectionCallback([](std::shared_ptr<Fire::TcpConnection> p)
                                 {
                                     if (p->connectionState() == Fire::TcpConnection::connected)
                                         std::cout << "connected callback was called from thread: " << std::this_thread::get_id() << "\n";
                                     else
                                         std::cout << "closed callback was called from thread: " << std::this_thread::get_id() << "\n";
                                 });
    server.start();

    //for http server test
    std::string root_dir = "../../";
    if (argc != 1)
        root_dir = argv[1];
    Fire::App::HttpServer http_server(&event_loop, 8072, root_dir, 0);
    http_server.Start();

    Fire::timerQueue queue(&event_loop);
    queue.addTimer([]()
                   { std::cout << "timer queue event 1s\n"; }, std::chrono::seconds(1));
    queue.addTimer([]()
                   { std::cout << "timer queue event 5s\n"; }, std::chrono::seconds(5));
    event_loop.loop();
    return 0;
}