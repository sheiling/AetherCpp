#pragma once

/**
 * @file datetime_resolution.h
 * @brief 日期时间精度枚举与截断/展示工具（无 i18n，供 ThemedDateTimeEdit 与各业务插件复用）
 */

#include <QDate>
#include <QDateTime>
#include <QString>
#include <QTime>

namespace ui {

enum class DateTimeResolution {
    Second = 0,
    Minute,
    Hour,
    Day,
};

inline QString dateTimeResolutionDisplayFormat(DateTimeResolution resolution)
{
    switch (resolution) {
    case DateTimeResolution::Minute:
        return QStringLiteral("yyyy-MM-dd HH:mm");
    case DateTimeResolution::Hour:
        return QStringLiteral("yyyy-MM-dd HH");
    case DateTimeResolution::Day:
        return QStringLiteral("yyyy-MM-dd");
    case DateTimeResolution::Second:
    default:
        return QStringLiteral("yyyy-MM-dd HH:mm:ss");
    }
}

inline QDateTime dateTimeTruncate(const QDateTime &value, DateTimeResolution resolution)
{
    if (!value.isValid())
        return {};

    const QDate d = value.date();
    const QTime t = value.time();

    switch (resolution) {
    case DateTimeResolution::Day:
        return QDateTime(d, QTime(0, 0, 0));
    case DateTimeResolution::Hour:
        return QDateTime(d, QTime(t.hour(), 0, 0));
    case DateTimeResolution::Minute:
        return QDateTime(d, QTime(t.hour(), t.minute(), 0));
    case DateTimeResolution::Second:
    default:
        return value;
    }
}

inline QDateTime dateTimeEndOfGranularity(const QDateTime &value, DateTimeResolution resolution)
{
    if (!value.isValid())
        return {};

    const QDate d = value.date();
    const QTime t = value.time();

    switch (resolution) {
    case DateTimeResolution::Day:
        return QDateTime(d, QTime(23, 59, 59));
    case DateTimeResolution::Hour:
        return QDateTime(d, QTime(t.hour(), 59, 59));
    case DateTimeResolution::Minute:
        return QDateTime(d, QTime(t.hour(), t.minute(), 59));
    case DateTimeResolution::Second:
    default:
        return value;
    }
}

inline QDateTime dateTimeFilterBound(const QDateTime &value, DateTimeResolution resolution,
                                     bool endInclusive)
{
    if (!value.isValid())
        return {};
    const QDateTime truncated = dateTimeTruncate(value, resolution);
    return endInclusive ? dateTimeEndOfGranularity(truncated, resolution) : truncated;
}

inline QString dateTimeFormatDisplayText(const QDateTime &value, DateTimeResolution resolution,
                                         bool endOfRangeBound)
{
    if (!value.isValid())
        return QString();

    const QDateTime bucket = dateTimeTruncate(value, resolution);
    const QDate           d = bucket.date();
    const QTime           t = bucket.time();

    switch (resolution) {
    case DateTimeResolution::Hour: {
        if (endOfRangeBound)
            return QDateTime(d, QTime(t.hour(), 59, 59)).toString(QStringLiteral("yyyy-MM-dd HH:mm"));
        return QDateTime(d, QTime(t.hour(), 0, 0)).toString(QStringLiteral("yyyy-MM-dd HH:mm"));
    }
    case DateTimeResolution::Day:
        if (endOfRangeBound)
            return QDateTime(d, QTime(23, 59, 59)).toString(QStringLiteral("yyyy-MM-dd"));
        return QDateTime(d, QTime(0, 0, 0)).toString(QStringLiteral("yyyy-MM-dd"));
    case DateTimeResolution::Minute:
        if (endOfRangeBound)
            return QDateTime(d, QTime(t.hour(), t.minute(), 59))
                .toString(QStringLiteral("yyyy-MM-dd HH:mm"));
        return QDateTime(d, QTime(t.hour(), t.minute(), 0))
            .toString(QStringLiteral("yyyy-MM-dd HH:mm"));
    case DateTimeResolution::Second:
    default:
        return bucket.toString(dateTimeResolutionDisplayFormat(resolution));
    }
}

} // namespace ui
