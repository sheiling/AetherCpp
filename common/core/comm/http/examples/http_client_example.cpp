#include <iostream>
#include "core/comm/http/client/http_client.h"

int main() {
    common::http::HttpClient client;
    common::http::RequestMsg req;

    // 简单示例：GET 请求本地 HTTP 服务
    req.method_ = common::http::Method::GET;
    req.url_ = "http://127.0.0.1:8080/echo";

    int statusCode = 0;
    std::string statusMsg;
    std::string body;

    int ret = client.SyncRequest(req, statusCode, statusMsg, body);
    if (ret != 0) {
        std::cerr << "SyncRequest failed, ret=" << ret << std::endl;
        return -1;
    }

    std::cout << "HTTP status: " << statusCode << " " << statusMsg << std::endl;
    std::cout << "Body: " << body << std::endl;

    return 0;
}

