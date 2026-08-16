#pragma once

/**
 * @file recipe_model.h
 * @brief 产品配方数据模型（基本信息、相机参数、算法阈值、特殊检测、持久化）
 */

#include <common/utility/json_tool/reflector.h>
#include <common/utility/json_tool/qt_json_adapters.h>
#include "lighten_types.h"

#include <QObject>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QList>
#include <QPointF>
#include <QMetaType>

namespace lighten {

/** RGB 阈值或曝光参数 */
struct RgbTriple
{
    double r = 0.0; ///< 红色通道值
    double g = 0.0; ///< 绿色通道值
    double b = 0.0; ///< 蓝色通道值

    bool operator==(const RgbTriple &o) const
    {
        return qFuzzyCompare(r, o.r) && qFuzzyCompare(g, o.g) && qFuzzyCompare(b, o.b);
    }
    bool operator!=(const RgbTriple &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, r);
        FROM_JSON_AUTO_SAME_NAME(j, g);
        FROM_JSON_AUTO_SAME_NAME(j, b);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, r);
        TO_JSON_AUTO_SAME_NAME(j, g);
        TO_JSON_AUTO_SAME_NAME(j, b);
    }
};

/** 配方列表项 */
struct RecipeListItem
{
    QString       id;   ///< 配方 ID
    QString       name; ///< 配方显示名称
    RecipeListTag tag = RecipeListTag::None; ///< 列表右侧标签（默认/空白，由 defaultRecipeId 推导）

    bool operator==(const RecipeListItem &o) const
    {
        return id == o.id && name == o.name && tag == o.tag;
    }
    bool operator!=(const RecipeListItem &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, id);
        FROM_JSON_AUTO_SAME_NAME(j, name);
        FROM_JSON_AUTO_SAME_NAME(j, tag);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, id);
        TO_JSON_AUTO_SAME_NAME(j, name);
        TO_JSON_AUTO_SAME_NAME(j, tag);
    }
};

/** 点亮产品基本信息（name 须与 RecipeDocument::id 保持一致） */
struct RecipeBasicInfo
{
    QString name;  ///< 产品名称（须与 RecipeDocument::id 一致）
    int     width            = 1920; ///< 面板宽度（像素）
    int     height           = 1080; ///< 面板高度（像素）
    int     coordSystemIndex = 0; ///< 0–7，与坐标系下拉一致

    bool operator==(const RecipeBasicInfo &o) const
    {
        return name == o.name && width == o.width && height == o.height
               && coordSystemIndex == o.coordSystemIndex;
    }
    bool operator!=(const RecipeBasicInfo &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, name);
        FROM_JSON_AUTO_SAME_NAME(j, width);
        FROM_JSON_AUTO_SAME_NAME(j, height);
        FROM_JSON_AUTO_SAME_NAME(j, coordSystemIndex);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, name);
        TO_JSON_AUTO_SAME_NAME(j, width);
        TO_JSON_AUTO_SAME_NAME(j, height);
        TO_JSON_AUTO_SAME_NAME(j, coordSystemIndex);
    }
};

/** 单相机参数（相机设置表 8 行语义） */
struct PerCameraParams
{
    RgbTriple pureExposureMs;   ///< 纯色曝光
    RgbTriple pureGrayLevel;    ///< 纯色灰阶
    RgbTriple seriesExposureMs; ///< 串灯曝光
    RgbTriple seriesGrayLevel;  ///< 串灯灰阶
    int       whiteExposureMs = 10;  ///< 白场曝光（毫秒）
    int       blackExposureMs = 6;   ///< 黑场曝光（毫秒）
    bool      qualityOkR      = true;  ///< 图像质量判定：R 通道 OK
    bool      qualityNgG      = false; ///< 图像质量判定：G 通道 NG
    bool      qualityOkB      = true;  ///< 图像质量判定：B 通道 OK
    int       rotationDeg     = 0; ///< 0/90/180/270

