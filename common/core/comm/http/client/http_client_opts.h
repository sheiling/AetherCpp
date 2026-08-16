#ifndef _HTTP_CLIENT_OPTS_HH
#define _HTTP_CLIENT_OPTS_HH

#include <string>
#include <map>
#include <functional>

namespace common
{

namespace http
{

using RespondHandleFunc = std::function<int(int statusCode, std::string statusMsg, const std::string &rspBody)>;

enum Method {
    POST = 0,
    GET,
    PUT,
    DEL
};

// HTTP header labels
#define CONNECTION      "Connection"
#define CONTENT_TYPE    "Content-Type"
#define CONTENT_LENGTH  "Content-Length"
#define CHARSET         "CharSet"

class RequestMsg {
public:
    std::string url_;
    Method method_;
    std::map<std::string, std::string> headers_;
    uint16_t timeout_; // 秒
    std::string reqBody_;                                   // Content-type = application/json
    std::map<std::string, std::string> formData_;           // Content-type = multipart/form-data
    std::map<std::string, std::string> formUrlencodedData_; // Content-type = application/x-www-form-urlencoded
    
    RequestMsg() {
        url_ = "http://127.0.0.1:80/test";
        method_ = POST;
        headers_[CONNECTION] = "keep-alive";
        // headers_[CONTENT_TYPE] = "Application/Json";
        // headers_[CONTENT_LENGTH] = "0";
        // headers_[CHARSET] = "UTF-8";
        timeout_ = 10;
        reqBody_.clear();
        formData_.clear();
        formUrlencodedData_.clear();
    }
};

} // end of namespace http

} // end of namespace common

#endif
