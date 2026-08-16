/**
 * @file recipe_config_view.cpp
 * @brief AOI 配方配置页薄壳实现
 *
 * 三列布局编排 + RecipeConfigViewModel 信号绑定；
 * 列表 CRUD、保存、实时预览、扫描识别状态 Toast 均在此汇聚。
 */

#include "recipe_config/recipe_config_view.h"

#include "component/behavior/no_wheel_input_guard.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "component/dialog/themed_message_box.h"

#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"
#include "flow_host/flow_host_coordinator.h"
#include "service/flow/scan_flow_progress_bridge.h"
#include "list/recipe_list_panel.h"
#include "component/feedback/cursor_toast.h"
#include "recipe_config/recipe_config_center_view.h"
#include "component/controls/button/themed_icon_button.h"
#include "recipe_config/recipe_config_mark_view.h"
#include "recipe_config/recipe_config_param_panel.h"
#include "recipe_config/recipe_config_position_calib_view.h"
#include "recipe_config_model.h"
#include "data/data.h"
#include "recipe_config_view_model.h"
#include "recipe_scan_detect_view_model.h"
#include "recipe_virtual_inspect_view_model.h"

#include "databinding.h"

#include <common/include/interface/igraphicsView.h>

#include "extensionsystem/pluginmanager.h"
#include "uibase/languagemanager.h"

#include <icamera_service.h>

#include <QHBoxLayout>
#include <QImage>
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

// ── 构造 / 生命周期 ──

RecipeConfigView::RecipeConfigView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId(QStringLiteral("aoi.recipe_config"));
    setPageTitle(TR("aoi.nav.recipe"));
}

void RecipeConfigView::onHide()
{
    if (auto *camSvc = ExtensionSystem::PluginManager::getInterface<ICameraService>()) {
        camSvc->onDisplayDetached();
    }
}

void RecipeConfigView::onLoad()
{
    RecipeRepository::setStorageDirectory(aoi::data::recipeStorageDir());

    m_viewModel = new RecipeConfigViewModel(QStringLiteral("aoi.nav.recipe"), this);
    buildLayout();

    if (m_recipeListPanel)
        m_recipeListPanel->setRecipeDirectory(RecipeRepository::storageDirectory());

    m_statusToast = new ui::CursorToast(this);
    m_statusToast->hide();

    applyTheme();
    ui::NoWheelInputGuard::installOn(this);

    setupBindings();
    retranslateUi();

    if (m_centerView) {
        m_centerView->syncPreviewToolbarHeights();
        QTimer::singleShot(0, this, [this]() {
            if (m_centerView)
                m_centerView->syncPreviewToolbarHeights();
        });
    }

    if (m_viewModel) {
        if (m_centerView)
            m_centerView->refreshAuxiliaryUi(m_viewModel->document());
        if (m_paramPanel)
            m_paramPanel->refreshFromDocument(m_viewModel->document());
    }
    refreshRecipeList();
    if (m_centerView)
        m_centerView->refreshDefectTable();
}

// ── 主题 ──

void RecipeConfigView::applyTheme()
{
    if (m_statusToast)
        m_statusToast->applyToastStyle();
    ensureThemedSpinStyles(this);
    if (m_recipeListPanel)
        m_recipeListPanel->applySelectionTheme();
    if (m_paramPanel)
        m_paramPanel->refreshPresentation();
    if (m_centerView)
        m_centerView->refreshPresentation();
}

void RecipeConfigView::retranslateUi()
{
    setPageTitle(TR("aoi.nav.recipe"));

    if (m_recipeListPanel)
        m_recipeListPanel->retranslatePanel();
    if (m_centerView)
        m_centerView->retranslatePanel();
    if (m_paramPanel)
        m_paramPanel->retranslatePanel();

    if (m_statusToast)
        m_statusToast->applyToastStyle();

    refreshRecipeList();
    if (m_paramPanel)
        m_paramPanel->refreshPresentation();
}

// ── 布局 ──

