//
// Created by xiangpu on 20-3-8.
//
#include "HttpData.hpp"
#include <iostream>

void Fire::App::HttpData::HandleRead(std::shared_ptr<Fire::TcpConnection> p, const char *buf, ssize_t len)
{
    parserHttpMsg(std::string(buf));
}

Fire::App::HttpData::STATUS Fire::App::HttpData::parserHttpMsg(std::string msg)
{
    auto pos = msg.find("\r\n");
    if (pos == std::string::npos)
        return STATUS::STATUS_ERROR;
    parserRequest(msg.substr(0, pos));
    auto start = pos + 1;
    pos = msg.find("\r\n", start);
    parserHeader(msg.substr(start, pos - start));
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
    if (pos_sig == std::string::npos)
    {
        request.file_name = requestLine.substr(start, pos_sig - start);
        request.query = requestLine.substr(pos_sig + 1, pos - pos_sig - 1);
    }
    else
    {
        request.file_name = requestLine.substr(start, pos);
    }
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

}

