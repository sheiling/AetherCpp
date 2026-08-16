#pragma once

/**
 * @file recipe_config_algo_view.h
 * @brief 配方配置 · 算法阈值子面板
 */

#include "binding_session.h"

#include <QVector>
#include <QWidget>

class QDoubleSpinBox;

namespace aoi {

class RecipeConfigViewModel;

/** @brief 算法阈值：各缺陷检测灵敏度 Spin 绑定 document.algo */
class RecipeConfigAlgoView : public QWidget
{
    Q_OBJECT

public:
    explicit RecipeConfigAlgoView(RecipeConfigViewModel *viewModel, QWidget *parent = nullptr);

    void setupBindings();
    void updateAllSources() { m_bindings.updateAllSources(); }
    void beginSuspendUpdates() { m_bindings.beginSuspendUpdates(); }
    void endSuspendUpdates() { m_bindings.endSuspendUpdates(); }
    void retranslatePanel();

private:
    void buildLayout();

    RecipeConfigViewModel *m_viewModel = nullptr;
    BindingSession         m_bindings;

    QVector<QDoubleSpinBox *> m_algoSpins;
};

} // namespace aoi
