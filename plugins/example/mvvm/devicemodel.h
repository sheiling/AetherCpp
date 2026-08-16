#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QString>
#include <QDateTime>
#include <QMetaType>

/**
 * @brief 设备基本信息结构体（Model 层）
 *
 * 演示 VIEWMODEL_STRUCT_PROPERTY + DataBinding::bindStructField 的使用。
 * 使用 Q_DECLARE_METATYPE 注册后可存入 QVariant，整体作为 ViewModel 属性管理。
 */
struct DeviceInfo
{
    QString  name;           ///< 设备名称
    QString  serialNumber;   ///< 序列号
    QString  firmware;       ///< 固件版本（下拉选择）
    int      health;         ///< 健康度 0-100
    bool     online;         ///< 在线状态

    DeviceInfo()
        : name("Device-001")
        , serialNumber("SN-20240101-001")
        , firmware("v3.2.1")
        , health(85)
        , online(true)
    {}

    bool operator==(const DeviceInfo &other) const {
        return name         == other.name
            && serialNumber == other.serialNumber
            && firmware     == other.firmware
            && health       == other.health
            && online       == other.online;
    }

    bool operator!=(const DeviceInfo &other) const {
        return !(*this == other);
    }

    bool operator<(const DeviceInfo &other) const {
        return name < other.name;
    }
};

// 注册到 Qt 元类型系统，使其可存入 QVariant 并跨信号槽传递
Q_DECLARE_METATYPE(DeviceInfo)

/**
 * @brief 传感器采样参数结构体（Model 层）
 *
 * 演示数值型字段（double、int、QDateTime）的结构体绑定。
 */
struct SensorConfig
{
    double   threshold;      ///< 报警阈值（浮点）
    int      sampleRate;     ///< 采样率 Hz（整数）
    QDate    calibDate;      ///< 最近校准日期
    QTime    triggerTime;    ///< 触发时间

    SensorConfig()
        : threshold(75.5)
        , sampleRate(100)
        , calibDate(QDate::currentDate())
        , triggerTime(QTime(8, 0, 0))
    {}

    bool operator==(const SensorConfig &other) const {
        return qFuzzyCompare(threshold, other.threshold)
            && sampleRate  == other.sampleRate
            && calibDate   == other.calibDate
            && triggerTime == other.triggerTime;
    }

    bool operator!=(const SensorConfig &other) const {
        return !(*this == other);
    }

    bool operator<(const SensorConfig &other) const {
        return sampleRate < other.sampleRate;
    }
};

Q_DECLARE_METATYPE(SensorConfig)

#endif // DEVICEMODEL_H
