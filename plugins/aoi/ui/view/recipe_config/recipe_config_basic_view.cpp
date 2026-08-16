/**
 * @file recipe_config_basic_view.cpp
 * @brief 配方配置 · 产品基本信息子面板实现
 */

#include "recipe_config/recipe_config_basic_view.h"

#include "component/controls/label/themed_form_label.h"
#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "recipe_config_view_model.h"
#include "component/font/fa_icon_button.h"
#include "binding_handle.h"
#include "binding_internal.h"
#include "binding_options.h"
#include "databinding.h"

#include "uibase/languagemanager.h"

#include <QPushButton>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QSpinBox>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

QPushButton *makeWhSyncButton(QWidget *parent)
{
    auto *btn = ui::makeFaIconButton(ui::FaIcon::ExpandArrows, parent,
                                             QStringLiteral("aoi.recipe.basic.fill_size"));
    btn->setObjectName(QStringLiteral("recipeToolIconBtn"));
    btn->setFixedSize(26, 26);
    return btn;
}

} // namespace

void RecipeConfigBasicView::addPointRow(QVBoxLayout *vl, const QString &labelKey,
                                        QDoubleSpinBox *&xSpin, QDoubleSpinBox *&ySpin)
{
    vl->addWidget(makeFormLabel(labelKey, this));
    auto *row = new QWidget(this);
    auto *hl  = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(6);
    xSpin = makeDoubleSpinBox(row, QStringLiteral("recipeCamSpin"));
    xSpin->setRange(-9999.0, 9999.0);
    xSpin->setDecimals(3);
    ySpin = makeDoubleSpinBox(row, QStringLiteral("recipeCamSpin"));
    ySpin->setRange(-9999.0, 9999.0);
    ySpin->setDecimals(3);
    hl->addWidget(xSpin, 1);
    hl->addWidget(ySpin, 1);
    vl->addWidget(row);
}

RecipeConfigBasicView::RecipeConfigBasicView(RecipeConfigViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    buildLayout();
}

