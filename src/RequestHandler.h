
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
        size_t index;
        while (((index = pathTemplate.find("{"))) != std::string::npos)
        {
            static const std::string replacing = "([^\\/]*)";
            size_t closing = pathTemplate.find("}");
            pathParamsNames.push_back(pathTemplate.substr(index + 1, closing - index - 1));
            pathTemplate.replace(index, closing - index + 1, replacing);
        }

        DEBUG << "GENERATED TEMPLATE" << pathTemplate;
        path = std::regex(pathTemplate);
    }

private:

    static void defaultChecks(HttpRequest &request, HttpResponse &response)
    {
        if (request.getContentType() == "application/json")
        {
            try {
                static_cast<void>(request.getBodyJson());
            } catch (json::parse_error &e) {
                RequestUtils::send400Json(response, "malformed JSON", "body");
            }
        }
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


    std::vector<CallbackType> processCallbacks;

    std::vector<std::string> pathParamsNames;
    std::regex path;
};


#endif
