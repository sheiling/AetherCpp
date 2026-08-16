/**
 * @file recipe_config_panel_calib_view.cpp
 * @brief 配方配置 · 灯板标定子面板实现
 */

#include "recipe_config/recipe_config_panel_calib_view.h"

#include "component/dialog/themed_message_box.h"
#include "service/fov_planner/fov_plan_service.h"
#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_push_button.h"
#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/radio/themed_radio_button.h"
#include "component/controls/label/themed_form_label.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "recipe_config_view_model.h"
#include "binding_options.h"
#include "databinding.h"

#include "uibase/languagemanager.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

QLabel *makeSectionTitle(const QString &key, QWidget *parent)
{
    auto *lbl = new QLabel(TR(key), parent);
    lbl->setObjectName(QStringLiteral("recipeMarkSectionLabel"));
    lbl->setProperty("translationKey", key);
    return lbl;
}

ui::ThemedPushButton *makeCalibActionButton(const QString &key, QWidget *parent)
{
    auto *btn = ui::makePushButton(TR(key), PushButtonVariant::Secondary, parent);
    btn->setObjectName(QStringLiteral("recipeCalibPickBtn"));
    btn->setProperty("translationKey", key);
    btn->setMinimumWidth(0);
    btn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    return btn;
}

QString labelTextOrTranslation(const QString &stored)
{
    if (stored.startsWith(QStringLiteral("aoi.")))
        return TR(stored);
    return stored;
}

} // namespace

// ── 构造 / 布局 ──

RecipeConfigPanelCalibView::RecipeConfigPanelCalibView(RecipeConfigViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    buildLayout();
}