void RecipeConfigBasicView::buildLayout()
{
    setObjectName(QStringLiteral("recipeBasicForm"));
    setAttribute(Qt::WA_StyledBackground, true);

    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(8, 2, 8, 8);
    vl->setSpacing(4);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.basic.name"), this));
    m_nameLabel = new QLabel(this);
    m_nameLabel->setObjectName(QStringLiteral("recipeMarkReadonly"));
    m_nameLabel->setMinimumWidth(0);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    vl->addWidget(m_nameLabel);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.basic.group_grid"), this));
    auto *groupGridRow = new QWidget(this);
    auto *groupGridHl  = new QHBoxLayout(groupGridRow);
    groupGridHl->setContentsMargins(0, 0, 0, 0);
    groupGridHl->setSpacing(6);
    m_widthSpin = makeSpinBox(groupGridRow, QStringLiteral("recipeCamSpin"));
    m_widthSpin->setRange(1, 99999);
    m_heightSpin = makeSpinBox(groupGridRow, QStringLiteral("recipeCamSpin"));
    m_heightSpin->setRange(1, 99999);
    groupGridHl->addWidget(m_widthSpin, 1);
    groupGridHl->addWidget(m_heightSpin, 1);
    vl->addWidget(groupGridRow);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.basic.group_pitch"), this));
    auto *pitchXYRow = new QWidget(this);
    auto *pitchXYHl  = new QHBoxLayout(pitchXYRow);
    pitchXYHl->setContentsMargins(0, 0, 0, 0);
    pitchXYHl->setSpacing(6);
    m_groupPitchXSpin = makeDoubleSpinBox(pitchXYRow, QStringLiteral("recipeCamSpin"));
    m_groupPitchXSpin->setRange(0.0, 9999.0);
    m_groupPitchXSpin->setDecimals(3);
    m_groupPitchYSpin = makeDoubleSpinBox(pitchXYRow, QStringLiteral("recipeCamSpin"));
    m_groupPitchYSpin->setRange(0.0, 9999.0);
    m_groupPitchYSpin->setDecimals(3);
    pitchXYHl->addWidget(m_groupPitchXSpin, 1);
    pitchXYHl->addWidget(m_groupPitchYSpin, 1);
    vl->addWidget(pitchXYRow);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.basic.chip_pitch_in_group"), this));
    m_chipPitchSpin = makeDoubleSpinBox(this, QStringLiteral("recipeCamSpin"));
    m_chipPitchSpin->setRange(0.0, 9999.0);
    m_chipPitchSpin->setDecimals(3);
    vl->addWidget(m_chipPitchSpin);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.basic.chip_size"), this));
    auto *chipRow = new QWidget(this);
    auto *chipHl  = new QHBoxLayout(chipRow);
    chipHl->setContentsMargins(0, 0, 0, 0);
    chipHl->setSpacing(6);
    m_chipWSpin = makeDoubleSpinBox(chipRow, QStringLiteral("recipeCamSpin"));
    m_chipWSpin->setRange(0.0, 9999.0);
    m_chipWSpin->setDecimals(3);
    m_chipHSpin = makeDoubleSpinBox(chipRow, QStringLiteral("recipeCamSpin"));
    m_chipHSpin->setRange(0.0, 9999.0);
    m_chipHSpin->setDecimals(3);
    chipHl->addWidget(m_chipWSpin, 1);
    chipHl->addWidget(m_chipHSpin, 1);
    auto *chipFill = makeWhSyncButton(chipRow);
    connect(chipFill, &QPushButton::clicked, this, [this]() {
        if (m_chipWSpin && m_chipHSpin)
            m_chipHSpin->setValue(m_chipWSpin->value());
    });
    chipHl->addWidget(chipFill, 0, Qt::AlignVCenter);
    vl->addWidget(chipRow);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.basic.pad_size"), this));
    auto *padRow = new QWidget(this);
    auto *padHl  = new QHBoxLayout(padRow);
    padHl->setContentsMargins(0, 0, 0, 0);
    padHl->setSpacing(6);
    m_padWSpin = makeDoubleSpinBox(padRow, QStringLiteral("recipeCamSpin"));
    m_padWSpin->setRange(0.0, 9999.0);
    m_padWSpin->setDecimals(3);
    m_padHSpin = makeDoubleSpinBox(padRow, QStringLiteral("recipeCamSpin"));
    m_padHSpin->setRange(0.0, 9999.0);
    m_padHSpin->setDecimals(3);
    padHl->addWidget(m_padWSpin, 1);
    padHl->addWidget(m_padHSpin, 1);
    auto *padFill = makeWhSyncButton(padRow);
    connect(padFill, &QPushButton::clicked, this, [this]() {
        if (m_padWSpin && m_padHSpin)
            m_padHSpin->setValue(m_padWSpin->value());
    });
    padHl->addWidget(padFill, 0, Qt::AlignVCenter);
    vl->addWidget(padRow);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.basic.group_size"), this));
    auto *groupSizeRow = new QWidget(this);
    auto *groupSizeHl  = new QHBoxLayout(groupSizeRow);
    groupSizeHl->setContentsMargins(0, 0, 0, 0);
    groupSizeHl->setSpacing(6);
    m_groupWSpin = makeDoubleSpinBox(groupSizeRow, QStringLiteral("recipeCamSpin"));
    m_groupWSpin->setRange(0.0, 9999.0);
    m_groupWSpin->setDecimals(3);
    m_groupHSpin = makeDoubleSpinBox(groupSizeRow, QStringLiteral("recipeCamSpin"));
    m_groupHSpin->setRange(0.0, 9999.0);
    m_groupHSpin->setDecimals(3);
    groupSizeHl->addWidget(m_groupWSpin, 1);
    groupSizeHl->addWidget(m_groupHSpin, 1);
    vl->addWidget(groupSizeRow);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.basic.chip_color_order"), this));
    m_colorOrderCombo = makeComboBox(this);
    m_colorOrderCombo->addItems({TR("aoi.recipe.basic.color_rgb"), TR("aoi.recipe.basic.color_bgr")});
    vl->addWidget(m_colorOrderCombo);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.basic.chip_arrangement"), this));
    m_arrangementCombo = makeComboBox(this);
    m_arrangementCombo->addItems(
        {TR("aoi.recipe.basic.arrange_horizontal"), TR("aoi.recipe.basic.arrange_vertical")});
    vl->addWidget(m_arrangementCombo);

    addPointRow(vl, QStringLiteral("aoi.recipe.basic.offset_group_mark_bl"), m_offsetBlX,
                m_offsetBlY);
    addPointRow(vl, QStringLiteral("aoi.recipe.basic.offset_group_mark_tr"), m_offsetTrX,
                m_offsetTrY);
}

