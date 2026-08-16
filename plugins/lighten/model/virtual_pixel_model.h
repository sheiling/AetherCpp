#pragma once

/**
 * @file virtual_pixel_model.h
 * @brief 虚拟像素配置数据模型（三通道布局、映射规则、文档持久化）
 */

#include <common/utility/json_tool/reflector.h>
#include <common/utility/json_tool/qt_json_adapters.h>
#include "lighten_types.h"
#include "home_model.h"

#include <QString>
#include <QVector>
#include <QMetaType>

namespace lighten {

/** 单通道（R/G/B）虚拟像素配置 */
struct VirtualPixelChannelConfig
{
    int     layoutDemoId = 0; ///< 0 正常 / 1 1-4 / 2 2-3
    int     lampRows     = 64; ///< 灯珠行数
    int     lampCols     = 128; ///< 灯珠列数
    QString mapRule0;  ///< 映射规则 0
    QString mapRule1;  ///< 映射规则 1
    QString mapRule2;  ///< 映射规则 2

    bool operator==(const VirtualPixelChannelConfig &o) const
    {
        return layoutDemoId == o.layoutDemoId && lampRows == o.lampRows && lampCols == o.lampCols
               && mapRule0 == o.mapRule0 && mapRule1 == o.mapRule1 && mapRule2 == o.mapRule2;
    }
    bool operator!=(const VirtualPixelChannelConfig &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, layoutDemoId);
        FROM_JSON_AUTO_SAME_NAME(j, lampRows);
        FROM_JSON_AUTO_SAME_NAME(j, lampCols);
        FROM_JSON_AUTO_SAME_NAME(j, mapRule0);
        FROM_JSON_AUTO_SAME_NAME(j, mapRule1);
        FROM_JSON_AUTO_SAME_NAME(j, mapRule2);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, layoutDemoId);
        TO_JSON_AUTO_SAME_NAME(j, lampRows);
        TO_JSON_AUTO_SAME_NAME(j, lampCols);
        TO_JSON_AUTO_SAME_NAME(j, mapRule0);
        TO_JSON_AUTO_SAME_NAME(j, mapRule1);
        TO_JSON_AUTO_SAME_NAME(j, mapRule2);
    }
};

/** 虚拟像素页观察区状态 */
struct VirtualPixelObserveState
{
    CameraIndex  camera        = CameraIndex::Main;  ///< 观察区相机
    ColorChannel colorChannel  = ColorChannel::Red;  ///< 观察区颜色通道

    bool operator==(const VirtualPixelObserveState &o) const
    {
        return camera == o.camera && colorChannel == o.colorChannel;
    }
    bool operator!=(const VirtualPixelObserveState &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, camera);
        FROM_JSON_AUTO_SAME_NAME(j, colorChannel);
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, camera);
        TO_JSON_AUTO_SAME_NAME(j, colorChannel);
    }
};

/** 三通道配置 + 当前配方 */
struct VirtualPixelDocument
{
    QString selectedRecipeId = QStringLiteral("PANEL-A01"); ///< 当前选中配方 ID
    VirtualPixelChannelConfig channels[3]; ///< R,G,B
    VirtualPixelObserveState  observe;     ///< 观察区状态
    QVector<RunLogEntry>      logs;        ///< 页面运行日志

    bool operator==(const VirtualPixelDocument &o) const;
    bool operator!=(const VirtualPixelDocument &o) const { return !(*this == o); }

    void reflectFromJson(const json &j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, selectedRecipeId);
        FROM_JSON_AUTO_SAME_NAME(j, observe);

        if (j.contains("channels") && j["channels"].is_array()) {
            const auto &arr = j["channels"];
            for (int i = 0; i < 3 && i < static_cast<int>(arr.size()); ++i)
                fromJson(arr[static_cast<size_t>(i)], "", channels[i]);
        }
    }

    void reflectToJson(json &j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, selectedRecipeId);
        TO_JSON_AUTO_SAME_NAME(j, observe);

        json arr = json::array();
        for (int i = 0; i < 3; ++i) {
            json ch;
            toJson(ch, channels[i]);
            arr.push_back(ch);
        }
        j["channels"] = arr;
    }

    /** 返回指定配方的默认虚拟像素文档 */
    static VirtualPixelDocument defaultDemo(const QString &recipeId = QStringLiteral("PANEL-A01"));

    /** 保存至 exe 目录下 config/recipe/{selectedRecipeId}.json（virtualPixel 节点，保留 general） */
    bool saveToFile() const;
    /** 保存至指定路径（virtualPixel 节点，与 saveToFile 格式一致） */
    bool saveToFilePath(const QString &filePath) const;
    /** 从指定 JSON 文件加载 virtualPixel 节点 */
    static bool loadFromFile(const QString &filePath, VirtualPixelDocument &out);
};

/**
 * @brief 虚拟像素仓储（与 RecipeRepository 共用 config/recipe/{id}.json）
 */
class VirtualPixelRepository
{
public:
    /** 加载 config/recipe 下各配方的 virtualPixel 节点 */
    static void load();
    /** 按配方 ID 加载虚拟像素文档至 out */
    static bool load(const QString &recipeId, VirtualPixelDocument &out);
    /** 判断仓储或磁盘上是否存在指定配方的虚拟像素配置 */
    static bool contains(const QString &recipeId);
    /** 保存虚拟像素文档至 config/recipe/{selectedRecipeId}.json */
    static bool save(const VirtualPixelDocument &doc);
    /** 从内存仓储移除（配方 JSON 被删除时由 RecipeRepository 调用） */
    static void evictFromStore(const QString &recipeId);
    /** 配方 ID 重命名时迁移 virtualPixel 节点与内存项 */
    static bool migrateRecipeId(const QString &fromId, const QString &toId);
};

} // namespace lighten

Q_DECLARE_METATYPE(lighten::VirtualPixelChannelConfig)
Q_DECLARE_METATYPE(lighten::VirtualPixelObserveState)
Q_DECLARE_METATYPE(lighten::VirtualPixelDocument)
