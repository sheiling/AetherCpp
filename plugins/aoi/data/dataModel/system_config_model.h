#pragma once

#include "param_define.h"
#include "inspection_types.h"
#include "param_meta.h"
#include "param_model_macros.h"

#include <QMap>
#include <QMetaType>

namespace aoi {

#define GENERAL_SWITCHES_FIELDS(X) \
    X(bool, scanEnabled, true, "scan_enabled") \
    X(bool, reviewEnabled, true, "review_enabled")

#define INTERFACE_SWITCHES_FIELDS(X) \
    X(bool, upstreamEnabled, true, "upstream_enabled") \
    X(bool, downstreamEnabled, false, "downstream_enabled")

#define PATH_SETTINGS_FIELDS(X) \
    X(QString, imagePath, QString(), "image_path") \
    X(QString, resultPath, QString(), "result_path") \
    X(QString, thirdPartyPath, QString(), "third_party_path")

#define DEFECT_ITEM_SETTING_FIELDS(X) \
    X(QString, displayName, QString(), "display_name") \
    X(bool, enabled, true, "enabled") \
    X(int, lower, 0, "lower") \
    X(int, upper, 3, "upper")

#define TOTAL_DEFECT_THRESHOLD_FIELDS(X) \
    X(int, lower, 0, "lower") \
    X(int, upper, 5, "upper")

#define COMMON_PARAMS_FIELDS(X) \
    X(TrackMode, trackMode, TrackMode::DualTrack, "track_mode") \
    X(QString, captureName, QStringLiteral("Camera0"), "capture_name") \
    X(QString, axisXName, QStringLiteral("AxisX"), "axis_x_name") \
    X(QString, axisYName, QStringLiteral("AxisY"), "axis_y_name") \
    X(QString, axisZName, QStringLiteral("AxisZ"), "axis_z_name") \
    X(double, motionSpeed, 50.0, "motion_speed") \
    X(int, motionTimeoutMs, 30000, "motion_timeout_ms")

#define AOI_SYSTEM_CONFIG_FIELDS(X) \
    X(GeneralSwitches, general, GeneralSwitches{}, "general") \
    X(CommonParams, common, CommonParams{}, "common") \
    X(InterfaceSwitches, iface, InterfaceSwitches{}, "interface") \
    X(PathSettings, paths, PathSettings{}, "paths") \
    X(DefectSettings, defects, DefectSettings{}, "defects")

/** 通用开关（是否扫码 / 是否复判） */
struct GeneralSwitches
{
    GENERAL_SWITCHES_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(GeneralSwitches, GENERAL_SWITCHES_FIELDS)
};

/**
 * @brief 通用设备参数（轨道模式等系统级配置）
 */
struct CommonParams
{
    COMMON_PARAMS_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(CommonParams, COMMON_PARAMS_FIELDS)
};

/** 对接开关 */
struct InterfaceSwitches
{
    INTERFACE_SWITCHES_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(InterfaceSwitches, INTERFACE_SWITCHES_FIELDS)
};

/** 存储路径 */
struct PathSettings
{
    PATH_SETTINGS_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(PathSettings, PATH_SETTINGS_FIELDS)
};

/**
 * @brief 单项缺陷配置：开关 + 上下限阈值
 */
struct DefectItemSetting
{
    DEFECT_ITEM_SETTING_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(DefectItemSetting, DEFECT_ITEM_SETTING_FIELDS)
};

/** QMap 键值对含逗号，不可直接写入 X-Macro 的 type 参数 */
using DefectItemMap = QMap<inspection::InspectionDefectType, DefectItemSetting>;

/** 缺陷总数阈值 */
struct TotalDefectThreshold
{
    TOTAL_DEFECT_THRESHOLD_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(TotalDefectThreshold, TOTAL_DEFECT_THRESHOLD_FIELDS)
};

/**
 * @brief 缺陷相关配置（开关 + 单项阈值 + 总数阈值）
 *
 * 内存使用 QMap<InspectionDefectType, DefectItemSetting> 存储 11 类缺陷；
 * JSON 以同层键值对象序列化（键为缺陷英文名，特殊键 total 表示总数阈值）：
 *
 * @code
 * "defects": {
 *   "missing_chip": { "display_name": "aoi.recipe.defect.missing_chip", "enabled": true, "lower": 0, "upper": 3 },
 *   "chip_offset":  { "enabled": true, "lower": 0, "upper": 3 },
 *   "total":        { "lower": 0, "upper": 5 }
 * }
 * @endcode
 */
struct DefectSettings
{
    DefectItemMap items;
    TotalDefectThreshold                total;

