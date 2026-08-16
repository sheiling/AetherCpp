#pragma once

/**
 * @file system_config_model.h
 * @brief 系统配置数据模型（开关、检测项、路径、判定阈值）
 */

#include <common/utility/json_tool/reflector.h>
#include <common/utility/json_tool/qt_json_adapters.h>
#include "lighten_types.h"

#include <QVector>
#include <QString>
#include <QMetaType>

namespace lighten {

/** 通用开关（系统配置 — 开关设置） */
struct GeneralSwitches
{
    bool scanBarcode              = true; ///< 扫码开关
    bool saveRawImage             = true; ///< 保存原图
    bool saveResultImage          = true; ///< 保存结果图
    bool saveAlgoProcessData      = true; ///< 保存算法过程数据
    bool algoFailAlarm            = true; ///< 算法失败报警

    bool operator==(const GeneralSwitches &o) const;
    bool operator!=(const GeneralSwitches &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, scanBarcode);
        FROM_JSON_AUTO_SAME_NAME(j, saveRawImage);
        FROM_JSON_AUTO_SAME_NAME(j, saveResultImage);
        FROM_JSON_AUTO_SAME_NAME(j, saveAlgoProcessData);
        FROM_JSON_AUTO_SAME_NAME(j, algoFailAlarm);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, scanBarcode);
        TO_JSON_AUTO_SAME_NAME(j, saveRawImage);
        TO_JSON_AUTO_SAME_NAME(j, saveResultImage);
        TO_JSON_AUTO_SAME_NAME(j, saveAlgoProcessData);
        TO_JSON_AUTO_SAME_NAME(j, algoFailAlarm);
    }
};

/** 对接开关 */
struct InterfaceSwitches
{
    bool upstreamEnabled  = true;  ///< 上游对接开关
    bool downstreamEnabled = false; ///< 下游对接开关

    bool operator==(const InterfaceSwitches &o) const;
    bool operator!=(const InterfaceSwitches &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, upstreamEnabled);
        FROM_JSON_AUTO_SAME_NAME(j, downstreamEnabled);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, upstreamEnabled);
        TO_JSON_AUTO_SAME_NAME(j, downstreamEnabled);
    }
};

/** 检测项分色开关（黑屏检测 / 黑屏亮灯 / Flash 无 RGB 分色） */
struct DetectItemColorSwitches
{
    DetectItemKind item = DetectItemKind::DeadLed; ///< 检测项类型
    bool redEnabled     = true;  ///< 红色分色开关
    bool greenEnabled   = true;  ///< 绿色分色开关
    bool blueEnabled    = true;  ///< 蓝色分色开关
    bool masterEnabled  = true;  ///< 总开关（无 RGB 分色项）
    bool hasRgbColumns  = true; ///< false 时 UI 显示 "—"

    bool operator==(const DetectItemColorSwitches &o) const;
    bool operator!=(const DetectItemColorSwitches &o) const { return !(*this == o); }

    /** 首页「检测项进度」是否显示该项（RGB 项：任一分色勾选；无分色项：总开关勾选） */
    bool showsHomeProgress() const;
    /** 首页进度槽位是否显示（RGB 项按红/绿/蓝列；无分色项等同 masterEnabled） */
    bool showsHomeProgressSlot(int colorIdx) const;

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, item);
        FROM_JSON_AUTO_SAME_NAME(j, redEnabled);
        FROM_JSON_AUTO_SAME_NAME(j, greenEnabled);
        FROM_JSON_AUTO_SAME_NAME(j, blueEnabled);
        FROM_JSON_AUTO_SAME_NAME(j, masterEnabled);
        FROM_JSON_AUTO_SAME_NAME(j, hasRgbColumns);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, item);
        TO_JSON_AUTO_SAME_NAME(j, redEnabled);
        TO_JSON_AUTO_SAME_NAME(j, greenEnabled);
        TO_JSON_AUTO_SAME_NAME(j, blueEnabled);
        TO_JSON_AUTO_SAME_NAME(j, masterEnabled);
        TO_JSON_AUTO_SAME_NAME(j, hasRgbColumns);
    }
};

/** 通用设置（路径 + 延时 + 寿命） */
struct GeneralSettings
{
    QString imageStoragePath;        ///< 原图存储路径
    QString resultStoragePath;       ///< 结果图存储路径
    QString thirdPartySoftwarePath;  ///< 第三方软件路径
    int     lightDelayMs      = 300;   ///< 点亮延时（毫秒）
    int     switchDelayMs     = 120;   ///< 切换延时（毫秒）
    int     maxUseCount       = 100000; ///< 新件使用寿命次数
    int     currentUseCount   = 35642;  ///< 当前使用次数

