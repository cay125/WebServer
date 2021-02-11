#include <iostream>
#include <pwd.h>
#include <gflags/gflags.h>

#include "apps/HttpServer.hpp"

std::string GetCurrentUser()
{
    uid_t userid = getuid();
    passwd *pwd = getpwuid(userid);
    return std::string(pwd->pw_name);
}

DEFINE_bool(background, false, "Wether program run in background?");
DEFINE_uint32(port, 8080, "the port to run http server");
DEFINE_string(resource_dir, "/home/" + GetCurrentUser() + "/resource", "Directory of resource files");

int main(int argc, char **argv)
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    FLAGS_log_dir = "/home/" + GetCurrentUser() + "/Documents/webserver_log";
    #ifndef NDEBUG // Debug mode 
    FLAGS_stderrthreshold = 0;
    #else // Release mode
    FLAGS_stderrthreshold = 2;
    #endif

    Fire::EventLoop event_loop;
    Fire::App::HttpServer http_server(&event_loop, FLAGS_port, FLAGS_resource_dir, 0);
    http_server.RegisterHandler("/proxy", [](std::shared_ptr<Fire::TcpConnection> p, Fire::App::httpRequest r)
    {
        std::cout << "Routing!!\n";
        std::string msg = "fucking master\n";
        p->send("HTTP/1.1 200 OK\r\nServer: fire\r\nConnection:  Keep-Alive\r\nContent-Length: +" \
                 + std::to_string(msg.length()) \
                 + "\r\nContent-Type: text/html\r\nKeep-Alive: timeout=120000\r\n\r\n" + msg);
    });
    http_server.Start();
    event_loop.loop();

    gflags::ShutDownCommandLineFlags();
    google::ShutdownGoogleLogging();
    return 0;
}