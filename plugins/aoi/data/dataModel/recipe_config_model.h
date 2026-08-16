/**
 * @file recipe_config_model.h
 * @brief 产品配方配置数据模型定义
 *
 * 本文件定义了 AOI 检测插件中产品配方（Recipe）的完整数据模型，
 * 采用 X-macro 模式实现字段声明、JSON 序列化、相等比较和元类型注册。
 *
 * ## 数据层次
 * ```
 * RecipeDocument
 * ├── id              — 配方唯一标识
 * ├── basic           — RecipeBasicInfo（基板/芯片尺寸、间距）
 * ├── camera          — CameraSettings（亮度、曝光、分辨率）
 * ├── algo            — AlgoThresholds（算法置信度阈值）
 * ├── marks[]         — MarkPointConfig（Mark 点检测参数）
 * ├── panelCalib      — PanelCalibSettings（面板标定模式）
 * └── positionCalib   — PositionCalibSettings（位置标定参数）
 * ```
 *
 * ## X-macro 模式说明
 * 每个子结构体通过 X-macro 宏定义字段，配合 PARAM_DECLARE / DEFINE_PARAM_STRUCT
 * 自动生成字段声明、JSON 读写、operator== 和 Qt 元类型注册代码。
 *
 * @see RecipeDocument
 * @see RecipeRepository
 * @see aoi::RecipeConfigViewModel
 */

#pragma once

#include "param_define.h"
#include "param_meta.h"
#include "param_model_macros.h"

#include <vision_algo/units/marker/mark_point_locate_data.h>

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QVector>

namespace aoi {

#define RECIPE_BASIC_INFO_FIELDS(X) \
    X(QString, name, QString(), "name") \
    X(int, width, 100, "width") \
    X(int, height, 160, "height") \
    X(double, groupPitchXmm, 1.25, "group_pitch_x_mm") \
    X(double, groupPitchYmm, 1.25, "group_pitch_y_mm") \
    X(double, chipPitchInGroupMm, 0.50, "chip_pitch_in_group_mm") \
    X(double, chipWidthMm, 0.15045, "chip_width_mm") \
    X(double, chipHeightMm, 0.059, "chip_height_mm") \
    X(double, padWidthMm, 0.32155, "pad_width_mm") \
    X(double, padHeightMm, 0.13275, "pad_height_mm") \
    X(double, groupWidthMm, 0.32155, "group_width_mm") \
    X(double, groupHeightMm, 0.51625, "group_height_mm") \
    X(int, chipColorOrder, 0, "chip_color_order") \
    X(int, chipArrangement, 0, "chip_arrangement") \
    X(Point, offsetGroupMarkBl, Point{}, "offset_group_mark_bl") \
    X(Point, offsetGroupMarkTr, Point{}, "offset_group_mark_tr")

#define CAMERA_SETTINGS_FIELDS(X) \
    X(int, lightBrightness, 153, "light_brightness") \
    X(int, exposureUs, 12000, "exposure_us") \
    X(int, imageWidth, 1920, "image_width") \
    X(int, imageHeight, 1080, "image_height") \
    X(double, captureAccuracy, 2.95, "capture_accuracy")

#define ALGO_THRESHOLDS_FIELDS(X) \
    X(double, groupConfidence, 0.72, "group_confidence") \
    X(double, chipConfidence, 0.68, "chip_confidence") \
    X(double, chipDefectClass, 0.70, "chip_defect_class") \
    X(double, padDefectClass, 0.65, "pad_defect_class") \
    X(double, offsetXUm, 15.0, "offset_x_um") \
    X(double, offsetYUm, 15.0, "offset_y_um") \
    X(double, angleOffsetDeg, 0.5, "angle_offset_deg")

#define MARKER_DETECT_INFO_FIELDS(X) \
    X(vision_algo::MarkerType, markType, vision_algo::MarkerType::Circle, "mark_type") \
    X(int, markWidth, 32, "mark_width") \
    X(int, markHeight, 32, "mark_height") \
    X(uint16_t, isWhiteMark, 0, "is_white_mark") \
    X(float, area, 100.0f, "area") \
    X(float, minThresh, 0.6f, "min_thresh") \
    X(float, maxThresh, 1.0f, "max_thresh") \
    X(std::string, modelFileName, std::string{}, "model_file_name") \
    X(std::string, modelName, std::string{}, "model_name")

#define MARK_POINT_CONFIG_FIELDS(X) \
    X(Point3, capturePos, Point3{}, "capture_pos") \
    X(vision_algo::MarkerDetectInfo, markerInfo, vision_algo::MarkerDetectInfo{}, "marker_info")

#define PANEL_CALIB_SETTINGS_FIELDS(X) \
    X(CalibMode, mode, CalibMode::Panel, "mode") \
    X(QString, fovInfo, QStringLiteral("未生成"), "fov_info") \
    X(int, pathModeIndex, 0, "path_mode_index") \
    X(QString, gerberFileName, QStringLiteral("未导入"), "gerber_file_name")

#define POSITION_CALIB_SETTINGS_FIELDS(X) \
    X(double, trackPosition, 0.0, "track_position") \
    X(Point3, scanPosition, Point3{}, "scan_position")

#define RECIPE_DOCUMENT_FIELDS(X) \
    X(QString, id, QString(), "id") \
    X(RecipeBasicInfo, basic, RecipeBasicInfo{}, "basic") \
    X(CameraSettings, camera, CameraSettings{}, "camera") \
    X(AlgoThresholds, algo, AlgoThresholds{}, "algo") \
    X(QVector<MarkPointConfig>, marks, QVector<MarkPointConfig>{}, "marks") \
    X(PanelCalibSettings, panelCalib, PanelCalibSettings{}, "panel_calib") \
    X(PositionCalibSettings, positionCalib, PositionCalibSettings{}, "position_calib")

/**
 * @brief 配方列表项（用于 UI 列表展示）
 *
 * 轻量级结构，仅包含配方在列表中显示所需的信息。
 * @see RecipeRepository::listRecipes()
 */
struct RecipeListItem
{
    QString id;
    QString name;
    bool    isDefault = false;

