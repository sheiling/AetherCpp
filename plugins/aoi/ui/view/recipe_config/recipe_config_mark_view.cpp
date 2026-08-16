/**
 * @file recipe_config_mark_view.cpp
 * @brief 配方配置 · Mark 制作子面板实现
 */

#include "recipe_config/recipe_config_mark_view.h"

#include <memory>

#include "service/vision_bridge/model_generator.h"
#include "service/vision_bridge/mark_template_kind.h"
#include "image/image_info_display.h"
#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_button_group.h"
#include "component/controls/button/themed_icon_button.h"
#include "component/font/fa_icon_button.h"
#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/radio/themed_radio_button.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "mark_template/mark_template_dialog.h"
#include "mark_template/mark_template_pick_dialog.h"
#include "component/dialog/themed_panel_dialog.h"
#include "component/dialog/themed_message_box.h"
#include "component/controls/label/themed_form_label.h"
#include "component/controls/input/themed_line_edit.h"
#include "recipe_config_model.h"
#include "system_config_model.h"
#include "component/theme/theme_font.h"
#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"
#include "recipe_config_view_model.h"
#include "recipe_mark_calc_view_model.h"

#include <icamera_service.h>
#include <extensionsystem/pluginmanager.h>
#include "uibase/languagemanager.h"
#include <common/include/interface/igraphicsView.h>

#include <QApplication>
#include <QAbstractSpinBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLocale>
#include <QPixmap>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

int tuneMarkPosItemsIn(QWidget *root)
{
    if (!root)
        return 0;

    QFont spinFont = themeUiFont(9);
    spinFont.setPixelSize(11);

    int maxItemH = 0;
    for (auto *item : root->findChildren<QWidget *>(QStringLiteral("recipeMarkPosItem"))) {
        auto *lbl = item->findChild<ui::ThemedFormLabel *>();
        auto *sp  = item->findChild<QLineEdit *>(QStringLiteral("recipeMarkPosEdit"));
        if (!sp)
            continue;

        if (lbl)
            lbl->applyFontMetrics();
        sp->setFont(spinFont);
        sp->ensurePolished();

        const int editH = qMax(28, sp->sizeHint().height());
        sp->setFixedHeight(editH);

        const int labelH = lbl ? lbl->fontMetrics().height() : 0;
        const int itemH  = labelH + 2 + editH + 2;
        item->setMinimumHeight(itemH);
        maxItemH         = qMax(maxItemH, itemH);
    }
    return maxItemH;
}

void refreshMarkCapturePosLayout(QWidget *markPanel)
{
    if (!markPanel)
        return;

    const int maxItemH = tuneMarkPosItemsIn(markPanel);
    if (auto *posRow = markPanel->findChild<QWidget *>(QStringLiteral("recipeMarkPosRow"))) {
        const int rowH = qMax(50, maxItemH + 4);
        posRow->setMinimumHeight(rowH);
        posRow->setMaximumHeight(rowH);
        posRow->setMinimumWidth(0);
        posRow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
}

void tuneRecipeFormGridColumns(QGridLayout *grid)
{
    if (!grid)
        return;
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 1);
    grid->setColumnMinimumWidth(1, 0);
}

QList<QPair<int, QString>> markTypeItems()
{
    return {{static_cast<int>(vision_algo::MarkerType::Circle), TR("aoi.recipe.mark.type_circle")},
            {static_cast<int>(vision_algo::MarkerType::Cross), TR("aoi.recipe.mark.type_cross")},
            {static_cast<int>(vision_algo::MarkerType::Rect), TR("aoi.recipe.mark.type_rect")},
            {static_cast<int>(vision_algo::MarkerType::Ring), TR("aoi.recipe.mark.type_ring")},
            {static_cast<int>(vision_algo::MarkerType::FalseCross),
             TR("aoi.recipe.mark.type_false_cross")},
            {static_cast<int>(vision_algo::MarkerType::RectCorner),
             TR("aoi.recipe.mark.type_rect_corner")},
            {static_cast<int>(vision_algo::MarkerType::Line), TR("aoi.recipe.mark.type_line")}};
}

void fillMarkTypeCombo(QComboBox *combo, vision_algo::MarkerType current)
{
    if (!combo)
        return;
    combo->clear();
    for (const auto &item : markTypeItems())
        combo->addItem(item.second, item.first);
    const int idx = combo->findData(static_cast<int>(current));
    combo->setCurrentIndex(idx >= 0 ? idx : 0);
}

vision_algo::MarkerType markTypeFromCombo(const QComboBox *combo)
{
    if (!combo)
        return vision_algo::MarkerType::Circle;
    return static_cast<vision_algo::MarkerType>(combo->currentData().toInt());
}

void addMarkGridRow(QGridLayout *grid, int row, const QString &labelKey, QWidget *control,
                    QWidget *parent, bool centerRow = false)
{
    auto *lbl = makeFormLabel(labelKey, parent);
    lbl->setMinimumWidth(0);
    lbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    if (control) {
        control->setMinimumWidth(0);
        control->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }

    const Qt::Alignment labelAlign =
        centerRow ? Qt::AlignLeft | Qt::AlignVCenter : Qt::AlignLeft | Qt::AlignTop;
    grid->addWidget(lbl, row, 0, labelAlign);
    grid->addWidget(control, row, 1);
}

QFrame *makeMarkBorderedSection(QWidget *parent, const QString &objectName)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName(objectName);
    frame->setFrameShape(QFrame::NoFrame);
    frame->setAttribute(Qt::WA_StyledBackground, true);
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    const auto refresh = [frame]() {
        const ui::ThemeTokens t = ui::ThemeTokens::resolve();
        frame->setStyleSheet(QStringLiteral(
                                 "QFrame#%1 { background-color: %2; border: 1px solid %3;"
                                 " border-radius: 6px; }")
                                 .arg(frame->objectName(), t.surface.name(), t.surfaceBorder.name()));
    };
    refresh();
    bindThemeRefresh(frame, refresh);
    return frame;
}

