//
// Created by xiangpu on 20-3-2.
//
#include <iostream>
#include <unistd.h>
#include <glog/logging.h>

#include "net/TcpServer.hpp"
#include "utils/AsyncLogger.hpp"

Fire::TcpServer::TcpServer(EventLoop *loop, uint16_t port, int thread_num) : event_loop(loop), TcpAcceptor(loop, NetAddr(NetAddr::ANY_ADDR, port)), thread_pool(loop, thread_num)
{
    LOG(INFO) << "Server is listening on port: " << port;
    thread_pool.Start();
    TcpAcceptor.setNewConnCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

void Fire::TcpServer::start()
{
    TcpAcceptor.listening();
}

void Fire::TcpServer::setConnectionCallback(connFcn &&cb)
{
    connectionCallback = cb;
}

void Fire::TcpServer::setMessageCallback(msgFcn &&cb)
{
    messageCallback = cb;
}

void Fire::TcpServer::newConnection(int fd, Fire::NetAddr addr)
{
    LOG(INFO) << "one connection established from Ip: " << addr.GetIpString() << " Port: " << addr.GetPort();
    EventLoop *pool = thread_pool.GetNextThread();
    std::shared_ptr<TcpConnection> conn(new TcpConnection(pool, fd, addr));
    conn->setConnectionCallback(std::move(connectionCallback));
    conn->setMessageCallback(std::move(messageCallback));
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    conn->Established();
    connSet.insert(conn);
}

void Fire::TcpServer::removeConnection(std::shared_ptr<Fire::TcpConnection> conn)
{
    event_loop->runInLoop([this, conn]()
                          {
                              auto n = connSet.erase(conn);
                              if (n != 1)
                                  LOG(ERROR) << "ERROR: Erase closed connection";
                          });
}

Fire::TcpConnection::TcpConnection(EventLoop *loop, int fd, NetAddr _addr) : event_loop(loop), connChannel(event_loop, fd), clientAddr(_addr),
                                                                             state(STATE::connecting)
{
    connChannel.setReadCallback(std::bind(&TcpConnection::HandleRead, this), false);
    connChannel.setWriteCallback(std::bind(&TcpConnection::HandleWrite, this), false);
    connChannel.setCloseCallback(std::bind(&TcpConnection::HandleClose, this), false);
    connChannel.setErrorCallback(std::bind(&TcpConnection::HandleError, this), false);
}

void Fire::TcpConnection::HandleRead()
{
    std::vector<char> buf(65536, 0);
    ssize_t n = read(connChannel.GetMonitorFd(), &*buf.begin(), buf.size());
    if (n > 0)
    {
        if (connChannel.GetTriggerMode() == Channel::TriggerMode::edge_mode && n == buf.size()) // there are still data inside buffer
        {
            size_t rest_n = 0;
            std::vector<char> rest_buf(65536, 0);
            do
            {
                LOG(INFO) << "Going to read more data";
                rest_n = read(connChannel.GetMonitorFd(), &*rest_buf.begin(), rest_buf.size());
                size_t index = buf.size();
                buf.resize(buf.size() + rest_n);
                std::copy(rest_buf.begin(), rest_buf.begin() + rest_n, buf.begin() + index);
            } while (rest_n == rest_buf.size());
        }
        messageCallback(shared_from_this(), &*buf.begin(), n);
    }
    else if (n == 0)
    {
        HandleClose();
    }
    else if (n < 0)
    {
        HandleError();
    }
}

void Fire::TcpConnection::HandleWrite()
{
    LOG(INFO) << "Enter write handler by: [" << clientAddr.GetUrlString() << "]";
    if (outputBuffer.readableBytes() == 0 || state != STATE::connected)
        return;
    ssize_t n = write(connChannel.GetMonitorFd(), outputBuffer.StartPoint(), outputBuffer.readableBytes());
    if (n < 0)
    {
        LOG(ERROR) << "Error: Write data failed. Reason: " << strerror(errno);
        n = 0;
    }
    outputBuffer.Remove(n);
    if (outputBuffer.readableBytes() == 0) // Finish writing
    {
        if (connChannel.GetTriggerMode() == Channel::TriggerMode::level_mode)
            connChannel.disableWriteCallback();
        if (writeCalback)
            writeCalback(shared_from_this());
        if (state == STATE::cloing)
            HandleClose();
    }
    else
    {
        LOG(INFO) << "Going to write more data: " << outputBuffer.readableBytes() << " bytes";
    }
}

void Fire::TcpConnection::HandleClose()
{
    LOG(INFO) << "one connection closed from Ip: " << clientAddr.GetIpString() << " Port: " << clientAddr.GetPort();
    CHECK(state == STATE::connected || state == STATE::cloing);
    connDestroyed();
    closeCallback(shared_from_this());
}

void Fire::TcpConnection::HandleError()
{
    LOG(ERROR) << "ERROR: TcpConnection has error occurs";
}

void Fire::TcpConnection::Shutdown()
{
    event_loop->runInLoop([this]()
                          {
                              if (state != STATE::connected)
                                  return;
                              state = STATE::cloing;
                              if (outputBuffer.readableBytes() == 0)
                                  HandleClose();
                          });
}

Fire::EventLoop *Fire::TcpConnection::GetLoop()
{
    return event_loop;
}

void Fire::TcpConnection::setWriteCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&cb)
{
    writeCalback = cb;
}

void Fire::TcpConnection::send(const std::string &msg)
{
    if (state != STATE::connected)
        return;
    event_loop->runInLoop([this, msg]()
                          {
                              if (outputBuffer.readableBytes() == 0)
                              {
                                  ssize_t n = write(connChannel.GetMonitorFd(), msg.data(), msg.size());
                                  if (n < 0)
                                  {
                                      LOG(ERROR) << "ERROR: Write data failed. Reason: " << strerror(errno);
                                      n = 0;
                                  }
                                  if (n < msg.size())
                                  {
                                      outputBuffer.Appand(msg.data() + n, msg.size() - n);
                                      if (!(connChannel.GetRegisterStatus() & Channel::RegisterStatusMask::write))
                                          connChannel.enableWriteCallback();
                                      LOG(INFO) << "Going to write more data: " << msg.size() - n << " bytes";
                                  }
                                  else
                                  {
                                      if (writeCalback)
                                          writeCalback(shared_from_this());
                                  }
                              }
                              else
                              {
                                  outputBuffer.Appand(msg.data(), msg.size());
                              }
                          });
}

void Fire::TcpConnection::setConnectionCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&cb)
{
    connectionCallback = cb;
}

void Fire::TcpConnection::setMessageCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>, const char *, ssize_t)> &&cb)
{
    messageCallback = cb;
}

void Fire::TcpConnection::setCloseCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&cb)
{
    closeCallback = cb;
}

Fire::NetAddr Fire::TcpConnection::GetPeerAddr() const
{
    return clientAddr;
}

void Fire::TcpConnection::connDestroyed()
{
    state = STATE::closed;
    if (connectionCallback)
        connectionCallback(shared_from_this());
    // connChannel.disableAll();
    event_loop->removeChannel(&connChannel);
    connChannel.clearCallback();
    Socket::closeSocket(connChannel.GetMonitorFd());
}

void Fire::TcpConnection::Established()
{
    event_loop->runInLoop([this]()
                          {
                              CHECK(state == STATE::connecting);
                              state = STATE::connected;
                              connChannel.enableReadCallback();
                              if (connectionCallback)
                                  connectionCallback(shared_from_this());
                          });
}

Fire::TcpConnection::STATE Fire::TcpConnection::connectionState()
{
    return state;
}