#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include <functional>
#include <future>
#include <fstream>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "File.h"
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

    std::unordered_map<std::string, std::string> extensionToType = {
        {".html", "text/html"},
        {".js", "text/javascript"},
        {".css", "text/css"},
        {".ico", "image/x-icon"},
        {".json", "application/json"},
        {".map", "application/octet-stream"},
        {".txt", "text/plain"}
    };

    void bindStaticFile(std::string &&path, const std::filesystem::path &filePath)
    {
        if (!std::filesystem::is_regular_file(filePath))
            throw std::runtime_error("file " + filePath.string() + " not found");
        std::string content = file::readAll(filePath);
        std::string contentType = extensionToType.at(filePath.extension().string());
        get(std::move(path), [=] (HttpRequest &request, HttpResponse &response) {
            response.setStatus(200).setBody(content, contentType);
        });
    }

    void bindStaticDirectory(const std::string &path, const std::filesystem::path &baseDir)
    {
        if (!std::filesystem::is_directory(baseDir))
            throw std::runtime_error("baseDir must be a folder");

        static const std::string paramName = "path343048903816";
        std::string fullPath = path + ((path.back() == '/') ? "" : "/") + "<" + paramName + ">";

        std::unordered_map<std::string, std::pair<std::string, std::string>> files;
        for (auto &p : std::filesystem::recursive_directory_iterator(baseDir))
        {
            std::filesystem::path path = p.path();
            DEBUG << path;
            DEBUG << path.extension().native();
            if (std::filesystem::is_regular_file(path))
                files.emplace(path.string(), std::make_pair(file::readAll(path), extensionToType.at(path.extension().string())));
        }

        get(std::move(fullPath), [this, baseDir, files] (HttpRequest &request, HttpResponse &response) {

            std::string pathString = request.getPathParam(paramName);
            std::filesystem::path path(pathString);
            std::filesystem::path fullPath = baseDir / path;
            auto it = files.find(fullPath.string());
            if (it == files.end())
                defaultCallback(request, response);
            else
                response.setStatus(200).setBody(it->second.first, it->second.second).send();
        });
    }

    using ExceptionHandler = std::function<void(const std::exception&, HttpResponse&)>;

    void addExceptionHandler(ExceptionHandler &&callback)
    {
        exceptionHandlers.emplace_back(std::move(callback));
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

    void addAccessControllAllowOrigin(const std::string &host)
    {
        if (defalutHeaders.find(accessControllAllowOrigin) != defalutHeaders.end())
            defalutHeaders[accessControllAllowOrigin] += ", ";
        defalutHeaders[accessControllAllowOrigin] += host;
    }

private:
    TcpServer tcpServer;

    std::unordered_map<std::string, std::string> defalutHeaders;

    inline static const std::string accessControllAllowOrigin = "Access-Control-Allow-Origin";

    std::unordered_map<HttpRequest::Method, std::vector<RequestHandler>> handlers = {
        {HttpRequest::Method::get, {}},
        {HttpRequest::Method::head, {}},
        {HttpRequest::Method::put, {}},
        {HttpRequest::Method::post, {}},
        {HttpRequest::Method::delet, {}}
    };

    std::vector<ExceptionHandler> exceptionHandlers;

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
        DEBUG << "New client starting";
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
                HttpResponse response(request, defalutHeaders);
                DEBUG << "received request" << request.getMethod() << request.getUriBase();

                if (request.isMalformed())
                    handleMalformed(response);
                else if (request.isBodyNotFetched())
                    handleTooLarge(response);
                else if (request.getMethod() == HttpRequest::Method::trace)
                    handleTrace(request, response);
                else if (request.getMethod() == HttpRequest::Method::options)
                    handleOptions(request, response);
                else
                {
                    auto callback = prepareCallback(request);

                    auto responsePromise = std::async(std::launch::async, [&](Notify notify) {
                        try {
                            try {
                                DEBUG << "launching callback for " << request.getUriBase();
                                callback(request, response);
                            } catch (const std::exception &e) {
                                for (auto &exceptionHandler : exceptionHandlers)
                                {
                                    if (response.isReady())
                                        break;
                                    exceptionHandler(e, response);
                                }
                                if (!response.isReady())
                                    throw;
                            }
                        } catch (const std::exception &e) {
                            defaultExceptionHandler(e, request, response);
                        } catch (...) {
                            fatal500(request, response);
                            DEBUG << "NOT EVEN DERIVED FROM std::exception";
                        }
                    }, notify.createCopy());
                    co_await std::suspend_always();
                    responsePromise.wait();
                }
                auto sendCoroutine = response.send(tcpClient);
                iterative_co_await(sendCoroutine);

                keepConnection = response.isPersistentConnection();
            }
        } catch (TcpClient::ConnectionClosedException &ignored)
        {
            DEBUG << R"*(Client closed connection despite no "Connection: close")*";
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

    static void defaultExceptionHandler(const std::exception &e, HttpRequest &request, HttpResponse &response)
    {
        try {
            auto ce = dynamic_cast<const HttpRequest::JsonParseError &>(e);
            RequestUtils::send400Json(response, std::string("malformed json as body: ") + ce.what(), "body");
            return;
        } catch (const std::bad_cast &ignored) {}
        fatal500(request, response);
        DEBUG << e.what();
    }

    static void fatal500(HttpRequest &request, HttpResponse &response)
    {
        DEBUG << "UNHANDLED EXCEPTION WHILE PROCESSING" << request.method << request.getUriBase();
        response.setStatus(500);
        response.setBody("<h1>500</h1> Unhandled exception happended while processing the request <br/> :(");
    }

    static void handleMalformed(HttpResponse &response)
    {
        response.setStatus(400).setBody("Request format is malformed and could not be at all interpreted").closeConnection();
    }

    static void handleTooLarge(HttpResponse &response)
    {
        response.setStatus(HttpResponse::Request_Entity_Too_Large).setBody("Entity too large").closeConnection();
    }

    void handleOptions(HttpRequest &request, HttpResponse &response)
    {
        if (request.getUriBase() == "*")
        {
            response.setStatus(200).setHeader("Allow", "GET, HEAD, PUT, POST, DELETE");
        }
        else
        {
            std::string allow = "";
            for (const auto &[method, methodString] : HttpRequest::methodToString)
            {
                if (prepareCallbackForMethod(request, method))
                    allow += methodString + ", ";
            }
            if (allow.empty())
                default404(request, response);
            else
            {
                allow.erase(allow.size() - 2);
                response.setStatus(200).setHeader("Allow", allow);
            }
        }
        if (request.hasHeader("Access-Control-Request-Headers"))
            response.setHeader("Access-Control-Allow-Headers", request.getHeader("Access-Control-Request-Headers"));
        if (request.hasHeader("Access-Control-Request-Methods"))
            response.setHeader("Access-Control-Allow-Methods", request.getHeader("Access-Control-Request-Methods"));
    }

    static void handleTrace(HttpRequest &request, HttpResponse &response)
    {
        response.setStatus(200).setBody(request.getUnparsedHeader(), "message/http");
    }

};

#endif