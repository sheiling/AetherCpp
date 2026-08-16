/**
 * @file system_config_switch_view.cpp
 * @brief 系统配置 · 开关设置子页实现
 */

#include "recipe_config_model.h"
#include "inspection_types.h"

#include "system_config/system_config_switch_view.h"

#include "system_config/system_config_helpers.h"

#include "component/behavior/no_wheel_input_guard.h"
#include "aoi_param_binding.h"
#include "component/group/form_group_box.h"
#include "system_config_view_model.h"
#include "binding_options.h"

#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>

namespace aoi {

// ── 构造 / 生命周期 ──

SystemConfigSwitchView::SystemConfigSwitchView(SystemConfigViewModel *viewModel, QWidget *parent)
    : PageWidget(parent)
    , m_viewModel(viewModel)
{
}

void SystemConfigSwitchView::onLoad()
{
    buildLayout();
    ui::NoWheelInputGuard::installOn(this);
    setupBindings();
    retranslateUi();
}

void SystemConfigSwitchView::buildLayout()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *inner = new QWidget(scroll);
    auto *vl = new QVBoxLayout(inner);
    vl->setSpacing(14);
    vl->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    auto *secGeneral = new ui::FormGroupBox(QStringLiteral("aoi.sys.sub.general_switch"),
                                        ui::FormGroupBoxTitleStyle::Static, inner);
    secGeneral->setMinimumGroupWidth(300);
    secGeneral->setMaximumGroupWidth(520);
    m_sections.append(secGeneral);
    m_scanSwitch   = secGeneral->addSwitchRow(QStringLiteral("aoi.sys.switch.scan"), true);
    m_reviewSwitch = secGeneral->addSwitchRow(QStringLiteral("aoi.sys.switch.review"), true);
    vl->addWidget(secGeneral, 0, Qt::AlignLeft);

    auto *secIface = new ui::FormGroupBox(QStringLiteral("aoi.sys.sub.interface_switch"),
                                      ui::FormGroupBoxTitleStyle::Static, inner);
    secIface->setMinimumGroupWidth(300);
    secIface->setMaximumGroupWidth(520);
    m_sections.append(secIface);
    m_upstreamSwitch   = secIface->addSwitchRow(QStringLiteral("aoi.sys.switch.upstream"), true);
    m_downstreamSwitch = secIface->addSwitchRow(QStringLiteral("aoi.sys.switch.downstream"), false);
    vl->addWidget(secIface, 0, Qt::AlignLeft);

    auto *secDefect = new ui::FormGroupBox(QStringLiteral("aoi.sys.section.defect_switch"),
                                       ui::FormGroupBoxTitleStyle::Static, inner);
    secDefect->setMinimumGroupWidth(300);
    secDefect->setMaximumGroupWidth(520);
    m_sections.append(secDefect);

    m_defectSwitches.resize(inspection::kInspectionDefectTypeCount);
    m_defectLabels.resize(inspection::kInspectionDefectTypeCount);
    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const auto  kind = static_cast<inspection::InspectionDefectType>(i);
        const QString key = inspection::inspectionDefectTypeDisplayKey(kind);
        QLabel *lbl = nullptr;
        m_defectSwitches[i] = secDefect->addSwitchRow(key, true, &lbl);
        m_defectLabels[i]   = lbl;
    }
    vl->addWidget(secDefect, 0, Qt::AlignLeft);
    vl->addStretch();

    scroll->setWidget(inner);
    root->addWidget(scroll);
}

// ── 数据绑定 ──

void SystemConfigSwitchView::setupBindings()
{
    if (!m_viewModel)
        return;

    using VM = SystemConfigViewModel;
    ViewModel *const vm   = m_viewModel;
    const QString    prop = VM::PROP_CONFIG;
    const BindingOptions onChanged;

    bindStructFieldPillSwitch<AoiSystemConfig>(
        vm, prop, [](const AoiSystemConfig &c) { return c.general.scanEnabled; },
        [](AoiSystemConfig &c, bool v) { c.general.scanEnabled = v; }, m_scanSwitch, onChanged,
        &m_bindings);
    bindStructFieldPillSwitch<AoiSystemConfig>(
        vm, prop, [](const AoiSystemConfig &c) { return c.general.reviewEnabled; },
        [](AoiSystemConfig &c, bool v) { c.general.reviewEnabled = v; }, m_reviewSwitch, onChanged,
        &m_bindings);
    bindStructFieldPillSwitch<AoiSystemConfig>(
        vm, prop, [](const AoiSystemConfig &c) { return c.iface.upstreamEnabled; },
        [](AoiSystemConfig &c, bool v) { c.iface.upstreamEnabled = v; }, m_upstreamSwitch,
        onChanged, &m_bindings);
    bindStructFieldPillSwitch<AoiSystemConfig>(
        vm, prop, [](const AoiSystemConfig &c) { return c.iface.downstreamEnabled; },
        [](AoiSystemConfig &c, bool v) { c.iface.downstreamEnabled = v; }, m_downstreamSwitch,
        onChanged, &m_bindings);

    for (int i = 0; i < inspection::kInspectionDefectTypeCount && i < m_defectSwitches.size(); ++i) {
        if (!m_defectSwitches[i])
            continue;
        const inspection::InspectionDefectType kind = static_cast<inspection::InspectionDefectType>(i);
        bindStructFieldPillSwitch<AoiSystemConfig>(
            vm, prop, [kind](const AoiSystemConfig &c) { return c.defects.item(kind).enabled; },
            [kind](AoiSystemConfig &c, bool v) { c.defects.item(kind).enabled = v; },
            m_defectSwitches[i], onChanged, &m_bindings);
    }
}

void SystemConfigSwitchView::retranslateUi()
{
    for (auto *sec : m_sections)
        sec->retranslateGroup();

    if (!m_viewModel)
        return;

    const AoiSystemConfig cfg = m_viewModel->config();
    for (int i = 0; i < inspection::kInspectionDefectTypeCount && i < m_defectLabels.size(); ++i) {
        if (!m_defectLabels[i])
            continue;
        const inspection::InspectionDefectType kind = static_cast<inspection::InspectionDefectType>(i);
        QString text = defectItemDisplayText(cfg.defects.item(kind).displayName);
        if (text.isEmpty())
            text = TR(inspection::inspectionDefectTypeDisplayKey(kind).toUtf8().constData());
        m_defectLabels[i]->setText(text);
    }
}

} // namespace aoi
