#pragma once

/**
 * @file motion_io_settings_view.h
 * @brief Motion 插件 IO 参数设置页（PageWidget）
 *
 * ## 页面结构
 * ```
 * [左导航] 模块列表（多模块时）
 * [输入区 SectionPanel | 输出区 SectionPanel]  各含 IoParamFormWidget 网格
 * [添加IO] [删除IO] [保存全部]
 * ```
 *
 * ## 生命周期
 * - onLoad：解析 IMotionControlService、建布局、订阅 ioStatesUpdated
 * - onShow：开 autoRefresh、reloadModuleList、刷新权限与 IO 状态
 * - onHide：关 autoRefresh
 *
 * ## 权限（MotionActionKeys）
 * | 权限 | 行为 |
 * |------|------|
 * | IoEdit | 显示参数区、启用保存全部（可编辑即可保存） |
 * | IoWrite | form.setOutputClickEnabled（输出标题栏点击翻转） |
 * | IoAdd | 显示「添加 IO」按钮 |
 * | IoDelete | 显示「删除 IO」；选中卡片后可用 |
 *
 * @see IoParamFormWidget
 */

#include "pagewidget.h"

class QListWidgetItem;
class QLabel;

namespace ui {
class ThemedListWidget;
class ThemedSectionPanel;
class ThemedPushButton;
} // namespace ui

class QGridLayout;

namespace motion_control_plugin {

class IoParamFormWidget;
class IMotionControlService;

class MotionIoSettingsView : public PageWidget
{
    Q_OBJECT

public:
    explicit MotionIoSettingsView(QWidget *parent = nullptr);

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
    /** reloadCatalog + 重建模块列表；单模块时直接 loadModule */
    void reloadModuleList();
    void onModuleSelected(QListWidgetItem *current, QListWidgetItem *previous);
    /** 清空网格，按模块 IO 名排序后创建 IoParamFormWidget 填入 input/output 网格 */
    void loadModule(const QString &moduleKey);
    void clearIoForms();
    void refreshIoFormStates();
    void onSaveAllClicked();
    void onAddIoClicked();
    void onDeleteIoClicked();
    void onDeleteIoRequested(const QString &ioName);
    void setSelectedIoForm(IoParamFormWidget *form);
    int ioGridColumnCount() const;
    static QString moduleDisplayName(const QString &moduleKey);

    IMotionControlService *m_service = nullptr;

    QWidget              *m_leftNav      = nullptr;
    ui::ThemedListWidget *m_moduleList   = nullptr;
    ui::ThemedPushButton *m_addIoBtn     = nullptr;
    QWidget              *m_emptyPanel   = nullptr;
    QLabel               *m_emptyLabel   = nullptr;
    ui::ThemedPushButton *m_emptyAddBtn  = nullptr;
    QWidget              *m_rightPanel   = nullptr;
    ui::ThemedSectionPanel *m_inputPanel  = nullptr;
    ui::ThemedSectionPanel *m_outputPanel = nullptr;
    QWidget                *m_inputHost   = nullptr;
    QWidget                *m_outputHost  = nullptr;
    QGridLayout            *m_inputGrid   = nullptr;
    QGridLayout            *m_outputGrid  = nullptr;
    ui::ThemedPushButton   *m_deleteIoBtn = nullptr;
    ui::ThemedPushButton   *m_saveAllBtn  = nullptr;

    QString                    m_currentModuleKey;
    QString                    m_selectedIoName;
    QStringList                m_cardNames;
    QList<IoParamFormWidget *> m_ioForms;
};

} // namespace motion_control_plugin
