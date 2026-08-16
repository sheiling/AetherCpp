/**
 * @file recipe_config_position_calib_view.cpp
 * @brief 配方配置 · 位置标定子面板实现
 */

#include "recipe_config/recipe_config_position_calib_view.h"

#include "component/controls/label/themed_form_label.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "component/controls/button/themed_icon_button.h"
#include "component/font/fa_icon_button.h"
#include "domain/axis_jog_panel.h"
#include "recipe_config_view_model.h"
#include "component/table/themed_table_widget.h"
#include "component/theme/theme_font.h"
#include "binding_options.h"
#include "databinding.h"

#include "uibase/languagemanager.h"

#include <imotion_control_service.h>
#include "extensionsystem/pluginmanager.h"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <optional>

namespace aoi {

using namespace ui;

namespace {

constexpr double kRecipeAxisPositionMax = 999999.0;

void applyRecipeAxisPositionRange(QDoubleSpinBox *sp)
{
    if (!sp)
        return;
    sp->setRange(-kRecipeAxisPositionMax, kRecipeAxisPositionMax);
}

void tuneRecipeValueSpin(QDoubleSpinBox *sp, int minHeight = 28)
{
    if (!sp)
        return;
    QFont spinFont = themeUiFont(9);
    spinFont.setPixelSize(11);
    sp->setFont(spinFont);
    sp->ensurePolished();
    sp->setFixedHeight(qMax(minHeight, sp->sizeHint().height()));
    sp->setMinimumWidth(0);
    sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

int tuneOtherScanPosItemsIn(QWidget *scanCell)
{
    if (!scanCell)
        return 0;

    QFont labelFont = themeUiFont(9);
    labelFont.setPixelSize(10);
    QFont spinFont = themeUiFont(9);
    spinFont.setPixelSize(10);

    int maxItemH = 0;
    for (auto *item : scanCell->findChildren<QWidget *>(QStringLiteral("recipeOtherScanPosItem"))) {
        auto *lbl = item->findChild<ui::ThemedFormLabel *>();
        auto *sp  = item->findChild<QDoubleSpinBox *>(QStringLiteral("recipeOtherScanSpin"));
        if (!sp)
            continue;

        if (lbl)
            lbl->setFont(labelFont);
        sp->setFont(spinFont);
        sp->ensurePolished();

        const int spinH = qMax(24, sp->sizeHint().height());
        sp->setFixedHeight(spinH);
        sp->setMinimumWidth(108);
        sp->setMaximumWidth(QWIDGETSIZE_MAX);
        sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        const int itemH = qMax(spinH, lbl ? lbl->fontMetrics().height() : 0);
        item->setMinimumWidth(108);
        item->setMinimumHeight(itemH);
        item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        maxItemH = qMax(maxItemH, itemH);
    }
    if (auto *xyzWrap = scanCell->findChild<QWidget *>(QStringLiteral("recipeOtherScanXyzWrap")))
        return xyzWrap->sizeHint().height();
    return maxItemH;
}

void resizeOtherTableToContents(QTableWidget *table)
{
    if (auto *themed = qobject_cast<ui::ThemedTableWidget *>(table)) {
        themed->resizeHeightToContents();
        return;
    }

    if (!table)
        return;

    int totalH = table->horizontalHeader()->height();
    for (int row = 0; row < table->rowCount(); ++row)
        totalH += table->rowHeight(row);
    table->setFixedHeight(totalH + table->frameWidth() * 2);
}

QWidget *makeOtherScanPosItem(const QString &axisLabelKey, double value, QWidget *parent,
                              QDoubleSpinBox **outSpin = nullptr)
{
    auto *item = new QWidget(parent);
    item->setObjectName(QStringLiteral("recipeOtherScanPosItem"));
    item->setMinimumWidth(108);
    item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *hl   = new QHBoxLayout(item);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(4);
    auto *lbl = makeFormLabel(axisLabelKey, item);
    lbl->setWordWrap(false);
    lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    auto *sp = makeDoubleSpinBox(item, QStringLiteral("recipeOtherScanSpin"));
    sp->setDecimals(3);
    applyRecipeAxisPositionRange(sp);
    sp->setValue(value);
    sp->setMinimumWidth(108);
    sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hl->addWidget(lbl, 0);
    hl->addWidget(sp, 1);
    if (outSpin)
        *outSpin = sp;
    return item;
}

} // namespace

// ── 构造 / 轴点动联动 ──

RecipeConfigPositionCalibView::RecipeConfigPositionCalibView(RecipeConfigViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    buildLayout();
}

void RecipeConfigPositionCalibView::setAxisJogPanel(AxisJogPanel *panel)
{
    m_axisJogPanel = panel;
}

void RecipeConfigPositionCalibView::setCameraIdProvider(std::function<QString()> provider)
{
    m_cameraIdProvider = std::move(provider);
}

void RecipeConfigPositionCalibView::buildLayout()
{
    setObjectName(QStringLiteral("recipeOtherPanel"));
    setMinimumWidth(0);

    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(4, 2, 4, 8);
    vl->setSpacing(4);

    const auto makeOtherIconBtn = [](QWidget *cell) {
        return ui::makeFaIconButton(ui::FaIcon::Crosshairs, cell,
                                            QStringLiteral("aoi.recipe.mark.get_pos"));
    };
    const auto makeOtherMoveBtn = [](QWidget *cell) {
        return ui::makeFaIconButton(ui::FaIcon::LocationArrow, cell,
                                            QStringLiteral("aoi.recipe.position.move_to_set_pos"));
    };
    const auto makeOtherValueCell = [&](QDoubleSpinBox *sp, QWidget *panel) {
        auto *cell = new QWidget(panel);
        auto *hl   = new QHBoxLayout(cell);
        hl->setContentsMargins(0, 2, 0, 4);
        hl->setSpacing(4);
        sp->setObjectName(QStringLiteral("recipeOtherSpin"));
        sp->setDecimals(3);
        applyRecipeAxisPositionRange(sp);
        hl->addWidget(sp, 1);
        hl->addWidget(makeOtherIconBtn(cell), 0, Qt::AlignVCenter);
        hl->addWidget(makeOtherMoveBtn(cell), 0, Qt::AlignVCenter);
        return cell;
    };

    auto *table = makeTableWidget(2, 2, this);
    table->setObjectName(QStringLiteral("recipeOtherTable"));
    table->applyCompactFormDefaults();
    table->setHorizontalStretchFactors({1, 3});
    table->setHorizontalHeaderLabels(
        {TR("aoi.recipe.position.col_item"), TR("aoi.recipe.position.col_value")});

    table->setItem(0, 0, new QTableWidgetItem(TR("aoi.recipe.position.track_position")));
    m_trackPosSpin = makeDoubleSpinBox(this);
    table->setCellWidget(0, 1, makeOtherValueCell(m_trackPosSpin, this));

    table->setItem(1, 0, new QTableWidgetItem(TR("aoi.recipe.position.scan_position")));
    auto *scanCell = new QWidget(this);
    scanCell->setObjectName(QStringLiteral("recipeOtherScanCell"));
    scanCell->setMinimumWidth(0);
    scanCell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *scanHl   = new QHBoxLayout(scanCell);
    scanHl->setContentsMargins(0, 2, 0, 4);
    scanHl->setSpacing(4);

    auto *xyzWrap = new QWidget(scanCell);
    xyzWrap->setObjectName(QStringLiteral("recipeOtherScanXyzWrap"));
    xyzWrap->setMinimumWidth(108);
    xyzWrap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *xyzVl   = new QVBoxLayout(xyzWrap);
    xyzVl->setContentsMargins(0, 0, 0, 0);
    xyzVl->setSpacing(4);
    xyzVl->addWidget(
        makeOtherScanPosItem(QStringLiteral("aoi.recipe.axis.x"), 0.000, xyzWrap, &m_scanPosXSpin));
    xyzVl->addWidget(
        makeOtherScanPosItem(QStringLiteral("aoi.recipe.axis.y"), 0.000, xyzWrap, &m_scanPosYSpin));
    xyzVl->addWidget(
        makeOtherScanPosItem(QStringLiteral("aoi.recipe.axis.z"), 0.000, xyzWrap, &m_scanPosZSpin));

    scanHl->addWidget(xyzWrap, 1);
    auto *scanGetPosBtn = makeOtherIconBtn(scanCell);
    scanHl->addWidget(scanGetPosBtn, 0, Qt::AlignVCenter);
    auto *scanMoveBtn = makeOtherMoveBtn(scanCell);
    scanHl->addWidget(scanMoveBtn, 0, Qt::AlignVCenter);
    connect(scanGetPosBtn, &ThemedIconButton::clicked, this,
            &RecipeConfigPositionCalibView::rememberScanPositionFromAxis);
    connect(scanMoveBtn, &ThemedIconButton::clicked, this,
            &RecipeConfigPositionCalibView::moveToScanPosition);
    table->setCellWidget(1, 1, scanCell);

    vl->addWidget(table);
}

// ── 数据绑定 ──

void RecipeConfigPositionCalibView::setupBindings()
{
    if (!m_viewModel)
        return;

    using VM = RecipeConfigViewModel;
    ViewModel *const vm   = m_viewModel;
    const QString    prop = VM::PROP_DOCUMENT;

    const BindingOptions onChanged;

    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.positionCalib.trackPosition; },
        [](RecipeDocument &d, double v) { d.positionCalib.trackPosition = v; }, m_trackPosSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.positionCalib.scanPosition.x; },
        [](RecipeDocument &d, double v) { d.positionCalib.scanPosition.x = v; }, m_scanPosXSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.positionCalib.scanPosition.y; },
        [](RecipeDocument &d, double v) { d.positionCalib.scanPosition.y = v; }, m_scanPosYSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.positionCalib.scanPosition.z; },
        [](RecipeDocument &d, double v) { d.positionCalib.scanPosition.z = v; }, m_scanPosZSpin, onChanged,
        &m_bindings);
}

