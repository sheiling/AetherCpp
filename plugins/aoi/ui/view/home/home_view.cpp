/**
 * @file home_view.cpp
 * @brief 首页 View：薄壳布局 + HomeViewModel 分区 property 绑定
 *
 * 架构约定：
 * - 侧栏设备控制按钮 → HomeViewModel::Command（canExecute/execute）
 * - KPI / 硬件 / 流程 / 虚拟图 / 日志 / 快照 / 预览 ← `binding/home_runtime_binding` 监听 runtime 属性
 *
 * 布局比例参考设计稿 `首页.png` / ui_aoi home 模块（左栏固定 + 主区 KPI 行 + 62/38 主体）。
 */

#include "home/home_view.h"

#include "binding/role_image_preview_binding.h"
#include "component/behavior/no_wheel_input_guard.h"
#include "component/controls/button/themed_cyber_pill_button.h"
#include "component/controls/label/kpi_metric_card.h"
#include "inspection_types.h"
#include "home/device_control_panel.h"
#include "component/disk/disk_storage_panel.h"
#include "home/hardware_status_panel.h"
#include "component/group/themed_section_panel.h"
#include "home/home_process_flow_widget.h"
#include "home/panel_virtual_map_widget.h"
#include "component/image/image_preview.h"
#include "component/logView/log_view.h"
#include "component/table/themed_table_widget.h"
#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"
#include "binding/command_button_binding.h"
#include "binding/home_runtime_binding.h"
#include "camera_types.h"
#include "extensionsystem/pluginmanager.h"
#include "icamera_service.h"
#include "vm/home/home_view_model.h"

#include "command.h"
#include "uibase/languagemanager.h"
#include "uibase/permission/permservice.h"

#include <QAbstractButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMargins>
#include <QPushButton>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

constexpr int kSnapshotMetaRowH = 40;

/** 透明背景 host，用于预览区 mark/fov 子块 */
QWidget *makeTransparentHost(QWidget *parent)
{
    auto *host = new QWidget(parent);
    host->setAttribute(Qt::WA_StyledBackground, true);
    host->setAutoFillBackground(false);
    host->setStyleSheet(QStringLiteral("background: transparent;"));
    return host;
}

/** 固定高度 meta 行容器（快照 SN / 总状态） */
QWidget *makeFixedHeightRow(QWidget *parent, int height, const char *objectName)
{
    auto *row = new QWidget(parent);
    row->setObjectName(QString::fromLatin1(objectName));
    row->setAttribute(Qt::WA_StyledBackground, true);
    row->setAutoFillBackground(false);
    row->setFixedHeight(height);
    row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    row->setStyleSheet(QStringLiteral("background: transparent;"));
    return row;
}

/** 快照 meta label：水平撑满 + 对齐 */
void pinMetaLabel(QLabel *label, Qt::Alignment align)
{
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label->setAlignment(align);
}

} // namespace

// ── 构造 / 生命周期 ──

HomeView::HomeView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId(QStringLiteral("aoi.home"));
}

void HomeView::onLoad()
{
    m_viewModel = new HomeViewModel(this);
    buildLayout();
    setupBindings();
    if (m_viewModel)
        m_viewModel->refreshHardwareStatus();
    ui::NoWheelInputGuard::installOn(this);

    // 主题切换时重刷 QSS，并重新推送 runtime 至各分区绑定
    bindThemeRefresh(this, [this]() {
        applyTheme();
        notifyRuntimeRefresh();
    });

    applyTheme();
    retranslateUi();
}

// ── 主题 ──

void HomeView::applyTheme()
{
    setObjectName(QStringLiteral("aoiHomeRoot"));
    setAttribute(Qt::WA_StyledBackground, true);
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    setStyleSheet(QStringLiteral("QWidget#aoiHomeRoot { background-color: %1; }").arg(t.background.name()));
    applyHomeDecorStyles();
}

void HomeView::applyHomeDecorStyles()
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    const QString decor = QStringLiteral(
        "QWidget#homeSnapshotMetaRow {"
        "  min-height: %4px; max-height: %4px; height: %4px; background: transparent;"
        "}"
        "QLabel#homeSnapshotMeta { color: %1; background: transparent; font-size: 12px; }"
        "QLabel#homeSnapshotStatusOk { color: %2; font-weight: 600; background: transparent; }"
        "QLabel#homeSnapshotStatusNg { color: %3; font-weight: 600; background: transparent; }"
        "QLabel#homePreviewCaption { color: %1; font-size: 11px; background: transparent; }")
                              .arg(t.textSecondary.name(),
                                   t.accent.name(),
                                   t.dangerText.name(),
                                   QString::number(kSnapshotMetaRowH));
    const QString rootBg =
        QStringLiteral("QWidget#aoiHomeRoot { background-color: %1; }").arg(t.background.name());
    setStyleSheet(rootBg + decor);
}

