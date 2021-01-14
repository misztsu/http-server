#ifndef REQUESTUTILS_H_
#define REQUESTUTILS_H_

class RequestUtils
{
public:
    static void send400Json(HttpResponse &response, const std::string &message, const std::string &location, const std::string &param = "")
    {
        auto j = json{
            {"message", message},
            {"location", location},
        };
        if (!param.empty())
            j["param"] = param;
        response.setStatus(400).setJsonBody(j).send();
    }
};

#endif