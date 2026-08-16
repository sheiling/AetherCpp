/**
 * @file recipe_config_algo_view.cpp
 * @brief 配方配置 · 算法阈值子面板实现
 */

#include "recipe_config/recipe_config_algo_view.h"

#include "component/controls/label/themed_form_label.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "recipe_config_view_model.h"
#include "binding_options.h"
#include "databinding.h"

#include "uibase/languagemanager.h"

#include <QGridLayout>
#include <QVector>

namespace aoi {

using namespace ui;

// ── 构造 / 布局 ──

RecipeConfigAlgoView::RecipeConfigAlgoView(RecipeConfigViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    buildLayout();
}

void RecipeConfigAlgoView::buildLayout()
{
    auto *grid = new QGridLayout(this);
    grid->setContentsMargins(8, 2, 8, 8);
    grid->setHorizontalSpacing(8);
    grid->setVerticalSpacing(4);
    grid->setColumnStretch(0, 2);
    grid->setColumnStretch(1, 3);

    struct Field {
        QString key;
        double  value;
        double  min;
        double  max;
        int     decimals;
    };
    const QVector<Field> fields = {
        {QStringLiteral("aoi.recipe.algo.group_conf"), 0.72, 0, 1, 2},
        {QStringLiteral("aoi.recipe.algo.chip_conf"), 0.68, 0, 1, 2},
        {QStringLiteral("aoi.recipe.algo.chip_defect"), 0.70, 0, 1, 2},
        {QStringLiteral("aoi.recipe.algo.pad_defect"), 0.65, 0, 1, 2},
        {QStringLiteral("aoi.recipe.algo.offset_x"), 15.0, 0, 9999, 1},
        {QStringLiteral("aoi.recipe.algo.offset_y"), 15.0, 0, 9999, 1},
        {QStringLiteral("aoi.recipe.algo.angle"), 0.5, 0, 360, 2},
    };

    int row = 0;
    m_algoSpins.clear();
    for (const auto &f : fields) {
        grid->addWidget(makeFormLabel(f.key, this), row, 0);
        auto *sp = makeDoubleSpinBox(this);
        sp->setRange(f.min, f.max);
        sp->setDecimals(f.decimals);
        sp->setSingleStep(0.1);
        sp->setValue(f.value);
        m_algoSpins.append(sp);
        grid->addWidget(sp, row, 1);
        ++row;
    }
}

// ── 数据绑定 ──

void RecipeConfigAlgoView::setupBindings()
{
    if (!m_viewModel || m_algoSpins.size() < 7)
        return;

    using VM = RecipeConfigViewModel;
    ViewModel *const vm   = m_viewModel;
    const QString    prop = VM::PROP_DOCUMENT;

    const BindingOptions onChanged;

    const auto bindAlgo = [&](int idx, auto get, auto set) {
        DataBinding::bindStructField<RecipeDocument>(
            vm, prop, get, set, m_algoSpins[idx], onChanged, &m_bindings);
    };

    bindAlgo(0, [](const RecipeDocument &d) { return d.algo.groupConfidence; },
             [](RecipeDocument &d, double v) { d.algo.groupConfidence = v; });
    bindAlgo(1, [](const RecipeDocument &d) { return d.algo.chipConfidence; },
             [](RecipeDocument &d, double v) { d.algo.chipConfidence = v; });
    bindAlgo(2, [](const RecipeDocument &d) { return d.algo.chipDefectClass; },
             [](RecipeDocument &d, double v) { d.algo.chipDefectClass = v; });
    bindAlgo(3, [](const RecipeDocument &d) { return d.algo.padDefectClass; },
             [](RecipeDocument &d, double v) { d.algo.padDefectClass = v; });
    bindAlgo(4, [](const RecipeDocument &d) { return d.algo.offsetXUm; },
             [](RecipeDocument &d, double v) { d.algo.offsetXUm = v; });
    bindAlgo(5, [](const RecipeDocument &d) { return d.algo.offsetYUm; },
             [](RecipeDocument &d, double v) { d.algo.offsetYUm = v; });
    bindAlgo(6, [](const RecipeDocument &d) { return d.algo.angleOffsetDeg; },
             [](RecipeDocument &d, double v) { d.algo.angleOffsetDeg = v; });
}

void RecipeConfigAlgoView::retranslatePanel()
{
    ui::ThemedFormLabel::retranslateIn(this);
}

} // namespace aoi