    bool operator==(const PerCameraParams &o) const;
    bool operator!=(const PerCameraParams &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, pureExposureMs);
        FROM_JSON_AUTO_SAME_NAME(j, pureGrayLevel);
        FROM_JSON_AUTO_SAME_NAME(j, seriesExposureMs);
        FROM_JSON_AUTO_SAME_NAME(j, seriesGrayLevel);
        FROM_JSON_AUTO_SAME_NAME(j, whiteExposureMs);
        FROM_JSON_AUTO_SAME_NAME(j, blackExposureMs);
        FROM_JSON_AUTO_SAME_NAME(j, qualityOkR);
        FROM_JSON_AUTO_SAME_NAME(j, qualityNgG);
        FROM_JSON_AUTO_SAME_NAME(j, qualityOkB);
        FROM_JSON_AUTO_SAME_NAME(j, rotationDeg);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, pureExposureMs);
        TO_JSON_AUTO_SAME_NAME(j, pureGrayLevel);
        TO_JSON_AUTO_SAME_NAME(j, seriesExposureMs);
        TO_JSON_AUTO_SAME_NAME(j, seriesGrayLevel);
        TO_JSON_AUTO_SAME_NAME(j, whiteExposureMs);
        TO_JSON_AUTO_SAME_NAME(j, blackExposureMs);
        TO_JSON_AUTO_SAME_NAME(j, qualityOkR);
        TO_JSON_AUTO_SAME_NAME(j, qualityNgG);
        TO_JSON_AUTO_SAME_NAME(j, qualityOkB);
        TO_JSON_AUTO_SAME_NAME(j, rotationDeg);
    }
};

/** 算法阈值表一行 + 额外浮点项 */
struct AlgoThresholdRow
{
    QString  name;   ///< 阈值项名称
    RgbTriple values; ///< RGB 阈值

    bool operator==(const AlgoThresholdRow &o) const
    {
        return name == o.name && values == o.values;
    }
    bool operator!=(const AlgoThresholdRow &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, name);
        FROM_JSON_AUTO_SAME_NAME(j, values);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, name);
        TO_JSON_AUTO_SAME_NAME(j, values);
    }
};

/** 算法阈值聚合（9 项检测 RGB 阈值 + 黑屏/亮屏/坏点） */
struct AlgoThresholds
{
    QVector<AlgoThresholdRow> rows; ///< 9 项检测 RGB 阈值行
    double blackScreenThresh       = 8.0;  ///< 黑屏检测阈值
    double blackScreenBrightThresh = 6.0;  ///< 黑屏亮灯阈值
    double badPixelThresh          = 6.0;  ///< 坏点阈值

    bool operator==(const AlgoThresholds &o) const
    {
        return rows == o.rows && qFuzzyCompare(blackScreenThresh, o.blackScreenThresh)
               && qFuzzyCompare(blackScreenBrightThresh, o.blackScreenBrightThresh)
               && qFuzzyCompare(badPixelThresh, o.badPixelThresh);
    }
    bool operator!=(const AlgoThresholds &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, rows);
        FROM_JSON_AUTO_SAME_NAME(j, blackScreenThresh);
        FROM_JSON_AUTO_SAME_NAME(j, blackScreenBrightThresh);
        FROM_JSON_AUTO_SAME_NAME(j, badPixelThresh);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, rows);
        TO_JSON_AUTO_SAME_NAME(j, blackScreenThresh);
        TO_JSON_AUTO_SAME_NAME(j, blackScreenBrightThresh);
        TO_JSON_AUTO_SAME_NAME(j, badPixelThresh);
    }

    /** 返回内置演示默认算法阈值 */
    static AlgoThresholds defaultDemo();
};

/** 特殊检测项表一行 */
struct SpecialDetectItem
{
    QString name;          ///< 检测项名称
    bool    enabled       = true; ///< 是否启用
    int     lampThreshold = 0;    ///< 灯珠数量阈值
    QString action; ///< 自动下料 / 报警