void RecipeConfigView::buildLayout()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(6, 4, 6, 6);
    root->setSpacing(6);

    auto *body = new QWidget(this);
    auto *bl   = new QHBoxLayout(body);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(8);

    m_recipeListPanel = new RecipeListPanel(body);
    m_recipeListPanel->setRecipeDirectory(RecipeRepository::storageDirectory());

    m_centerView = new RecipeConfigCenterView(m_viewModel, body);
    m_paramPanel = new RecipeConfigParamPanel(m_viewModel, body);

    if (m_paramPanel->positionCalibView() && m_centerView->axisJogPanel())
        m_paramPanel->positionCalibView()->setAxisJogPanel(m_centerView->axisJogPanel());

    if (auto *posView = m_paramPanel->positionCalibView()) {
        posView->setCameraIdProvider([this]() {
            return m_centerView ? m_centerView->currentCameraId() : QStringLiteral("main");
        });
    }

    if (auto *markView = m_paramPanel->markView()) {
        markView->setPreviewContext(m_centerView->graphicsView(),
                                    m_centerView->previewWinId());
        markView->setCameraIdProvider([this]() {
            return m_centerView ? m_centerView->currentCameraId() : QStringLiteral("main");
        });
    }

    bl->addWidget(m_recipeListPanel, 0);
    bl->addWidget(m_centerView, 1);
    bl->addWidget(m_paramPanel, 0);

    root->addWidget(body, 1);
}

// ── ViewModel 绑定 ──

