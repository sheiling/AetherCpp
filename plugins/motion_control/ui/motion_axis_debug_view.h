#pragma once

/**
 * @file motion_axis_debug_view.h
 * @brief Motion 插件轴调试页（PageWidget）
 *
 * ## 页面结构
 * ```
 * [模块列表]（仅 axesByModule 模块数 > 1 时显示）
 * [滚动区] 2 列网格 AxisDebugPanel（当前模块下全部轴）
 * ```
 *
 * @see AxisDebugPanel
 */

#include "pagewidget.h"

class QListWidgetItem;

namespace ui {
class ThemedListWidget;
} // namespace ui

class QGridLayout;

namespace motion_control_plugin {

class AxisDebugPanel;
class IMotionControlService;

class MotionAxisDebugView : public PageWidget
{
    Q_OBJECT

public:
    explicit MotionAxisDebugView(QWidget *parent = nullptr);

protected:
    void onLoad() override;
    void onShow() override;
    void onHide() override;
    void applyTheme() override;

private:
    void buildLayout();
    void reloadModuleList();
    void onModuleSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void loadModule(const QString &moduleKey);
    void clearDebugPanels();
    void refreshDebugPanels();
    static QString moduleDisplayName(const QString &moduleKey);

    IMotionControlService *m_service = nullptr;

    ui::ThemedListWidget *m_moduleList = nullptr;
    QWidget              *m_panelHost  = nullptr;
    QGridLayout          *m_panelGrid   = nullptr;

    QString                  m_currentModuleKey;
    QList<AxisDebugPanel *> m_debugPanels;
};

} // namespace motion_control_plugin
