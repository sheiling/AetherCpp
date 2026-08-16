#pragma once

/**
 * @file perm_datetime_utils.h
 * @brief 权限插件内日期时间公共工具：精度、范围预设（各业务页复用）
 */

#include "controls/datetime/datetime_resolution.h"

#include <QDate>
#include <QDateTime>
#include <QString>
#include <QTime>

namespace permission_ui {

using PermDateTimeResolution = ui::DateTimeResolution;

struct PermDateTimeRange {
    QDateTime from;
    QDateTime to;
    bool      unlimited = false;
};

inline QString permDateTimeResolutionDisplayFormat(PermDateTimeResolution resolution)
{
    return ui::dateTimeResolutionDisplayFormat(resolution);
}

inline QDateTime permDateTimeTruncate(const QDateTime &value, PermDateTimeResolution resolution)
{
    return ui::dateTimeTruncate(value, resolution);
}

inline QDateTime permDateTimeEndOfGranularity(const QDateTime &value,
                                               PermDateTimeResolution resolution)
{
    return ui::dateTimeEndOfGranularity(value, resolution);
}

inline QDateTime permDateTimeFilterBound(const QDateTime &value, PermDateTimeResolution resolution,
                                          bool endInclusive)
{
    return ui::dateTimeFilterBound(value, resolution, endInclusive);
}

inline QString permDateTimeFormatDisplayText(const QDateTime &value,
                                              PermDateTimeResolution resolution,
                                              bool endOfRangeBound)
{
    return ui::dateTimeFormatDisplayText(value, resolution, endOfRangeBound);
}

inline QString permDateTimeFormatText(const QDateTime &value, PermDateTimeResolution resolution)
{
    return permDateTimeFormatDisplayText(value, resolution, false);
}

inline PermDateTimeRange permDateTimeRangeLast30Days()
{
    const QDate today = QDate::currentDate();
    PermDateTimeRange r;
    r.from = QDateTime(today.addDays(-30), QTime(0, 0, 0));
    r.to   = QDateTime(today, QTime(23, 59, 59));
    return r;
}

inline PermDateTimeRange permDateTimeRangeLast7Days()
{
    const QDate today = QDate::currentDate();
    PermDateTimeRange r;
    r.from = QDateTime(today.addDays(-7), QTime(0, 0, 0));
    r.to   = QDateTime(today, QTime(23, 59, 59));
    return r;
}

inline PermDateTimeRange permDateTimeRangeToday()
{
    const QDate today = QDate::currentDate();
    PermDateTimeRange r;
    r.from = QDateTime(today, QTime(0, 0, 0));
    r.to   = QDateTime(today, QTime(23, 59, 59));
    return r;
}

inline PermDateTimeRange permDateTimeRangeYesterday()
{
    const QDate day = QDate::currentDate().addDays(-1);
    PermDateTimeRange r;
    r.from = QDateTime(day, QTime(0, 0, 0));
    r.to   = QDateTime(day, QTime(23, 59, 59));
    return r;
}

inline PermDateTimeRange permDateTimeRangeUnlimited()
{
    PermDateTimeRange r;
    r.unlimited = true;
    return r;
}

} // namespace permission_ui