void RecipeConfigPanelCalibView::buildLayout()
{
    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(8, 2, 8, 8);
    vl->setSpacing(6);

    m_calibWidgets.panelModeRb = makeRadioButton(TR("aoi.recipe.calib.mode_panel"), this);
    m_calibWidgets.panelModeRb->setChecked(true);
    m_calibWidgets.panelModeRb->setMinimumWidth(0);
    m_calibWidgets.panelModeRb->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_calibWidgets.gerberModeRb = makeRadioButton(TR("aoi.recipe.calib.mode_gerber"), this);
    m_calibWidgets.gerberModeRb->setMinimumWidth(0);
    m_calibWidgets.gerberModeRb->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    auto *modeRow = new QHBoxLayout;
    modeRow->setSpacing(6);
    modeRow->addWidget(m_calibWidgets.panelModeRb, 1);
    modeRow->addWidget(m_calibWidgets.gerberModeRb, 1);
    vl->addLayout(modeRow);

    auto *panelPanel = new QWidget(this);
    panelPanel->setMinimumWidth(0);
    panelPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *panelLayout = new QVBoxLayout(panelPanel);
    panelLayout->setContentsMargins(0, 0, 0, 0);
    panelLayout->setSpacing(4);

    panelLayout->addWidget(
        makeSectionTitle(QStringLiteral("aoi.recipe.calib.fov_title"), panelPanel));

    panelLayout->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.calib.fov_partition"), panelPanel));
    m_calibWidgets.fovGenerateBtn =
        ui::makePushButton(TR("aoi.recipe.calib.fov_generate"), PushButtonVariant::Secondary, panelPanel);
    m_calibWidgets.fovGenerateBtn->setProperty("translationKey",
                                               QStringLiteral("aoi.recipe.calib.fov_generate"));
    panelLayout->addWidget(m_calibWidgets.fovGenerateBtn);

    panelLayout->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.calib.fov_info"), panelPanel));
    m_calibWidgets.fovInfo = new QLabel(TR("aoi.recipe.calib.fov_not_generated"), panelPanel);
    m_calibWidgets.fovInfo->setObjectName(QStringLiteral("recipeMarkReadonly"));
    m_calibWidgets.fovInfo->setProperty("translationKey",
                                          QStringLiteral("aoi.recipe.calib.fov_not_generated"));
    panelLayout->addWidget(m_calibWidgets.fovInfo);

    panelLayout->addWidget(makeSectionTitle(QStringLiteral("aoi.recipe.calib.path_title"), panelPanel));

    panelLayout->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.calib.path_mode"), panelPanel));
    m_calibWidgets.pathMode = makeComboBox(panelPanel);
    m_calibWidgets.pathMode->setMinimumWidth(0);
    m_calibWidgets.pathMode->addItems({TR("aoi.recipe.calib.path_s"), TR("aoi.recipe.calib.path_z"),
                                       TR("aoi.recipe.calib.path_row"),
                                       TR("aoi.recipe.calib.path_col")});
    panelLayout->addWidget(m_calibWidgets.pathMode);

    panelLayout->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.calib.path_diagram"), panelPanel));
    auto *pathDiagram = new QLabel(TR("aoi.recipe.calib.path_diagram_hint"), panelPanel);
    pathDiagram->setObjectName(QStringLiteral("recipeBasicIllus"));
    pathDiagram->setProperty("translationKey", QStringLiteral("aoi.recipe.calib.path_diagram_hint"));
    pathDiagram->setAlignment(Qt::AlignCenter);
    pathDiagram->setMinimumHeight(80);
    panelLayout->addWidget(pathDiagram);

    vl->addWidget(panelPanel);

    auto *gerberPanel = new QWidget(this);
    gerberPanel->setVisible(false);
    auto *gpLayout = new QVBoxLayout(gerberPanel);
    gpLayout->setContentsMargins(0, 0, 0, 0);
    auto *gerberHint = new QLabel(TR("aoi.recipe.calib.gerber_hint"), gerberPanel);
    gerberHint->setObjectName(QStringLiteral("recipeMarkReadonly"));
    gerberHint->setProperty("translationKey", QStringLiteral("aoi.recipe.calib.gerber_hint"));
    gpLayout->addWidget(gerberHint);
    m_calibWidgets.gerberImportBtn =
        ui::makePushButton(TR("aoi.recipe.calib.import_gerber"), PushButtonVariant::Secondary, gerberPanel);
    m_calibWidgets.gerberImportBtn->setProperty("translationKey",
                                                QStringLiteral("aoi.recipe.calib.import_gerber"));
    gpLayout->addWidget(m_calibWidgets.gerberImportBtn);
    m_calibWidgets.gerberFileLbl =
        new QLabel(TR("aoi.recipe.calib.gerber_not_imported"), gerberPanel);
    m_calibWidgets.gerberFileLbl->setObjectName(QStringLiteral("recipeMarkReadonly"));
    m_calibWidgets.gerberFileLbl->setProperty("translationKey",
                                              QStringLiteral("aoi.recipe.calib.gerber_not_imported"));
    gpLayout->addWidget(m_calibWidgets.gerberFileLbl);
    vl->addWidget(gerberPanel);

    if (m_calibWidgets.fovGenerateBtn) {
        connect(m_calibWidgets.fovGenerateBtn, &ui::ThemedPushButton::clicked, this, [this]() {
            if (!m_viewModel)
                return;

            const RecipeDocument doc = m_viewModel->document();
            const auto         result = fov_planner::FovPlanService::generate(doc);

            if (!result.ok) {
                const QString detail =
                    result.message.startsWith(QStringLiteral("aoi.")) ? TR(result.message)
                                                                      : result.message;
                ui::ThemedMessageBox::information(
                    this, TR("aoi.recipe.calib.fov_generate_fail_title"), detail, QString(),
                    ui::ThemedMessageBoxIcon::Warning, ui::PushButtonVariant::Primary);
                if (m_calibWidgets.fovInfo)
                    m_calibWidgets.fovInfo->setText(labelTextOrTranslation(detail));
                return;
            }

            m_viewModel->modifyDocument([&result](RecipeDocument &mutableDoc) {
                mutableDoc.panelCalib.fovInfo = result.message;
            });
            refreshFromDocument(m_viewModel->document());
        });
    }

    connect(m_calibWidgets.gerberImportBtn, &ui::ThemedPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this,
            TR("aoi.recipe.calib.import_gerber"),
            QString(),
            TR("aoi.recipe.calib.gerber_filter"));
        if (path.isEmpty())
            return;
        if (m_calibWidgets.gerberFileLbl)
            m_calibWidgets.gerberFileLbl->setText(QFileInfo(path).fileName());
        m_bindings.updateAllSources();
    });

    connect(m_calibWidgets.panelModeRb, &ui::ThemedRadioButton::toggled, this,
            [panelPanel, gerberPanel](bool on) {
                if (on) {
                    panelPanel->setVisible(true);
                    gerberPanel->setVisible(false);
                }
            });
    connect(m_calibWidgets.gerberModeRb, &ui::ThemedRadioButton::toggled, this,
            [panelPanel, gerberPanel](bool on) {
                if (on) {
                    panelPanel->setVisible(false);
                    gerberPanel->setVisible(true);
                }
            });
}

