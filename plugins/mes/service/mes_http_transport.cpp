#include "mes_http_transport.h"

#include <core/comm/http/client/http_client.h>
#include <core/comm/http/client/http_client_opts.h>

#include <QtGlobal>

namespace mes {
namespace {

QString trimmedBaseUrl(const QString &address)
{
    QString url = address.trimmed();
    while (url.endsWith(QLatin1Char('/')))
        url.chop(1);
    return url;
}

std::string toUtf8Std(const QString &text)
{
    return text.toUtf8().toStdString();
}

} // namespace

MesHttpTransport::MesHttpTransport(MesConfig config)
    : m_config(std::move(config))
{
}

void MesHttpTransport::setConfig(const MesConfig &config)
{
    m_config = config;
}

void MesHttpTransport::setLogCallback(MesLogCallback callback)
{
    m_logCallback = std::move(callback);
}

MesResult MesHttpTransport::post(const char *path,
                                 const nlohmann::json &body,
                                 const QString &logTitle) const
{
    if (!m_config.enabled)
        return MesResult::makeSkipped();

    const QString baseUrl = trimmedBaseUrl(m_config.address);
    if (baseUrl.isEmpty()) {
        return MesResult::failure(MesResult::Code::Transport,
                                  QStringLiteral("MES 地址未配置"));
    }

    common::http::RequestMsg request;
    request.url_     = toUtf8Std(baseUrl + QLatin1String(path));
    request.method_  = common::http::Method::POST;
    request.timeout_ = static_cast<uint16_t>(qMax(1, m_config.timeout));
    request.headers_.emplace("appKey", toUtf8Std(m_config.appKey));
    request.headers_[CONTENT_TYPE] = "Application/Json";
    request.headers_[CHARSET]      = "UTF-8";

    const std::string bodyStr = body.dump();
    request.reqBody_          = bodyStr;

    log(logTitle + QStringLiteral(" 请求"), QString::fromUtf8(bodyStr.c_str()));

    common::http::HttpClient client;
    const int maxAttempts = qMax(1, m_config.requestTimes);

    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        int         statusCode = 0;
        std::string statusMsg;
        std::string rspBody;

        const int ret = client.SyncRequest(request, statusCode, statusMsg, rspBody);
        log(logTitle + QStringLiteral(" 回复"),
            QString::fromUtf8(rspBody.c_str()));

        if (ret != 0)
            continue;

        return parseResponse(rspBody);
    }

    return MesResult::failure(MesResult::Code::Transport,
                              logTitle + QStringLiteral(" 超时"));
}

MesResult MesHttpTransport::parseResponse(const std::string &body) const
{
    const QString raw = QString::fromUtf8(body.c_str());

    try {
        const auto json = nlohmann::json::parse(body);
        if (!json.contains("state") || !json.contains("msg")) {
            return MesResult::failure(MesResult::Code::Protocol,
                                      QStringLiteral("MES 回复缺少 state 或 msg"),
                                      raw);
        }

        const bool state = json["state"].get<bool>();
        const QString msg = QString::fromStdString(json["msg"].get<std::string>());
        if (!state) {
            return MesResult::failure(MesResult::Code::Business, msg, raw);
        }
        return MesResult::success(msg);
    } catch (const std::exception &) {
        return MesResult::failure(MesResult::Code::Protocol,
                                  QStringLiteral("MES 回复解析失败"),
                                  raw);
    }
}

void MesHttpTransport::log(const QString &title, const QString &content) const
{
    if (m_logCallback)
        m_logCallback(title, content);
}

} // namespace mes
