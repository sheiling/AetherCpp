/**
 * @file recipe_config_center_view.cpp
 * @brief 配方配置 · 中间调试区实现
 */

#include "recipe_config/recipe_config_center_view.h"

#include <common/utility/diagnostics/timing_debug.h>

#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_push_button.h"
#include "component/controls/combo/themed_combo_box.h"
#include "inspection_types.h"
#include "domain/axis_jog_panel.h"
#include "domain/panel_navigator_overlay.h"
#include "component/feedback/progress_overlay.h"
#include "recipe_config_view_model.h"
#include "recipe_scan_detect_view_model.h"
#include "recipe_virtual_inspect_view_model.h"
#include "recipe_virtual_inspect_view_model.h"
#include "flow_host/flow_host_coordinator.h"
#include "service/inspection/result/inspection_result_service.h"
#include "data/dataDefinition/inspection_types.h"
#include "component/controls/button/themed_icon_button.h"
#include "component/font/fa_icon_button.h"
#include "fov_planner/fov_coordinate_system.h"
#include "fov_planner/recipe_fov_layout_adapter.h"
#include "component/table/themed_table_widget.h"
#include "component/theme/theme_tokens.h"

#include "uibase/languagemanager.h"
#include <icamera_service.h>
#include <imotion_control_service.h>
#include <common/include/interface/igraphicsView.h>
#include "extensionsystem/pluginmanager.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QEvent>
#include <QResizeEvent>
#include <QCoreApplication>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSize>
#include <QSplitter>
#include <QTableWidgetItem>
#include <QTimer>
#include <QTableWidgetItem>
#include <QVBoxLayout>

using namespace ExtensionSystem;

