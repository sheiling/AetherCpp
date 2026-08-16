#include "http_server.h"

namespace common
{

namespace http
{

int HttpServer::Start(const HttpServerOpts &httpServerOpts) {
    this->requestHandleFunc_ = httpServerOpts.requestHandleFunc_;

    switch (httpServerOpts.method_) {
	case common::http::Method::POST: {
        router_.POST(httpServerOpts.urn_.c_str(), (std::function<int(const HttpContextPtr& ctx)>)(std::bind(&HttpServer::OnRequest, this, std::placeholders::_1)));
        break;
    }
    case common::http::Method::GET: {
        router_.GET(httpServerOpts.urn_.c_str(), (std::function<int(const HttpContextPtr& ctx)>)(std::bind(&HttpServer::OnRequest, this, std::placeholders::_1)));
        break;
    }
    default: {
        break;
    }
    }

    server_.service = &router_;
    server_.port = httpServerOpts.port_;
    server_.start();
    return 0;
}

void HttpServer::Stop() {
    server_.stop();
}

int HttpServer::OnRequest(const HttpContextPtr& ctx) {
    if (requestHandleFunc_ == nullptr) {
        return -1;
    }

    std::string rspBody;
    int ret = requestHandleFunc_(ctx->body(), rspBody);
    if (ret < 0) {
        return -1;
    }

    ret = ctx->send(rspBody);

    return ret;
}

} // end of namespace http

} // end of namespace common
