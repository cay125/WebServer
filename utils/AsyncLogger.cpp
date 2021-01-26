//
// Created by xiangpu on 20-3-11.
//
#include <functional>
#include <iostream>
#include <sys/time.h>

#include "utils/AsyncLogger.hpp"

Fire::fixedBuffer::fixedBuffer() : currentPtr(data)
{

}

Fire::Logger::Logger(const char *_filename, int _line)
{
    formate_time();
    filename = _filename;
    line = _line;
}

static Fire::asyncLogger *async_logger;
std::once_flag m_flag;

static void output(const char *_data, int _len)
{
    std::call_once(m_flag, []()
    {
        async_logger = new Fire::asyncLogger("log.txt", 1024);
        async_logger->start();
        std::cout << "Log system start!\n";
    });
    async_logger->appand(_data, _len);
}

Fire::Logger::~Logger()
{
    log_stream << " --- " << filename << " : " << line << "\n";
    output(log_stream.buffer.getData(), log_stream.buffer.getCurrentLength());
}

void Fire::Logger::formate_time()
{
    timeval tv = {0, 0};
    time_t time;
    char str_t[26] = {0};
    gettimeofday(&tv, nullptr);
    time = tv.tv_sec;
    tm *p_time = localtime(&time);
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S ", p_time);
    log_stream << str_t;
}

Fire::asyncLogger::asyncLogger(std::string filename, int flush_times) : log_file(std::move(filename), flush_times), currentBuffer(new fixedBuffer),
                                                                        nextBuffer(new fixedBuffer), running(true)
{

}

void Fire::asyncLogger::start()
{
    writer_thread = std::thread(std::bind(&Fire::asyncLogger::asyncFcn, this));
    if (writer_thread.joinable())
        writer_thread.detach();
    else
        std::cout << "Error: can not detach thread\n";
}

void Fire::asyncLogger::appand(const char *_data, int len)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    if (currentBuffer->getRestSpace() < len)
    {
        buffersWaitWriter.push_back(currentBuffer);
        currentBuffer.reset();
        if (nextBuffer)
            currentBuffer = std::move(nextBuffer);
        else
            currentBuffer = std::make_shared<fixedBuffer>();
        cond.notify_one();
    }
    currentBuffer->appand(_data, len);
}

void Fire::asyncLogger::asyncFcn()
{
    std::shared_ptr<fixedBuffer> writerBuffer1(new fixedBuffer);
    std::shared_ptr<fixedBuffer> writerBuffer2(new fixedBuffer);
    std::vector<std::shared_ptr<fixedBuffer>> buffersToWriter;
    while (running)
    {
        if (!buffersToWriter.empty() || !writerBuffer1 || !writerBuffer2 || writerBuffer1->getCurrentLength() || writerBuffer2->getCurrentLength())
            std::cout << "Error: " << "Writing log failed\n";
        {
            std::unique_lock<std::mutex> lk(m_mutex);
            if (buffersWaitWriter.empty())
                cond.wait_for(lk, std::chrono::seconds(1));
            buffersWaitWriter.push_back(currentBuffer);
            currentBuffer.reset();
            currentBuffer = std::move(writerBuffer1);
            if (!nextBuffer)
                nextBuffer = std::move(writerBuffer2);
            buffersToWriter.swap(buffersWaitWriter);
        }
        if (buffersToWriter.size() > 20) //to much data to write!
        {
            std::cout << "to much data!\n";
            buffersToWriter.erase(buffersToWriter.begin() + 2, buffersToWriter.end());
        }
        for (auto &buffer:buffersToWriter)
        {
            log_file.append(buffer->getData(), buffer->getCurrentLength());
            buffer->reset();
        }
        if (!writerBuffer1)
        {
            writerBuffer1 = std::move(buffersToWriter.back());
            buffersToWriter.pop_back();
        }
        if (!writerBuffer2)
        {
            writerBuffer2 = std::move(buffersToWriter.back());
            buffersToWriter.pop_back();
        }
        buffersToWriter.clear();
        log_file.flush();
    }
    log_file.flush();
}