void RecipeConfigView::setupBindings()
{
    if (!m_recipeListPanel || !m_viewModel || !m_paramPanel || !m_centerView)
        return;

    m_paramPanel->setupBindings();
    if (auto *markView = m_paramPanel->markView())
        markView->setupExternalConnections(m_viewModel);

    connect(m_recipeListPanel, &RecipeListPanel::searchKeywordChanged, m_viewModel,
            &RecipeConfigViewModel::applySearch);
    connect(m_recipeListPanel, &RecipeListPanel::recipeSelected, this,
            [this](const QString &id) {
                if (!m_viewModel || id.isEmpty() || id == m_viewModel->currentRecipeId())
                    return;
                m_paramPanel->updateAllSources();
                m_viewModel->selectRecipe(id);
            });
    connect(m_recipeListPanel, &RecipeListPanel::addRecipeRequested, m_viewModel,
            &RecipeConfigViewModel::addRecipe);
    connect(m_recipeListPanel, &RecipeListPanel::removeRecipeRequested, this, [this]() {
        m_paramPanel->updateAllSources();
        if (m_viewModel && m_viewModel->removeCommand())
            m_viewModel->removeCommand()->execute();
    });
    connect(m_recipeListPanel, &RecipeListPanel::copyRecipeRequested, this, [this]() {
        m_paramPanel->updateAllSources();
        if (m_viewModel && m_viewModel->copyCommand())
            m_viewModel->copyCommand()->execute();
    });
    connect(m_recipeListPanel, &RecipeListPanel::setDefaultRecipeRequested, this, [this]() {
        m_paramPanel->updateAllSources();
        if (m_viewModel && m_viewModel->setDefaultCommand())
            m_viewModel->setDefaultCommand()->execute();
    });
    connect(m_recipeListPanel, &RecipeListPanel::importRecipesRequested, this,
            [this](const QStringList &paths) {
                if (!m_viewModel || paths.isEmpty())
                    return;
                m_paramPanel->updateAllSources();
                const QString msg = m_viewModel->importRecipesFromFiles(paths);
                ui::ThemedMessageBox::information(this, TR("aoi.recipe.action.import"), msg);
            });
    connect(m_recipeListPanel, &RecipeListPanel::exportRecipesRequested, this,
            [this](const QString &dir) {
                if (!m_viewModel || dir.isEmpty())
                    return;
                m_paramPanel->updateAllSources();
                const QString msg = m_viewModel->exportAllRecipes(dir);
                ui::ThemedMessageBox::information(this, TR("aoi.recipe.action.export"), msg);
            });
    connect(m_recipeListPanel, &RecipeListPanel::renameRecipeRequested, m_viewModel,
            &RecipeConfigViewModel::renameRecipe);

    if (auto *saveBtn = m_centerView->saveRecipeBtn()) {
        DataBinding::bindCommand(m_viewModel->saveCommand(), saveBtn, [this]() {
            m_paramPanel->updateAllSources();
        });
    }

    connect(m_viewModel, &RecipeConfigViewModel::statusChanged, this,
            &RecipeConfigView::onViewModelStatusChanged);

    if (auto *scanVm = m_viewModel->scanDetectVm()) {
        connect(scanVm, &RecipeScanDetectViewModel::scanDetectingChanged, this, [this, scanVm]() {
            if (auto *btn = m_centerView->scanDetectBtn()) {
                btn->setEnabled(!scanVm->scanDetecting());
                if (!scanVm->scanDetecting())
                    btn->setFocus(Qt::OtherFocusReason);
            }
        });
    }

    if (auto *inspectVm = m_viewModel->virtualInspectVm()) {
        connect(inspectVm, &RecipeVirtualInspectViewModel::virtualInspectingChanged, this,
                [this, inspectVm]() {
                    if (m_centerView)
                        m_centerView->refreshInspectButtonEnabled();
                    if (m_statusToast && inspectVm->virtualInspecting())
                        m_statusToast->hide();
                });

        connect(inspectVm, &RecipeVirtualInspectViewModel::virtualInspectSnapshotChanged, this,
                [this, inspectVm]() {
                    if (!m_centerView)
                        return;
                    const auto &snap = inspectVm->virtualInspectSnapshot();
                    m_centerView->refreshDefectTable();
                    if (auto *detailBtn = m_centerView->resultDetailBtn()) {
                        detailBtn->setEnabled(!snap.serialNumber.isEmpty()
                                              && snap.overallResult
                                                     == aoi::InspectionOverallResult::Ng);
                    }
                });
    }

    if (auto *detailBtn = m_centerView->resultDetailBtn()) {
        detailBtn->setEnabled(false);
        connect(detailBtn, &QPushButton::clicked, this, [this]() {
            auto *inspectVm =
                m_viewModel ? m_viewModel->virtualInspectVm() : nullptr;
            if (!inspectVm || inspectVm->virtualInspectSnapshot().serialNumber.isEmpty())
                return;
            if (auto *svc = ExtensionSystem::PluginManager::getObject<IMainWindowService>())
                svc->switchToPage(QStringLiteral("aoi.nav.review"));
        });
    }

    if (auto *inspectBtn = m_centerView->inspectBtn()) {
        connect(inspectBtn, &QPushButton::clicked, this, [this, inspectBtn]() {
            if (!m_viewModel || !m_centerView)
                return;
            auto *inspectVm = m_viewModel->virtualInspectVm();
            if (!inspectVm)
                return;
            m_paramPanel->updateAllSources();
            QImage image;
            if (!m_centerView->currentPreviewImage(image))
                inspectVm->virtualInspectFromImage({});
            else
                inspectVm->virtualInspectFromImage(image);
            inspectBtn->setFocus(Qt::OtherFocusReason);
        });
    }

    if (auto *snapBtn = m_centerView->toolbarSnapBtn()) {
        connect(snapBtn, &QPushButton::clicked, this, [this]() {
            if (!m_viewModel)
                return;
            m_paramPanel->updateAllSources();
            m_viewModel->snapFromToolbar(m_centerView->currentCameraId(),
                                         m_centerView->previewWinId());
        });
    }

    connect(m_viewModel, &RecipeConfigViewModel::previewImageReady, this,
            [this](QImage image) {
                if (m_centerView)
                    m_centerView->applyPreviewImage(std::move(image));
            });

    if (auto *previewBtn = m_centerView->toolbarPreviewBtn()) {
        const auto updatePreviewTooltip = [previewBtn]() {
            previewBtn->setToolTip(
                TR(previewBtn->isChecked() ? QStringLiteral("aoi.recipe.toolbar.pause")
                                           : QStringLiteral("aoi.recipe.toolbar.preview")));
        };
        connect(previewBtn, &QPushButton::clicked, this,
                [this, previewBtn, updatePreviewTooltip]() {
                    if (!m_viewModel)
                        return;
                    const QString cameraId = m_centerView->currentCameraId();
                    auto *camSvc = ExtensionSystem::PluginManager::getInterface<ICameraService>();
                    const bool previewRunning = camSvc && camSvc->isPreviewActive();

                    if (previewRunning) {
                        {
                            QSignalBlocker blocker(previewBtn);
                            previewBtn->setChecked(false);
                        }
                        updatePreviewTooltip();
                        m_viewModel->stopLivePreview(cameraId, false);
                        return;
                    }

                    m_paramPanel->updateAllSources();
                    const bool started = m_viewModel->tryStartLivePreview(cameraId);
                    {
                        QSignalBlocker blocker(previewBtn);
                        previewBtn->setChecked(started);
                    }
                    updatePreviewTooltip();
                });
        connect(m_viewModel, &RecipeConfigViewModel::previewStopped, this,
                [previewBtn, updatePreviewTooltip]() {
                    QSignalBlocker blocker(previewBtn);
                    previewBtn->setChecked(false);
                    updatePreviewTooltip();
                });
        if (auto *camSvc = ExtensionSystem::PluginManager::getInterface<ICameraService>()) {
            connect(camSvc, &ICameraService::previewActiveChanged, previewBtn,
                    [previewBtn, updatePreviewTooltip](bool active) {
                        QSignalBlocker blocker(previewBtn);
                        previewBtn->setChecked(active);
                        updatePreviewTooltip();
                    });
        }
    }

    connect(m_viewModel, &RecipeConfigViewModel::recipesChanged, this,
            &RecipeConfigView::refreshRecipeList);
    connect(m_viewModel, &RecipeConfigViewModel::documentChanged, this, [this]() {
        if (!m_viewModel)
            return;
        m_paramPanel->beginSuspendUpdates();
        m_centerView->refreshAuxiliaryUi(m_viewModel->document());
        m_paramPanel->refreshFromDocument(m_viewModel->document());
        m_paramPanel->endSuspendUpdates();
        m_centerView->refreshDefectTable();
    });
    connect(m_viewModel, &RecipeConfigViewModel::currentRecipeIdChanged, this,
            &RecipeConfigView::refreshRecipeList);

    if (m_centerView)
        m_centerView->tryLoadDefaultPreviewImage();

    if (auto *bridge = flow_host::flowHostCoordinator().activeBridge()) {
        connect(bridge, &flow::ScanFlowProgressBridge::flowFinished, m_centerView,
                &RecipeConfigCenterView::refreshInspectButtonEnabled);
    }
    if (m_centerView)
        m_centerView->refreshInspectButtonEnabled();
}

