#include "param_meta.h"

#include <QRegularExpression>

namespace aoi::parammeta {

QString normalizeArrayPattern(const QString &label)
{
    static const QRegularExpression kArrayIndexRe(QStringLiteral("\\[\\d+\\]"));
    QString normalized = label;
    normalized.replace(kArrayIndexRe, QStringLiteral("[*]"));
    return normalized;
}

QString normalizeMapPattern(const QString &label)
{
    static const QRegularExpression kMapKeyRe(QStringLiteral("\\{[^\\}]*\\}"));
    QString normalized = label;
    normalized.replace(kMapKeyRe, QStringLiteral("{*}"));
    return normalized;
}

} // namespace aoi::parammeta
