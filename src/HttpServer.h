#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include <functional>
#include <future>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "RequestHandler.h"
#include "TcpServer.h"
#include "TcpClient.h"

class HttpServer
{
public:
    HttpServer() = default;

    void setDefaultCallback(RequestHandler::CallbackType &&callback) { defaultCallback = std::move(callback); }

    template <HttpRequest::Method method, typename... CallbackT>
    void bind(std::string &&path, CallbackT&&... callbacks)
    {
        handlers[method].emplace_back(std::move(path), std::forward<CallbackT>(callbacks)...);
    }

    template <typename... CallbackT>
    void get(std::string &&path, CallbackT&&... callbacks)
    {
        bind<HttpRequest::Method::get>(std::move(path), std::forward<CallbackT>(callbacks)...);
    }
    template <typename... CallbackT>
    void head(std::string &&path, CallbackT&&... callbacks)
    {
        bind<HttpRequest::Method::head>(std::move(path), std::forward<CallbackT>(callbacks)...);
    }
    template <typename... CallbackT>
    void put(std::string &&path, CallbackT&&... callbacks)
    {
        bind<HttpRequest::Method::put>(std::move(path), std::forward<CallbackT>(callbacks)...);
    }
    template <typename... CallbackT>
    void post(std::string &&path, CallbackT&&... callbacks)
    {
        bind<HttpRequest::Method::post>(std::move(path), std::forward<CallbackT>(callbacks)...);
    }
    template <typename... CallbackT>
    void delet(std::string &&path, CallbackT&&... callbacks)
    {
        bind<HttpRequest::Method::delet>(std::move(path), std::forward<CallbackT>(callbacks)...);
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
        {HttpRequest::Method::get, {}},
        {HttpRequest::Method::head, {}},
        {HttpRequest::Method::put, {}},
        {HttpRequest::Method::post, {}},
        {HttpRequest::Method::delet, {}}
    };


    using OptionalCallback = std::optional<RequestHandler::CallbackType>;

    OptionalCallback prepareCallbackForMethod(HttpRequest &request, HttpRequest::Method method)
    {
        for (auto &handler : handlers.at(method))
        {
            auto callback = handler.prepareCallback(request);
            if (callback)
                return callback;
        }
        return {};
    }

    RequestHandler::CallbackType prepareCallback(HttpRequest &request)
    {
        auto callback = prepareCallbackForMethod(request, request.getMethod());
        // if there is no matching HEAD handler use
        // a GET handler (and the body won't be send later)
        if (!callback && request.getMethod() == HttpRequest::Method::head)
            callback = prepareCallbackForMethod(request, HttpRequest::Method::get);
        if (!callback)
            return defaultCallback;
        else
            return callback.value();
    }

    Coroutine<void> clientHandlingTask(TcpClient&& client, Notify notify)
    {
        TcpClient tcpClient = std::move(client);
        co_await std::suspend_always();

        std::string buffer;

        bool keepConnection = true;
        try {
            while (keepConnection)
            {
                auto requestCoroutine = HttpRequest::fetchRequest(tcpClient, buffer);
                iterative_co_await(requestCoroutine);
                HttpRequest request = requestCoroutine.value();

                DEBUG << "received request" << request.method << request.uriBase;

                auto callback = prepareCallback(request);

                auto response = std::async(std::launch::async, [&](Notify notify) {
                    HttpResponse response(request);
                    try {
                        callback(request, response);
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

                    return response;
                }, notify.createCopy());
                co_await std::suspend_always();

                auto sendCoroutine = response.get().send(tcpClient);
                iterative_co_await(sendCoroutine);

                keepConnection = request.isPersistentConnection();
            }
        } catch (TcpClient::ConnectionClosedException &ignored)
        {
            DEBUG << "Client closed connection despite no \"Connection: closed\"";
        }

        notify.release();

        DEBUG << "connection ending";
        tcpClient.close();
    }

    static void default404(HttpRequest &request, HttpResponse &response)
    {
        response.setStatus(404);
        response.setBody("<h1>404</h1> Resource not found <br/> :(");
    }
    RequestHandler::CallbackType defaultCallback = default404;

    static void fatal500(HttpRequest &request, HttpResponse &response)
    {
        DEBUG << "UNHANDLED EXCEPTION WHILE PROCESSING" << request.method << request.uri;
        response.setStatus(500);
        response.setBody("<h1>500</h1> Unhandled exception happended while processing the request <br/> :(");
    }

};

#endif