    bool operator==(const SpecialDetectItem &o) const
    {
        return name == o.name && enabled == o.enabled && lampThreshold == o.lampThreshold
               && action == o.action;
    }
    bool operator!=(const SpecialDetectItem &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, name);
        FROM_JSON_AUTO_SAME_NAME(j, enabled);
        FROM_JSON_AUTO_SAME_NAME(j, lampThreshold);
        FROM_JSON_AUTO_SAME_NAME(j, action);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, name);
        TO_JSON_AUTO_SAME_NAME(j, enabled);
        TO_JSON_AUTO_SAME_NAME(j, lampThreshold);
        TO_JSON_AUTO_SAME_NAME(j, action);
    }
};

/** 其他参数 */
struct RecipeOtherSettings
{
    double trackWidthMm = 320.0; ///< 轨道宽度（毫米）

    bool operator==(const RecipeOtherSettings &o) const
    {
        return qFuzzyCompare(trackWidthMm, o.trackWidthMm);
    }
    bool operator!=(const RecipeOtherSettings &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, trackWidthMm);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, trackWidthMm);
    }
};

/** 调试区 ROI（单相机+颜色通道） */
struct RecipeDebugToolbar
{
    CameraIndex camera         = CameraIndex::Main;  ///< 调试相机
    ColorChannel colorChannel  = ColorChannel::Red;  ///< 调试颜色通道
    bool         roiEnabled    = true;  ///< ROI 是否启用
    int          roiModeIndex  = 0; ///< 界面 ROI 模式：0 不绘制 / 1 矩形 / 2 四点（不持久化）
    QList<double> rois;             ///< GetRoi 返回的扁平 double 序列

    bool operator==(const RecipeDebugToolbar &o) const;
    bool operator!=(const RecipeDebugToolbar &o) const { return !(*this == o); }

    void reflectFromJson(const json &j);
    void reflectToJson(json &j) const;
};

/** 按相机+颜色通道查找 debug ROI 条目；不存在时返回 nullptr */
const RecipeDebugToolbar *findDebugEntry(const QVector<RecipeDebugToolbar> &debugs,
                                         CameraIndex camera, ColorChannel channel);
/** 按相机+颜色通道查找或创建 debug ROI 条目 */
RecipeDebugToolbar &findOrCreateDebugEntry(QVector<RecipeDebugToolbar> &debugs,
                                           CameraIndex camera, ColorChannel channel);

/** 结果输出区芯片（单缺陷 RGB 统计） */
struct DefectResultChip
{
    DetectItemKind item = DetectItemKind::DeadLed; ///< 缺陷类型
    int            r = 0, g = 0, b = 0;            ///< RGB 分色统计
    int            total = 0; ///< <0 显示 "-"

    bool operator==(const DefectResultChip &o) const
    {
        return item == o.item && r == o.r && g == o.g && b == o.b && total == o.total;
    }
    bool operator!=(const DefectResultChip &o) const { return !(*this == o); }
};

/** 完整配方文档 */
struct RecipeDocument
{
    QString id;  ///< 配方 ID
    RecipeBasicInfo basic;                  ///< 基本信息
    QVector<PerCameraParams> cameras; ///< size kCameraCount
    AlgoThresholds algo;                  ///< 算法阈值
    QVector<SpecialDetectItem> specialItems; ///< 特殊检测项
    RecipeOtherSettings other;              ///< 其他参数
    QVector<RecipeDebugToolbar> debugs;     ///< 调试区 ROI 配置
    QVector<DefectResultChip> debugResultChips; ///< 调试区结果输出芯片