    bool operator==(const RecipeListItem &o) const
    {
        return id == o.id && name == o.name && isDefault == o.isDefault;
    }
    bool operator!=(const RecipeListItem &o) const { return !(*this == o); }
};

/**
 * @brief 配方基本信息 — 基板/芯片几何尺寸与布局参数
 *
 * 包含基板尺寸、组间距、芯片间距、焊盘尺寸、颜色顺序和排列方式。
 * 这些参数决定了 FOV 规划、路径规划和检测区域划分。
 */
struct RecipeBasicInfo
{
    RECIPE_BASIC_INFO_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(RecipeBasicInfo, RECIPE_BASIC_INFO_FIELDS)
};

/**
 * @brief 相机设置 — 图像采集参数
 *
 * 控制相机采集时的亮度、曝光时间、图像分辨率和采集精度。
 */
struct CameraSettings
{
    CAMERA_SETTINGS_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(CameraSettings, CAMERA_SETTINGS_FIELDS)
};

/**
 * @brief 算法阈值 — 检测置信度与判定参数
 *
 * 包含组/芯片/焊盘各层级的置信度阈值，以及偏移量和角度容差。
 * 检测结果低于对应阈值的区域将被标记为缺陷。
 */
struct AlgoThresholds
{
    ALGO_THRESHOLDS_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(AlgoThresholds, ALGO_THRESHOLDS_FIELDS)
};

/**
 * @brief Mark 点检测配置 — 单个 Mark 点的定位参数
 *
 * 包含 Mark 点的采集坐标和视觉检测参数（类型、尺寸、阈值、模板）。
 * 用于基板定位和对位补偿。
 */
struct MarkPointConfig
{
    MARK_POINT_CONFIG_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(MarkPointConfig, MARK_POINT_CONFIG_FIELDS)
};

/**
 * @brief 面板标定设置 — 标定模式与 FOV 信息
 *
 * 控制面板标定的模式（Panel/Strip）、FOV 信息、路径模式和 Gerber 文件。
 */
struct PanelCalibSettings
{
    PANEL_CALIB_SETTINGS_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(PanelCalibSettings, PANEL_CALIB_SETTINGS_FIELDS)
};

/**
 * @brief 位置标定设置 — 轨道与扫描位置参数
 *
 * 包含轨道位置和扫描坐标，用于检测平台的物理定位。
 */
struct PositionCalibSettings
{
    POSITION_CALIB_SETTINGS_FIELDS(PARAM_DECLARE)
    DEFINE_PARAM_STRUCT(PositionCalibSettings, POSITION_CALIB_SETTINGS_FIELDS)
};

/**
 * @brief 配方文档 — 完整的产品配方数据
 *
 * 核心数据结构，包含一个产品配方的所有配置参数。
 * 通过 X-macro 模式自动生成 JSON 序列化、反序列化和相等比较。
 *
 * ## 使用示例
 * @code
 * // 创建默认配方
 * auto doc = RecipeDocument::defaultDemo("recipe_001", "PANEL-A01");
 *
 * // 修改参数
 * doc.basic.width = 120;
 * doc.camera.exposureUs = 15000;
 *
 * // 保存到文件
 * doc.saveToFile();
 *
 * // 从文件加载
 * RecipeDocument loaded;
 * RecipeDocument::loadFromFile("path/to/recipe.json", loaded);
 * @endcode
 *
 * @see RecipeRepository
 * @see RecipeBasicInfo
 * @see CameraSettings
 * @see AlgoThresholds
 */
struct RecipeDocument
{
    RECIPE_DOCUMENT_FIELDS(PARAM_DECLARE)

