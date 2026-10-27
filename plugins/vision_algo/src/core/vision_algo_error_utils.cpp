#include "vision_algo_error_utils.h"

#include "vision_algo_i18n.h"

#include <QString>

namespace vision_algo {

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
