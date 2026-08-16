#pragma once

#include "mes_config_model.h"
#include "mes_types.h"

#include <common/3rdparty/nlohmann/json.hpp>

namespace mes {

/**
 * @brief 兆驰 MES 通用 HTTP 传输层
 *
 * 封装 common::http::HttpClient，统一处理：
 *  - 请求头（appKey / Content-Type / CharSet）
 *  - 超时与重试
 *  - 标准响应解析（state / msg）
 */
class MesHttpTransport
{
public:
    explicit MesHttpTransport(MesConfig config = {});

    void setConfig(const MesConfig &config);
    const MesConfig &config() const { return m_config; }

    void setLogCallback(MesLogCallback callback);

    /** POST JSON 到 baseUrl + path，MES 未启用时返回 Skipped */
    MesResult post(const char *path,
                   const nlohmann::json &body,
                   const QString &logTitle) const;

private:
    MesResult parseResponse(const std::string &body) const;
    void log(const QString &title, const QString &content) const;

    MesConfig      m_config;
    MesLogCallback m_logCallback;
};

} // namespace mes