namespace aoi {

using namespace ui;

namespace {

QLabel *makeSectionTitle(const QString &key, QWidget *parent, int fixedHeight = 0)
{
    auto *lbl = new QLabel(TR(key), parent);
    lbl->setObjectName(QStringLiteral("recipeSectionTitle"));
    if (fixedHeight > 0) {
        lbl->setFixedHeight(fixedHeight);
        lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return lbl;
}

ui::ThemedPushButton *makeToolbarActionButton(const QString &key, QWidget *parent)
{
    auto *btn = ui::makePushButton(TR(key), PushButtonVariant::Secondary, parent);
    btn->setObjectName(QStringLiteral("recipeToolbarBtn"));
    btn->setProperty("translationKey", key);
    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return btn;
}

ui::ThemedPushButton *makeRoiToolButton(const QString &key, QWidget *parent)
{
    auto *btn = ui::makePushButton(TR(key), PushButtonVariant::Secondary, parent);
    btn->setObjectName(QStringLiteral("recipeRoiToolBtn"));
    btn->setProperty("translationKey", key);
    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return btn;
}

/** 明细表：扁平表头，替代霓虹自绘 ThemedTableHeaderView */
void applyDefectDetailTablePlainStyle(ui::ThemedTableWidget *table)
{
    if (!table)
        return;

    const ui::ThemeTokens tokens = ui::ThemeTokens::resolve();
    table->setStyleSheet(
        QStringLiteral("QTableWidget#recipeDefectDetailTable { gridline-color: %1; }"
                       "QTableWidget#recipeDefectDetailTable QHeaderView::section {"
                       "  background-color: %2;"
                       "  color: %3;"
                       "  border: none;"
                       "  border-bottom: 1px solid %4;"
                       "  padding: 2px 4px;"
                       "  font-weight: normal;"
                       "}")
            .arg(tokens.inputBorder.name(QColor::HexArgb))
            .arg(tokens.surfaceElevated.name(QColor::HexArgb))
            .arg(tokens.textSecondary.name(QColor::HexArgb))
            .arg(tokens.inputBorder.name(QColor::HexArgb)));
}

} // namespace

// ── 构造 / 生命周期 ──

RecipeConfigCenterView::RecipeConfigCenterView(RecipeConfigViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    buildLayout();
}

QString RecipeConfigCenterView::currentCameraId() const
{
    if (m_toolbarCamCombo)
        return m_toolbarCamCombo->currentData().toString();
    return QStringLiteral("main");
}

// ── 布局 ──

void RecipeConfigCenterView::buildLayout()
{
    setObjectName(QStringLiteral("recipeCenterShell"));
    setAttribute(Qt::WA_StyledBackground, true);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    auto *toolbar = new QFrame(this);
    toolbar->setObjectName(QStringLiteral("recipePreviewToolbar"));
    toolbar->setAttribute(Qt::WA_StyledBackground, true);
    auto *tbRow = new QHBoxLayout(toolbar);
    tbRow->setContentsMargins(8, 4, 8, 4);
    tbRow->setSpacing(6);
    tbRow->setAlignment(Qt::AlignVCenter);

    m_toolbarCamCombo = ui::makeComboBox(toolbar);
    m_toolbarCamCombo->setObjectName(QStringLiteral("recipeCameraSelect"));
    if (auto *camSvc = PluginManager::getInterface<ICameraService>()) {
        const QStringList captureIds = camSvc->captureNames();
        for (const QString &id : captureIds) {
            const QString label = (id == QStringLiteral("main")) ? TR("aoi.recipe.camera.aoi") : id;
            m_toolbarCamCombo->addItem(label, id);
        }
    }
    if (m_toolbarCamCombo->count() == 0) {
        m_toolbarCamCombo->addItem(TR("aoi.recipe.camera.aoi"), QStringLiteral("main"));
    }
    m_toolbarCamCombo->setFixedWidth(144);
    tbRow->addWidget(m_toolbarCamCombo);

    const QSize toolbarIconSize(14, 14);
    m_toolbarSnapBtn = ui::makeFaIconButton(ui::FaIcon::Camera, toolbar,
                                                     QStringLiteral("aoi.recipe.toolbar.snap"));
    m_toolbarSnapBtn->setObjectName(QStringLiteral("recipeToolbarIconBtn"));
    m_toolbarSnapBtn->setIconPixelSize(toolbarIconSize.height());
    tbRow->addWidget(m_toolbarSnapBtn);

    m_toolbarPreviewBtn = new ThemedIconButton(toolbar);
    m_toolbarPreviewBtn->setObjectName(QStringLiteral("recipeToolbarIconBtn"));
    ui::applyFaToggleIcons(m_toolbarPreviewBtn, ui::FaIcon::Play,
                                   ui::FaIcon::Pause, toolbarIconSize.height());
    m_toolbarPreviewBtn->setCheckable(true);
    m_toolbarPreviewBtn->setToolTip(TR("aoi.recipe.toolbar.preview"));
    tbRow->addWidget(m_toolbarPreviewBtn);

    auto *divider = new QFrame(toolbar);
    divider->setObjectName(QStringLiteral("recipeToolbarDivider"));
    divider->setFrameShape(QFrame::VLine);
    tbRow->addWidget(divider);

    const QStringList toolbarKeys = {
        QStringLiteral("aoi.recipe.toolbar.feed_in"),
        QStringLiteral("aoi.recipe.toolbar.feed_out"),
        QStringLiteral("aoi.recipe.toolbar.inspect"),
        QStringLiteral("aoi.recipe.toolbar.scan_detect"),
        QStringLiteral("aoi.recipe.toolbar.read_image"),
    };
    m_toolbarKeys = toolbarKeys.toVector();
    m_toolbarButtons.clear();
    for (const QString &key : toolbarKeys) {
        auto *btn = makeToolbarActionButton(key, toolbar);
        m_toolbarButtons.append(btn);
        if (key == QStringLiteral("aoi.recipe.toolbar.read_image")) {
            m_readImageBtn = btn;
            connect(btn, &QPushButton::clicked, this, [this]() {
                const QString path = QFileDialog::getOpenFileName(
                    this,
                    TR("aoi.recipe.toolbar.read_image"),
                    QString(),
                    TR("aoi.recipe.image_filter"));
                if (!path.isEmpty())
                    loadPreviewImage(path);
            });
        } else if (key == QStringLiteral("aoi.recipe.toolbar.scan_detect")) {
            m_scanDetectBtn = btn;
            connect(btn, &QPushButton::clicked, this, [this]() {
                if (!m_viewModel)
                    return;
                auto *scanVm = m_viewModel->scanDetectVm();
                if (!scanVm)
                    return;
                QImage image;
                if (!currentPreviewImage(image)) {
                    scanVm->scanDetectFromImage({});
                } else {
                    scanVm->scanDetectFromImage(image);
                }
                if (m_scanDetectBtn)
                    m_scanDetectBtn->setFocus(Qt::OtherFocusReason);
            });
        } else if (key == QStringLiteral("aoi.recipe.toolbar.inspect")) {
            m_inspectBtn = btn;
        }
        tbRow->addWidget(btn);
    }

    auto *roiDivider = new QFrame(toolbar);
    roiDivider->setObjectName(QStringLiteral("recipeToolbarDivider"));
    roiDivider->setFrameShape(QFrame::VLine);
    tbRow->addWidget(roiDivider);

    m_roiShapeKeys = {
        QStringLiteral("aoi.recipe.roi.rect"),
        QStringLiteral("aoi.recipe.roi.rot_rect"),
        QStringLiteral("aoi.recipe.roi.circle"),
        QStringLiteral("aoi.recipe.roi.ring"),
        QStringLiteral("aoi.recipe.roi.polygon"),
    };
    m_roiShapeCombo = ui::makeComboBox(toolbar);
    m_roiShapeCombo->setObjectName(QStringLiteral("recipeRoiShapeSelect"));
    m_roiShapeCombo->setFixedWidth(110);
    for (const QString &key : m_roiShapeKeys)
        m_roiShapeCombo->addItem(TR(key));
    m_roiShapeCombo->setCurrentIndex(0);
    tbRow->addWidget(m_roiShapeCombo);
    connect(m_roiShapeCombo, QOverload<int>::of(&QComboBox::activated), this,
            &RecipeConfigCenterView::applyRoiShape);

    m_roiToolKeys = {
        QStringLiteral("aoi.recipe.roi.crosshair"),
        QStringLiteral("aoi.recipe.roi.clear"),
    };
    m_roiToolButtons.clear();
    for (const QString &key : m_roiToolKeys) {
        auto *btn = makeRoiToolButton(key, toolbar);
        m_roiToolButtons.append(btn);
        tbRow->addWidget(btn);

        connect(btn, &QPushButton::clicked, this, [this, key]() {
            if (!m_graphicsView)
                return;
            if (key == QStringLiteral("aoi.recipe.roi.crosshair"))
                m_graphicsView->DrawCross(kPreviewWinId);
            else if (key == QStringLiteral("aoi.recipe.roi.clear"))
                m_graphicsView->DrawClean(kPreviewWinId);
        });
    }

    m_panelNavBtn = makeRoiToolButton(QStringLiteral("aoi.recipe.nav.toggle"), toolbar);
    m_panelNavBtn->setCheckable(true);
    tbRow->addWidget(m_panelNavBtn);
    connect(m_panelNavBtn, &ui::ThemedPushButton::toggled, this, [this](bool checked) {
        if (m_panelNavigator) {
            m_panelNavigator->setActive(checked);
            if (checked && m_viewModel)
                m_panelNavigator->rebuildFromDocument(m_viewModel->document());
        }
        syncPanelNavigatorFov();
    });

    tbRow->addStretch();

    m_saveRecipeBtn = ui::makePushButton(TR("aoi.recipe.action.save"), PushButtonVariant::Primary, toolbar);
    m_saveRecipeBtn->setObjectName(QStringLiteral("recipeToolbarBtn"));
    m_saveRecipeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tbRow->addWidget(m_saveRecipeBtn);
    layout->addWidget(toolbar);

    auto *centerTop = new QWidget(this);
    auto *ctLayout  = new QHBoxLayout(centerTop);
    ctLayout->setContentsMargins(0, 0, 0, 0);
    ctLayout->setSpacing(8);

    auto *previewWrap = new QWidget(centerTop);
    auto *pvLayout    = new QVBoxLayout(previewWrap);
    pvLayout->setContentsMargins(0, 0, 0, 0);
    pvLayout->setSpacing(4);

    m_graphicsView = PluginManager::getInterface<IGraphicsVew>();
    if (m_graphicsView) {
        auto *previewWidget = m_graphicsView->CreateWidget(kPreviewWinId);
        previewWidget->setMinimumHeight(300);
        m_previewHostWidget = previewWidget;
        pvLayout->addWidget(previewWidget, 1);
        m_graphicsView->SetVisible(kPreviewWinId, true, false);
    } else {
        m_previewLabel = new QLabel(previewWrap);
        m_previewHostWidget = m_previewLabel;
        m_previewLabel->setObjectName(QStringLiteral("previewPane"));
        m_previewLabel->setAlignment(Qt::AlignCenter);
        m_previewLabel->setMinimumHeight(300);
        pvLayout->addWidget(m_previewLabel, 1);

        m_roiInfoLabel = new QLabel(previewWrap);
        m_roiInfoLabel->setObjectName(QStringLiteral("recipeRoiInfo"));
        m_roiInfoLabel->setText(QStringLiteral("x:0 y:0 w:0 h:0"));
        m_roiInfoLabel->setAlignment(Qt::AlignRight);
        pvLayout->addWidget(m_roiInfoLabel);
    }

    if (m_previewHostWidget) {
        m_inspectOverlay = new ui::ProgressOverlay(m_previewHostWidget);
        m_inspectOverlay->setCancelText(TR("aoi.recipe.inspect.cancel"));
        m_inspectOverlay->setAnchorWidget(m_previewHostWidget);
        m_inspectOverlay->setGeometry(m_previewHostWidget->rect());
        m_inspectOverlay->raise();

        m_previewHostWidget->installEventFilter(this);
    }

    if (m_viewModel && m_inspectOverlay) {
        if (auto *inspectVm = m_viewModel->virtualInspectVm()) {
            connect(inspectVm, &RecipeVirtualInspectViewModel::virtualInspectProgress, this,
                    &RecipeConfigCenterView::updateVirtualInspectProgress);
            connect(inspectVm, &RecipeVirtualInspectViewModel::virtualInspectingChanged, this,
                    [this, inspectVm]() {
                        setVirtualInspectOverlayVisible(inspectVm->virtualInspecting());
                        refreshInspectButtonEnabled();
                    });
            connect(m_inspectOverlay, &ui::ProgressOverlay::cancelClicked, inspectVm,
                    &RecipeVirtualInspectViewModel::cancelVirtualInspect);
        }
    }

    ctLayout->addWidget(previewWrap, 3);

    auto *resultBlock = new QFrame(centerTop);
    m_resultBlock     = resultBlock;
    resultBlock->setObjectName(QStringLiteral("recipeResultGroup"));
    resultBlock->setAttribute(Qt::WA_StyledBackground, true);
    resultBlock->setMinimumWidth(200);
    resultBlock->setMaximumWidth(260);
    auto *rbLayout = new QVBoxLayout(resultBlock);
    rbLayout->setContentsMargins(8, 8, 8, 8);
    rbLayout->setSpacing(6);
    m_resultSectionTitle =
        makeSectionTitle(QStringLiteral("aoi.recipe.result_output"), resultBlock, 40);
    rbLayout->addWidget(m_resultSectionTitle);

    m_defectTable = makeTableWidget(inspection::kInspectionDefectTypeCount, 2, resultBlock);
    m_defectTable->setObjectName(QStringLiteral("recipeDefectTable"));
    m_defectTable->applyCompactFormDefaults();
    rbLayout->addWidget(m_defectTable, 1);

    refreshDefectTable();

    m_detailBtn = ui::makePushButton(TR("aoi.recipe.result_detail"), PushButtonVariant::Primary, resultBlock);
    m_detailBtn->setObjectName(QStringLiteral("recipeResultDetailBtn"));
    m_detailBtn->setProperty("translationKey", QStringLiteral("aoi.recipe.result_detail"));
    rbLayout->addWidget(m_detailBtn);
    ctLayout->addWidget(resultBlock, 1);

    auto *mainSplit = new QSplitter(Qt::Vertical, this);
    mainSplit->setObjectName(QStringLiteral("recipeCenterSplit"));
    mainSplit->setChildrenCollapsible(false);
    mainSplit->addWidget(centerTop);

    auto *debugBlock = new QFrame(mainSplit);
    debugBlock->setObjectName(QStringLiteral("recipeDebugBlock"));
    debugBlock->setAttribute(Qt::WA_StyledBackground, true);
    auto *dbLayout = new QVBoxLayout(debugBlock);
    dbLayout->setContentsMargins(8, 4, 8, 10);
    dbLayout->setSpacing(4);

    m_axisJogPanel = new AxisJogPanel({}, debugBlock);
    dbLayout->addWidget(m_axisJogPanel, 0, Qt::AlignTop);

    debugBlock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    debugBlock->setMinimumHeight(96);
    mainSplit->addWidget(debugBlock);
    mainSplit->setStretchFactor(0, 1);
    mainSplit->setStretchFactor(1, 0);
    mainSplit->setSizes({520, 120});

    m_debugBlock = debugBlock;
    layout->addWidget(mainSplit, 1);

    wireAxisJogPanel();
    wirePanelNavigator();
    if (m_viewModel)
        syncGraphicsPixelScale(m_viewModel->document());
}

// ── 轴点动 / 采集器轴列表 ──

void RecipeConfigCenterView::wireAxisJogPanel()
{
    if (m_toolbarCamCombo) {
        connect(m_toolbarCamCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &RecipeConfigCenterView::refreshAxisJogFromCapture);
    }

    if (!m_jogFovRefreshTimer) {
        m_jogFovRefreshTimer = new QTimer(this);
        m_jogFovRefreshTimer->setInterval(50);
        connect(m_jogFovRefreshTimer, &QTimer::timeout, this, [this]() {
            if (auto *motionSvc =
                    PluginManager::getInterface<motion_control_plugin::IMotionControlService>())
                motionSvc->refreshAxisStates();
        });
    }

    if (m_axisJogPanel) {
        connect(m_axisJogPanel, &AxisJogPanel::jogPressed, this,
                &RecipeConfigCenterView::onAxisJogPressed);
        connect(m_axisJogPanel, &AxisJogPanel::jogReleased, this,
                &RecipeConfigCenterView::onAxisJogReleased);
        connect(m_axisJogPanel, &AxisJogPanel::absoluteMoveRequested, this,
                &RecipeConfigCenterView::onAxisAbsoluteMoveRequested);
        connect(m_axisJogPanel, &AxisJogPanel::relativeMoveRequested, this,
                &RecipeConfigCenterView::onAxisRelativeMoveRequested);
    }

    if (auto *motionSvc = PluginManager::getInterface<motion_control_plugin::IMotionControlService>()) {
        connect(motionSvc, &motion_control_plugin::IMotionControlService::axisStatesUpdated, this,
                &RecipeConfigCenterView::syncAxisJogPositionsFromService);
    }

    refreshAxisJogFromCapture();
}

void RecipeConfigCenterView::refreshAxisJogFromCapture()
{
    if (!m_axisJogPanel)
        return;

    QStringList axes;
    if (auto *camSvc = PluginManager::getInterface<ICameraService>())
        axes = camSvc->captureAxisNames(currentCameraId());

    m_axisJogPanel->setAxisNames(axes);
    m_axisJogPanel->resetForm();
    m_axisJogPanel->refreshCompactLayout();
    if (m_debugBlock) {
        m_debugBlock->adjustSize();
        m_debugBlock->setMinimumHeight(qMax(96, m_debugBlock->sizeHint().height() + 4));
    }
    updateAxisJogAutoRefresh();
    syncAxisJogPositionsFromService();
}

void RecipeConfigCenterView::updateAxisJogAutoRefresh()
{
    auto *motionSvc = PluginManager::getInterface<motion_control_plugin::IMotionControlService>();
    if (!motionSvc)
        return;

    const bool needRefresh = m_axisJogPanel && !m_axisJogPanel->axisNames().isEmpty();
    motionSvc->setAutoRefreshEnabled(needRefresh);
    if (needRefresh)
        motionSvc->setRefreshIntervalMs(200);
}

void RecipeConfigCenterView::syncAxisJogPositionsFromService()
{
    if (!m_axisJogPanel)
        return;

    auto *motionSvc = PluginManager::getInterface<motion_control_plugin::IMotionControlService>();
    if (!motionSvc)
        return;

    for (const QString &axisName : m_axisJogPanel->axisNames()) {
        const auto state = motionSvc->axisState(axisName);
        m_axisJogPanel->setAxisPositions(axisName, state.cmdPos, state.fbPos);
    }

    syncPanelNavigatorFov();
}

void RecipeConfigCenterView::onAxisJogPressed(const QString &axisName, int direction,
                                              double speedRatio)
{
    auto *motionSvc = PluginManager::getInterface<motion_control_plugin::IMotionControlService>();
    if (!motionSvc)
        return;
    motionSvc->startJog(axisName, direction, speedRatio);
    if (++m_activeJogCount == 1 && m_jogFovRefreshTimer)
        m_jogFovRefreshTimer->start();
}

void RecipeConfigCenterView::onAxisJogReleased(const QString &axisName)
{
    auto *motionSvc = PluginManager::getInterface<motion_control_plugin::IMotionControlService>();
    if (!motionSvc)
        return;
    motionSvc->stopJog(axisName);
    if (m_activeJogCount > 0)
        --m_activeJogCount;
    if (m_activeJogCount == 0 && m_jogFovRefreshTimer)
        m_jogFovRefreshTimer->stop();
    motionSvc->refreshAxisStates();
}

void RecipeConfigCenterView::onAxisAbsoluteMoveRequested(const QString &axisName, double targetPos)
{
    moveAxisWithMarkClamp(axisName, true, targetPos);
}

void RecipeConfigCenterView::onAxisRelativeMoveRequested(const QString &axisName, double delta)
{
    moveAxisWithMarkClamp(axisName, false, delta);
}

fov_planner::FovCoordinateSystem::MarkPhysicalAabb RecipeConfigCenterView::captureMarkPhysicalAabb() const
{
    if (!m_viewModel)
        return {};

    const fov_planner::FovLayoutConfig cfg =
        fov_planner::buildFovLayoutConfig(m_viewModel->document());
    fov_planner::FovCoordinateSystem coords;
    if (!coords.build(cfg))
        return {};
    return coords.markPhysicalAabb();
}

void RecipeConfigCenterView::clampCapturePhysicalXY(double &xMm, double &yMm) const
{
    const auto aabb = captureMarkPhysicalAabb();
    if (!aabb.valid)
        return;
    xMm = qBound(aabb.minX, xMm, aabb.maxX);
    yMm = qBound(aabb.minY, yMm, aabb.maxY);
}

bool RecipeConfigCenterView::moveAxisWithMarkClamp(const QString &axisName, bool absolute,
                                                   double value)
{
    auto *motionSvc = PluginManager::getInterface<motion_control_plugin::IMotionControlService>();
    if (!motionSvc)
        return false;

    if (!absolute && qAbs(value) < 1e-9)
        return false;

    QStringList axes;
    if (auto *camSvc = PluginManager::getInterface<ICameraService>())
        axes = camSvc->captureAxisNames(currentCameraId());
    else if (m_axisJogPanel)
        axes = m_axisJogPanel->axisNames();

    const int axisIndex = axes.indexOf(axisName);
    if (axisIndex < 0) {
        const bool ok = motionSvc->moveToPosition(axisName, value, absolute);
        if (ok)
            motionSvc->refreshAxisStates();
        return ok;
    }

    if (axisIndex >= 2) {
        const bool ok = motionSvc->moveToPosition(axisName, value, absolute);
        if (ok)
            motionSvc->refreshAxisStates();
        return ok;
    }

    const auto aabb = captureMarkPhysicalAabb();
    if (!aabb.valid) {
        const bool ok = motionSvc->moveToPosition(axisName, value, absolute);
        if (ok)
            motionSvc->refreshAxisStates();
        return ok;
    }

    double xMm = axes.size() > 0 ? motionSvc->axisState(axes.at(0)).fbPos : 0.0;
    double yMm = axes.size() > 1 ? motionSvc->axisState(axes.at(1)).fbPos : 0.0;

    auto syncClampedFov = [&]() {
        clampCapturePhysicalXY(xMm, yMm);
        if (m_panelNavigator && m_panelNavigator->isActive())
            m_panelNavigator->syncFovFromMotion(axes, xMm, yMm);
    };

    if (axisIndex == 0) {
        double target = absolute ? value : xMm + value;
        target          = qBound(aabb.minX, target, aabb.maxX);
        if (qAbs(target - xMm) < 1e-9) {
            xMm = target;
            syncClampedFov();
            return false;
        }
        const bool ok = motionSvc->moveToPosition(axes.at(0), target, true);
        if (ok)
            motionSvc->refreshAxisStates();
        return ok;
    }

    double target = absolute ? value : yMm + value;
    target          = qBound(aabb.minY, target, aabb.maxY);
    if (qAbs(target - yMm) < 1e-9) {
        yMm = target;
        syncClampedFov();
        return false;
    }
    const bool ok = motionSvc->moveToPosition(axes.at(1), target, true);
    if (ok)
        motionSvc->refreshAxisStates();
    return ok;
}

void RecipeConfigCenterView::wirePanelNavigator()
{
    if (!m_previewHostWidget || !m_viewModel)
        return;

    m_panelNavigator = new PanelNavigatorOverlay(
        m_previewHostWidget, m_viewModel, [this]() { return currentCameraId(); },
        m_previewHostWidget);

    if (m_toolbarCamCombo) {
        connect(m_toolbarCamCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this]() {
                    if (m_panelNavigator && m_panelNavigator->isActive() && m_viewModel)
                        m_panelNavigator->rebuildFromDocument(m_viewModel->document());
                    syncPanelNavigatorFov();
                });
    }
}

void RecipeConfigCenterView::syncPanelNavigatorFov()
{
    if (!m_panelNavigator || !m_panelNavigator->isActive())
        return;

    QStringList axes;
    if (auto *camSvc = PluginManager::getInterface<ICameraService>())
        axes = camSvc->captureAxisNames(currentCameraId());
    else if (m_axisJogPanel)
        axes = m_axisJogPanel->axisNames();

    double xMm = 0.0;
    double yMm = 0.0;

    const bool preferCmdPos = m_activeJogCount > 0;

    if (auto *motionSvc =
            PluginManager::getInterface<motion_control_plugin::IMotionControlService>()) {
        if (axes.size() > 0) {
            const auto state = motionSvc->axisState(axes.at(0));
            xMm              = preferCmdPos ? state.cmdPos : state.fbPos;
        }
        if (axes.size() > 1) {
            const auto state = motionSvc->axisState(axes.at(1));
            yMm              = preferCmdPos ? state.cmdPos : state.fbPos;
        }
    } else if (m_axisJogPanel) {
        if (axes.size() > 0)
            xMm = m_axisJogPanel->feedbackPosition(axes.at(0));
        if (axes.size() > 1)
            yMm = m_axisJogPanel->feedbackPosition(axes.at(1));
    }

    clampCapturePhysicalXY(xMm, yMm);
    m_panelNavigator->syncFovFromMotion(axes, xMm, yMm);
}

void RecipeConfigCenterView::refreshPanelNavigator(const RecipeDocument &doc)
{
    if (m_panelNavigator && m_panelNavigator->isActive())
        m_panelNavigator->rebuildFromDocument(doc);
    syncPanelNavigatorFov();
}

// ── 预览 / 缺陷表刷新 ──

void RecipeConfigCenterView::applyPreviewImage(QImage image)
{
    if (image.isNull())
        return;

    common::utility::TimingDebugScope viewTiming(
        common::utility::TimingDebugKeys::CaptureTiming, "UI.图像显示刷新",
        std::to_string(image.width()) + "x" + std::to_string(image.height()));

    if (m_graphicsView) {
        m_graphicsView->SetImage(kPreviewWinId, std::move(image));
    } else if (m_previewLabel) {
        m_previewLabel->setPixmap(QPixmap::fromImage(image));
        m_previewLabel->setScaledContents(true);
    }
}

bool RecipeConfigCenterView::currentPreviewImage(QImage &out) const
{
    if (m_graphicsView)
        return m_graphicsView->GetImage(kPreviewWinId, out) && !out.isNull();

    if (m_previewLabel) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Qt6 QLabel::pixmap() returns QPixmap by value, not const QPixmap*.
        const QPixmap pixmap = m_previewLabel->pixmap();
        if (!pixmap.isNull()) {
            out = pixmap.toImage();
            return true;
        }
#else
        const QPixmap *pixmap = m_previewLabel->pixmap();
        if (pixmap && !pixmap->isNull()) {
            out = pixmap->toImage();
            return true;
        }
#endif
    }
    return false;
}

