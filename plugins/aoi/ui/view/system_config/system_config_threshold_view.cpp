/**
 * @file system_config_threshold_view.cpp
 * @brief 系统配置 · 阈值范围子页实现
 */

#include "recipe_config_model.h"
#include "inspection_types.h"

#include "system_config/system_config_threshold_view.h"

#include "system_config/system_config_helpers.h"

#include "component/behavior/no_wheel_input_guard.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "component/group/form_group_box.h"
#include "system_config_view_model.h"
#include "binding_options.h"
#include "databinding.h"

#include "uibase/languagemanager.h"

#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

// ── 构造 / 生命周期 ──

SystemConfigThresholdView::SystemConfigThresholdView(SystemConfigViewModel *viewModel,
                                                     QWidget *parent)
    : PageWidget(parent)
    , m_viewModel(viewModel)
{
}

void SystemConfigThresholdView::onLoad()
{
    buildLayout();
    ui::NoWheelInputGuard::installOn(this);
    ensureThemedSpinStyles(this);
    setupBindings();
    retranslateUi();
}

void SystemConfigThresholdView::buildLayout()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *inner = new QWidget(scroll);
    auto *pageVl = new QVBoxLayout(inner);
    pageVl->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    auto *sec = new ui::FormGroupBox(QStringLiteral("aoi.sys.section.threshold_title"),
                                 ui::FormGroupBoxTitleStyle::Static, inner);
    sec->setMaximumGroupWidth(560);

    m_threshLo.resize(inspection::kInspectionDefectTypeCount);
    m_threshHi.resize(inspection::kInspectionDefectTypeCount);
    m_threshNameLabels.resize(inspection::kInspectionDefectTypeCount);

    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const auto kind = static_cast<inspection::InspectionDefectType>(i);
        const QString label =
            defectItemDisplayText(inspection::inspectionDefectTypeDisplayKey(kind))
            + TR("aoi.sys.threshold.quantity_suffix");
        ui::ThemedSpinBox *lo = nullptr;
        ui::ThemedSpinBox *hi = nullptr;
        QLabel        *nameLbl = nullptr;
        sec->addThresholdBlock(label, 0, 3, &lo, &hi, &nameLbl);
        m_threshLo[i] = lo;
        m_threshHi[i] = hi;
        m_threshNameLabels[i] = nameLbl;
    }

    const QString totalLabel =
        TR("aoi.sys.threshold.total_defects") + TR("aoi.sys.threshold.quantity_suffix");
    sec->addThresholdBlock(totalLabel, 0, 5, &m_totalLo, &m_totalHi, &m_totalNameLabel);

    pageVl->addWidget(sec, 0, Qt::AlignLeft);
    pageVl->addStretch();

    scroll->setWidget(inner);
    root->addWidget(scroll);
}

QString SystemConfigThresholdView::defectThresholdLabel(inspection::InspectionDefectType kind) const
{
    QString base;
    if (m_viewModel)
        base = defectItemDisplayText(m_viewModel->config().defects.item(kind).displayName);
    if (base.isEmpty())
        base = TR(inspection::inspectionDefectTypeDisplayKey(kind).toUtf8().constData());
    return base + TR("aoi.sys.threshold.quantity_suffix");
}

// ── 数据绑定 ──

void SystemConfigThresholdView::setupBindings()
{
    if (!m_viewModel)
        return;

    using VM = SystemConfigViewModel;
    ViewModel *const vm   = m_viewModel;
    const QString    prop = VM::PROP_CONFIG;
    const BindingOptions onChanged;

    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const inspection::InspectionDefectType kind = static_cast<inspection::InspectionDefectType>(i);
        if (i < m_threshLo.size() && m_threshLo[i]) {
            DataBinding::bindStructField<AoiSystemConfig>(
                vm, prop, [kind](const AoiSystemConfig &c) { return c.defects.item(kind).lower; },
                [kind](AoiSystemConfig &c, int v) { c.defects.item(kind).lower = v; },
                static_cast<QSpinBox *>(m_threshLo[i]), onChanged, &m_bindings);
        }
        if (i < m_threshHi.size() && m_threshHi[i]) {
            DataBinding::bindStructField<AoiSystemConfig>(
                vm, prop, [kind](const AoiSystemConfig &c) { return c.defects.item(kind).upper; },
                [kind](AoiSystemConfig &c, int v) { c.defects.item(kind).upper = v; },
                static_cast<QSpinBox *>(m_threshHi[i]), onChanged, &m_bindings);
        }
    }

    DataBinding::bindStructField<AoiSystemConfig>(
        vm, prop, [](const AoiSystemConfig &c) { return c.defects.total.lower; },
        [](AoiSystemConfig &c, int v) { c.defects.total.lower = v; },
        static_cast<QSpinBox *>(m_totalLo), onChanged, &m_bindings);
    DataBinding::bindStructField<AoiSystemConfig>(
        vm, prop, [](const AoiSystemConfig &c) { return c.defects.total.upper; },
        [](AoiSystemConfig &c, int v) { c.defects.total.upper = v; },
        static_cast<QSpinBox *>(m_totalHi), onChanged, &m_bindings);
}

void SystemConfigThresholdView::retranslateUi()
{
    for (auto *sec : findChildren<ui::FormGroupBox *>())
        sec->retranslateGroup();

    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        if (i < m_threshNameLabels.size() && m_threshNameLabels[i])
            m_threshNameLabels[i]->setText(defectThresholdLabel(static_cast<inspection::InspectionDefectType>(i)));
    }
    if (m_totalNameLabel) {
        m_totalNameLabel->setText(TR("aoi.sys.threshold.total_defects")
                                  + TR("aoi.sys.threshold.quantity_suffix"));
    }
}

} // namespace aoi
