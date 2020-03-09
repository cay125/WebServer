//
// Created by xiangpu on 20-3-8.
//
#include "HttpData.hpp"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

std::map<std::string, std::string> Fire::App::cType::suffix2type;

Fire::App::HttpData::HttpData(std::string _root_dir) : root_dir(_root_dir)
{
    if (!root_dir.is_absolute())
        root_dir = fs::canonical(root_dir);
    cType::typeInit();
}

void Fire::App::HttpData::HandleRead(std::shared_ptr<Fire::TcpConnection> p, const char *buf, ssize_t len)
{
    if (parserHttpMsg(std::string(buf)) == STATUS_ERROR)
    {
        response.status_code = httpResponse::StatusCode::c400BadRequest;
        response.status_msg = "Bad Request";
    }
    else
    {
        analyseRequest();
    }
    std::string outputBuf;
    response.makeString(outputBuf);
    p->send(outputBuf);
    reset();
}

Fire::App::HttpData::STATUS Fire::App::HttpData::parserHttpMsg(std::string msg)
{
    ParserStatus parser_status = ParserStatus::parser_request;
    unsigned long pos = 0, currentIndex = 0;
    while (parser_status != parser_finish)
    {
        if (parser_status == ParserStatus::parser_request)
        {
            pos = msg.find("\r\n", currentIndex);
            if (pos == std::string::npos)
                return STATUS::STATUS_ERROR;
            if (parserRequest(msg.substr(currentIndex, pos)) == STATUS::STATUS_SUCCESS)
                parser_status = ParserStatus::parser_head;
            else
                return STATUS_ERROR;
            currentIndex = pos + 2;
        }
        else if (parser_status == ParserStatus::parser_head)
        {
            pos = msg.find("\r\n", currentIndex);
            if (pos == std::string::npos)
                return STATUS_ERROR;
            if (pos == currentIndex) //means no head line now
            {
                if (request.method == httpRequest::Method::POST)
                    parser_status = ParserStatus::parser_body;
                else
                    parser_status = ParserStatus::parser_finish;
            }
            else
            {
                if (parserHeader(msg.substr(currentIndex, pos - currentIndex)) != STATUS_SUCCESS)
                    return STATUS_ERROR;
                else
                    currentIndex = pos + 2;
            }
        }
        else if (parser_status == ParserStatus::parser_body)
        {
            //TODO: analyse body
            parser_status = ParserStatus::parser_finish;
        }
    }
    return STATUS::STATUS_SUCCESS;
}

Fire::App::HttpData::STATUS Fire::App::HttpData::parserRequest(std::string requestLine)
{
    auto pos = requestLine.find(' ');
    if (pos == std::string::npos)
        return STATUS_ERROR;
    std::string method_string = requestLine.substr(0, pos);
    if (method_string == "GET")
        request.method = httpRequest::Method::GET;
    else if (method_string == "PUT")
        request.method = httpRequest::Method::PUT;
    else if (method_string == "HEAD")
        request.method = httpRequest::Method::HEAD;
    else if (method_string == "POST")
        request.method = httpRequest::Method::POST;
    else
        return STATUS_ERROR;
    auto start = pos + 1;
    pos = requestLine.find(' ', start);
    if (pos == std::string::npos)
        return STATUS_ERROR;
    auto pos_sig = requestLine.substr(start, pos - start).find('?');
    if (pos_sig != std::string::npos)
    {
        request.file_name = requestLine.substr(start, pos_sig - start);
        request.query = requestLine.substr(pos_sig + 1, pos - pos_sig - 1);
    }
    else
    {
        request.file_name = requestLine.substr(start, pos - start);
    }
    if (request.file_name == "/")
        request.file_name = "/index.html";
    request.file_name = root_dir / request.file_name.substr(1);
    std::string version_string = requestLine.substr(pos + 1, requestLine.length() - pos - 1);
    if (version_string == "HTTP/1.1")
        request.version = httpRequest::Version::v11;
    else if (version_string == "HTTP/1.0")
        request.version = httpRequest::Version::v10;
    else
        request.version = httpRequest::Version::vunknown;

    return STATUS_SUCCESS;
}