void RecipeConfigCenterView::tryLoadDefaultPreviewImage()
{
    const QString defaultImagePath =
        QCoreApplication::applicationDirPath() + QStringLiteral("/00.jpg");
    loadPreviewImage(defaultImagePath);
}

void RecipeConfigCenterView::loadPreviewImage(const QString &path)
{
    if (path.isEmpty())
        return;

    const QImage image(path);
    if (image.isNull())
        return;

    applyPreviewImage(image);
    if (m_viewModel)
        m_viewModel->setPreviewImage(image, path);
}

void RecipeConfigCenterView::refreshDefectTable()
{
    if (!m_defectTable)
        return;

    m_defectTable->setHorizontalHeaderLabels(
        {TR("aoi.recipe.defect_col_item"), TR("aoi.recipe.defect_col_count")});

    const aoi::CurrentInspectionSnapshot snapshot =
        m_viewModel && m_viewModel->virtualInspectVm()
            ? m_viewModel->virtualInspectVm()->virtualInspectSnapshot()
            : aoi::CurrentInspectionSnapshot{};

    const bool hasSnapshot = !snapshot.serialNumber.isEmpty();

    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const auto kind  = static_cast<inspection::InspectionDefectType>(i);
        const int  count = (hasSnapshot && i < snapshot.rows.size()) ? snapshot.rows[i].total : 0;
        QTableWidgetItem *nameItem = m_defectTable->item(i, 0);
        if (!nameItem) {
            nameItem = new QTableWidgetItem(TR(inspection::inspectionDefectTypeDisplayKey(kind)));
            m_defectTable->setItem(i, 0, nameItem);
        } else {
            nameItem->setText(TR(inspection::inspectionDefectTypeDisplayKey(kind)));
        }
        nameItem->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *countItem = m_defectTable->item(i, 1);
        if (!countItem) {
            countItem = new QTableWidgetItem(QString::number(count));
            m_defectTable->setItem(i, 1, countItem);
        } else {
            countItem->setText(QString::number(count));
        }
        countItem->setTextAlignment(Qt::AlignCenter);
    }
}