void RecipeConfigPositionCalibView::rememberScanPositionFromAxis()
{
    QStringList axisNames;
    if (m_axisJogPanel)
        axisNames = m_axisJogPanel->axisNames();

    const auto axisFeedback = [&](int index) -> std::optional<double> {
        if (index < 0 || index >= axisNames.size())
            return std::nullopt;
        if (auto *motionSvc =
                ExtensionSystem::PluginManager::getInterface<
                    motion_control_plugin::IMotionControlService>()) {
            return motionSvc->axisState(axisNames.at(index)).fbPos;
        }
        if (m_axisJogPanel)
            return m_axisJogPanel->feedbackPosition(axisNames.at(index));
        return std::nullopt;
    };

    if (const auto v = axisFeedback(0); v && m_scanPosXSpin)
        m_scanPosXSpin->setValue(*v);
    if (const auto v = axisFeedback(1); v && m_scanPosYSpin)
        m_scanPosYSpin->setValue(*v);
    if (const auto v = axisFeedback(2); v && m_scanPosZSpin)
        m_scanPosZSpin->setValue(*v);

    m_bindings.updateAllSources();
    if (m_viewModel)
        m_viewModel->setStatus(TR("aoi.recipe.scan.pos_remembered"));
}

void RecipeConfigPositionCalibView::moveToScanPosition()
{
    if (!m_viewModel || !m_scanPosXSpin || !m_scanPosYSpin || !m_scanPosZSpin)
        return;

    const QString cameraId =
        m_cameraIdProvider ? m_cameraIdProvider() : QStringLiteral("main");
    m_viewModel->moveCaptureToPhysicalXYZ(cameraId, m_scanPosXSpin->value(),
                                          m_scanPosYSpin->value(), m_scanPosZSpin->value());
}

