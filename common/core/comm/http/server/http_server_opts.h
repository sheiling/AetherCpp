#ifndef _HTTP_SERVER_OPTS_HH
#define _HTTP_SERVER_OPTS_HH

#include <string>
#include <functional>
#include <vector>

namespace common
{

namespace http
{

using RequestHandleFunc = std::function<int(const std::string &reqBody, std::string &rspBody)>;
using RequestHandleFuncEap = std::function<int(const std::string& reqBody, std::string& rspBody, std::string reqUrl)>;

enum Method {
    POST = 0,
    GET,
    PUT,
    DEL
};

class HttpServerOpts {
public:

    std::string urn_;
    std::vector<std::string> urns_;
    int port_ = 0;
    Method method_;
    RequestHandleFunc requestHandleFunc_;
    RequestHandleFuncEap requestHandleFuncEap_;
    HttpServerOpts() {
        urn_ = "";
        urns_.clear();
        port_ = 80;
        method_ = POST;
        requestHandleFunc_ = nullptr;
        requestHandleFuncEap_ = nullptr;
    }
};

} // end of namespace http

} // end of namespace common

#endif