void RecipeConfigCenterView::refreshInspectButtonEnabled()
{
    if (!m_inspectBtn || !m_viewModel)
        return;

    RecipeVirtualInspectViewModel *inspectVm =
        m_viewModel ? m_viewModel->virtualInspectVm() : nullptr;
    const bool inspecting = inspectVm && inspectVm->virtualInspecting();
    bool       blocked    = false;
    if (inspectVm)
        blocked = !inspectVm->canVirtualInspect(nullptr);

    m_inspectBtn->setEnabled(!inspecting && !blocked);
    if (blocked)
        m_inspectBtn->setToolTip(TR("aoi.recipe.inspect.blocked_while_running"));
    else
        m_inspectBtn->setToolTip(QString());
}

void RecipeConfigCenterView::applyRoiShape(int index)
{
    if (!m_graphicsView)
        return;

    switch (index) {
    case 0:
        m_graphicsView->DrawRect(kPreviewWinId);
        break;
    case 1:
        m_graphicsView->DrawRotRect(kPreviewWinId);
        break;
    case 2:
        m_graphicsView->DrawCircle(kPreviewWinId);
        break;
    case 3:
        m_graphicsView->DrawRing(kPreviewWinId);
        break;
    case 4:
        m_graphicsView->DrawPolygon(kPreviewWinId);
        break;
    default:
        break;
    }
}

