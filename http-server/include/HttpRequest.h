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
    bool isBodyNotFetched() { return bodyNotFetched; }
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

    static inline constexpr size_t maxHeaderLength = 1000000;
    static inline constexpr size_t maxBodyLength = 1000000;

private:

    bool malformed = false;
    bool bodyNotFetched = false;

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

    void setUri(std::string uri)
    {
        // special case allowed only for OPTIONS requests
        if (method == Method::options && uri == "*")
        {
            uriBase = uri;
            return;
        }

        size_t index = 0;
        while(((index = uri.find('%', index)) != std::string::npos))
        {
            if (index > uri.size() - 3)
            {
                malformed = true;
                return;
            }
            uri.replace(index, 3, std::string(1, static_cast<char>(stoi(uri.substr(index + 1, 2), 0, 16))));
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

    inline static constexpr size_t invalidIndex = -1;

    static Coroutine<void> readFor(TcpClient &client, std::string &buffer, const size_t length)
    {
        while (buffer.size() < length)
        {
            auto readCoroutine = client.receive();
            iterative_co_await(readCoroutine);
            buffer += readCoroutine.value();
        }
    }

    // returns the position of start of the end string or -1 if maxLength is exceeded
    static Coroutine<size_t> readUntil(TcpClient &client, std::string &buffer, const std::string &end, size_t maxLength)
    {
        size_t i;
        while (((i = buffer.find(end, i))) == std::string::npos)
        {
            i = buffer.size();
            if (buffer.size() > maxLength)
            {
                co_return -1;
            }
            auto readCoroutine = client.receive();
            iterative_co_await(readCoroutine);
            buffer += readCoroutine.value();
        }
        co_return i;
    }

    size_t getContentLength()
    {
        try { 
            return static_cast<size_t>(std::stoi(getHeader("Content-Length")));
        } catch (const std::invalid_argument &e) {
            malformed = true;
            return 0;
        }
    }

    void parseHeaders(const std::vector<std::string> &headerLines, size_t index = 0)
    {
        std::string lastHeader;
        for (size_t i = index; i < headerLines.size(); i++)
        {
            const std::string &line = headerLines[i];
            if (line[0] == '\t' || line[0] == ' ')
                rawHeaders[lastHeader] += line;
            else
            {
                size_t splitIndex = line.find(':');
                if (splitIndex != std::string::npos)
                {
                    lastHeader = line.substr(0, splitIndex);
                    rawHeaders[lastHeader] += line.substr(splitIndex + 1) + ' ';
                }
                else
                {
                    malformed = true;
                    return;
                }
            }
        }

        for (auto &[key, value] : rawHeaders)
            value = removeWhitespaces(std::move(value));
    }

    static Coroutine<HttpRequest> fetchRequest(TcpClient &tcpClient, std::string &buffer)
    {
        HttpRequest request;

        auto readCoroutine = readUntil(tcpClient, buffer, crlf2, maxHeaderLength); 
        iterative_co_await(readCoroutine);
        size_t headerEnd = readCoroutine.value();

        if (headerEnd == invalidIndex)
        {
            request.malformed = true;
            co_return request;
        }

        DEBUG << "header fetched";
        std::string header = buffer.substr(0, headerEnd + crlf.size());
        buffer = buffer.erase(0 , headerEnd + crlf2.size());
        
        DEBUG << header;

        std::vector<std::string> headerLines = split(header);

        request.parseRequestLine(headerLines[0]);
        if (request.malformed)
            co_return request;

        // save header in unparsed form only for TRACE requests
        if (request.getMethod() == trace)
            request.unparsedHeader = header;
        
        request.parseHeaders(headerLines, 1);
        if (request.malformed)
            co_return request;

        if (request.method != head && request.method != get)
        {
            if (request.hasHeader("Expect"))
            {
                if (request.hasHeader("Content-Length") && request.getContentLength() > maxBodyLength)
                {
                    request.bodyNotFetched = true;
                    co_return;
                }
                else // if (buffer.empty())
                    tcpClient.send("HTTP/1.1 100 Continue" + crlf2);
            }
            if (request.hasHeader("Transfer-Encoding") && request.getHeader("Transfer-Encoding") != "identity")
            {
                // chunked encoding
                while (true) // chunks
                {
                    if (request.body.size() > maxBodyLength)
                    {
                        request.bodyNotFetched = true;
                        co_return request;
                    }
                    auto rc = readUntil(tcpClient, buffer, crlf, maxBodyLength);
                    iterative_co_await(rc);
                    size_t index = rc.value();
                    if (index == invalidIndex)
                    {
                        request.malformed = true;
                        co_return request;
                    }
                    size_t chunkLength;
                    try {
                        chunkLength = stoul(buffer.substr(0, index), 0, 16);
                    } catch (const std::invalid_argument &e) {
                        request.malformed = true;
                        co_return request;
                    }
                    if (chunkLength == 0)
                    {
                        buffer.erase(0, index);
                        break;
                    }
                    else
                        buffer.erase(0, index + crlf.size());
                    auto rc2 = readFor(tcpClient, buffer, chunkLength + crlf.size());
                    iterative_co_await(rc2);
                    request.body += buffer.substr(0, chunkLength);
                    buffer.erase(0, chunkLength + crlf.size());
                }
                auto readCoroutine = readUntil(tcpClient, buffer, crlf2, maxHeaderLength); 
                iterative_co_await(readCoroutine);
                size_t trailerEnd = readCoroutine.value();
                std::string trailer = buffer.substr(0, trailerEnd + crlf.size());
                buffer = buffer.erase(0 , trailerEnd + crlf2.size());
                request.parseHeaders(split(trailer));
            }
            else if (request.hasHeader("Content-Length"))
            {
                size_t bodyLength = request.getContentLength(); 

                if (bodyLength > maxBodyLength)
                {
                    request.bodyNotFetched = true;
                    co_return request;
                }

                auto rc = readFor(tcpClient, buffer, bodyLength);
                iterative_co_await(rc);

                request.body = buffer.substr(0, bodyLength);
                buffer.erase(0, bodyLength);

                DEBUG << "body fetched:" << request.body;
            }
        }

        if (request.rawHeaders.find("Content-type") == request.rawHeaders.end())
            request.rawHeaders.emplace("Content-type", noContentTypeString);

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