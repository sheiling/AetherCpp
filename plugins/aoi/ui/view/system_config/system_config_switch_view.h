#pragma once

/**
 * @file system_config_switch_view.h
 * @brief 系统配置 · 开关设置子页（二级导航子页之一）
 *
 * 挂载于 `PageWithSubNav` 容器，与通用配置、阈值范围共享同一 `SystemConfigViewModel`。
 * 绑定 scan/review、上下游对接及各缺陷类型 enabled 开关；样式依赖全局 ACSS。
 */

#include "pagewidget.h"
#include "binding_session.h"

#include <QVector>

class QLabel;

namespace ui {

class FormGroupBox;
class PillSwitch;

} // namespace ui

namespace aoi {

class SystemConfigViewModel;

/**
 * @brief 开关设置：通用开关、对接开关、各缺陷类型启用开关（各为独立 FormGroupBox）
 *
 * 数据映射：
 * - `general.scanEnabled` / `reviewEnabled`
 * - `iface.upstreamEnabled` / `downstreamEnabled`
 * - `defects[kind].enabled`（`kDefectKindCount` 项）
 *
 * 绑定策略：全部 `OnChanged`；持久化由容器底栏 `saveCommand` 触发。
 */
class SystemConfigSwitchView : public PageWidget
{
    Q_OBJECT

public:
    explicit SystemConfigSwitchView(SystemConfigViewModel *viewModel, QWidget *parent = nullptr);

    void updateAllSources() { m_bindings.updateAllSources(); }

protected:
    void onLoad() override;
    void retranslateUi() override;

private:
    void buildLayout();
    void setupBindings();

    SystemConfigViewModel *m_viewModel = nullptr;
    BindingSession         m_bindings;

    QVector<ui::FormGroupBox *> m_sections;

    ui::PillSwitch *m_scanSwitch = nullptr;
    ui::PillSwitch *m_reviewSwitch = nullptr;
    ui::PillSwitch *m_upstreamSwitch = nullptr;
    ui::PillSwitch *m_downstreamSwitch = nullptr;
    QVector<ui::PillSwitch *> m_defectSwitches;
    QVector<QLabel *>     m_defectLabels;
};

} // namespace aoi