// ── 布局子模块 ──

void HomeView::buildSidebar()
{
    m_sidebar = new QWidget(this);
    m_sidebar->setObjectName(QStringLiteral("homeSidebar"));
    m_sidebar->setAttribute(Qt::WA_StyledBackground, true);
    m_sidebar->setFixedWidth(350);
    m_sidebar->setStyleSheet(QStringLiteral("QWidget#homeSidebar { background: transparent; }"));

    auto *layout = new QVBoxLayout(m_sidebar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_deviceControlPanel = new DeviceControlPanel(m_sidebar);
    layout->addWidget(m_deviceControlPanel, 1);

    auto *bottomHost = new QWidget(m_sidebar);
    bottomHost->setAttribute(Qt::WA_StyledBackground, true);
    bottomHost->setStyleSheet(QStringLiteral("background: transparent;"));
    auto *bottomLayout = new QVBoxLayout(bottomHost);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(8);

    m_hardwarePanel = new HardwareStatusPanel(bottomHost);
    m_diskPanel     = new ui::DiskStoragePanel(bottomHost);
    bottomLayout->addWidget(m_hardwarePanel, 2);
    bottomLayout->addWidget(m_diskPanel, 1);

    layout->addWidget(bottomHost, 2);
}

QHBoxLayout *HomeView::buildKpiRow(QWidget *parent)
{
    auto *kpiRow = new QHBoxLayout;
    kpiRow->setSpacing(4);
    const char *kpiKeys[] = {
        "aoi.home.kpi.total", "aoi.home.kpi.ok", "aoi.home.kpi.ng", "aoi.home.kpi.rw",
        "aoi.home.kpi.pass_rate", "aoi.home.kpi.tt", "aoi.home.kpi.run_mode",
        "aoi.home.kpi.device_state", "aoi.home.kpi.current_recipe",
    };
    for (int i = 0; i < 9; ++i) {
        auto *card = new ui::KpiMetricCard(parent);
        card->setProperty("kpiIndex", i);
        card->setTitle(TR(kpiKeys[i]));
        m_kpiCards.append(card);
        kpiRow->addWidget(card, 1);
    }
    return kpiRow;
}

ui::ThemedSectionPanel *HomeView::buildVirtualPanel(QWidget *parent)
{
    auto *virtualPanel = new ui::ThemedSectionPanel(QStringLiteral("aoi.home.panel_virtual"), parent);
    virtualPanel->setContentSpacing(6);
    m_virtualMapWidget = new PanelVirtualMapWidget(virtualPanel->contentHost());
    m_virtualMapWidget->setMinimumHeight(220);
    virtualPanel->contentLayout()->addWidget(m_virtualMapWidget, 1);
    return virtualPanel;
}

HomeProcessFlowWidget *HomeView::buildProcessFlow(QWidget *parent)
{
    m_processFlow = new HomeProcessFlowWidget(parent);
    return m_processFlow;
}

ui::ThemedSectionPanel *HomeView::buildLogPanel(QWidget *parent)
{
    // 左侧竖排标题释放原顶部标题区（约 30px + spacing），日志区加高保持面板总高度相近
    constexpr int kLogBodyH = 124;
    m_logPanel = new ui::ThemedSectionPanel(QStringLiteral("aoi.home.operation_log"),
                                            parent,
                                            28,
                                            ui::SectionTitlePlacement::LeftVertical);
    m_logView  = new ui::LogView(m_logPanel->contentHost());
    m_logView->setMaxEntries(100);
    m_logView->setFixedHeight(kLogBodyH);
    m_logView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_logPanel->contentLayout()->addWidget(m_logView, 0);
    const QMargins panelMargins = m_logPanel->layout()->contentsMargins();
    m_logPanel->setFixedHeight(kLogBodyH + panelMargins.top() + panelMargins.bottom());
    m_logPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    return m_logPanel;
}

ui::ThemedSectionPanel *HomeView::buildPreviewPanel(QWidget *parent)
{
    auto *previewPanel = new ui::ThemedSectionPanel(QStringLiteral("aoi.home.image_preview"), parent);

    auto *markRow = new QHBoxLayout;
    markRow->setSpacing(6);

    auto *mark1Wrap = makeTransparentHost(previewPanel->contentHost());
    auto *m1l       = new QVBoxLayout(mark1Wrap);
    m1l->setContentsMargins(0, 0, 0, 0);
    m1l->setSpacing(2);
    m_mark1Title   = new QLabel(TR("aoi.home.preview.mark1"), mark1Wrap);
    m_mark1Title->setObjectName(QStringLiteral("homePreviewCaption"));
    m_mark1Preview = new ui::ImagePreview(mark1Wrap);
    m_mark1Preview->setMinimumHeight(72);
    m_mark1Preview->setWheelZoomEnabled(false);
    m1l->addWidget(m_mark1Title);
    m1l->addWidget(m_mark1Preview, 1);

    auto *mark2Wrap = makeTransparentHost(previewPanel->contentHost());
    auto *m2l       = new QVBoxLayout(mark2Wrap);
    m2l->setContentsMargins(0, 0, 0, 0);
    m2l->setSpacing(2);
    m_mark2Title   = new QLabel(TR("aoi.home.preview.mark2"), mark2Wrap);
    m_mark2Title->setObjectName(QStringLiteral("homePreviewCaption"));
    m_mark2Preview = new ui::ImagePreview(mark2Wrap);
    m_mark2Preview->setMinimumHeight(72);
    m_mark2Preview->setWheelZoomEnabled(false);
    m2l->addWidget(m_mark2Title);
    m2l->addWidget(m_mark2Preview, 1);
    markRow->addWidget(mark1Wrap, 1);
    markRow->addWidget(mark2Wrap, 1);

    auto *fovWrap = makeTransparentHost(previewPanel->contentHost());
    auto *fovLayout = new QVBoxLayout(fovWrap);
    fovLayout->setContentsMargins(0, 0, 0, 0);
    fovLayout->setSpacing(2);
    m_fovTitle   = new QLabel(TR("aoi.home.preview.current_fov"), fovWrap);
    m_fovTitle->setObjectName(QStringLiteral("homePreviewCaption"));
    m_fovPreview = new ui::ImagePreview(fovWrap);
    m_fovPreview->setMinimumHeight(140);
    fovLayout->addWidget(m_fovTitle);
    fovLayout->addWidget(m_fovPreview, 1);

    previewPanel->contentLayout()->addLayout(markRow);
    previewPanel->contentLayout()->addWidget(fovWrap, 1);
    return previewPanel;
}

ui::ThemedSectionPanel *HomeView::buildSnapshotPanel(QWidget *parent)
{
    auto *snapshotPanel = new ui::ThemedSectionPanel(QStringLiteral("aoi.home.snapshot_title"), parent);

    auto *metaWrap = makeFixedHeightRow(snapshotPanel->contentHost(), kSnapshotMetaRowH, "homeSnapshotMetaRow");
    auto *metaRow = new QHBoxLayout(metaWrap);
    metaRow->setContentsMargins(0, 0, 0, 0);
    metaRow->setSpacing(0);

    m_snapshotSnLbl = new QLabel(metaWrap);
    m_snapshotSnLbl->setObjectName(QStringLiteral("homeSnapshotMeta"));
    pinMetaLabel(m_snapshotSnLbl, Qt::AlignLeft | Qt::AlignVCenter);

    m_snapshotStatusLbl = new QLabel(metaWrap);
    m_snapshotStatusLbl->setObjectName(QStringLiteral("homeSnapshotStatusNg"));
    pinMetaLabel(m_snapshotStatusLbl, Qt::AlignRight | Qt::AlignVCenter);

    metaRow->addWidget(m_snapshotSnLbl, 1, Qt::AlignVCenter);
    metaRow->addWidget(m_snapshotStatusLbl, 0, Qt::AlignVCenter);

    m_snapshotTable = ui::makeTableWidget(inspection::kInspectionDefectTypeCount, 6, snapshotPanel->contentHost());
    m_snapshotTable->setObjectName(QStringLiteral("homeSnapshotTable"));
    m_snapshotTable->applyCompactFormDefaults();
    m_snapshotTable->setHorizontalStretchFactors({2, 1, 1, 1, 1, 1});
    m_snapshotTable->setMinimumRowHeight(22);

    snapshotPanel->contentLayout()->addWidget(metaWrap, 0);
    snapshotPanel->contentLayout()->addWidget(m_snapshotTable, 1);
    metaWrap->setFixedHeight(kSnapshotMetaRowH);

    return snapshotPanel;
}

// ── 根布局 ──

void HomeView::buildLayout()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    buildSidebar();
    root->addWidget(m_sidebar);

    auto *main = new QWidget(this);
    auto *ml   = new QVBoxLayout(main);
    ml->setContentsMargins(0, 0, 0, 0);
    ml->setSpacing(6);
    ml->addLayout(buildKpiRow(main));

    auto *body = new QHBoxLayout;
    body->setSpacing(8);

    auto *centerCol = new QVBoxLayout;
    centerCol->setSpacing(8);
    centerCol->addWidget(buildVirtualPanel(main), 6);
    centerCol->addWidget(buildProcessFlow(main));
    centerCol->addWidget(buildLogPanel(main), 0);
    body->addLayout(centerCol, 62);

    auto *rightCol = new QVBoxLayout;
    rightCol->setSpacing(8);
    rightCol->addWidget(buildPreviewPanel(main), 5);
    rightCol->addWidget(buildSnapshotPanel(main), 5);
    body->addLayout(rightCol, 38);

    ml->addLayout(body, 1);
    root->addWidget(main, 1);
}

