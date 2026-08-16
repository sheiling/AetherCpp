#pragma once

#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE
class QJsonValue;
QT_END_NAMESPACE

namespace ExtensionSystem {
namespace Utils {

bool readMultiLineString(const QJsonValue &value, QString *out);

} // namespace Utils
} // namespace ExtensionSystem