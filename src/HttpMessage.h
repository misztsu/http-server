#ifndef HTTPMESSAGE_H_
#define HTTPMESSAGE_H_

#include <string>

class HttpMessage
{
public:

    inline static const std::string crlf = "\r\n";
    inline static const std::string crlf2 = "\r\n\r\n";

protected:
    std::unordered_map<std::string, std::string> rawHeaders;
    std::string body;

    const std::string &getHeader(const std::string &header) const
    {
        return rawHeaders.at(header);
    }
    void setHeader(const std::string &header, const std::string &value)
    {
        rawHeaders[header] = value;
    }
};

#endif
