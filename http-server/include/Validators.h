#ifndef VALIDATORS_H_
#define VALIDATORS_H_

#include <string>
#include <regex>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "RequestHandler.h"
#include "RequestUtils.h"

class Validators
{
public:
    //TODO enum???
    inline static const std::string body = "body";
    inline static const std::string path = "path";


    static RequestHandler::CallbackType pathParamInt(const std::string &param)
    {
        return [=](auto &request, auto &response) {
            try {
                static_cast<void>(std::stoi(request.getPathParam(param)));
            } catch (std::invalid_argument &e) {
                RequestUtils::send400Json(response, "Argument " + param + " should be integer", path, param);
            }
        };
    }

    static RequestHandler::CallbackType bodyRegex(const json::json_pointer &param, const std::regex &regex, const std::string &patternName = "matching specific regex")
    {
        return [=](auto &request, auto &response) {
            try {
                const json &value = request.getBodyJson().at(param);
                if (!value.is_string())
                    RequestUtils::send400Json(response, "Body element " + param.to_string() + " must be string", body, param.to_string());
                else if (!std::regex_match(value.get<std::string>(), regex))
                    RequestUtils::send400Json(response, "Argument " + param.to_string() + " must be " + patternName, body, param);
            } catch (json::out_of_range &e) {
                RequestUtils::send400Json(response, "Body element " + param.to_string() + " is required", body, param.to_string());
            }
        };
    }

    static RequestHandler::CallbackType bodyType(const json::json_pointer &param, json::value_t type, std::string name = "specific type")
    {
        return [=](auto &request, auto &response) {
            try {
                if (request.getBodyJson().at(param).type() != type)
                    RequestUtils::send400Json(response, "Body element " + param.to_string() + " must be " + name, body, param.to_string());
            } catch (const json::out_of_range &e) {
                RequestUtils::send400Json(response, "Body element " + param.to_string() + " is required", body, param.to_string());
            }
        };
    }

    static RequestHandler::CallbackType bodyString(const json::json_pointer &param)
    {
        return bodyType(param, json::value_t::string, "string");
    }

    static RequestHandler::CallbackType bodyEmail(const json::json_pointer &param)
    {
        static const std::regex emailRegex{"(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+"};
        return bodyRegex(param, emailRegex, "email");
    }

    static RequestHandler::CallbackType bodyStringNonEmptyNoWhitespaces(const json::json_pointer &param)
    {
        static const std::regex nonEmptyNoWhitespacesRegex("^\\w+$");
        return bodyRegex(param, nonEmptyNoWhitespacesRegex, "non empty string with no whitespaces");
    }

    static RequestHandler::CallbackType bodyStringNonEmpty(const json::json_pointer &param)
    {
        static const std::regex nonEmptyRegex("[^]+$");
        return bodyRegex(param,  nonEmptyRegex, "non empty string");
    }

private:
};

#endif
