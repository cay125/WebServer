#include <iostream>
#include <stdexcept>
#include <sstream>
#include <locale>
#include <iomanip>
#include <sys/timerfd.h>
#include <unistd.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "net/EventLoop.hpp"
#include "net/Channel.hpp"
#include "net/Acceptor.hpp"
#include "net/TcpServer.hpp"
#include "net/TimerQueue.hpp"
#include "net/Connector.hpp"
#include "net/TcpClient.hpp"

#include "apps/HttpServer.hpp"

#include "utils/AsyncLogger.hpp"

DEFINE_bool(background, false, "Wether program run in background?");
DEFINE_uint32(port, 8080, "the port to run http server");
DEFINE_string(resource_dir, "/home/nano/resource", "Directory of resource files");
DEFINE_uint32(test_case, 0, "select which test case to run");

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
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    FLAGS_log_dir = "/home/nano/Documents/webserver_log";
    #ifndef NDEBUG // Debug mode 
    FLAGS_stderrthreshold = 0;
    #else // Release mode
    FLAGS_stderrthreshold = 2;
    #endif

    if (FLAGS_test_case == 0) //for timer test
    {
        Fire::EventLoop event_loop;
        int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
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
        event_loop.loop();
    }
    else if (FLAGS_test_case == 1) // for acceptor class test
    {
        Fire::EventLoop event_loop;
        Fire::NetAddr listen_addr(Fire::NetAddr::ANY_ADDR, FLAGS_port);
        Fire::Acceptor ac(&event_loop, listen_addr);
        ac.setNewConnCallback([](int fd, Fire::NetAddr addr)
                            {
                                std::cout << "client: " << addr.GetPort() << " Ip: " << addr.GetIpString() << "\n";
                                write(fd, "echo\n", 5);
                                Fire::Socket::closeSock(fd);
                            });
        ac.listening();
        event_loop.loop();
    }
    else if (FLAGS_test_case == 2) //for echo server test
    {  
        Fire::EventLoop event_loop;
        std::cout << "max hardware thread: " << std::thread::hardware_concurrency() << "\n";
        Fire::TcpServer server(&event_loop, 8079, 0);
        server.setMessageCallback([](std::shared_ptr<Fire::TcpConnection> p, const char *buf, ssize_t len)
                                {
                                    fwrite(buf, len, 1, stdout);
                                    p->send(buf);
                                });
        server.setConnectionCallback([](std::shared_ptr<Fire::TcpConnection> p)
                                    {
                                        if (p->connectionState() == Fire::TcpConnection::STATE::connected)
                                            std::cout << "connected callback was called from thread: " << std::this_thread::get_id() << "\n";
                                        else
                                            std::cout << "closed callback was called from thread: " << std::this_thread::get_id() << "\n";
                                    });
        server.start();
        event_loop.loop();
    }
    else if (FLAGS_test_case == 3) //for http server test
    {
        Fire::EventLoop event_loop;
        Fire::App::HttpServer http_server(&event_loop, FLAGS_port, FLAGS_resource_dir, 0);
        http_server.RegisterHandler("/proxy", [](std::shared_ptr<Fire::TcpConnection> p, Fire::App::httpRequest r)
        {
            std::cout << "Routing!!\n";
            std::string msg = "fucking master\n";
            p->send("HTTP/1.1 200 OK\r\nServer: fire\r\nConnection:  Keep-Alive\r\nContent-Length: +" + std::to_string(msg.length()) +
                    "\r\nContent-Type: text/html\r\nKeep-Alive: timeout=120000\r\n\r\n" + msg);
        });
        http_server.Start();
        event_loop.loop();
    }
    else if (FLAGS_test_case == 4) //for timer queue test
    {
        Fire::EventLoop event_loop;
        Fire::TimerQueue queue(&event_loop);
        queue.addTimer([]()
                    { std::cout << "timer queue event 1s\n"; }, std::chrono::seconds(1));
        queue.addTimer([]()
                    { std::cout << "timer queue event 5s\n"; }, std::chrono::seconds(5));
        queue.addTimer([]()
                    { static int count = 0; std::cout << "timer queue event 6s - " << count++ << " \n"; }, std::chrono::seconds(6), std::chrono::milliseconds(1000));
        tm t{};
        std::istringstream ss("2021-02-08 15:15:10");
        ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");
        queue.addTimer([]()
                      { 
                          auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); 
                          auto local_time = std::localtime(&tt);
                          std::cout << "evetn happen in: " << std::asctime(local_time) << "\n";
                      }, t);

        event_loop.loop();
    }
    else if (FLAGS_test_case == 5) //for Connector test
    {  
        Fire::EventLoop event_loop;
        Fire::TimerQueue queue(&event_loop);
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
        event_loop.loop();
    }
    else if (FLAGS_test_case == 6) //for TcpClient test
    {
        Fire::EventLoop event_loop;
        Fire::App::HttpServer http_server(&event_loop, 8070, FLAGS_resource_dir, 0);
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
    }
    else if (FLAGS_test_case == 7)
    {
        try
        {
            Fire::NetAddr addr("www.baidu.com", 80);
            std::cout << addr.GetIpString() << "\n";
        }
        catch(const std::exception& e)
        {
            std::cout << "Enter exception: " << e.what() << "\n";
        }
    }

    gflags::ShutDownCommandLineFlags();
    google::ShutdownGoogleLogging();
    return 0;
}