QWidget *makeMarkPosItem(const QString &axisLabelKey, double value, QWidget *parent,
                         QLineEdit **outEdit)
{
    auto *item = new QWidget(parent);
    item->setObjectName(QStringLiteral("recipeMarkPosItem"));
    auto *vl   = new QVBoxLayout(item);
    vl->setContentsMargins(0, 0, 0, 2);
    vl->setSpacing(2);
    vl->addWidget(makeFormLabel(axisLabelKey, item));
    auto *edit = makeLineEdit(item);
    edit->setObjectName(QStringLiteral("recipeMarkPosEdit"));
    edit->setText(QString::number(value, 'f', 3));
    edit->setMinimumWidth(0);
    edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    vl->addWidget(edit);
    if (outEdit)
        *outEdit = edit;
    return item;
}

} // namespace

// ── 构造 / 预览上下文 ──

RecipeConfigMarkView::RecipeConfigMarkView(RecipeConfigViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    m_markRoiTimer = new QTimer(this);
    m_markRoiTimer->setInterval(200);
    connect(m_markRoiTimer, &QTimer::timeout, this,
            &RecipeConfigMarkView::refreshMarkTemplateRoiHint);

    buildLayout();
    QTimer::singleShot(0, this, [this]() { refreshCompactLayout(); });
}

void RecipeConfigMarkView::setPreviewContext(IGraphicsVew *graphicsView, int winId)
{
    m_graphicsView   = graphicsView;
    m_previewWinId   = winId;
}

void RecipeConfigMarkView::setCameraIdProvider(std::function<QString()> provider)
{
    m_cameraIdProvider = std::move(provider);
}

// ── 布局 ──

void RecipeConfigMarkView::buildLayout()
{
    setObjectName(QStringLiteral("recipeMarkPanel"));
    setMinimumWidth(0);

    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(4, 2, 4, 8);
    vl->setSpacing(8);

    m_markTabGroup = new ui::ThemedButtonGroup(this);
    m_activeMarkSlot = 0;
    refreshMarkTabs();
    vl->addWidget(m_markTabGroup);

    auto *captureBlock = new QWidget(this);
    captureBlock->setObjectName(QStringLiteral("recipeMarkCaptureBlock"));
    captureBlock->setMinimumWidth(0);
    captureBlock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *captureVl = new QVBoxLayout(captureBlock);
    captureVl->setContentsMargins(0, 0, 0, 6);
    captureVl->setSpacing(4);

    auto *sectionPos = new QLabel(TR("aoi.recipe.mark.capture_pos"), captureBlock);
    sectionPos->setObjectName(QStringLiteral("recipeMarkSectionLabel"));
    captureVl->addWidget(sectionPos);

    auto *posRow = new QWidget(captureBlock);
    posRow->setObjectName(QStringLiteral("recipeMarkPosRow"));
    posRow->setMinimumWidth(0);
    auto *posHl  = new QHBoxLayout(posRow);
    posHl->setContentsMargins(0, 0, 0, 4);
    posHl->setSpacing(4);
    posHl->addWidget(makeMarkPosItem(QStringLiteral("aoi.recipe.axis.x"), 12.500, posRow,
                                     &m_markWidgets.capture.x),
                     1);
    posHl->addWidget(makeMarkPosItem(QStringLiteral("aoi.recipe.axis.y"), 8.200, posRow,
                                     &m_markWidgets.capture.y),
                     1);
    posHl->addWidget(makeMarkPosItem(QStringLiteral("aoi.recipe.axis.z"), 0.000, posRow,
                                     &m_markWidgets.capture.z),
                     1);
    auto *getPosBtn = ui::makeFaIconButton(ui::FaIcon::Crosshairs, posRow,
                                                    QStringLiteral("aoi.recipe.mark.get_pos"));
    auto *movePosBtn = ui::makeFaIconButton(ui::FaIcon::LocationArrow, posRow,
                                                     QStringLiteral("aoi.recipe.mark.move_pos"));
    posHl->addWidget(getPosBtn, 0, Qt::AlignBottom);
    posHl->addWidget(movePosBtn, 0, Qt::AlignBottom);
    captureVl->addWidget(posRow);
    vl->addWidget(captureBlock);

    auto *gridHost = new QWidget(this);
    gridHost->setObjectName(QStringLiteral("recipeMarkFields"));
    gridHost->setMinimumWidth(0);
    gridHost->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *grid     = new QGridLayout(gridHost);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(6);
    grid->setVerticalSpacing(6);
    tuneRecipeFormGridColumns(grid);

    int row = 0;

    auto *sectionTpl = new QLabel(TR("aoi.recipe.mark.template_info"), gridHost);
    sectionTpl->setObjectName(QStringLiteral("recipeMarkSectionLabel"));
    grid->addWidget(sectionTpl, row, 0, 1, 2);
    ++row;

    auto *tplActions = new QWidget(gridHost);
    tplActions->setObjectName(QStringLiteral("recipeMarkTemplateActions"));
    auto *tplHl = new QHBoxLayout(tplActions);
    tplHl->setContentsMargins(0, 0, 0, 0);
    tplHl->setSpacing(8);
    auto *pickBtn = ui::makePushButton(TR("aoi.recipe.mark.pick_template"), PushButtonVariant::Secondary,
                                  tplActions);
    pickBtn->setMinimumWidth(0);
    pickBtn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_pickTemplateBtn = pickBtn;
    auto *newBtn = ui::makePushButton(TR("aoi.recipe.mark.new_template"), PushButtonVariant::Secondary,
                                  tplActions);
    newBtn->setMinimumWidth(0);
    newBtn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_newTemplateBtn = newBtn;
    tplHl->addWidget(pickBtn, 1);
    tplHl->addWidget(newBtn, 1);
    grid->addWidget(tplActions, row, 0, 1, 2);
    ++row;

    m_markWidgets.templateFileLbl = new QLabel(QStringLiteral("mark_std_t1.tpl"), gridHost);
    m_markWidgets.templateFileLbl->setObjectName(QStringLiteral("recipeMarkReadonly"));
    m_markWidgets.templateFileLbl->setWordWrap(true);
    addMarkGridRow(grid, row, QStringLiteral("aoi.recipe.mark.file_name"),
                   m_markWidgets.templateFileLbl, gridHost);
    ++row;

    m_markWidgets.templateNameLbl = new QLabel(TR("aoi.recipe.mark.demo_name"), gridHost);
    m_markWidgets.templateNameLbl->setObjectName(QStringLiteral("recipeMarkReadonly"));
    m_markWidgets.templateNameLbl->setWordWrap(true);
    addMarkGridRow(grid, row, QStringLiteral("aoi.recipe.mark.template_name"),
                   m_markWidgets.templateNameLbl, gridHost);
    ++row;

    auto *thumb = new QLabel(TR("aoi.recipe.mark.thumb_placeholder"), gridHost);
    thumb->setObjectName(QStringLiteral("recipeMarkThumb"));
    thumb->setAlignment(Qt::AlignCenter);
    thumb->setMinimumHeight(72);
    m_markWidgets.thumb = thumb;
    addMarkGridRow(grid, row, QStringLiteral("aoi.recipe.mark.thumb"), thumb, gridHost);
    ++row;

    connect(m_pickTemplateBtn, &ui::ThemedPushButton::clicked, this,
            &RecipeConfigMarkView::openMarkTemplatePickDialog);
    connect(m_newTemplateBtn, &ui::ThemedPushButton::clicked, this,
            &RecipeConfigMarkView::openMarkTemplateDialog);

    vl->addWidget(gridHost);
    vl->addWidget(buildMarkParamsSection(this));
    vl->addWidget(buildMarkCalcSection(this), 1);

    if (m_markTabGroup)
        connect(m_markTabGroup, &ui::ThemedButtonGroup::currentIndexChanged, this,
                &RecipeConfigMarkView::onMarkTabChanged);
}

