#pragma once

/**
 * @file motion_axis_settings_view.h
 * @brief Motion 插件轴参数设置页（PageWidget）
 *
 * ## 页面结构
 * ```
 * [左导航]  多模块 → ThemedTreeWidget；单模块 → ThemedListWidget 轴名列表
 *           [添加轴] [删除轴]
 * [右 Splitter 垂直]
 *   ├── AxisParamFormWidget（静态参数 + 保存）
 *   └── AxisDebugPanel（运行时状态 + 调试操作）
 * ```
 *
 * 轴选中后同步 loadAxis / setAxisName；onShow 时 reloadAxisNav 并 refreshState。
 * 无轴时隐藏左右工作区，仅保留添加轴入口（需 motion.axis.add 权限）；二级导航由 MotionControlPlugin::syncMotionSubNav 控制。
 *
 * @see AxisParamFormWidget
 * @see AxisDebugPanel
 */

#include "pagewidget.h"

class QListWidgetItem;
class QSplitter;
class QLabel;

namespace ui {
class ThemedListWidget;
class ThemedPushButton;
class ThemedTreeWidget;
}

namespace motion_control_plugin {

class AxisDebugPanel;
class AxisParamFormWidget;
class IMotionControlService;

class MotionAxisSettingsView : public PageWidget
{
    Q_OBJECT

public:
    explicit MotionAxisSettingsView(QWidget *parent = nullptr);

protected:
    void onLoad() override;
    void onShow() override;
    void onHide() override;
    void applyTheme() override;
    void retranslateUi() override;

private:
    void buildLayout();
    void bindPermissions();
    void applyPermissions();
    /** reloadCatalog 后按模块数切换树/列表导航并选中首轴 */
    void reloadAxisNav();
    void onAxisSelected(const QString &moduleName, const QString &axisName);
    void onAxisListSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void onAddAxisClicked();
    void onDeleteAxisClicked();
    QString currentAxisName() const;

    IMotionControlService *m_service = nullptr;

    QWidget              *m_leftNav       = nullptr;
    ui::ThemedTreeWidget *m_tree          = nullptr; ///< axesByModule 多模块时分组树
    ui::ThemedListWidget *m_axisList      = nullptr; ///< 单模块时平铺轴名
    ui::ThemedPushButton *m_addAxisBtn    = nullptr;
    ui::ThemedPushButton *m_deleteAxisBtn = nullptr;
    QWidget              *m_navBtnRow     = nullptr; ///< 左导航底部增删按钮行
    QWidget              *m_emptyPanel    = nullptr;
    QLabel               *m_emptyLabel    = nullptr;
    ui::ThemedPushButton *m_emptyAddBtn   = nullptr; ///< 空状态「添加轴」
    QSplitter            *m_rightSplitter = nullptr;
    AxisParamFormWidget  *m_paramForm     = nullptr;
    AxisDebugPanel       *m_debugPanel    = nullptr;
    QString               m_currentAxisName;
};

} // namespace motion_control_plugin
