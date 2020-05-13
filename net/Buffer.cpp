//
// Created by xiangpu on 20-3-3.
//
#include "Buffer.hpp"
#include <unistd.h>
#include <sys/uio.h>

Fire::Buffer::Buffer(ulong size) : data(size), readIndex(0), writeIndex(0)
{

}

unsigned long Fire::Buffer::GetSize()
{
    return data.size();
}

ssize_t Fire::Buffer::readFromFd(int fd)
{
    char buf[65536] = {0};
    ulong restLen = data.size() - writeIndex;
    iovec vec[2];
    vec[0].iov_base = &*data.begin() + writeIndex;
    vec[0].iov_len = restLen;
    vec[1].iov_base = buf;
    vec[1].iov_len = sizeof(buf);
    ssize_t n = readv(fd, vec, 2);
    if (n > restLen)
    {
        data.resize(n - restLen + data.size());
        std::copy(buf, buf + n - restLen, &*data.begin() + data.size());
    }
    writeIndex += n;
    return n;
}

void Fire::Buffer::Appand(const char *_data, int len)
{
    if ((data.size() - writeIndex) < len)
        data.resize(len + writeIndex);
    std::copy(_data, _data + len, &*data.begin() + writeIndex);
    writeIndex += len;
}

const char *Fire::Buffer::StartPoint()
{
    return &*data.begin() + readIndex;
}

ulong Fire::Buffer::readableBytes()
{
    return writeIndex - readIndex;
}

void Fire::Buffer::Remove(ssize_t n)
{
    readIndex += n;
    if (readableBytes() == 0)
    {
        writeIndex = 0;
        readIndex = 0;
    }
}
