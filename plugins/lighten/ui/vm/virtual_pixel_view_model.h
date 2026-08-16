#pragma once

/**
 * @file virtual_pixel_view_model.h
 * @brief 虚拟像素 ViewModel（三通道配置、配方关联、日志）
 */

#include "lighten_viewmodel_macros.h"
#include "command.h"
#include "virtual_pixel_model.h"

namespace lighten {

/**
 * @brief 虚拟像素 ViewModel
 *
 * 绑定 VirtualPixelView：观察区相机/颜色、三通道配置、配方选择、日志。
 */
class VirtualPixelViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_DOCUMENT       = "document";
    static constexpr const char *PROP_ACTIVE_CHANNEL = "activeChannel";
    static constexpr const char *PROP_STATUS         = "status";

    LIGHTEN_VIEWMODEL_STRUCT_PROPERTY(VirtualPixelDocument, document, Document)
    LIGHTEN_VIEWMODEL_PROPERTY(int, activeChannel, ActiveChannel)
    LIGHTEN_VIEWMODEL_PROPERTY(QString, status, Status)

signals:
    void sig_documentChanged();
    void sig_activeChannelChanged();
    void sig_statusChanged();
    /** 每次保存写入成功时发出（与 status 是否变化无关） */
    void sig_saveSucceeded();

public:
    explicit VirtualPixelViewModel(QObject *parent = nullptr);

    /** 保存虚拟像素配置命令 */
    Command *saveCommand() const { return m_saveCommand; }
    /** 按配方加载虚拟像素配置命令 */
    Command *loadRecipeCommand() const { return m_loadRecipeCommand; }

public slots:
    /** 按配方 ID 加载虚拟像素文档 */
    void slot_loadForRecipe(const QString &recipeId);
    /** 保存当前虚拟像素文档 */
    void slot_save();
    /** 更新指定 RGB 通道的配置 */
    void slot_setChannelConfig(int rgbIndex, const VirtualPixelChannelConfig &cfg);
    /** 追加一条运行日志 */
    void slot_appendLog(const QString &message, const QString &level = QStringLiteral("INFO"));

private:
    FunctionalCommand *m_saveCommand       = nullptr;
    FunctionalCommand *m_loadRecipeCommand = nullptr;
};

} // namespace lighten