QWidget *RecipeConfigMarkView::buildMarkParamsSection(QWidget *parent)
{
    auto *section = makeMarkBorderedSection(parent, QStringLiteral("recipeMarkParamsBlock"));
    auto *layout  = new QVBoxLayout(section);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    auto *grid = new QGridLayout;
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(6);
    grid->setVerticalSpacing(6);
    tuneRecipeFormGridColumns(grid);

    int row = 0;

    m_markWidgets.markType = makeComboBox(section);
    fillMarkTypeCombo(m_markWidgets.markType, vision_algo::MarkerType::Circle);
    addMarkGridRow(grid, row, QStringLiteral("aoi.recipe.mark.type"), m_markWidgets.markType, section,
                   true);
    ++row;

    auto *sizeRow = new QWidget(section);
    sizeRow->setObjectName(QStringLiteral("recipeMarkSizeRow"));
    sizeRow->setMinimumWidth(0);
    sizeRow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *sizeHl  = new QHBoxLayout(sizeRow);
    sizeHl->setContentsMargins(0, 0, 0, 0);
    sizeHl->setSpacing(4);
    m_markWidgets.markWidth = makeSpinBox(sizeRow, QStringLiteral("recipeMarkSizeSpin"));
    m_markWidgets.markWidth->setRange(1, 9999);
    m_markWidgets.markWidth->setValue(32);
    m_markWidgets.markHeight = makeSpinBox(sizeRow, QStringLiteral("recipeMarkSizeSpin"));
    m_markWidgets.markHeight->setRange(1, 9999);
    m_markWidgets.markHeight->setValue(32);
    sizeHl->addWidget(m_markWidgets.markWidth, 1);
    sizeHl->addWidget(m_markWidgets.markHeight, 1);
    addMarkGridRow(grid, row, QStringLiteral("aoi.recipe.mark.size"), sizeRow, section, true);
    ++row;

    m_markWidgets.areaThreshold = makeSpinBox(section);
    m_markWidgets.areaThreshold->setRange(0, 999999);
    m_markWidgets.areaThreshold->setValue(120);
    addMarkGridRow(grid, row, QStringLiteral("aoi.recipe.mark.area_threshold"),
                   m_markWidgets.areaThreshold, section, true);
    ++row;

    m_markWidgets.matchMax = makeDoubleSpinBox(section);
    m_markWidgets.matchMax->setRange(0, 1);
    m_markWidgets.matchMax->setDecimals(2);
    m_markWidgets.matchMax->setSingleStep(0.1);
    m_markWidgets.matchMax->setValue(0.95);
    addMarkGridRow(grid, row, QStringLiteral("aoi.recipe.mark.match_max"), m_markWidgets.matchMax,
                   section, true);
    ++row;

    m_markWidgets.matchMin = makeDoubleSpinBox(section);
    m_markWidgets.matchMin->setRange(0, 1);
    m_markWidgets.matchMin->setDecimals(2);
    m_markWidgets.matchMin->setSingleStep(0.1);
    m_markWidgets.matchMin->setValue(0.75);
    addMarkGridRow(grid, row, QStringLiteral("aoi.recipe.mark.match_min"), m_markWidgets.matchMin,
                   section, true);
    ++row;

    auto *bgWrap = new QWidget(section);
    bgWrap->setObjectName(QStringLiteral("recipeMarkBgOptions"));
    bgWrap->setMinimumWidth(0);
    bgWrap->setAutoFillBackground(false);
    bgWrap->setAttribute(Qt::WA_StyledBackground, true);
    bgWrap->setStyleSheet(QStringLiteral("background: transparent;"));
    bgWrap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *bgRow  = new QHBoxLayout(bgWrap);
    bgRow->setContentsMargins(0, 0, 0, 0);
    bgRow->setSpacing(12);
    m_markWidgets.bgBlack = makeRadioButton(TR("aoi.recipe.mark.bg_black"), bgWrap);
    m_markWidgets.bgBlack->setChecked(true);
    m_markWidgets.bgWhite = makeRadioButton(TR("aoi.recipe.mark.bg_white"), bgWrap);
    bgRow->addWidget(m_markWidgets.bgBlack, 1);
    bgRow->addWidget(m_markWidgets.bgWhite, 1);
    addMarkGridRow(grid, row, QStringLiteral("aoi.recipe.mark.background"), bgWrap, section, true);

    layout->addLayout(grid);
    return section;
}

