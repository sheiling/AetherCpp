#pragma once

/**
 * @file recipe_config_panel_calib_view.h
 * @brief 配方配置 · 灯板标定子面板
 */

#include "binding_session.h"
#include "recipe_config/recipe_config_widgets.h"
#include "recipe_config_model.h"

#include <QWidget>

namespace aoi {

class RecipeConfigViewModel;

/** @brief 灯板标定：面板/Gerber 模式、五点、FOV 生成与路径模式 */
class RecipeConfigPanelCalibView : public QWidget
{
    Q_OBJECT

public:
    explicit RecipeConfigPanelCalibView(RecipeConfigViewModel *viewModel, QWidget *parent = nullptr);

    void setupBindings();
    void updateAllSources() { m_bindings.updateAllSources(); }
    void beginSuspendUpdates() { m_bindings.beginSuspendUpdates(); }
    void endSuspendUpdates() { m_bindings.endSuspendUpdates(); }
    void retranslatePanel();
    void refreshFromDocument(const RecipeDocument &doc);

private:
    void buildLayout();

    RecipeConfigViewModel *m_viewModel = nullptr;
    BindingSession         m_bindings;

    RecipeCalibrationWidgets m_calibWidgets;
};

} // namespace aoi
