#include <iostream>
#include <sys/timerfd.h>
#include <unistd.h>

#include "net/EventLoop.hpp"
#include "net/Channel.hpp"
#include "net/Acceptor.hpp"
#include "net/TcpServer.hpp"
#include "net/TimerQueue.hpp"
#include "net/Connector.hpp"
#include "net/TcpClient.hpp"

#include "apps/HttpServer.hpp"

#include "utils/AsyncLogger.hpp"

void proxy(std::shared_ptr<Fire::TcpConnection> p, Fire::App::httpRequest r)
{
    if (r.query.empty())
    {
        std::string msg = "Proy format error. Please try again. The valid format is [http://127.0.0.1:9099/proxy?http://www.so.com/robots.txt]\n";
        std::cout << msg;
        p->send("HTTP/1.1 200 OK\r\nServer: fire\r\nConnection:  Keep-Alive\r\nContent-Length: +" + std::to_string(msg.length()) +
                "\r\nContent-Type: text/html\r\nKeep-Alive: timeout=120000\r\n\r\n" + msg);
        return;
    }
    auto loop = p->GetLoop();
    Fire::TcpClient *client = new Fire::TcpClient(loop, Fire::NetAddr(r.query, 80));
    client->setNewConnCallback([](std::shared_ptr<Fire::TcpConnection> p)
                               {
                                   std::cout << "client connection established\n";
                                   p->send("GET / HTTP/1.1\r\n\r\n");
                               });
    client->setMessageCallback([p](std::shared_ptr<Fire::TcpConnection> p2, const char *buf, ssize_t len)
                               {
                                   std::string m;
                                   for (int i = 0; i < len; i++)
                                       m += buf[i];
                                   p->send(m);
                                   //client->Disconnect();
                               });
    client->Connect();
}

int main(int argc, char **argv)
{
    FLOG << "LOG TEST!!";
    //for timer test
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Fire::EventLoop event_loop;
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
    Fire::NetAddr listen_addr(Fire::NetAddr::ANY_ADDR, 8080);
    Fire::Acceptor ac(&event_loop, listen_addr);
    ac.setNewConnCallback([](int fd, Fire::NetAddr addr)
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
    Fire::App::HttpServer http_server(&event_loop, 8070, root_dir, 0);
    http_server.RegisterHandler("/proxy", [](std::shared_ptr<Fire::TcpConnection> p, Fire::App::httpRequest r)
    {
        std::cout << "Routing!!\n";
        std::string msg = "fucking master\n";
        p->send("HTTP/1.1 200 OK\r\nServer: fire\r\nConnection:  Keep-Alive\r\nContent-Length: +" + std::to_string(msg.length()) +
                "\r\nContent-Type: text/html\r\nKeep-Alive: timeout=120000\r\n\r\n" + msg);
    });
    http_server.Start();

    //for timer queue test
    Fire::TimerQueue queue(&event_loop);
    queue.addTimer([]()
                   { std::cout << "timer queue event 1s\n"; }, std::chrono::seconds(1));
    queue.addTimer([]()
                   { std::cout << "timer queue event 5s\n"; }, std::chrono::seconds(5));

    //for Connector test
    Fire::Connector conn(&event_loop, Fire::NetAddr("36.152.44.95", 80));
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
    Fire::TcpClient client(&event_loop, Fire::NetAddr("36.152.44.95", 80));
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
    http_server.RegisterHandler("/fuck", &proxy);
    event_loop.loop();
    return 0;
}