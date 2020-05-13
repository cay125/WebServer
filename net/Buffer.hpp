//
// Created by xiangpu on 20-3-3.
//

#ifndef FIRESERVER_BUFFER_HPP
#define FIRESERVER_BUFFER_HPP

#include <vector>
#include <monetary.h>

namespace Fire
{
    class Buffer
    {
        typedef unsigned long ulong;
    public:
        explicit Buffer(ulong size = 1024);

        unsigned long GetSize();

        ssize_t readFromFd(int fd);

        void Appand(const char *_data, int len);

        const char *StartPoint();

        void Remove(ssize_t n);

        ulong readableBytes();

    private:
        std::vector<char> data;
        ulong writeIndex, readIndex;
    };
}

#endif //FIRESERVER_BUFFER_HPP