    bool operator==(const RecipeDocument &o) const;
    bool operator!=(const RecipeDocument &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, id);
        FROM_JSON_AUTO_SAME_NAME(j, basic);
        FROM_JSON_AUTO_SAME_NAME(j, cameras);
        FROM_JSON_AUTO_SAME_NAME(j, algo);
        FROM_JSON_AUTO_SAME_NAME(j, specialItems);
        FROM_JSON_AUTO_SAME_NAME(j, other);
        FROM_JSON_AUTO_SAME_NAME(j, debugs);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, id);
        TO_JSON_AUTO_SAME_NAME(j, basic);
        TO_JSON_AUTO_SAME_NAME(j, cameras);
        TO_JSON_AUTO_SAME_NAME(j, algo);
        TO_JSON_AUTO_SAME_NAME(j, specialItems);
        TO_JSON_AUTO_SAME_NAME(j, other);
        TO_JSON_AUTO_SAME_NAME(j, debugs);
    }

    /** 返回指定 ID 的演示默认配方文档 */
    static RecipeDocument defaultDemo(const QString &id);

    /** 保存至 exe 目录下 config/recipe/{id}.json（general 节点） */
    bool saveToFile() const;
    /** 保存至指定路径（general 节点，与 saveToFile 格式一致） */
    bool saveToFilePath(const QString &filePath) const;
    /** 从指定 JSON 文件加载 general 节点 */
    static bool loadFromFile(const QString &filePath, RecipeDocument &out);
};

/** 规范化配方 ID（去除非法文件名字符）；结果为空表示无效 */
QString sanitizeRecipeId(const QString &rawId);

/** 使 RecipeDocument::id 与 basic.name 保持一致（以 id 为准，id 为空时取 basic.name） */
void syncRecipeIdAndName(RecipeDocument &doc);

/** 暂时隐藏配方页图像质量 UI（顶栏 Pill、相机设置表格行）；恢复展示时改为 true */
inline constexpr bool kRecipeShowImageQualityUi = false;

/** 顶栏状态摘要 */
struct RecipeStatusBar
{
    QString currentRecipeName = QStringLiteral("PANEL-A01"); ///< 当前配方名称
    QString imageQualityText  = QStringLiteral("通过(92%)"); ///< 图像质量文本
    QString paramStatusText   = QStringLiteral("可保存");    ///< 参数状态文本
    bool    imageQualityOk    = true;  ///< 图像质量是否通过
    bool    canSave           = true;  ///< 是否允许保存

    bool operator==(const RecipeStatusBar &o) const
    {
        return currentRecipeName == o.currentRecipeName
               && imageQualityText == o.imageQualityText
               && paramStatusText == o.paramStatusText && imageQualityOk == o.imageQualityOk
               && canSave == o.canSave;
    }
    bool operator!=(const RecipeStatusBar &o) const { return !(*this == o); }
};

/** 单条配方导出结果 */
struct RecipeExportItemResult
{
    QString recipeId;   ///< 配方 ID
    QString recipeName; ///< 配方名称
    QString filePath;   ///< 导出文件路径
    bool    ok      = false; ///< 是否成功
    QString message;        ///< 结果说明
};

/** 一次导出操作的完整报告 */
struct RecipeExportReport
{
    QDateTime                  timestamp = QDateTime::currentDateTime(); ///< 导出时间
    QString                    logFilePath;       ///< 日志文件路径
    QString                    exportDirectory;     ///< 导出目录
    bool                       overallOk        = false; ///< 总体是否成功
    QString                    overallMessage;          ///< 总体结果说明
    bool                       attemptedSaveCurrent = false; ///< 是否尝试保存当前配方
    bool                       savedCurrentOk       = false; ///< 当前配方保存是否成功
    QString                    currentRecipeId;           ///< 当前配方 ID
    QString                    currentRecipeName;         ///< 当前配方名称
    QString                    currentSaveMessage;        ///< 当前配方保存说明
    QVector<RecipeExportItemResult> items;                ///< 各配方导出明细
};

/** 单条配方导入结果 */
struct RecipeImportItemResult
{
    QString filePath;   ///< 源 JSON 文件路径
    QString recipeId;   ///< 导入后的配方 ID
    QString recipeName; ///< 导入后的配方名称
    bool    ok      = false; ///< 是否成功
    QString message;        ///< 结果说明
};