// ── ViewModel 绑定 ──

void HomeView::setupBindings()
{
    if (!m_viewModel || !m_deviceControlPanel)
        return;

    // 侧栏 6 钮顺序须与 DeviceControlPanel::controlButtons 一致
    const QVector<Command *> commands = {
        m_viewModel->startCommand(),
        m_viewModel->stopCommand(),
        m_viewModel->resetCommand(),
        m_viewModel->clearAlarmCommand(),
        m_viewModel->clearMaterialCommand(),
        m_viewModel->singleInspectCommand(),
    };

    const auto &buttons = m_deviceControlPanel->controlButtons();
    for (int i = 0; i < buttons.size() && i < commands.size(); ++i) {
        aoi::binding::bindCommandToButton(buttons[i],
                                          qobject_cast<FunctionalCommand *>(commands[i]));
    }

    binding::HomeRuntimeBindingTargets runtimeTargets;
    runtimeTargets.kpiCards          = m_kpiCards;
    runtimeTargets.hardwarePanel     = m_hardwarePanel;
    runtimeTargets.processFlow       = m_processFlow;
    runtimeTargets.virtualMapWidget  = m_virtualMapWidget;
    runtimeTargets.logView           = m_logView;
    runtimeTargets.snapshotSnLbl     = m_snapshotSnLbl;
    runtimeTargets.snapshotStatusLbl = m_snapshotStatusLbl;
    runtimeTargets.snapshotTable     = m_snapshotTable;
    runtimeTargets.mark1Preview      = m_mark1Preview;
    runtimeTargets.mark2Preview      = m_mark2Preview;
    runtimeTargets.fovPreview        = m_fovPreview;
    runtimeTargets.applyDecorStyles  = [this]() { applyHomeDecorStyles(); };
    binding::bindHomeRuntime(m_viewModel, runtimeTargets, this);

    if (auto *cam = ExtensionSystem::PluginManager::getInterface<ICameraService>()) {
        aoi::ui_binding::bindRoleImagePreview(
            m_mark1Preview, cam, QStringLiteral("Marker1"), QString(), this);
        aoi::ui_binding::bindRoleImagePreview(
            m_mark2Preview, cam, QStringLiteral("Marker2"), QString(), this);
        aoi::ui_binding::bindRoleImagePreview(
            m_fovPreview, cam, QStringLiteral("Fov"), QString(), this);
    }
}