    void reflectFromJson(const json &j);
    void reflectToJson(json &j) const;

    bool operator==(const DefectSettings &o) const;
    bool operator!=(const DefectSettings &o) const { return !(*this == o); }

    /** 补齐 11 类缺陷默认项 */
    void ensureDefaults();

    /** 按缺陷类型访问（不存在时自动插入默认值） */
    DefectItemSetting &item(inspection::InspectionDefectType type);
    const DefectItemSetting &item(inspection::InspectionDefectType type) const;
};

/**
 * @brief AOI 系统配置完整文档
 */
struct AoiSystemConfig
{
    AOI_SYSTEM_CONFIG_FIELDS(PARAM_DECLARE)

    void reflectFromJson(const json &j)
    {
        AOI_SYSTEM_CONFIG_FIELDS(REFLECT_FROM_FIELD)
        defects.ensureDefaults();
    }

    void reflectToJson(json &j) const
    {
        j["version"] = 1;
        AOI_SYSTEM_CONFIG_FIELDS(REFLECT_TO_FIELD)
    }

    bool operator==(const AoiSystemConfig &o) const
    {
        return true AOI_SYSTEM_CONFIG_FIELDS(EQ_FIELD_SUFFIX);
    }

    bool operator!=(const AoiSystemConfig &o) const { return !(*this == o); }

    static AoiSystemConfig defaultDemo();
};

/**
 * @brief 系统配置持久化（JSON 文件：param/aoi_system.json）
 */
class SystemConfigRepository
{
public:
    static QString configFilePath();
    static AoiSystemConfig load();
    static bool         save(const AoiSystemConfig &cfg);
};

} // namespace aoi

DEFINE_PARAM_TYPE_META(aoi::GeneralSwitches, "general", GENERAL_SWITCHES_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::CommonParams, "common", COMMON_PARAMS_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::InterfaceSwitches, "interface", INTERFACE_SWITCHES_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::PathSettings, "paths", PATH_SETTINGS_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::DefectItemSetting, "defect_item", DEFECT_ITEM_SETTING_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::TotalDefectThreshold, "total", TOTAL_DEFECT_THRESHOLD_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::AoiSystemConfig, "system_config", AOI_SYSTEM_CONFIG_FIELDS)

namespace aoi::parammeta {

/**
 * DefectSettings 的 JSON 为扁平键对象（missing_chip / … / total），无法用 DEFINE_PARAM_TYPE_META。
 * 字段列表在 system_config_model.cpp 中按 inspection::inspectionDefectTypeStorageKey 动态生成。
 */
template <>
struct ParamTypeMetaProvider<aoi::DefectSettings>
{
    static const ParamTypeMeta *get();
};

} // namespace aoi::parammeta

Q_DECLARE_METATYPE(aoi::GeneralSwitches)
Q_DECLARE_METATYPE(aoi::CommonParams)
Q_DECLARE_METATYPE(aoi::InterfaceSwitches)
Q_DECLARE_METATYPE(aoi::PathSettings)
Q_DECLARE_METATYPE(aoi::DefectItemSetting)
Q_DECLARE_METATYPE(aoi::TotalDefectThreshold)
Q_DECLARE_METATYPE(aoi::DefectSettings)
Q_DECLARE_METATYPE(aoi::AoiSystemConfig)

#undef AOI_SYSTEM_CONFIG_FIELDS
#undef COMMON_PARAMS_FIELDS
#undef TOTAL_DEFECT_THRESHOLD_FIELDS
#undef DEFECT_ITEM_SETTING_FIELDS
#undef PATH_SETTINGS_FIELDS
#undef INTERFACE_SWITCHES_FIELDS
#undef GENERAL_SWITCHES_FIELDS
#undef DEFINE_PARAM_TYPE_META
#undef PARAM_META_FIELD_ITEM