// ── 数据绑定 ──

void RecipeConfigPanelCalibView::setupBindings()
{
    if (!m_viewModel)
        return;

    using VM = RecipeConfigViewModel;
    ViewModel *const vm   = m_viewModel;
    const QString    prop = VM::PROP_DOCUMENT;

    const BindingOptions onChanged;

    DataBinding::bindStructField<RecipeDocument>(
        vm, prop,
        [](const RecipeDocument &d) { return d.panelCalib.mode == CalibMode::Panel; },
        [](RecipeDocument &d, bool panel) {
            d.panelCalib.mode = panel ? CalibMode::Panel : CalibMode::Gerber;
        },
        m_calibWidgets.panelModeRb, onChanged, &m_bindings);

    DataBinding::bindStructIndexField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.panelCalib.pathModeIndex; },
        [](RecipeDocument &d, int v) { d.panelCalib.pathModeIndex = v; },
        m_calibWidgets.pathMode, onChanged, &m_bindings);
}

void RecipeConfigPanelCalibView::refreshFromDocument(const RecipeDocument &doc)
{
    if (m_calibWidgets.fovInfo)
        m_calibWidgets.fovInfo->setText(labelTextOrTranslation(doc.panelCalib.fovInfo));
    if (m_calibWidgets.gerberFileLbl)
        m_calibWidgets.gerberFileLbl->setText(
            labelTextOrTranslation(doc.panelCalib.gerberFileName));
}

void RecipeConfigPanelCalibView::retranslatePanel()
{
    ui::ThemedFormLabel::retranslateIn(this);

    if (m_calibWidgets.panelModeRb)
        m_calibWidgets.panelModeRb->setText(TR("aoi.recipe.calib.mode_panel"));
    if (m_calibWidgets.gerberModeRb)
        m_calibWidgets.gerberModeRb->setText(TR("aoi.recipe.calib.mode_gerber"));

    for (auto *lbl : findChildren<QLabel *>(QStringLiteral("recipeMarkSectionLabel"))) {
        const QVariant key = lbl->property("translationKey");
        if (key.isValid())
            lbl->setText(TR(key.toString()));
    }

    if (m_calibWidgets.fovGenerateBtn) {
        const QVariant key = m_calibWidgets.fovGenerateBtn->property("translationKey");
        if (key.isValid())
            m_calibWidgets.fovGenerateBtn->setText(TR(key.toString()));
    }
    if (m_calibWidgets.gerberImportBtn) {
        const QVariant key = m_calibWidgets.gerberImportBtn->property("translationKey");
        if (key.isValid())
            m_calibWidgets.gerberImportBtn->setText(TR(key.toString()));
    }

    if (m_calibWidgets.pathMode) {
        const int idx = m_calibWidgets.pathMode->currentIndex();
        m_calibWidgets.pathMode->blockSignals(true);
        m_calibWidgets.pathMode->clear();
        m_calibWidgets.pathMode->addItems({TR("aoi.recipe.calib.path_s"), TR("aoi.recipe.calib.path_z"),
                                           TR("aoi.recipe.calib.path_row"),
                                           TR("aoi.recipe.calib.path_col")});
        m_calibWidgets.pathMode->setCurrentIndex(idx);
        m_calibWidgets.pathMode->blockSignals(false);
    }
}

} // namespace aoi