void HomeView::notifyRuntimeRefresh()
{
    if (m_viewModel)
        m_viewModel->refreshRuntimeBindings();
}

// ── 国际化 ──

void HomeView::retranslateUi()
{
    const char *kpiKeys[] = {
        "aoi.home.kpi.total", "aoi.home.kpi.ok", "aoi.home.kpi.ng", "aoi.home.kpi.rw",
        "aoi.home.kpi.pass_rate", "aoi.home.kpi.tt", "aoi.home.kpi.run_mode",
        "aoi.home.kpi.device_state", "aoi.home.kpi.current_recipe",
    };
    for (int i = 0; i < m_kpiCards.size() && i < 9; ++i)
        m_kpiCards[i]->setTitle(TR(kpiKeys[i]));

    if (m_deviceControlPanel)
        m_deviceControlPanel->retranslateUi();
    if (m_hardwarePanel)
        m_hardwarePanel->retranslateUi();
    if (m_diskPanel)
        m_diskPanel->retranslateUi();
    if (m_processFlow)
        m_processFlow->retranslateUi();
    if (m_virtualMapWidget)
        m_virtualMapWidget->retranslateUi();
    for (auto *panel : findChildren<ui::ThemedSectionPanel *>())
        panel->retranslatePanel();
    if (m_mark1Title)
        m_mark1Title->setText(TR("aoi.home.preview.mark1"));
    if (m_mark2Title)
        m_mark2Title->setText(TR("aoi.home.preview.mark2"));
    if (m_fovTitle)
        m_fovTitle->setText(TR("aoi.home.preview.current_fov"));

    notifyRuntimeRefresh();
}

} // namespace aoi
