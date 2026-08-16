/**
 * @file system_config_general_view.cpp
 * @brief 系统配置 · 通用配置子页实现
 */

#include "system_config/system_config_general_view.h"

#include "component/behavior/no_wheel_input_guard.h"
#include "component/group/form_group_box.h"
#include "system_config_view_model.h"
#include "system_config_model.h"
#include "binding_options.h"
#include "databinding.h"

#include <QFrame>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>

namespace aoi {

// ── 构造 / 生命周期 ──

SystemConfigGeneralView::SystemConfigGeneralView(SystemConfigViewModel *viewModel, QWidget *parent)
    : PageWidget(parent)
    , m_viewModel(viewModel)
{
}

void SystemConfigGeneralView::onLoad()
{
    buildLayout();
    ui::NoWheelInputGuard::installOn(this);
    setupBindings();
    retranslateUi();
}

void SystemConfigGeneralView::buildLayout()
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

    auto *sec = new ui::FormGroupBox(QStringLiteral("aoi.sys.section.general_title"),
                                 ui::FormGroupBoxTitleStyle::Static, inner);
    sec->setMaximumGroupWidth(820);
    sec->setMinimumGroupWidth(520);

    const char *pathKeys[] = {
        "aoi.sys.path.image",
        "aoi.sys.path.result",
        "aoi.sys.path.third_party",
    };
    m_pathEdits.resize(3);
    for (int i = 0; i < 3; ++i)
        m_pathEdits[i] = sec->addPathRow(QString::fromUtf8(pathKeys[i]));

    vl->addWidget(sec, 0, Qt::AlignLeft);
    vl->addStretch();

    scroll->setWidget(inner);
    root->addWidget(scroll);
}

// ── 数据绑定 ──

void SystemConfigGeneralView::setupBindings()
{
    if (!m_viewModel || m_pathEdits.size() < 3)
        return;

    using VM = SystemConfigViewModel;
    ViewModel *const vm   = m_viewModel;
    const QString    prop = VM::PROP_CONFIG;

    BindingOptions onLostFocus;
    onLostFocus.trigger = UpdateSourceTrigger::OnLostFocus;

    DataBinding::bindStructField<AoiSystemConfig>(
        vm, prop, [](const AoiSystemConfig &c) { return c.paths.imagePath; },
        [](AoiSystemConfig &c, const QString &v) { c.paths.imagePath = v; }, m_pathEdits[0],
        onLostFocus, &m_bindings);
    DataBinding::bindStructField<AoiSystemConfig>(
        vm, prop, [](const AoiSystemConfig &c) { return c.paths.resultPath; },
        [](AoiSystemConfig &c, const QString &v) { c.paths.resultPath = v; }, m_pathEdits[1],
        onLostFocus, &m_bindings);
    DataBinding::bindStructField<AoiSystemConfig>(
        vm, prop, [](const AoiSystemConfig &c) { return c.paths.thirdPartyPath; },
        [](AoiSystemConfig &c, const QString &v) { c.paths.thirdPartyPath = v; }, m_pathEdits[2],
        onLostFocus, &m_bindings);
}

void SystemConfigGeneralView::retranslateUi()
{
    for (auto *sec : findChildren<ui::FormGroupBox *>())
        sec->retranslateGroup();
}

} // namespace aoi