/** 一次导入操作的完整报告 */
struct RecipeImportReport
{
    QDateTime                     timestamp = QDateTime::currentDateTime(); ///< 导入时间
    QString                       logFilePath;     ///< 日志文件路径
    bool                          overallOk      = false; ///< 总体是否成功
    QString                       overallMessage;         ///< 总体结果说明
    QVector<RecipeImportItemResult> items;                ///< 各文件导入明细
};

/**
 * @brief 配方目录变更通知（新增/删除/导入/重命名后发出，供虚拟像素等页面同步下拉列表）
 */
class RecipeCatalogNotifier : public QObject
{
    Q_OBJECT

public:
    /** 返回配方目录变更通知单例 */
    static RecipeCatalogNotifier *instance();

public slots:
    /** 配方列表变更后发出通知（新增/删除/导入/重命名） */
    void slot_notifyRecipeListChanged();
    /** 默认配方 ID 变更后发出，供首页 KPI 等同步「当前配方」显示（不刷新 FlowContext） */
    void slot_notifyDefaultRecipeChanged(const QString &recipeId);

signals:
    void sig_recipeListChanged();
    void sig_defaultRecipeChanged(const QString &recipeId);
};

/**
 * @brief 配方仓储（内存实现，后续可换持久化）
 */
class RecipeRepository
{
public:
    /** 加载 config/recipe；若默认配方存在则同步 config/current_recipe.json */
    static void                    load();
    /** 返回全部配方列表项 */
    static QVector<RecipeListItem> listRecipes();
    /** 按 ID 加载配方文档至 out */
    static bool                    load(const QString &recipeId, RecipeDocument &out);
    /** 判断仓储中是否包含指定配方 ID */
    static bool                    contains(const QString &recipeId);
    /** @param previousRecipeId 重命名前 ID；与 doc.id 不同时删除旧文件与仓储项 */
    static bool                    save(const RecipeDocument &doc,
                                       const QString &previousRecipeId = QString());
    /** 从仓储删除指定配方及其 JSON 文件 */
    static bool                    remove(const QString &recipeId);
    /** 设为默认并覆盖 config/current_recipe.json（先删后拷 config/recipe/{id}.json）；不刷新 FlowContext */
    static bool                    setDefault(const QString &recipeId);
    /** 返回当前默认配方 ID */
    static QString                 defaultRecipeId();
    /** 将全部配方导出为 {配方名}.json 至指定目录，并填充 report */
    static bool exportAllToDirectory(const QString &dirPath, RecipeExportReport &report);
    /** 将导出报告写入 exe/import_export/export_yyyyMMdd_HHmmss.txt，并设置 report.logFilePath */
    static void writeExportLog(RecipeExportReport &report);
    /** 从 JSON 文件批量导入配方（general 节点），并填充 report */
    static bool importFromJsonFiles(const QStringList &filePaths, RecipeImportReport &report);
    /** 将导入报告写入 exe/import_export/import_yyyyMMdd_HHmmss.txt，并设置 report.logFilePath */
    static void writeImportLog(RecipeImportReport &report);
};

} // namespace lighten

Q_DECLARE_METATYPE(lighten::RgbTriple)
Q_DECLARE_METATYPE(lighten::RecipeListItem)
Q_DECLARE_METATYPE(lighten::RecipeBasicInfo)
Q_DECLARE_METATYPE(lighten::PerCameraParams)
Q_DECLARE_METATYPE(lighten::AlgoThresholds)
Q_DECLARE_METATYPE(lighten::SpecialDetectItem)
Q_DECLARE_METATYPE(lighten::RecipeOtherSettings)
Q_DECLARE_METATYPE(lighten::RecipeDebugToolbar)
Q_DECLARE_METATYPE(lighten::DefectResultChip)
Q_DECLARE_METATYPE(lighten::RecipeDocument)
Q_DECLARE_METATYPE(lighten::RecipeStatusBar)
