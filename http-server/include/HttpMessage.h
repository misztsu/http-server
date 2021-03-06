#ifndef HTTPMESSAGE_H_
#define HTTPMESSAGE_H_

#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class HttpMessage
{
public:

    inline static const std::string crlf = "\r\n";
    inline static const std::string crlf2 = "\r\n\r\n";

    bool isPersistentConnection()
    {
        auto it = rawHeaders.find("Connection");
        if (it != rawHeaders.end())
            return it->second != "close";
        else
            return false;
    }

    void closeConnection()
    {
        rawHeaders["Connection"] = "close";
    }

    const std::string &getBody() { return body; }

protected:

    HttpMessage(std::unordered_map<std::string, std::string> defaultHeaders = {}) : rawHeaders(std::move(defaultHeaders)) {}

    std::unordered_map<std::string, std::string> rawHeaders;
    std::string body;
    std::optional<json> jsonBody = {};

    bool hasHeader(const std::string &header) const
    {
        return rawHeaders.find(header) != rawHeaders.end();
    }
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
