#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include <coroutine>
#include <regex>
#include <string>

#include "Coroutine.h"
#include "HttpMessage.h"
#include "TcpClient.h"

class HttpRequest : public HttpMessage
{
public:
    enum Method
    {
        get, head, post, put, delet, options, trace
    };

    Method getMethod() const { return method; }
    const std::string &getUriBase() const { return uriBase; }
    const std::string &getQuery() const { return query; }
    const std::string &getHttpVersion() const { return httpVersion; }
    const std::string &getPathParam(const std::string &name) const
    {
        return pathParams.at(name);
    }
    bool hasPathParam(const std::string &name) const
    {
        return pathParams.find(name) != pathParams.end();
    }
    const std::string &getContentType() const { return rawHeaders.at("Content-type"); }
    Method getmethod() const { return method; }
    bool hasCookie(const std::string &name) const
    {
        return cookies.find(name) != cookies.end();
    }
    const std::string &getCookie(const std::string &name) const { return cookies.at(name); }
    bool isMalformed() { return malformed; }
    using HttpMessage::getHeader;
    using HttpMessage::hasHeader;

    class JsonParseError : public std::invalid_argument
    {
    public:
        JsonParseError(const json::parse_error &e) : invalid_argument(e.what()) {}
    };
    const json &getBodyJson()
    {
        if (!jsonBody)
        {
            try {
                jsonBody = {json::parse(body)};
            } catch (const json::parse_error &e) {
                throw JsonParseError(e);
            }
        }
        return jsonBody.value();
    }

    inline static const std::string noContentTypeString = "UNKNOWN_CONTENT_TYPE";

private:

    bool malformed = false;

    Method method;
    std::string uriBase;
    std::string query;
    std::string part;
    std::string httpVersion;

    std::string unparsedHeader;
    const std::string &getUnparsedHeader()
    {
        return unparsedHeader;
    }

    std::unordered_map<std::string, std::string> pathParams;
    std::unordered_map<std::string, std::string> cookies;

    void setUri(std::string&& uri)
    {
        // special case allowed only for OPTIONS requests
        if (method == Method::options && uri == "*")
        {
            uriBase = uri;
            return;
        }

        static const std::regex uriRegex{"(.*:\\/\\/[^\\/]*)?(/[^\\?#]*)(\\?([^#]*))?(#(.*))?"};
        std::smatch match;
        if (std::regex_match(uri, match, uriRegex))
        {
            uriBase = match.str(2);
            if (uriBase.back() == '/')
                uriBase.pop_back();
            query = match.str(4);
            part = match.str(6);
        }
        else
            malformed = true;
    }

    inline static const std::unordered_map<std::string, Method> toMethod = {
        {"GET", get},
        {"HEAD", head},
        {"POST", post},
        {"PUT", put},
        {"DELETE", delet},
        {"OPTIONS", options},
        {"TRACE", trace}
    };
    inline static const std::unordered_map<Method, std::string> methodToString {
        {get, "GET"},
        {head, "HEAD"},
        {put, "PUT"},
        {post, "POST"},
        {delet, "DELETE"}
    };

    void parseRequestLine(const std::string& line)
    {
        static const std::regex startLineRegex{"([^ ]+) ([^ ]+) ([^ ]+)"};
        std::smatch match;
        if (std::regex_match(line, match, startLineRegex))
        {
            try {
                method = toMethod.at(match.str(1));
            } catch (std::out_of_range &e) {
                DEBUG << "malformed request - bad method";
                malformed = true;
            }
            setUri(match.str(2));
            httpVersion = match.str(3);
        }
        else
        {
            DEBUG << "malformed request - first line regex does not match";
            malformed = true;
        }
    }

    static Coroutine<HttpRequest> fetchRequest(TcpClient &tcpClient, std::string &buffer)
    {
        // TODO proper error handling -> server MUST return 400 on malformed request
        size_t headerEnd;
        while (((headerEnd = buffer.find(crlf2, headerEnd))) == std::string::npos)
        {
            headerEnd = buffer.size();
            auto readCoroutine = tcpClient.receive();
            iterative_co_await(readCoroutine);
            buffer += readCoroutine.value();
        }
        DEBUG << "header fetched";
        std::string header = buffer.substr(0, headerEnd + crlf.size());
        buffer = buffer.substr(headerEnd + crlf2.size());
        
        DEBUG << header;

        HttpRequest request;

        std::vector<std::string> headerLines = split(header);

        request.parseRequestLine(headerLines[0]);
        if (request.malformed)
            co_return request;

        // save header in unparsed form only for TRACE requests
        if (request.getMethod() == trace)
            request.unparsedHeader = header;

        std::string lastHeader;
        for (int i = 1; i < (int)headerLines.size(); i++)
        {
            const std::string &line = headerLines[i];
            if (line[0] == '\t' || line[0] == ' ')
                request.rawHeaders[lastHeader] += line;
            else
            {
                size_t splitIndex = line.find(':');
                if (splitIndex != std::string::npos)
                {
                    lastHeader = line.substr(0, splitIndex);
                    request.rawHeaders[lastHeader] += line.substr(splitIndex + 1);
                }
                else
                {
                    request.malformed = true;
                    DEBUG << "malformed request - bad header line";
                    co_return request;
                }
            }
        }

        for (auto &[key, value] : request.rawHeaders)
            value = removeWhitespaces(std::move(value));

        if (request.method != head && request.method != get)
        {
            // TODO server MUST support chunked transfer encoding
            if (request.hasHeader("Content-Length"))
            {
                size_t bodyLength = size_t(std::stoi(request.getHeader("Content-Length")));
                while (buffer.size() < bodyLength)
                {
                    auto readCoroutine = tcpClient.receive();
                    iterative_co_await(readCoroutine);
                    buffer += readCoroutine.value();
                }

                request.body = buffer.substr(0, bodyLength);
                buffer.erase(0, bodyLength);

                DEBUG << "body fetched:" << request.body;
            }
        }

        if (request.rawHeaders.find("Content-type") == request.rawHeaders.end())
            request.rawHeaders.insert({"Content-type", noContentTypeString});

        if (request.hasHeader("Cookie"))
        {
            static std::regex cookieRegex("([a-zA-Z0-9!#$%^&*'~\\-._+`|]*)=(.*?)($|;|,(?! ))");
            auto begin = std::sregex_iterator(request.getHeader("Cookie").begin(), request.getHeader("Cookie").end(), cookieRegex);
            auto end = std::sregex_iterator();
            for (std::sregex_iterator it = begin; it != end; it++)
                request.cookies[(*it)[1]] = (*it)[2];
        }

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