QWidget *RecipeConfigMarkView::buildMarkCalcSection(QWidget *parent)
{
    auto *section = makeMarkBorderedSection(parent, QStringLiteral("recipeMarkCalcBlock"));
    auto *layout  = new QVBoxLayout(section);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    auto *calcBtn = ui::makePushButton(TR("aoi.recipe.mark.calc"), PushButtonVariant::Primary, section);
    m_markWidgets.calcBtn = calcBtn;
    layout->addWidget(calcBtn);

    auto *calcGrid = new QGridLayout;
    calcGrid->setHorizontalSpacing(6);
    calcGrid->setVerticalSpacing(6);
    tuneRecipeFormGridColumns(calcGrid);

    auto *offsetXLbl = new QLabel(TR("aoi.recipe.mark.no_value"), section);
    offsetXLbl->setObjectName(QStringLiteral("recipeMarkReadonly"));
    m_markWidgets.offsetX = offsetXLbl;
    addMarkGridRow(calcGrid, 0, QStringLiteral("aoi.recipe.mark.offset_x"), offsetXLbl, section, true);

    auto *offsetYLbl = new QLabel(TR("aoi.recipe.mark.no_value"), section);
    offsetYLbl->setObjectName(QStringLiteral("recipeMarkReadonly"));
    m_markWidgets.offsetY = offsetYLbl;
    addMarkGridRow(calcGrid, 1, QStringLiteral("aoi.recipe.mark.offset_y"), offsetYLbl, section, true);

    auto *resultThumb = new QLabel(TR("aoi.recipe.mark.result_placeholder"), section);
    resultThumb->setObjectName(QStringLiteral("recipeMarkResultThumb"));
    resultThumb->setAlignment(Qt::AlignCenter);
    resultThumb->setMinimumHeight(64);
    m_markWidgets.calcResultThumb = resultThumb;
    addMarkGridRow(calcGrid, 2, QStringLiteral("aoi.recipe.mark.result_thumb"), resultThumb, section,
                   true);

    layout->addLayout(calcGrid);
    return section;
}

// ── 数据绑定 / 写回 ──

void RecipeConfigMarkView::syncActiveMarkSlotToViewModel()
{
    if (!m_viewModel)
        return;

    const int slot  = m_activeMarkSlot;
    const int count = recipeMarkCount(SystemConfigRepository::load().common.trackMode);

    m_viewModel->modifyDocument([&](RecipeDocument &doc) {
        MarkPointConfig &m = mutableMarkAt(doc, slot, count);

        const auto readCoord = [](QLineEdit *edit, double &out) {
            if (!edit)
                return;
            bool           ok = false;
            const double v  = QLocale::c().toDouble(edit->text().trimmed(), &ok);
            if (ok)
                out = v;
        };
        readCoord(m_markWidgets.capture.x, m.capturePos.x);
        readCoord(m_markWidgets.capture.y, m.capturePos.y);
        readCoord(m_markWidgets.capture.z, m.capturePos.z);

        if (m_markWidgets.templateFileLbl)
            m.markerInfo.modelFileName = m_markWidgets.templateFileLbl->text().trimmed().toStdString();
        if (m_markWidgets.templateNameLbl)
            m.markerInfo.modelName = m_markWidgets.templateNameLbl->text().trimmed().toStdString();

        if (m_markWidgets.markType)
            m.markerInfo.markType = markTypeFromCombo(m_markWidgets.markType);
        if (m_markWidgets.markWidth)
            m.markerInfo.markWidth = m_markWidgets.markWidth->value();
        if (m_markWidgets.markHeight)
            m.markerInfo.markHeight = m_markWidgets.markHeight->value();
        if (m_markWidgets.areaThreshold)
            m.markerInfo.area = static_cast<float>(m_markWidgets.areaThreshold->value());
        if (m_markWidgets.matchMax)
            m.markerInfo.maxThresh = static_cast<float>(m_markWidgets.matchMax->value());
        if (m_markWidgets.matchMin)
            m.markerInfo.minThresh = static_cast<float>(m_markWidgets.matchMin->value());
        if (m_markWidgets.bgBlack)
            m.markerInfo.isWhiteMark = m_markWidgets.bgBlack->isChecked() ? static_cast<uint16_t>(0)
                                                                      : static_cast<uint16_t>(1);
    });
}