Fire::App::HttpData::STATUS Fire::App::HttpData::parserHeader(std::string headLine)
{
    auto pos = headLine.find(' ');
    if (pos == std::string::npos)
        return STATUS_ERROR;
    request.headers[headLine.substr(0, pos - 1)] = headLine.substr(pos + 1, headLine.length() - pos - 1);
    return STATUS_SUCCESS;
}

Fire::App::HttpData::STATUS Fire::App::HttpData::analyseRequest()
{
    if (request.headers.find("Connection") != request.headers.end())
    {
        if (request.headers["Connection"] == "Keep-Alive" || request.headers["Connection"] == "keep-alive")
        {
            response.headers["Connection"] = " Keep-Alive";
            response.headers["Keep-Alive"] = "timeout=" + std::to_string(DEFAULT_ALIVE_TIME);
        }
    }
    if (request.method == httpRequest::Method::GET || request.method == httpRequest::Method::HEAD)
    {
        if (!fs::exists(request.file_name))
        {
            response.headers["Connection"] = "Close";
            response.headers.erase("Keep-Alive");
            response.status_code = httpResponse::StatusCode::c404NotFound;
            response.status_msg = "Not Found";
            return STATUS_ERROR;
        }
        response.status_code = httpResponse::StatusCode::c200ok;
        response.status_msg = "OK";
        auto pos = request.file_name.find('.');
        if (pos == std::string::npos)
            response.headers["Content-Type"] = cType::GetType("default");
        else
            response.headers["Content-Type"] = cType::GetType(request.file_name.substr(pos + 1));
        auto file_size = fs::file_size(request.file_name);
        response.headers["Content-Length"] = std::to_string(file_size);
        if (request.method == httpRequest::Method::HEAD)
            return STATUS_SUCCESS;
        int fd = open(request.file_name.c_str(), O_RDONLY, 0);
        void *mmapRet = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
        char *file_addr = static_cast<char *>(mmapRet);
        response.body += std::string(file_addr, file_addr + file_size);
        munmap(mmapRet, file_size);
        return STATUS_SUCCESS;
    }
    else if (request.method == httpRequest::Method::POST)
    {
        //TODO: process POST request
    }
    return STATUS_SUCCESS;
}

void Fire::App::HttpData::reset()
{
    request.file_name.clear();
    request.body.clear();
    request.query.clear();
    request.headers.clear();

    response.status_msg.clear();
    response.body.clear();
    response.headers.clear();
}

void Fire::App::httpResponse::makeString(std::string &msg)
{
    msg += "HTTP/1.1 " + std::to_string(status_code) + " " + status_msg + "\r\n";
    msg += "Server: fire\r\n";
    for (const auto &header:headers)
    {
        msg += header.first;
        msg += ": ";
        msg += header.second;
        msg += "\r\n";
    }
    msg += "\r\n";
    msg += body;
}

void Fire::App::cType::typeInit()
{
    suffix2type[".html"] = "text/html";
    suffix2type[".avi"] = "video/x-msvideo";
    suffix2type[".bmp"] = "image/bmp";
    suffix2type[".c"] = "text/plain";
    suffix2type[".doc"] = "application/msword";
    suffix2type[".gif"] = "image/gif";
    suffix2type[".gz"] = "application/x-gzip";
    suffix2type[".htm"] = "text/html";
    suffix2type[".ico"] = "image/x-icon";
    suffix2type[".jpg"] = "image/jpeg";
    suffix2type[".png"] = "image/png";
    suffix2type[".txt"] = "text/plain";
    suffix2type[".mp3"] = "audio/mp3";
    suffix2type["default"] = "text/html";
}

std::string Fire::App::cType::GetType(std::string suffix)
{
    if (suffix2type.find(suffix) == suffix2type.end())
        return suffix2type[std::string("default")];
    else
        return suffix2type[suffix];
}