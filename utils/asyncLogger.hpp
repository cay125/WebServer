//
// Created by xiangpu on 20-3-11.
//

#ifndef FIRESERVER_ASYNCLOGGER_HPP
#define FIRESERVER_ASYNCLOGGER_HPP

#include "LogFile.hpp"
#include <string.h>
#include <vector>
#include <thread>
#include <condition_variable>

namespace Fire
{
    class fixedBuffer
    {
    public:
        fixedBuffer(fixedBuffer &) = delete;

        fixedBuffer &operator=(fixedBuffer &) = delete;

        fixedBuffer();

        void appand(const char *_data, int len)
        {
            if (getRestSpace() >= len)
                memcpy(currentPtr, _data, len);
            currentPtr += len;
        }

        const char *getData()
        { return data; };

        int getCurrentLength()
        { return static_cast<int>(currentPtr - data); }

        int getRestSpace()
        { return static_cast<int>(data + sizeof(data) - currentPtr); }

        void setZero()
        { memset(data, 0, sizeof(data)); }

        void reset()
        { currentPtr = data; }

    private:
        char data[4096] = {0};
        char *currentPtr;
    };

    class LogStream
    {
    public:
        LogStream &operator<<(short d)
        {
            makeString(d);
            return *this;
        }

        LogStream &operator<<(unsigned short d)
        {
            makeString(d);
            return *this;
        }

        LogStream &operator<<(int d)
        {
            makeString(d);
            return *this;
        }

        LogStream &operator<<(unsigned int d)
        {
            makeString(d);
            return *this;
        }

        LogStream &operator<<(long d)
        {
            makeString(d);
            return *this;
        }

        LogStream &operator<<(unsigned long d)
        {
            makeString(d);
            return *this;
        }

        LogStream &operator<<(long long d)
        {
            makeString(d);
            return *this;
        }

        LogStream &operator<<(unsigned long long d)
        {
            makeString(d);
            return *this;
        }

        LogStream &operator<<(char d)
        {
            makeString(d);
            return *this;
        }

        LogStream &operator<<(unsigned char d)
        {
            makeString(d);
            return *this;
        }

        LogStream &operator<<(const std::string &d)
        {
            buffer.appand(d.c_str(), d.length());
            return *this;
        }

        fixedBuffer buffer;
    private:
        template<class T>
        void makeString(T d)
        {
            std::string msg = std::to_string(d);
            buffer.appand(msg.c_str(), msg.length());
        }
    };

    class Logger
    {
    public:
        ~Logger();

        Logger(const char *_filename, int _line);

        LogStream log_stream;
    private:
        void formate_time();

        const char *filename = nullptr;
        int line = 0;
    };


    class asyncLogger
    {
    public:
        asyncLogger(asyncLogger &) = delete;

        asyncLogger &operator=(asyncLogger &) = delete;

        asyncLogger(std::string filename, int flush_times);

        void start();

        void appand(const char *_data, int len);

    private:
        void asyncFcn();

        LogFile log_file;
        std::thread writer_thread;
        std::shared_ptr<Fire::fixedBuffer> currentBuffer;
        std::shared_ptr<Fire::fixedBuffer> nextBuffer;
        std::vector<std::shared_ptr<Fire::fixedBuffer>> buffersWaitWriter;
        std::mutex m_mutex;
        std::condition_variable cond;
        bool running;
    };

}
#define FLOG Fire::Logger(__FILE__,__LINE__).log_stream

#endif //FIRESERVER_ASYNCLOGGER_HPP