void RecipeConfigMarkView::updateAllSources()
{
    syncActiveMarkSlotToViewModel();
}

void RecipeConfigMarkView::setupBindings()
{
    // Mark 槽位字段仅在 updateAllSources（保存前 flush）时写入 document.marks[activeSlot]；
    // 切 Tab 只刷新 UI，不写回 ViewModel。
}

// ── 外部 VM 连接（计算按钮 / 定位结果） ──

void RecipeConfigMarkView::setupExternalConnections(RecipeConfigViewModel *viewModel)
{
    if (!viewModel)
        return;

    RecipeMarkCalcViewModel *markVm = viewModel->markCalcVm();
    if (!markVm)
        return;

    if (m_markWidgets.calcBtn) {
        connect(m_markWidgets.calcBtn, &ui::ThemedPushButton::clicked, this, [this, markVm]() {
            if (!m_viewModel)
                return;
            if (!m_graphicsView) {
                notifyUser(TR("aoi.recipe.mark.no_preview"),
                           ui::ThemedMessageBoxIcon::Warning);
                return;
            }

            // 实时预览：先停流锁定最后一帧，再从 GraphicsView 取图（与虚拟调测锁帧策略一致）
            if (auto *camSvc = ExtensionSystem::PluginManager::getInterface<ICameraService>()) {
                if (camSvc->isPreviewActive())
                    camSvc->stopPreview(false);
            }

            QImage preview;
            if (!m_graphicsView->GetImage(m_previewWinId, preview) || preview.isNull()) {
                notifyUser(TR("aoi.recipe.mark.no_preview"),
                           ui::ThemedMessageBoxIcon::Warning);
                return;
            }

            markVm->runMarkCalcFromImage(preview.copy(), m_activeMarkSlot);
        });
    }

    connect(markVm, &RecipeMarkCalcViewModel::markCalculatingChanged, this, [this, markVm]() {
        if (m_markWidgets.calcBtn)
            m_markWidgets.calcBtn->setEnabled(!markVm->markCalculating());
    });

    connect(markVm, &RecipeMarkCalcViewModel::markLocateResultChanged, this, [this, markVm]() {
        if (markVm->markLocateSlot() != m_activeMarkSlot)
            return;
        applyMarkCalcResult(markVm->markLocateResult());
    });
}

// ── Mark 槽位 UI 刷新 ──

void RecipeConfigMarkView::refreshMarkSlotUi(const RecipeDocument &doc, int slotIndex)
{
    const MarkPointConfig &m = markAt(doc, slotIndex);

    if (m_markWidgets.capture.x) {
        QSignalBlocker b(m_markWidgets.capture.x);
        m_markWidgets.capture.x->setText(QString::number(m.capturePos.x, 'f', 3));
    }
    if (m_markWidgets.capture.y) {
        QSignalBlocker b(m_markWidgets.capture.y);
        m_markWidgets.capture.y->setText(QString::number(m.capturePos.y, 'f', 3));
    }
    if (m_markWidgets.capture.z) {
        QSignalBlocker b(m_markWidgets.capture.z);
        m_markWidgets.capture.z->setText(QString::number(m.capturePos.z, 'f', 3));
    }
    if (m_markWidgets.templateFileLbl) {
        const QString file = QString::fromStdString(m.markerInfo.modelFileName).trimmed();
        m_markWidgets.templateFileLbl->setText(
            file.isEmpty() ? QStringLiteral("mark_std_t1.tpl") : file);
    }
    if (m_markWidgets.templateNameLbl) {
        const QString name = QString::fromStdString(m.markerInfo.modelName).trimmed();
        m_markWidgets.templateNameLbl->setText(
            name.isEmpty() ? TR("aoi.recipe.mark.demo_name") : name);
    }
    if (m_markWidgets.markType) {
        QSignalBlocker b(m_markWidgets.markType);
        fillMarkTypeCombo(m_markWidgets.markType, m.markerInfo.markType);
    }
    if (m_markWidgets.markWidth) {
        QSignalBlocker b(m_markWidgets.markWidth);
        m_markWidgets.markWidth->setValue(m.markerInfo.markWidth);
    }
    if (m_markWidgets.markHeight) {
        QSignalBlocker b(m_markWidgets.markHeight);
        m_markWidgets.markHeight->setValue(m.markerInfo.markHeight);
    }
    if (m_markWidgets.areaThreshold) {
        QSignalBlocker b(m_markWidgets.areaThreshold);
        m_markWidgets.areaThreshold->setValue(static_cast<int>(m.markerInfo.area));
    }
    if (m_markWidgets.matchMax) {
        QSignalBlocker b(m_markWidgets.matchMax);
        m_markWidgets.matchMax->setValue(m.markerInfo.maxThresh);
    }
    if (m_markWidgets.matchMin) {
        QSignalBlocker b(m_markWidgets.matchMin);
        m_markWidgets.matchMin->setValue(m.markerInfo.minThresh);
    }
    if (m_markWidgets.bgBlack && m_markWidgets.bgWhite) {
        const bool blackBg = m.markerInfo.isWhiteMark == 0;
        QSignalBlocker b1(m_markWidgets.bgBlack);
        QSignalBlocker b2(m_markWidgets.bgWhite);
        m_markWidgets.bgBlack->setChecked(blackBg);
        m_markWidgets.bgWhite->setChecked(!blackBg);
    }
    if (m_viewModel) {
        if (auto *markVm = m_viewModel->markCalcVm())
            applyMarkCalcResult(markVm->markLocateResultForSlot(slotIndex));
        else
            applyMarkCalcResult({});
    } else
        applyMarkCalcResult({});
}

