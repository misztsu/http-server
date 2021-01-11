
#ifndef REQUESTHANDLER_H_
#define REQUESTHANDLER_H_

#include <vector>
#include <regex>

#include "Coroutine.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class RequestHandler
{
public:
    using CallbackType = std::function<void(HttpRequest&, HttpResponse&)>;
    // using ChainCallbackType = std::function<bool(HttpRequest&, HttpResponse&);

    RequestHandler(std::string pathTemplate, CallbackType callback)
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
        processCallback = std::move(callback);
    }

    void process(CallbackType callback)
    {
        processCallback = callback;
    }

private:

    friend class HttpServer;
    std::optional<CallbackType> prepareCallback(HttpRequest &request)
    {
        static std::smatch match;
        if (!regex_match(request.getUriBase(), match, path))
            return {};
        
        for (size_t i = 0; i < pathParamsNames.size(); i++)
            request.pathParams[pathParamsNames[i]] = match.str(i + 1);

        return {processCallback};
    }

    CallbackType processCallback;

    std::vector<std::string> pathParamsNames;
    std::regex path;
};


#endif
