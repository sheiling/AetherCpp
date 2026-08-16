#include "stringutils.h"

#include "algorithm.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonValue>
#include <QRegularExpression>
#include <QSet>

#include <limits.h>

namespace ExtensionSystem {
namespace Utils {

bool readMultiLineString(const QJsonValue &value, QString *out)
{
    if (value.isString()) {
        *out = value.toString();
    } else if (value.isArray()) {
        QJsonArray array = value.toArray();
        QStringList lines;
        foreach (const QJsonValue &v, array) {
            if (!v.isString())
                return false;
            lines.append(v.toString());
        }
        *out = lines.join(QLatin1Char('\n'));
    } else {
        return false;
    }
    return true;
}



} // namespace Utils
}