// ── 列表刷新 / 状态 Toast ──

void RecipeConfigView::refreshRecipeList()
{
    if (!m_recipeListPanel || !m_viewModel)
        return;

    m_recipeListPanel->setRecipes(m_viewModel->recipes());
    m_recipeListPanel->setCurrentRecipeId(m_viewModel->currentRecipeId());
}

void RecipeConfigView::onViewModelStatusChanged()
{
    if (!m_viewModel || !m_statusToast)
        return;

    const QString text = m_viewModel->status();
    if (text.isEmpty())
        return;

    if (m_skipReadyStatusOnce && text == TR("aoi.recipe.status.ready")) {
        m_skipReadyStatusOnce = false;
        return;
    }

    const QString recognizingBase = TR("aoi.recipe.scan.recognizing");
    if (text.startsWith(recognizingBase)) {
        m_statusToast->showMessage(text, 0);
        return;
    }

    const QString inspectingBase = TR("aoi.recipe.inspect.detecting");
    if (text.startsWith(inspectingBase)) {
        // 检测进度已在预览区居中进度条展示，不再跟随鼠标 Toast
        m_statusToast->hide();
        return;
    }

    m_skipReadyStatusOnce = false;
    m_statusToast->showMessage(text, 2500);
}

} // namespace aoi