void RecipeConfigBasicView::setupBindings()
{
    if (!m_viewModel)
        return;

    using VM = RecipeConfigViewModel;
    ViewModel *const vm   = m_viewModel;
    const QString    prop = VM::PROP_DOCUMENT;

    const BindingOptions onChanged;
    BindingOptions       onLostFocus;
    onLostFocus.trigger = UpdateSourceTrigger::OnLostFocus;

    if (m_nameLabel) {
        BindingInternal::connectStructVmToView(
            vm, prop, m_nameLabel,
            [this](const QVariant &val) {
                m_nameLabel->setText(val.value<RecipeDocument>().basic.name);
            },
            &m_bindings);
    }

    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.width; },
        [](RecipeDocument &d, int v) { d.basic.width = v; }, m_widthSpin, onChanged, &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.height; },
        [](RecipeDocument &d, int v) { d.basic.height = v; }, m_heightSpin, onChanged, &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.groupPitchXmm; },
        [](RecipeDocument &d, double v) { d.basic.groupPitchXmm = v; }, m_groupPitchXSpin,
        onChanged, &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.groupPitchYmm; },
        [](RecipeDocument &d, double v) { d.basic.groupPitchYmm = v; }, m_groupPitchYSpin,
        onChanged, &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.chipPitchInGroupMm; },
        [](RecipeDocument &d, double v) { d.basic.chipPitchInGroupMm = v; }, m_chipPitchSpin,
        onChanged, &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.chipWidthMm; },
        [](RecipeDocument &d, double v) { d.basic.chipWidthMm = v; }, m_chipWSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.chipHeightMm; },
        [](RecipeDocument &d, double v) { d.basic.chipHeightMm = v; }, m_chipHSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.padWidthMm; },
        [](RecipeDocument &d, double v) { d.basic.padWidthMm = v; }, m_padWSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.padHeightMm; },
        [](RecipeDocument &d, double v) { d.basic.padHeightMm = v; }, m_padHSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.groupWidthMm; },
        [](RecipeDocument &d, double v) { d.basic.groupWidthMm = v; }, m_groupWSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.groupHeightMm; },
        [](RecipeDocument &d, double v) { d.basic.groupHeightMm = v; }, m_groupHSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructIndexField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.chipColorOrder; },
        [](RecipeDocument &d, int v) { d.basic.chipColorOrder = v; }, m_colorOrderCombo, onChanged,
        &m_bindings);
    DataBinding::bindStructIndexField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.basic.chipArrangement; },
        [](RecipeDocument &d, int v) { d.basic.chipArrangement = v; }, m_arrangementCombo,
        onChanged, &m_bindings);

    auto bindPoint = [&](auto getter, auto setter, QDoubleSpinBox *spin) {
        DataBinding::bindStructField<RecipeDocument>(
            vm, prop, getter, setter, spin, onChanged, &m_bindings);
    };
    bindPoint([](const RecipeDocument &d) { return d.basic.offsetGroupMarkBl.x; },
              [](RecipeDocument &d, double v) { d.basic.offsetGroupMarkBl.x = v; }, m_offsetBlX);
    bindPoint([](const RecipeDocument &d) { return d.basic.offsetGroupMarkBl.y; },
              [](RecipeDocument &d, double v) { d.basic.offsetGroupMarkBl.y = v; }, m_offsetBlY);
    bindPoint([](const RecipeDocument &d) { return d.basic.offsetGroupMarkTr.x; },
              [](RecipeDocument &d, double v) { d.basic.offsetGroupMarkTr.x = v; }, m_offsetTrX);
    bindPoint([](const RecipeDocument &d) { return d.basic.offsetGroupMarkTr.y; },
              [](RecipeDocument &d, double v) { d.basic.offsetGroupMarkTr.y = v; }, m_offsetTrY);
}

void RecipeConfigBasicView::retranslatePanel()
{
    ui::ThemedFormLabel::retranslateIn(this);
    if (m_colorOrderCombo) {
        const int idx = m_colorOrderCombo->currentIndex();
        m_colorOrderCombo->blockSignals(true);
        m_colorOrderCombo->clear();
        m_colorOrderCombo->addItems(
            {TR("aoi.recipe.basic.color_rgb"), TR("aoi.recipe.basic.color_bgr")});
        m_colorOrderCombo->setCurrentIndex(idx);
        m_colorOrderCombo->blockSignals(false);
    }
    if (m_arrangementCombo) {
        const int idx = m_arrangementCombo->currentIndex();
        m_arrangementCombo->blockSignals(true);
        m_arrangementCombo->clear();
        m_arrangementCombo->addItems({TR("aoi.recipe.basic.arrange_horizontal"),
                                      TR("aoi.recipe.basic.arrange_vertical")});
        m_arrangementCombo->setCurrentIndex(idx);
        m_arrangementCombo->blockSignals(false);
    }
}

} // namespace aoi
