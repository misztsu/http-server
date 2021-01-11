#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include <coroutine>
#include <regex>

#include "Coroutine.h"
#include "HttpMessage.h"
#include "TcpClient.h"

class HttpRequest : public HttpMessage
{
public:
    enum Method
    {
        get
    };

    const std::string &getUriBase() const { return uri; }
    const std::string &getQuery() const { return query; }
    const std::string &getHttpVersion() const { return httpVersion; }
    const std::string &getPathParam(const std::string &name) const
    {
        return pathParams.at(name);
    }
    Method getmethod() const { return method; }
    using HttpMessage::getHeader;

private:

    Method method;
    std::string uri;
    std::string uriBase;
    std::string query;
    std::string part;
    std::string httpVersion;

    std::unordered_map<std::string, std::string> pathParams;

    void setUri(std::string&& uri)
    {
        static const std::regex uriRegex{"(.*:\\/\\/[^\\/]*)?(/[^\\?#]*)(\\?([^#]*))?(#(.*))?"};
        std::smatch match;
        if (!std::regex_match(uri, match, uriRegex))
            throw std::runtime_error("error while parsing URI");

        uriBase = match.str(2);
        query = match.str(4);
        part = match.str(6);
        this->uri = std::move(uri);
    }

    inline static const std::unordered_map<std::string, Method> toMethod = {
        {"GET", get}
    };

    void setRequestLine(const std::string& line)
    {
        static const std::regex startLineRegex{"([^ ]+) ([^ ]+) ([^ ]+)"};
        std::smatch match;
        std::regex_match(line, match, startLineRegex);
        method = toMethod.at(match.str(1));
        setUri(match.str(2));
        httpVersion = match.str(3);
    }

    static Coroutine<HttpRequest> fetchRequest(TcpClient &tcpClient, std::string &buffer)
    {
        // TODO proper error handling -> server MUST return 400 on malformed request
        size_t headerEnd;
        while(((headerEnd = buffer.find(crlf2, headerEnd))) == std::string::npos)
        {
            headerEnd = buffer.size();
            auto readCoroutine = tcpClient.receive();
            iterative_co_await(readCoroutine);
            buffer += readCoroutine.value();
        }
        DEBUG << "header fetched";
        std::string header = buffer.substr(0, headerEnd + crlf.size());
        buffer = buffer.substr(headerEnd + crlf2.size());

        HttpRequest request;

        std::vector<std::string> headerLines = split(header);

        request.setRequestLine(headerLines[0]);

        std::string lastHeader;
        for (int i = 1; i < (int)headerLines.size(); i++)
        {
            const std::string &line = headerLines[i];
            if (line[0] == '\t' || line[0] == ' ')
                request.rawHeaders[lastHeader] += line;
            else
            {
                int splitIndex = line.find(':');
                lastHeader = line.substr(0, splitIndex - 1);
                request.rawHeaders[lastHeader] += line.substr(splitIndex + 1);
            }
        }

        for (auto &[key, value] : request.rawHeaders)
            value = removeWhitespaces(std::move(value));
        
        // TODO fetch request body

        co_return request;
    }

    static std::vector<std::string> split(const std::string &string, const std::string& delim = crlf)
    {
        std::vector<std::string> vector;
        size_t index = 0;
        while (index < string.size())
        {
            size_t newIndex = string.find(delim, index);
            if (newIndex == std::string::npos)
                newIndex = string.size();
            if (index != newIndex)
                vector.push_back(string.substr(index, newIndex - index));
            index = newIndex + delim.size();
        }
        return vector;
    }

    static std::string removeWhitespaces(std::string &&string)
    {
        static const std::regex whitespaces{"[\n\t\r]+"};
        string = std::regex_replace(string, whitespaces, " ");
        if (string.back() == ' ')
            string.pop_back();
        if (string[0] == ' ')
            string = string.substr(1);
        return string;
    }

    friend class HttpServer;
    friend class RequestHandler;
};


#endif