/**
 * @file recipe_config_param_panel.cpp
 * @brief 配方配置 · 右侧参数面板实现
 */

#include "recipe_config/recipe_config_param_panel.h"

#include "component/group/form_group_box.h"
#include "recipe_config/recipe_config_algo_view.h"
#include "recipe_config/recipe_config_basic_view.h"
#include "recipe_config/recipe_config_panel_calib_view.h"
#include "recipe_config/recipe_config_camera_view.h"
#include "recipe_config/recipe_config_mark_view.h"
#include "recipe_config/recipe_config_position_calib_view.h"
#include "component/controls/label/themed_form_label.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "recipe_config_view_model.h"
#include "component/theme/theme_font.h"

#include <QAbstractSpinBox>
#include <QComboBox>
#include <QEvent>
#include <QFrame>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSlider>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

void resetWidgetFontToStylesheet(QWidget *w)
{
    if (!w || !w->style())
        return;
    w->setFont(QFont());
    w->style()->unpolish(w);
    w->style()->polish(w);
}

void refreshParamPanelInputFonts(QWidget *paramPanel)
{
    if (!paramPanel)
        return;

    ui::ThemedFormLabel::applyFontsIn(paramPanel);

    for (auto *w : paramPanel->findChildren<QWidget *>()) {
        if (qobject_cast<ui::ThemedFormLabel *>(w))
            continue;
        if (qobject_cast<QLineEdit *>(w) || qobject_cast<QAbstractSpinBox *>(w)
            || qobject_cast<QComboBox *>(w) || qobject_cast<QPushButton *>(w)
            || qobject_cast<QRadioButton *>(w) || qobject_cast<QSlider *>(w))
            resetWidgetFontToStylesheet(w);
    }

    ensureThemedSpinStyles(paramPanel);
}

} // namespace

class RecipeParamScrollGuard final : public QObject
{
public:
    RecipeParamScrollGuard(QScrollArea *scroll, QWidget *content, QObject *parent = nullptr)
        : QObject(parent)
        , m_scroll(scroll)
        , m_content(content)
    {
        if (m_scroll && m_scroll->viewport())
            m_scroll->viewport()->installEventFilter(this);
        if (m_content)
            m_content->installEventFilter(this);
        syncContentWidth();
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (m_scroll && watched == m_scroll->viewport()
            && (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
            syncContentWidth();
        }
        if (watched == m_content
            && (event->type() == QEvent::LayoutRequest || event->type() == QEvent::Show)) {
            syncContentWidth();
        }
        return QObject::eventFilter(watched, event);
    }

private:
    void syncContentWidth()
    {
        if (!m_scroll || !m_content || !m_scroll->viewport())
            return;

        const int w = m_scroll->viewport()->width();
        if (w <= 0)
            return;

        m_content->setMaximumWidth(w);
        m_content->setMinimumWidth(0);

        const auto sections = m_content->findChildren<ui::FormGroupBox *>();
        for (auto *section : sections)
            section->setMaximumWidth(w);
    }

    QScrollArea *m_scroll   = nullptr;
    QWidget     *m_content  = nullptr;
};

// ── 构造 / 六段 FormGroup ──

RecipeConfigParamPanel::RecipeConfigParamPanel(RecipeConfigViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    buildLayout();
}

void RecipeConfigParamPanel::buildLayout()
{
    setObjectName(QStringLiteral("recipeParamPanel"));
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedWidth(360);
    setMaximumWidth(360);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 10, 8, 10);
    outer->setSpacing(8);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *content = new QWidget(scroll);
    content->setObjectName(QStringLiteral("recipeParamSectionsWrap"));
    content->setMinimumWidth(0);
    auto *layout  = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_basicView  = new RecipeConfigBasicView(m_viewModel, nullptr);
    m_cameraView = new RecipeConfigCameraView(m_viewModel, nullptr);
    m_algoView   = new RecipeConfigAlgoView(m_viewModel, nullptr);
    m_markView   = new RecipeConfigMarkView(m_viewModel, nullptr);
    m_panelCalibView    = new RecipeConfigPanelCalibView(m_viewModel, nullptr);
    m_positionCalibView = new RecipeConfigPositionCalibView(m_viewModel, nullptr);

    const auto addSection = [&](const QString &titleKey, QWidget *sectionView) {
        layout->addWidget(makeFormGroup(titleKey, sectionView, content, true), 0, Qt::AlignTop);
    };

