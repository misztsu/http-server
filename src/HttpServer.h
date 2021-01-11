#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include <functional>
#include <future>
#include <vector>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "RequestHandler.h"
#include "TcpServer.h"
#include "TcpClient.h"

class HttpServer
{
public:
    HttpServer() : defaultCallback(default404) {}

    void setDefaultCallback(RequestHandler::CallbackType &&callback) { defaultCallback = std::move(callback); }

    void get(std::string &&path, RequestHandler::CallbackType &&callback)
    {
        handlers[HttpRequest::Method::get].emplace_back(path, callback);
    }

    [[noreturn]]
    void listen(int port = 80)
    {
        tcpServer.bind(port);
        tcpServer.setClientTaskCallback([this](auto client, auto notify) {
            return clientHandlingTask(std::move(client), std::move(notify));
        });
        DEBUG << "Starting server on port" << port;
        tcpServer.waitForever();
    }

private:
    TcpServer tcpServer;

    std::unordered_map<HttpRequest::Method, std::vector<RequestHandler>> handlers = {
        {HttpRequest::Method::get, {}}
    };

    Coroutine<void> clientHandlingTask(TcpClient&& client, Notify notify)
    {
        TcpClient tcpClient = std::move(client);
        co_await std::suspend_always();

        std::string buffer;

        auto requestCoroutine = HttpRequest::fetchRequest(tcpClient, buffer);
        iterative_co_await(requestCoroutine);
        HttpRequest request = requestCoroutine.value();

        HttpResponse response;

        DEBUG << "received request" << request.method << request.uriBase;

        //TODO if request.method == HEAD
        std::optional<RequestHandler::CallbackType> callback;
        for (auto &handler : handlers.at(request.method))
        {
            callback = handler.prepareCallback(request);
            if (callback)
                break;
        }
        if (!callback)
            callback = {defaultCallback};

        bool resume = false;
        auto promise = std::async(std::launch::async, [&](Notify notify) {
            try {
                callback.value()(request, response);
            } catch (const std::exception &e) {
                fatal500(request, response);
                DEBUG << "what():" << e.what();
            } catch (...) {
                fatal500(request, response);
                DEBUG << "NOT EVEN DERIVED FROM std::exception";
            }

            //TODO DELETE THIS
            std::this_thread::sleep_for(std::chrono::milliseconds(60)); // simulate blocking operation

            DEBUG << "Response callback ending";
            resume = true;
        }, std::move(notify));
        while (!resume)
            co_await std::suspend_always();

        auto sendCoroutine = response.send(tcpClient);
        iterative_co_await(sendCoroutine);

        tcpClient.close();
    }

    RequestHandler::CallbackType defaultCallback;
    static void default404(HttpRequest &request, HttpResponse &response)
    {
        response.setStatus(404);
        response.setBody("<h1>404</h1> Resource not found<br/>:(");
    }
    static void fatal500(HttpRequest &request, HttpResponse &response)
    {
        DEBUG << "UNHANDLED EXCEPTION WHILE PROCESSING" << request.method << request.uri;
        response.setStatus(500);
        response.setBody("<h1>500</h1>Unhandled exception happended while processing the request<br/>:(");
    }

};

#endif