void RecipeConfigCenterView::syncPreviewToolbarHeights()
{
    if (!m_toolbarCamCombo || !m_toolbarSnapBtn || !m_toolbarPreviewBtn || m_toolbarButtons.isEmpty())
        return;

    auto *refBtn = m_toolbarButtons.first();
    refBtn->ensurePolished();
    m_toolbarCamCombo->ensurePolished();
    if (m_roiShapeCombo)
        m_roiShapeCombo->ensurePolished();

    const int h = refBtn->sizeHint().height();
    if (h <= 0)
        return;

    m_toolbarCamCombo->setFixedHeight(h);
    if (m_roiShapeCombo)
        m_roiShapeCombo->setFixedHeight(h);
    m_toolbarSnapBtn->setFixedSize(h, h);
    m_toolbarPreviewBtn->setFixedSize(h, h);
}

void RecipeConfigCenterView::refreshAuxiliaryUi(const RecipeDocument &doc)
{
    if (m_roiInfoLabel)
        m_roiInfoLabel->setText(QStringLiteral("x:0 y:0 w:0 h:0"));
    if (m_toolbarCamCombo)
        m_toolbarCamCombo->setCurrentIndex(0);
    syncGraphicsPixelScale(doc);
    refreshAxisJogFromCapture();
    refreshPanelNavigator(doc);
}