    bool operator==(const GeneralSettings &o) const;
    bool operator!=(const GeneralSettings &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, imageStoragePath);
        FROM_JSON_AUTO_SAME_NAME(j, resultStoragePath);
        FROM_JSON_AUTO_SAME_NAME(j, thirdPartySoftwarePath);
        FROM_JSON_AUTO_SAME_NAME(j, lightDelayMs);
        FROM_JSON_AUTO_SAME_NAME(j, switchDelayMs);
        FROM_JSON_AUTO_SAME_NAME(j, maxUseCount);
        FROM_JSON_AUTO_SAME_NAME(j, currentUseCount);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, imageStoragePath);
        TO_JSON_AUTO_SAME_NAME(j, resultStoragePath);
        TO_JSON_AUTO_SAME_NAME(j, thirdPartySoftwarePath);
        TO_JSON_AUTO_SAME_NAME(j, lightDelayMs);
        TO_JSON_AUTO_SAME_NAME(j, switchDelayMs);
        TO_JSON_AUTO_SAME_NAME(j, maxUseCount);
        TO_JSON_AUTO_SAME_NAME(j, currentUseCount);
    }
};

/** 判定阈值范围（下限 / 上限） */
struct ThresholdRange
{
    QString name;  ///< 阈值项名称
    int     lower = 0; ///< 下限
    int     upper = 0; ///< 上限

    bool operator==(const ThresholdRange &o) const
    {
        return name == o.name && lower == o.lower && upper == o.upper;
    }
    bool operator!=(const ThresholdRange &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, name);
        FROM_JSON_AUTO_SAME_NAME(j, lower);
        FROM_JSON_AUTO_SAME_NAME(j, upper);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, name);
        TO_JSON_AUTO_SAME_NAME(j, lower);
        TO_JSON_AUTO_SAME_NAME(j, upper);
    }
};

/** 系统配置完整文档 */
struct SystemConfig
{
    GeneralSwitches               generalSwitches;   ///< 通用开关
    InterfaceSwitches             interfaceSwitches; ///< 对接开关
    QVector<DetectItemColorSwitches> detectItems;    ///< 检测项分色开关（9 项）
    GeneralSettings               generalSettings;   ///< 路径、延时与寿命
    QVector<ThresholdRange>       thresholdRanges;   ///< 判定阈值范围

    bool operator==(const SystemConfig &o) const;
    bool operator!=(const SystemConfig &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, generalSwitches);
        FROM_JSON_AUTO_SAME_NAME(j, interfaceSwitches);
        FROM_JSON_AUTO_SAME_NAME(j, detectItems);
        FROM_JSON_AUTO_SAME_NAME(j, generalSettings);
        FROM_JSON_AUTO_SAME_NAME(j, thresholdRanges);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, generalSwitches);
        TO_JSON_AUTO_SAME_NAME(j, interfaceSwitches);
        TO_JSON_AUTO_SAME_NAME(j, detectItems);
        TO_JSON_AUTO_SAME_NAME(j, generalSettings);
        TO_JSON_AUTO_SAME_NAME(j, thresholdRanges);
    }

    /** 返回内置演示默认配置 */
    static SystemConfig defaultDemo();
    /** 从 config/system_config.json 加载配置 */
    static SystemConfig load();
    /** 将配置写入 config/system_config.json */
    static bool         save(const SystemConfig &cfg);
};

/**
 * @brief 系统配置持久化（当前为内存单例，可替换为文件/DB）
 */
class SystemConfigRepository
{
public:
    /** 加载当前系统配置（内存单例） */
    static SystemConfig load();
    /** 保存系统配置至 config/system_param.json 并更新内存单例 */
    static bool         save(const SystemConfig &cfg);
};

} // namespace lighten

Q_DECLARE_METATYPE(lighten::GeneralSwitches)
Q_DECLARE_METATYPE(lighten::InterfaceSwitches)
Q_DECLARE_METATYPE(lighten::DetectItemColorSwitches)
Q_DECLARE_METATYPE(lighten::GeneralSettings)
Q_DECLARE_METATYPE(lighten::ThresholdRange)
Q_DECLARE_METATYPE(lighten::SystemConfig)
