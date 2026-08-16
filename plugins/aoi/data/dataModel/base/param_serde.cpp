#include "param_serde.h"

namespace aoi::serde {

void fromJson(const json &j, const std::string &key, QString &out)
{
    std::string s;
    ::fromJson(j, key, s);
    out = QString::fromStdString(s);
}

void toJson(json &j, const QString &val)
{
    j = val.toStdString();
}

} // namespace aoi::serde
