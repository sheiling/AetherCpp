#ifndef _HTTP_SERVER_HH
#define _HTTP_SERVER_HH

#include <3rdparty/libhv/include/hv/HttpServer.h>
#include "http_server_opts.h"
#include "../http_comm_global.h"

namespace common
{

namespace http
{

class HTTP_COMM_EXPORT HttpServer {
public:
    HttpServer() = default;
    ~HttpServer() = default;
public:
    int Start(const HttpServerOpts &httpServerOpts);
    void Stop();
private:
    int OnRequest(const HttpContextPtr& ctx);
private:
    hv::HttpServer server_;
    hv::HttpService router_;
    RequestHandleFunc requestHandleFunc_ = nullptr;
    RequestHandleFuncEap requestHandleFuncEap_ = nullptr;
};

} // end of namespace http

} // end of namespace common

#endif
