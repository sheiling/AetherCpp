#include <iostream>
#include "core/comm/http/server/http_server.h"

int main() {
    common::http::HttpServer server;
    common::http::HttpServerOpts opts;

    opts.port_ = 8080;
    opts.method_ = common::http::Method::POST;
    opts.urn_ = "/echo";

    opts.requestHandleFunc_ = [](const std::string& reqBody, std::string& rspBody) -> int {
        std::cout << "Received request body: " << reqBody << std::endl;
        rspBody = "echo: " + reqBody;
        return 0;
    };

    int ret = server.Start(opts);
    if (ret != 0) {
        std::cerr << "HttpServer start failed, ret=" << ret << std::endl;
        return -1;
    }

    std::cout << "HttpServer listening on http://127.0.0.1:8080/echo" << std::endl;
    std::cout << "Press ENTER to stop..." << std::endl;
    std::cin.get();

    server.Stop();
    return 0;
}

