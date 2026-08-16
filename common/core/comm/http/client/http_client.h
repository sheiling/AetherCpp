#ifndef _HTTP_CLIENT_HH
#define _HTTP_CLIENT_HH

#include <3rdparty/libhv/include/hv/requests.h>
#include "http_client_opts.h"
#include "../http_comm_global.h"

namespace common
{

namespace http
{

class HTTP_COMM_EXPORT HttpClient {
public:
    HttpClient() = default;
    ~HttpClient() = default;
public:
    int SyncRequest(const RequestMsg &requestMsg, int &statusCode, std::string &statusMsg, std::string &rspBody);
    int AsyncRequest(const RequestMsg &requestMsg, RespondHandleFunc respondHandleFunc);
private:
    void httpCB(HttpMessage* httpMessage, http_parser_state state, const char* data, size_t size);
private:
    hv::HttpClient client_;
    std::string chunkedBody_;
};

} // end of namespace http

} // end of namespace common

#endif
