#include <iostream>
#include <sys/timerfd.h>
#include <unistd.h>
#include "eventLoop.hpp"
#include "Channel.hpp"
#include "Acceptor.hpp"
#include "TcpServer.hpp"
#include "HttpServer.hpp"
#include "timerQueue.hpp"
#include "asyncLogger.hpp"
#include "Connector.hpp"
#include "TcpClient.hpp"

int main(int argc, char **argv)
{
    FLOG << "LOG TEST!!";
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
    std::string root_dir = "../../../resource";
    if (argc != 1)
        root_dir = argv[1];
    Fire::App::HttpServer http_server(&event_loop, 8072, root_dir, 0);
    http_server.RegisterHandler("/proxy", [](std::shared_ptr<Fire::TcpConnection> p)
    {
        std::cout << "Routing!!\n";
        std::string msg = "fucking master\n";
        p->send("HTTP/1.1 200 OK\r\nServer: fire\r\nConnection:  Keep-Alive\r\nContent-Length: +" + std::to_string(msg.length()) +
                "\r\nContent-Type: text/html\r\nKeep-Alive: timeout=120000\r\n\r\n" + msg);
    });
    http_server.Start();

    Fire::timerQueue queue(&event_loop);
    queue.addTimer([]()
                   { std::cout << "timer queue event 1s\n"; }, std::chrono::seconds(1));
    queue.addTimer([]()
                   { std::cout << "timer queue event 5s\n"; }, std::chrono::seconds(5));

    //for Connector test
    Fire::Connector conn(&event_loop, Fire::netAddr("36.152.44.95", 80));
    conn.setNewConnCallback([](int fd)
                            {
                                std::cout << "active connection established\n";
                            });
    conn.Start();
    queue.addTimer([&]()
                   {
                       std::cout << "going to stop connection\n";
                       conn.Stop();
                   }, std::chrono::seconds(2));

    //for TcpClient test
    Fire::TcpClient client(&event_loop, Fire::netAddr("36.152.44.95", 80));
    client.setNewConnCallback([](std::shared_ptr<Fire::TcpConnection> p)
                              {
                                  std::cout << "client connection established\n";
                                  p->send("GET / HTTP/1.1\r\n\r\n");
                              });
    client.setMessageCallback([&](std::shared_ptr<Fire::TcpConnection> p, const char *buf, ssize_t len)
                              {
                                  fwrite(buf, len, 1, stdout);
                                  client.Disconnect();
                              });
    client.Connect();
    event_loop.loop();
    return 0;
}