void RecipeConfigMarkView::refreshMarkTabs()
{
    if (!m_markTabGroup)
        return;

    const TrackMode mode  = SystemConfigRepository::load().common.trackMode;
    const int       count = recipeMarkCount(mode);

    QSignalBlocker blocker(m_markTabGroup);
    m_markTabGroup->setItemKeys(recipeMarkTabKeys(mode));
    if (m_activeMarkSlot >= count)
        m_activeMarkSlot = 0;
    m_markTabGroup->setCurrentIndex(m_activeMarkSlot);
}

void RecipeConfigMarkView::onMarkTabChanged(int newSlotIndex)
{
    if (!m_viewModel)
        return;
    RecipeMarkCalcViewModel *markVm = m_viewModel->markCalcVm();
    if (!markVm || newSlotIndex < 0 || newSlotIndex >= markVm->markSlotCount())
        return;
    if (newSlotIndex == m_activeMarkSlot)
        return;

    m_activeMarkSlot = newSlotIndex;
    m_bindings.beginSuspendUpdates();
    refreshMarkSlotUi(m_viewModel->document(), m_activeMarkSlot);
    m_bindings.endSuspendUpdates();
}

void RecipeConfigMarkView::retranslatePanel()
{
    if (m_markWidgets.calcBtn)
        m_markWidgets.calcBtn->setText(TR("aoi.recipe.mark.calc"));
    if (m_pickTemplateBtn)
        m_pickTemplateBtn->setText(TR("aoi.recipe.mark.pick_template"));
    if (m_newTemplateBtn)
        m_newTemplateBtn->setText(TR("aoi.recipe.mark.new_template"));
    if (m_markWidgets.bgBlack)
        m_markWidgets.bgBlack->setText(TR("aoi.recipe.mark.bg_black"));
    if (m_markWidgets.bgWhite)
        m_markWidgets.bgWhite->setText(TR("aoi.recipe.mark.bg_white"));
    if (m_markWidgets.markType)
        fillMarkTypeCombo(m_markWidgets.markType, markTypeFromCombo(m_markWidgets.markType));
}

void RecipeConfigMarkView::refreshCompactLayout()
{
    refreshMarkCapturePosLayout(this);
}

void RecipeConfigMarkView::notifyUser(const QString &text, const ui::ThemedMessageBoxIcon icon)
{
    if (m_viewModel)
        m_viewModel->setStatus(text);
    ui::ThemedMessageBox::information(window(),
                                  TR("aoi.recipe.mark.template_dialog.title"),
                                  text,
                                  QString(),
                                  icon);
}

void RecipeConfigMarkView::applyMarkCalcResult(const MarkLocateResult &result)
{
    if (m_markWidgets.offsetX) {
        m_markWidgets.offsetX->setText(result.ok ? QString::number(result.pixelX, 'f', 1)
                                                 : TR("aoi.recipe.mark.no_value"));
    }
    if (m_markWidgets.offsetY) {
        m_markWidgets.offsetY->setText(result.ok ? QString::number(result.pixelY, 'f', 1)
                                                 : TR("aoi.recipe.mark.no_value"));
    }
    if (m_markWidgets.calcResultThumb) {
        if (result.ok && result.resultImage) {
            // MarkLocateResult.resultImage 为 RGB24 ImageInfo；缩略图不二次缩放（maxEdge=0）
            const QImage thumb = image_display::imageInfoToQImage(result.resultImage, 0);
            if (!thumb.isNull()) {
                m_markWidgets.calcResultThumb->setPixmap(
                    QPixmap::fromImage(thumb)
                        .scaled(m_markWidgets.calcResultThumb->size(), Qt::KeepAspectRatio,
                                Qt::SmoothTransformation));
            } else {
                m_markWidgets.calcResultThumb->clear();
                m_markWidgets.calcResultThumb->setText(TR("aoi.recipe.mark.result_placeholder"));
            }
        } else {
            m_markWidgets.calcResultThumb->clear();
            m_markWidgets.calcResultThumb->setText(TR("aoi.recipe.mark.result_placeholder"));
        }
    }
}

// ── Mark 模板对话框 ──

void RecipeConfigMarkView::openMarkTemplateDialog()
{
    if (!m_graphicsView) {
        notifyUser(TR("aoi.recipe.mark.no_preview"),
                           ui::ThemedMessageBoxIcon::Warning);
        return;
    }

    QImage preview;
    if (!m_graphicsView->GetImage(m_previewWinId, preview) || preview.isNull()) {
        notifyUser(TR("aoi.recipe.mark.no_preview"),
                           ui::ThemedMessageBoxIcon::Warning);
        return;
    }

    const QString modelDir =
        QCoreApplication::applicationDirPath() + QStringLiteral("/model");
    QDir().mkpath(modelDir);

    QString fileName = m_markWidgets.templateFileLbl
                           ? m_markWidgets.templateFileLbl->text().trimmed()
                           : QString{};
    if (fileName.isEmpty())
        fileName = QStringLiteral("mark_std_t1.model");
    else if (!fileName.endsWith(QStringLiteral(".model"), Qt::CaseInsensitive))
        fileName = QFileInfo(fileName).completeBaseName() + QStringLiteral(".model");

    const QString defaultName =
        m_markWidgets.templateNameLbl
            ? m_markWidgets.templateNameLbl->text().trimmed()
            : TR("aoi.recipe.mark.demo_name");

    const bool wasVisible = m_markTemplateDialog && m_markTemplateDialog->isVisible();

    MarkTemplateDialog::ShowParams params;
    params.defaultPath = modelDir + QLatin1Char('/') + fileName;
    params.defaultName = defaultName;
    params.modelType   = MarkTemplateKind::Mark;

    MarkTemplateDialog *dlg = MarkTemplateDialog::open(this, params);
    if (!dlg)
        return;

    if (!m_markTemplateDialog) {
        m_markTemplateDialog = dlg;
        connect(m_markTemplateDialog, &MarkTemplateDialog::makeRequested, this,
                &RecipeConfigMarkView::onMarkTemplateMakeRequested);
        connect(m_markTemplateDialog, &ui::ThemedPanelDialog::dialogClosed, this,
                &RecipeConfigMarkView::closeMarkTemplateDialog);
    } else {
        m_markTemplateDialog = dlg;
    }

    if (!wasVisible) {
        refreshMarkTemplateRoiHint();
        m_markRoiTimer->start();
    }
}

