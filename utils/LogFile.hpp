//
// Created by xiangpu on 20-3-10.
//

#ifndef FIRESERVER_LOGFILE_HPP
#define FIRESERVER_LOGFILE_HPP

#include <stdio.h>
#include <string>
#include <mutex>
#include <memory>

namespace Fire
{
    class fileWriter
    {
    public:
        explicit fileWriter(std::string filename);

        ~fileWriter();

        void append(const char *logLine, const size_t len);

        void flush();

    private:
        size_t write(const char *logLine, size_t len);

        FILE *fp;
        char buffer[64 * 1024];
    };

    class LogFile
    {
    public:
        LogFile(LogFile &) = delete;

        LogFile &operator=(LogFile &) = delete;

        explicit LogFile(std::string _filename, int _flush_times = 1024);

        ~LogFile();

        void append(const char *logLine, int len);

        void flush();

    private:
        void append_unlocked(const char *logline, int len);

        const std::string filename;
        const int flush_times;

        int count;
        std::mutex m_mutex;
        std::unique_ptr<Fire::fileWriter> file_writer;
    };
}

#endif //FIRESERVER_LOGFILE_HPP