void RecipeConfigCenterView::syncGraphicsPixelScale(const RecipeDocument &doc)
{
    if (!m_graphicsView)
        return;

    const double umPerPx =
        doc.camera.captureAccuracy > 0.0 ? doc.camera.captureAccuracy : 0.0;
    m_graphicsView->SetPixelScaleUmPerPx(kPreviewWinId, umPerPx);
}

void RecipeConfigCenterView::refreshToolbarIcons()
{
    if (m_toolbarSnapBtn)
        m_toolbarSnapBtn->update();
    if (m_toolbarPreviewBtn)
        m_toolbarPreviewBtn->update();
}

void RecipeConfigCenterView::refreshPresentation()
{
    refreshToolbarIcons();
    if (m_axisJogPanel)
        m_axisJogPanel->refreshCompactLayout();
}

void RecipeConfigCenterView::retranslatePanel()
{
    if (m_previewLabel)
        m_previewLabel->setText(TR("aoi.recipe.preview_placeholder").arg(TR("aoi.recipe.camera.aoi")));

    for (int i = 0; i < m_toolbarButtons.size() && i < m_toolbarKeys.size(); ++i)
        m_toolbarButtons[i]->setText(TR(m_toolbarKeys[i]));

    for (int i = 0; i < m_roiToolButtons.size() && i < m_roiToolKeys.size(); ++i)
        m_roiToolButtons[i]->setText(TR(m_roiToolKeys[i]));

    if (m_panelNavBtn)
        m_panelNavBtn->setText(TR("aoi.recipe.nav.toggle"));

    if (m_roiShapeCombo) {
        const QSignalBlocker blocker(m_roiShapeCombo);
        const int idx = m_roiShapeCombo->currentIndex();
        for (int i = 0; i < m_roiShapeKeys.size() && i < m_roiShapeCombo->count(); ++i)
            m_roiShapeCombo->setItemText(i, TR(m_roiShapeKeys[i]));
        if (idx >= 0 && idx < m_roiShapeCombo->count())
            m_roiShapeCombo->setCurrentIndex(idx);
    }

    if (m_saveRecipeBtn)
        m_saveRecipeBtn->setText(TR("aoi.recipe.action.save"));

    if (m_readImageBtn)
        m_readImageBtn->setText(TR("aoi.recipe.toolbar.read_image"));

    if (m_detailBtn) {
        m_detailBtn->setProperty("translationKey", QStringLiteral("aoi.recipe.result_detail"));
        m_detailBtn->setText(TR("aoi.recipe.result_detail"));
    }

    refreshDefectTable();

    if (m_toolbarPreviewBtn) {
        m_toolbarPreviewBtn->setToolTip(
            TR(m_toolbarPreviewBtn->isChecked() ? QStringLiteral("aoi.recipe.toolbar.pause")
                                                : QStringLiteral("aoi.recipe.toolbar.preview")));
    }

    if (m_axisJogPanel)
        m_axisJogPanel->retranslatePanel();

    if (m_inspectOverlay)
        m_inspectOverlay->setCancelText(TR("aoi.recipe.inspect.cancel"));
}

void RecipeConfigCenterView::setVirtualInspectOverlayVisible(bool visible)
{
    if (!m_inspectOverlay)
        return;
    if (visible && m_previewHostWidget)
        m_inspectOverlay->setGeometry(m_previewHostWidget->rect());
    m_inspectOverlay->setVisible(visible);
    if (visible)
        m_inspectOverlay->raise();
}

void RecipeConfigCenterView::updateVirtualInspectProgress(int percent, const QString &stageKey)
{
    if (percent < 0) {
        setVirtualInspectOverlayVisible(false);
        return;
    }
    if (!m_inspectOverlay)
        return;
    setVirtualInspectOverlayVisible(true);
    m_inspectOverlay->setProgress(percent, TR(stageKey));
}

bool RecipeConfigCenterView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_previewHostWidget && event->type() == QEvent::Resize) {
        const QRect r = m_previewHostWidget->rect();
        if (m_inspectOverlay) {
            m_inspectOverlay->setGeometry(r);
            m_inspectOverlay->raise();
        }
    }
    return QWidget::eventFilter(watched, event);
}

} // namespace aoi