void RecipeConfigMarkView::closeMarkTemplateDialog()
{
    if (m_markRoiTimer)
        m_markRoiTimer->stop();
    if (m_markTemplateDialog)
        m_markTemplateDialog->hide();
}

void RecipeConfigMarkView::openMarkTemplatePickDialog()
{
    const QString modelDir = ModelGenerator::defaultModelDir();
    QDir().mkpath(modelDir);

    QString fileName = m_markWidgets.templateFileLbl
                           ? m_markWidgets.templateFileLbl->text().trimmed()
                           : QString{};
    if (fileName.isEmpty())
        fileName = QStringLiteral("mark_std_t1.model");
    else if (!fileName.endsWith(QStringLiteral(".model"), Qt::CaseInsensitive))
        fileName = QFileInfo(fileName).completeBaseName() + QStringLiteral(".model");

    const QString fullPath = QFileInfo(fileName).isAbsolute()
                                 ? fileName
                                 : modelDir + QLatin1Char('/') + fileName;
    const QString defaultName =
        m_markWidgets.templateNameLbl
            ? m_markWidgets.templateNameLbl->text().trimmed()
            : QString{};

    MarkTemplatePickDialog::ShowParams params;
    params.defaultPath = fullPath;
    params.defaultName = defaultName;

    MarkTemplatePickDialog *dlg = MarkTemplatePickDialog::open(this, params);
    if (!dlg)
        return;

    if (!m_markTemplatePickDialog) {
        m_markTemplatePickDialog = dlg;
        connect(m_markTemplatePickDialog, &MarkTemplatePickDialog::pickConfirmed, this,
                &RecipeConfigMarkView::onMarkTemplatePickConfirmed);
        connect(m_markTemplatePickDialog, &MarkTemplatePickDialog::deleteRequested, this,
                &RecipeConfigMarkView::onMarkTemplateDeleteRequested);
        connect(m_markTemplatePickDialog, &ui::ThemedPanelDialog::dialogClosed, this,
                &RecipeConfigMarkView::closeMarkTemplatePickDialog);
    } else {
        m_markTemplatePickDialog = dlg;
    }
}

void RecipeConfigMarkView::closeMarkTemplatePickDialog()
{
    if (m_markTemplatePickDialog)
        m_markTemplatePickDialog->hide();
}

bool RecipeConfigMarkView::applyMarkTemplateToForm(const QString &modelPath, const QString &modelName,
                                                   bool showError)
{
    if (modelPath.trimmed().isEmpty() || modelName.trimmed().isEmpty())
        return false;

    if (!ModelGenerator::sdkReady()) {
        if (showError)
            notifyUser(ModelGenerator::sdkIssue(), ui::ThemedMessageBoxIcon::Critical);
        return false;
    }

    std::shared_ptr<common::device::ImageInfo> thumbImage;
    // extractModelImage 输出 RGB24 ImageInfo，此处转 QImage 供控件显示
    const ModelOpResult imgResult =
        ModelGenerator::extractModelImage(modelPath, modelName, thumbImage);
    if (!imgResult.ok) {
        if (showError)
            notifyUser(imgResult.errorKey, ui::ThemedMessageBoxIcon::Critical);
        return false;
    }

    const QString fileName = QFileInfo(modelPath).fileName();
    if (m_markWidgets.templateFileLbl)
        m_markWidgets.templateFileLbl->setText(fileName);
    if (m_markWidgets.templateNameLbl)
        m_markWidgets.templateNameLbl->setText(modelName);

    if (m_markWidgets.thumb) {
        const QImage thumb = image_display::imageInfoToQImage(thumbImage, 0);
        if (!thumb.isNull()) {
            m_markWidgets.thumb->setPixmap(
                QPixmap::fromImage(thumb)
                    .scaled(m_markWidgets.thumb->size(), Qt::KeepAspectRatio,
                            Qt::SmoothTransformation));
        } else {
            m_markWidgets.thumb->clear();
            m_markWidgets.thumb->setText(TR("aoi.recipe.mark.thumb_placeholder"));
        }
    }

    return true;
}

void RecipeConfigMarkView::onMarkTemplatePickConfirmed(const QString &modelPath,
                                                       const QString &modelName)
{
    if (!m_markTemplatePickDialog || !m_viewModel)
        return;

    if (!applyMarkTemplateToForm(modelPath, modelName))
        return;

    notifyUser(TR("aoi.recipe.mark.template_pick_success"));
    closeMarkTemplatePickDialog();
}