    void reflectFromJson(const json &j) { RECIPE_DOCUMENT_FIELDS(REFLECT_FROM_FIELD) }

    void reflectToJson(json &j) const { RECIPE_DOCUMENT_FIELDS(REFLECT_TO_FIELD) }

    bool operator==(const RecipeDocument &o) const
    {
        return true RECIPE_DOCUMENT_FIELDS(EQ_FIELD_SUFFIX);
    }

    bool operator!=(const RecipeDocument &o) const { return !(*this == o); }

    static RecipeDocument defaultDemo(const QString &id, const QString &name);

    bool saveToFile() const;
    bool saveToFilePath(const QString &filePath) const;
    static bool loadFromFile(const QString &filePath, RecipeDocument &out);
};

/**
 * @brief 配方状态栏信息 — 当前配方名称与保存状态
 *
 * UI 状态模型，用于首页状态栏显示当前配方信息。
 */
struct RecipeStatusBar
{
    QString currentRecipeName = QStringLiteral("PANEL-A01");
    bool    canSave           = true;

    bool operator==(const RecipeStatusBar &o) const
    {
        return currentRecipeName == o.currentRecipeName && canSave == o.canSave;
    }
    bool operator!=(const RecipeStatusBar &o) const { return !(*this == o); }
};

struct RecipeExportItemResult
{
    QString recipeId;
    QString recipeName;
    QString filePath;
    bool    ok      = false;
    QString message;
};

struct RecipeExportReport
{
    QDateTime                       timestamp = QDateTime::currentDateTime();
    QString                         logFilePath;
    QString                         exportDirectory;
    bool                            overallOk     = false;
    QString                         overallMessage;
    QVector<RecipeExportItemResult> items;
};

struct RecipeImportItemResult
{
    QString filePath;
    QString recipeId;
    QString recipeName;
    bool    ok      = false;
    QString message;
};

struct RecipeImportReport
{
    QDateTime                        timestamp = QDateTime::currentDateTime();
    QString                          logFilePath;
    bool                             overallOk = false;
    QString                          overallMessage;
    QVector<RecipeImportItemResult> items;
};

/**
 * @brief 规范化配方文档（修复无效字段、填充默认值）
 *
 * @param doc 待规范化的配方文档，原地修改
 */
void normalizeRecipeDocument(RecipeDocument &doc);

/**
 * @brief 配方仓库 — 配方的持久化存储与 CRUD 管理
 *
 * 提供静态方法集，用于配方的文件存储、读取、查询、导入/导出。
 * 配方以 JSON 格式存储在配置目录下。
 *
 * ## 存储结构
 * ```
 * <storage_dir>/
 * ├── recipe_001.json
 * ├── recipe_002.json
 * └── index.json（可选索引缓存）
 * ```
 *
 * ## 使用示例
 * @code
 * // 初始化仓库
 * RecipeRepository::setStorageDirectory("/path/to/recipes");
 * RecipeRepository::load();
 *
 * // 列出所有配方
 * auto list = RecipeRepository::listRecipes();
 *
 * // 加载指定配方
 * RecipeDocument doc;
 * RecipeRepository::load("recipe_001", doc);
 *
 * // 保存配方
 * RecipeRepository::save(doc);
 *
 * // 删除配方
 * RecipeRepository::remove("recipe_001");
 * @endcode
 *
 * @see RecipeDocument
 * @see RecipeExportReport
 * @see RecipeImportReport
 */
class RecipeRepository
{
public:
    static void                    load();
    static QVector<RecipeListItem> listRecipes();
    static bool                    load(const QString &recipeId, RecipeDocument &out);
    static bool                    contains(const QString &recipeId);
    static bool                    save(const RecipeDocument &doc,
                                        const QString &previousRecipeId = {});
    static bool                    remove(const QString &recipeId);
    static bool                    setDefault(const QString &recipeId);
    static QString                 defaultRecipeId();
    static QString                 generateNewRecipeId();

