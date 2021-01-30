
#ifndef REQUESTHANDLER_H_
#define REQUESTHANDLER_H_

#include <vector>
#include <regex>

#include "Coroutine.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "RequestUtils.h"

class RequestHandler
{
public:
    using CallbackType = std::function<void(HttpRequest&, HttpResponse&)>;
    // using ChainCallbackType = std::function<bool(HttpRequest&, HttpResponse&);

    template <typename... CallbackT>
    RequestHandler(std::string pathTemplate, CallbackT&&... callbacks) : processCallbacks({callbacks...})
    {
        replaceString(pathTemplate, "([^\\/]*)", "{", "}");
        replaceString(pathTemplate, "(.*)", "<", ">");

        if (pathTemplate[0] != '/')
            pathTemplate = "/" + std::move(pathTemplate);
        if (pathTemplate.back() == '/')
            pathTemplate.pop_back();

        DEBUG << "GENERATED TEMPLATE" << pathTemplate;
        path = std::regex(pathTemplate);
    }

private:
    std::vector<CallbackType> processCallbacks;

    std::vector<std::string> pathParamsNames;
    std::regex path;

    void replaceString(std::string &pathTemplate, const std::string &replacement, const std::string &opening, const std::string &closing)
    {
        size_t index;
        while (((index = pathTemplate.find(opening))) != std::string::npos)
        {
            size_t closingIndex = pathTemplate.find(closing);
            pathParamsNames.push_back(pathTemplate.substr(index + 1, closingIndex - index - 1));
            pathTemplate.replace(index, closingIndex - index + 1, replacement);
        }
    }

    static void defaultChecks(HttpRequest &request, HttpResponse &response)
    {
        if (request.getContentType() == "application/json")
            static_cast<void>(request.getBodyJson());
    }

    friend class HttpServer;
    std::optional<CallbackType> prepareCallback(HttpRequest &request) const
    {
        static std::smatch match;
        if (!regex_match(request.getUriBase(), match, path))
            return {};
        
        for (size_t i = 0; i < pathParamsNames.size(); i++)
            request.pathParams[pathParamsNames[i]] = match.str(i + 1);

        return {[this](HttpRequest &request, HttpResponse &response) {
            defaultChecks(request, response);
            for (auto &callback : processCallbacks)
                if (!response.isReady())
                    callback(request, response);
        }};
    }
};


#endif