void RecipeConfigMarkView::onMarkTemplateDeleteRequested(const QString &modelPath,
                                                           const QString &modelName)
{
    if (!m_markTemplatePickDialog || !m_viewModel)
        return;

    if (!ui::ThemedMessageBox::ask(window(),
                               TR("aoi.recipe.mark.pick_dialog.title"),
                               TR("aoi.recipe.mark.template_delete_confirm").arg(modelName),
                               TR("aoi.recipe.mark.template_confirm"),
                               TR("aoi.recipe.add_dialog.cancel"),
                               ui::ThemedMessageBoxIcon::Warning,
                               false,
                               PushButtonVariant::Danger))
        return;

    if (!ModelGenerator::sdkReady()) {
        notifyUser(ModelGenerator::sdkIssue(), ui::ThemedMessageBoxIcon::Critical);
        return;
    }

    m_markTemplatePickDialog->setBusy(true);
    const ModelOpResult result = ModelGenerator::removeModelName(modelPath, modelName);
    m_markTemplatePickDialog->setBusy(false);

    if (!result.ok) {
        notifyUser(result.errorKey, ui::ThemedMessageBoxIcon::Critical);
        return;
    }

    m_markTemplatePickDialog->refreshModelNames();

    const QString currentFile = m_markWidgets.templateFileLbl
                                    ? m_markWidgets.templateFileLbl->text().trimmed()
                                    : QString{};
    const QString currentName = m_markWidgets.templateNameLbl
                                    ? m_markWidgets.templateNameLbl->text().trimmed()
                                    : QString{};
    if (currentFile == QFileInfo(modelPath).fileName() && currentName == modelName) {
        if (m_markWidgets.templateNameLbl)
            m_markWidgets.templateNameLbl->clear();
        if (m_markWidgets.thumb) {
            m_markWidgets.thumb->clear();
            m_markWidgets.thumb->setText(TR("aoi.recipe.mark.thumb_placeholder"));
        }
    }

    notifyUser(TR("aoi.recipe.mark.template_delete_success"));
}

void RecipeConfigMarkView::refreshMarkTemplateRoiHint()
{
    if (!m_markTemplateDialog || !m_markTemplateDialog->isVisible() || !m_graphicsView)
        return;
    if (m_markTemplateDialog->isMaking())
        return;

    QRect roi;
    const bool valid = m_graphicsView->GetRoiRect(m_previewWinId, roi);
    m_markTemplateDialog->updateRoiHint(roi, valid);
}

void RecipeConfigMarkView::onMarkTemplateMakeRequested()
{
    if (!m_markTemplateDialog || !m_graphicsView || !m_viewModel)
        return;

    if (m_markRoiTimer)
        m_markRoiTimer->stop();

    refreshMarkTemplateRoiHint();

    QImage image;
    if (!m_graphicsView->GetImage(m_previewWinId, image) || image.isNull()) {
        notifyUser(TR("aoi.recipe.mark.no_preview"),
                           ui::ThemedMessageBoxIcon::Warning);
        if (m_markTemplateDialog && m_markTemplateDialog->isVisible() && m_markRoiTimer)
            m_markRoiTimer->start();
        return;
    }

    QRect roi;
    if (!m_graphicsView->GetRoiRect(m_previewWinId, roi)) {
        notifyUser(TR("aoi.recipe.mark.roi_required"),
                           ui::ThemedMessageBoxIcon::Warning);
        if (m_markTemplateDialog && m_markTemplateDialog->isVisible() && m_markRoiTimer)
            m_markRoiTimer->start();
        return;
    }

    const QRect clippedRoi = roi.intersected(image.rect());
    if (clippedRoi.width() < 1 || clippedRoi.height() < 1) {
        notifyUser(TR("aoi.recipe.mark.roi_required"),
                           ui::ThemedMessageBoxIcon::Warning);
        if (m_markTemplateDialog && m_markTemplateDialog->isVisible() && m_markRoiTimer)
            m_markRoiTimer->start();
        return;
    }

    ModelGenerateRequest request;
    // 画布截图为 QImage；进入 ModelGenerator 前转为 RGB24 ImageInfo
    request.sourceImage = image_display::qImageToRgbImageInfo(image);
    request.roi         = clippedRoi;
    request.modelPath   = m_markTemplateDialog->modelPath();
    request.modelName   = m_markTemplateDialog->modelName();
    request.modelKind   = toVisionModelKind(m_markTemplateDialog->modelType());

    if (request.modelPath.isEmpty()) {
        notifyUser(TR("aoi.recipe.mark.path_required"),
                           ui::ThemedMessageBoxIcon::Warning);
        if (m_markTemplateDialog->isVisible() && m_markRoiTimer)
            m_markRoiTimer->start();
        return;
    }
    if (request.modelName.isEmpty()) {
        notifyUser(TR("aoi.recipe.mark.name_required"),
                           ui::ThemedMessageBoxIcon::Warning);
        if (m_markTemplateDialog->isVisible() && m_markRoiTimer)
            m_markRoiTimer->start();
        return;
    }

    if (!ModelGenerator::sdkReady()) {
        notifyUser(ModelGenerator::sdkIssue(), ui::ThemedMessageBoxIcon::Critical);
        if (m_markTemplateDialog->isVisible() && m_markRoiTimer)
            m_markRoiTimer->start();
        return;
    }

    m_markTemplateDialog->setMaking(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    const ModelOpResult result = ModelGenerator::generate(request);

    QApplication::restoreOverrideCursor();
    m_markTemplateDialog->setMaking(false);

    if (!result.ok) {
        notifyUser(result.errorKey, ui::ThemedMessageBoxIcon::Critical);
        if (m_markTemplateDialog->isVisible() && m_markRoiTimer)
            m_markRoiTimer->start();
        return;
    }

    applyMarkTemplateToForm(m_markTemplateDialog->modelPath(), m_markTemplateDialog->modelName(),
                            false);
    notifyUser(TR("aoi.recipe.mark.make_success"));
    if (m_markTemplateDialog->isVisible() && m_markRoiTimer)
        m_markRoiTimer->start();
}

} // namespace aoi
