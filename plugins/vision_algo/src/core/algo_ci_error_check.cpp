#include "algo_ci_error_check.h"

#include "vision_algo_i18n.h"

#include <QString>

namespace vision_algo {

/** ALGO_PASS → nullptr；否则组装带 API 名与 CISDK 错误文案的 ErrorPtr */
common::errors::ErrorPtr checkAlgoError(const char *apiName, AlgoError err)
{
    if (err == ALGO_PASS)
        return nullptr;

    const QString msg =
        tr("vision_algo.error.sdk_failed")
            .arg(QString::fromUtf8(apiName ? apiName : ""))
            .arg(QString::fromStdString(getAlgoErrorString(err)));
    return common::errors::New(common::errors::algo_module::kSdkError, msg.toStdString());
}

common::errors::ErrorPtr algoErrorFromKey(int code, const char *i18nKey)
{
    return common::errors::New(code, tr(i18nKey).toStdString());
}

common::errors::ErrorPtr algoInvalidArgument(const char *i18nKey)
{
    return algoErrorFromKey(common::errors::algo_module::kInvalidArgument, i18nKey);
}

common::errors::ErrorPtr algoInvalidImage(const char *i18nKey)
{
    return algoErrorFromKey(common::errors::algo_module::kInvalidImage, i18nKey);
}

common::errors::ErrorPtr algoUnitNotAvailable(const char *unitName)
{
    const QString msg =
        tr("vision_algo.error.unit_not_available").arg(QString::fromUtf8(unitName ? unitName : ""));
    return common::errors::New(common::errors::algo_module::kUnitNotAvailable, msg.toStdString());
}

common::errors::ErrorPtr algoServiceNotReady()
{
    return algoErrorFromKey(common::errors::algo_module::kServiceNotReady,
                            "vision_algo.error.service_not_ready");
}

} // namespace vision_algo
