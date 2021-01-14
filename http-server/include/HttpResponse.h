
#ifndef HTTPRESPONSE_H_
#define HTTPRESPONSE_H_

#include <unordered_map>

#include "Coroutine.h"
#include "HttpMessage.h"
#include "TcpClient.h"

class HttpResponse : public HttpMessage
{
    
public:
    enum Status
    {
        Continue = 100,
        Switching_Protocols = 101,
        OK = 200,
        Created = 201,
        Accepted = 202,
        Non_Authoritative_Information = 203,
        No_Content = 204,
        Reset_Content = 205,
        Partial_Content = 206,
        Multiple_Choices = 300,
        Moved_Permanently = 301,
        Found = 302,
        See_Other = 303,
        Not_Modified = 304,
        Use_Proxy = 305,
        Temporary_Redirect = 307,
        Bad_Request = 400,
        Unauthorized = 401,
        Payment_Required = 402,
        Forbidden = 403,
        Not_Found = 404,
        Method_Not_Allowed = 405,
        Not_Acceptable = 406,
        Proxy_Authentication_Required = 407,
        Request_Time_out = 408,
        Conflict = 409,
        Gone = 410,
        Length_Required = 411,
        Precondition_Failed = 412,
        Request_Entity_Too_Large = 413,
        Request_URI_Too_Large = 414,
        Unsupported_Media_Type = 415,
        Requested_range_not_satisfiable = 416,
        Expectation_Failed = 417,
        Im_a_teapot = 418,
        Internal_Server_Error = 500,
        Not_Implemented = 501,
        Bad_Gateway = 502,
        Service_Unavailable = 503,
        Gateway_Time_out = 504,
        HTTP_Version_not_supported = 505
    };
    static inline const std::unordered_map<Status, std::string> statusToLine = {
        {Continue, "100 Continue"},
        {Switching_Protocols, "101 Switching Protocols"},
        {OK, "200 OK"},
        {Created, "201 Created"},
        {Accepted, "202 Accepted"},
        {Non_Authoritative_Information, "203 Non Authoritative Information"},
        {No_Content, "204 No Content"},
        {Reset_Content, "205 Reset Content"},
        {Partial_Content, "206 Partial Content"},
        {Multiple_Choices, "300 Multiple Choices"},
        {Moved_Permanently, "301 Moved Permanently"},
        {Found, "302 Found"},
        {See_Other, "303 See Other"},
        {Not_Modified, "304 Not Modified"},
        {Use_Proxy, "305 Use Proxy"},
        {Temporary_Redirect, "307 Temporary Redirect"},
        {Bad_Request, "400 Bad Request"},
        {Unauthorized, "401 Unauthorized"},
        {Payment_Required, "402 Payment Required"},
        {Forbidden, "403 Forbidden"},
        {Not_Found, "404 Not Found"},
        {Method_Not_Allowed, "405 Method Not Allowed"},
        {Not_Acceptable, "406 Not Acceptable"},
        {Proxy_Authentication_Required, "407 Proxy Authentication Required"},
        {Request_Time_out, "408 Request Time out"},
        {Conflict, "409 Conflict"},
        {Gone, "410 Gone"},
        {Length_Required, "411 Length Required"},
        {Precondition_Failed, "412 Precondition Failed"},
        {Request_Entity_Too_Large, "413 Request Entity Too Large"},
        {Request_URI_Too_Large, "414 Request URI Too Large"},
        {Unsupported_Media_Type, "415 Unsupported Media Type"},
        {Requested_range_not_satisfiable, "416 Requested range not satisfiable"},
        {Expectation_Failed, "417 Expectation Failed"},
        {Im_a_teapot, "418 I'm a teapot"},
        {Internal_Server_Error, "500 Internal Server Error"},
        {Not_Implemented, "501 Not Implemented"},
        {Bad_Gateway, "502 Bad Gateway"},
        {Service_Unavailable, "503 Service Unavailable"},
        {Gateway_Time_out, "504 Gateway Time out"},
        {HTTP_Version_not_supported, "505 HTTP Version not supported"}
    };
    static Status toStatus(int status) { return static_cast<Status>(status); }

    Status getStatus() const { return status; }
    HttpResponse &setStatus(Status status)
    {
        this->status = status;
        return *this;
    }
    HttpResponse &setStatus(int status)
    {
        this->status = toStatus(status);
        return *this;
    }

    HttpResponse &setBody(const std::string &body, const std::string &contentType = "text/html", std::vector<std::string> parameters = {"charset=utf-8"})
    {
        this->body = body;
        rawHeaders["Content-type"] = contentType;
        for (auto &parameter : parameters)
            rawHeaders["Content-type"] += "; " + parameter;
        return *this;
    }
    HttpResponse &setJsonBody(const json &jsonBody)
    {
        setBody(jsonBody.dump(), "application/json");
        return *this;
    }

    void send()
    {
        done = true;
    }

private:

    HttpResponse(HttpRequest &request, Status status = OK) : request(request), status(status) {}

    HttpRequest &request;
    Status status;

    bool done = false;

    friend class RequestHandler;
    bool isReady() { return done; }

    Coroutine<void> send(TcpClient &tcpClient)
    {
        if (!body.empty())
            rawHeaders["Content-Length"] = std::to_string(body.size());

        rawHeaders["Connection"] = request.getHeader("Connection");

        std::string message = "HTTP/1.1 " + statusToLine.at(status) + crlf;

        for (auto &[header, value] : rawHeaders)
            message += header + ": " + value + crlf;
        
        message += crlf;
        
        if (request.getMethod() != HttpRequest::Method::head)
            message += body;

        DEBUG << "Completed message:" << message;

        auto sendCoroutine = tcpClient.send(message);
        iterative_co_await(sendCoroutine);
    }
    friend class HttpServer;
};

#endif