    addSection(QStringLiteral("aoi.recipe.section.basic"), m_basicView);
    addSection(QStringLiteral("aoi.recipe.section.camera"), m_cameraView);
    addSection(QStringLiteral("aoi.recipe.section.algo"), m_algoView);
    addSection(QStringLiteral("aoi.recipe.section.mark"), m_markView);
    addSection(QStringLiteral("aoi.recipe.section.panel_calib"), m_panelCalibView);
    addSection(QStringLiteral("aoi.recipe.section.position_calib"), m_positionCalibView);
    layout->addStretch();

    scroll->setWidget(content);
    outer->addWidget(scroll, 1);

    new RecipeParamScrollGuard(scroll, content, scroll);
}

ui::FormGroupBox *RecipeConfigParamPanel::makeFormGroup(const QString &titleKey, QWidget *sectionContent,
                                                    QWidget *parent, bool expanded)
{
    auto *section = new ui::FormGroupBox(titleKey, ui::FormGroupBoxTitleStyle::Collapsible, parent, expanded);
    section->setContentWidget(sectionContent);
    m_formGroups.append(section);
    return section;
}

// ── 子面板绑定 / 文档同步 ──

void RecipeConfigParamPanel::setupBindings()
{
    if (m_basicView)
        m_basicView->setupBindings();
    if (m_cameraView)
        m_cameraView->setupBindings();
    if (m_algoView)
        m_algoView->setupBindings();
    if (m_markView)
        m_markView->setupBindings();
    if (m_panelCalibView)
        m_panelCalibView->setupBindings();
    if (m_positionCalibView)
        m_positionCalibView->setupBindings();
}

void RecipeConfigParamPanel::updateAllSources()
{
    if (m_basicView)
        m_basicView->updateAllSources();
    if (m_cameraView)
        m_cameraView->updateAllSources();
    if (m_algoView)
        m_algoView->updateAllSources();
    if (m_markView)
        m_markView->updateAllSources();
    if (m_panelCalibView)
        m_panelCalibView->updateAllSources();
    if (m_positionCalibView)
        m_positionCalibView->updateAllSources();
}

void RecipeConfigParamPanel::beginSuspendUpdates()
{
    if (m_basicView)
        m_basicView->beginSuspendUpdates();
    if (m_cameraView)
        m_cameraView->beginSuspendUpdates();
    if (m_algoView)
        m_algoView->beginSuspendUpdates();
    if (m_markView)
        m_markView->beginSuspendUpdates();
    if (m_panelCalibView)
        m_panelCalibView->beginSuspendUpdates();
    if (m_positionCalibView)
        m_positionCalibView->beginSuspendUpdates();
}

void RecipeConfigParamPanel::endSuspendUpdates()
{
    if (m_basicView)
        m_basicView->endSuspendUpdates();
    if (m_cameraView)
        m_cameraView->endSuspendUpdates();
    if (m_algoView)
        m_algoView->endSuspendUpdates();
    if (m_markView)
        m_markView->endSuspendUpdates();
    if (m_panelCalibView)
        m_panelCalibView->endSuspendUpdates();
    if (m_positionCalibView)
        m_positionCalibView->endSuspendUpdates();
}

void RecipeConfigParamPanel::retranslatePanel()
{
    for (auto *section : m_formGroups)
        section->retranslateGroup();

    if (m_basicView)
        m_basicView->retranslatePanel();
    if (m_cameraView)
        m_cameraView->retranslatePanel();
    if (m_algoView)
        m_algoView->retranslatePanel();
    if (m_markView)
        m_markView->retranslatePanel();
    if (m_panelCalibView)
        m_panelCalibView->retranslatePanel();
    if (m_positionCalibView)
        m_positionCalibView->retranslatePanel();

    ui::ThemedFormLabel::retranslateIn(this);
}

void RecipeConfigParamPanel::refreshPresentation()
{
    refreshParamPanelInputFonts(this);
    if (m_markView)
        m_markView->refreshCompactLayout();
    if (m_positionCalibView)
        m_positionCalibView->refreshCompactLayout();
}

void RecipeConfigParamPanel::refreshFromDocument(const RecipeDocument &doc)
{
    if (m_panelCalibView)
        m_panelCalibView->refreshFromDocument(doc);
    if (m_markView) {
        m_markView->refreshMarkTabs();
        m_markView->refreshMarkSlotUi(doc, m_markView->activeMarkSlot());
    }
}

} // namespace aoi