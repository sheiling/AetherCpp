#include "http_client.h"

namespace common
{

namespace http
{

int HttpClient::SyncRequest(const RequestMsg &requestMsg, int &statusCode, std::string &statusMsg, std::string &rspBody) {
    HttpResponse rsp;
    HttpRequest req;
    req.http_cb = std::bind(&HttpClient::httpCB, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    switch (requestMsg.method_) {
    case common::http::Method::POST: {
        req.method = HTTP_POST;
        break;
    }
    case common::http::Method::GET: {
        req.method = HTTP_GET;
        break;
    }
    default: {
        req.method = HTTP_POST;
        break;
    }
    }
    req.url = requestMsg.url_;

    for (auto iter = requestMsg.headers_.begin(); iter != requestMsg.headers_.end(); ++iter) {
        (req.headers).insert(std::make_pair(iter->first, iter->second));
    }

    req.timeout = requestMsg.timeout_;
    req.body = requestMsg.reqBody_;

    for (auto& param : requestMsg.formData_) {
        req.SetFormData(param.first.c_str(), param.second);
    }

    for (auto& param : requestMsg.formUrlencodedData_) {
        req.SetUrlEncoded(param.first.c_str(), param.second);
    }

    int ret = client_.send(&req, &rsp);
    if (ret != 0) {
        statusCode = -1;
        return -1;
    } else {
        statusCode = rsp.status_code;
        statusMsg = rsp.status_message();
        // rspBody = rsp.body;
        rspBody = chunkedBody_;
    }

    return 0;
}

int HttpClient::AsyncRequest(const RequestMsg &requestMsg, RespondHandleFunc respondHandleFunc) {
    HttpRequestPtr req(new HttpRequest);
    req->http_cb = std::bind(&HttpClient::httpCB, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    switch (requestMsg.method_) {
    case common::http::Method::POST: {
        req->method = HTTP_POST;
        break;
    }
    case common::http::Method::GET: {
        req->method = HTTP_GET;
        break;
    }
    default: {
        req->method = HTTP_POST;
        break;
    }
    }
    req->url = requestMsg.url_;

    for (auto iter = requestMsg.headers_.begin(); iter != requestMsg.headers_.end(); ++iter) {
        (req->headers).insert(std::make_pair(iter->first, iter->second));
    }

    req->timeout = requestMsg.timeout_;
    req->body = requestMsg.reqBody_;

    for (auto& param : requestMsg.formData_) {
        req->SetFormData(param.first.c_str(), param.second);
    }
    
    for (auto& param : requestMsg.formUrlencodedData_) {
        req->SetUrlEncoded(param.first.c_str(), param.second);
    }

    client_.sendAsync(req, [this, respondHandleFunc](const HttpResponsePtr& rsp) {
        if (rsp == NULL) {
            respondHandleFunc(-1, "", "");
        } else {
            // respondHandleFunc(rsp->status_code, rsp->status_message(), rsp->body);
            respondHandleFunc(rsp->status_code, rsp->status_message(), chunkedBody_);
        }
    });

    return 0;
}

void HttpClient::httpCB(HttpMessage* httpMessage, http_parser_state state, const char* data, size_t size) {
    switch (state) {
    case http_parser_state::HP_START_REQ_OR_RES: {
        break;
    }
    case http_parser_state::HP_MESSAGE_BEGIN: {
        chunkedBody_.clear();
        break;
    }
    case http_parser_state::HP_URL: {
        break;
    }
    case http_parser_state::HP_STATUS: {
        break;
    }
    case http_parser_state::HP_HEADER_FIELD: {
        break;
    }
    case http_parser_state::HP_HEADER_VALUE: {
        break;
    }
    case http_parser_state::HP_HEADERS_COMPLETE: {
        break;
    }
    case http_parser_state::HP_CHUNK_HEADER: {
        break;
    }
    case http_parser_state::HP_BODY: {
        chunkedBody_.append(data, size);
        break;
    }
    case http_parser_state::HP_CHUNK_COMPLETE: {
        break;
    }
    case http_parser_state::HP_MESSAGE_COMPLETE: {
        break;
    }
    case http_parser_state::HP_ERROR: {
        break;
    }
    default:
        break;
    }
}

} // end of namespace http

} // end of namespace common