    static bool exportAllToDirectory(const QString &dirPath, RecipeExportReport &report);
    static bool importFromJsonFiles(const QStringList &filePaths, RecipeImportReport &report);
    static void writeExportLog(RecipeExportReport &report);
    static void writeImportLog(RecipeImportReport &report);

    static void    setStorageDirectory(const QString &dir);
    static QString storageDirectory();

private:
    static void ensureSeeded();
};

/**
 * @brief 获取可修改的 Mark 点配置引用（自动扩容）
 *
 * 如果文档中 Mark 点数量不足，自动扩容到指定数量。
 * 使用 qBound 确保 slot 不越界。
 *
 * @param doc 目标配方文档
 * @param slot Mark 点索引
 * @param markCount 期望的 Mark 点总数
 * @return MarkPointConfig& 对应 slot 的配置引用
 */
inline MarkPointConfig &mutableMarkAt(RecipeDocument &doc, int slot, int markCount)
{
    if (doc.marks.size() < markCount)
        doc.marks.resize(markCount);
    return doc.marks[qBound(0, slot, markCount - 1)];
}

/**
 * @brief 获取只读的 Mark 点配置（越界返回空引用）
 *
 * @param doc 目标配方文档
 * @param slot Mark 点索引
 * @return const MarkPointConfig& 有效索引返回对应配置，越界返回静态空引用
 */
inline const MarkPointConfig &markAt(const RecipeDocument &doc, int slot)
{
    static const MarkPointConfig kEmpty;
    if (slot < 0 || slot >= doc.marks.size())
        return kEmpty;
    return doc.marks[slot];
}

} // namespace aoi

DEFINE_PARAM_TYPE_META(aoi::RecipeBasicInfo, "basic", RECIPE_BASIC_INFO_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::CameraSettings, "camera", CAMERA_SETTINGS_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::AlgoThresholds, "algo", ALGO_THRESHOLDS_FIELDS)
DEFINE_PARAM_TYPE_META(vision_algo::MarkerDetectInfo, "marker_detect", MARKER_DETECT_INFO_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::MarkPointConfig, "mark_point", MARK_POINT_CONFIG_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::PanelCalibSettings, "panel_calib", PANEL_CALIB_SETTINGS_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::PositionCalibSettings, "position_calib", POSITION_CALIB_SETTINGS_FIELDS)
DEFINE_PARAM_TYPE_META(aoi::RecipeDocument, "recipe", RECIPE_DOCUMENT_FIELDS)

Q_DECLARE_METATYPE(aoi::RecipeListItem)
Q_DECLARE_METATYPE(aoi::RecipeBasicInfo)
Q_DECLARE_METATYPE(aoi::CameraSettings)
Q_DECLARE_METATYPE(aoi::AlgoThresholds)
Q_DECLARE_METATYPE(aoi::MarkPointConfig)
Q_DECLARE_METATYPE(aoi::PanelCalibSettings)
Q_DECLARE_METATYPE(aoi::PositionCalibSettings)
Q_DECLARE_METATYPE(aoi::RecipeDocument)
Q_DECLARE_METATYPE(aoi::RecipeStatusBar)

#undef RECIPE_DOCUMENT_FIELDS
#undef POSITION_CALIB_SETTINGS_FIELDS
#undef PANEL_CALIB_SETTINGS_FIELDS
#undef MARK_POINT_CONFIG_FIELDS
#undef MARKER_DETECT_INFO_FIELDS
#undef ALGO_THRESHOLDS_FIELDS
#undef CAMERA_SETTINGS_FIELDS
#undef RECIPE_BASIC_INFO_FIELDS
