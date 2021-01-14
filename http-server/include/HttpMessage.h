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
            return it->first != "closed";
        else
            return false;
    }

    const std::string &getBody() { return body; }
    const json &getBodyJson()
    {
        if (!jsonBody)
            jsonBody = {json::parse(body)};
        return jsonBody.value();
    }

protected:
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