void RecipeConfigPositionCalibView::refreshCompactLayout()
{
    auto *table = findChild<ui::ThemedTableWidget *>(QStringLiteral("recipeOtherTable"));
    if (!table)
        return;

    for (auto *sp : findChildren<QDoubleSpinBox *>(QStringLiteral("recipeOtherSpin")))
        tuneRecipeValueSpin(sp);

    int scanItemH = 0;
    if (auto *scanCell = findChild<QWidget *>(QStringLiteral("recipeOtherScanCell")))
        scanItemH = tuneOtherScanPosItemsIn(scanCell);

    for (int row = 0; row < table->rowCount(); ++row) {
        auto *cell = table->cellWidget(row, 1);
        if (!cell)
            continue;
        cell->ensurePolished();
        const int minRowH = row == 1 ? qMax(96, scanItemH + 8) : 36;
        table->setRowHeight(row, qMax(minRowH, cell->sizeHint().height() + 6));
    }

    resizeOtherTableToContents(table);
}

void RecipeConfigPositionCalibView::retranslatePanel()
{
    ui::ThemedFormLabel::retranslateIn(this);
    if (auto *table = findChild<ui::ThemedTableWidget *>(QStringLiteral("recipeOtherTable"))) {
        table->setHorizontalHeaderLabels(
            {TR("aoi.recipe.position.col_item"), TR("aoi.recipe.position.col_value")});
        if (table->item(0, 0))
            table->item(0, 0)->setText(TR("aoi.recipe.position.track_position"));
        if (table->item(1, 0))
            table->item(1, 0)->setText(TR("aoi.recipe.position.scan_position"));
    }
}

} // namespace aoi
