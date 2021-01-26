//
// Created by xiangpu on 20-3-10.
//
#include "utils/LogFile.hpp"

Fire::fileWriter::fileWriter(std::string filename) : fp(fopen(filename.c_str(), "ae"))
{
    setbuffer(fp, buffer, sizeof(buffer));
}


Fire::fileWriter::~fileWriter()
{ fclose(fp); }

void Fire::fileWriter::append(const char *logLine, const size_t len)
{
    size_t n = this->write(logLine, len);
    size_t remain = len - n;
    while (remain > 0)
    {
        size_t x = this->write(logLine + n, remain);
        if (x == 0)
        {
            int err = ferror(fp);
            if (err)
                fprintf(stderr, "Fire::fileWriter::append() failed !\n");
            break;
        }
        n += x;
        remain = len - n;
    }
}

void Fire::fileWriter::flush()
{ fflush(fp); }

size_t Fire::fileWriter::write(const char *logLine, size_t len)
{
    return fwrite_unlocked(logLine, 1, len, fp);
}


Fire::LogFile::LogFile(std::string _filename, int _flush_times) : filename(_filename), flush_times(_flush_times), count(0),
                                                                  file_writer(new fileWriter(_filename))
{
}

Fire::LogFile::~LogFile()
{}

void Fire::LogFile::append(const char *logLine, int len)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    append_unlocked(logLine, len);
}

void Fire::LogFile::flush()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    file_writer->flush();
}

void Fire::LogFile::append_unlocked(const char *logLine, int len)
{
    file_writer->append(logLine, len);
    ++count;
    if (count >= flush_times)
    {
        count = 0;
        file_writer->flush();
    }
}
