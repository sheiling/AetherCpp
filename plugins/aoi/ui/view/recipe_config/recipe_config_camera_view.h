#pragma once

/**
 * @file recipe_config_camera_view.h
 * @brief 配方配置 · 相机设置子面板
 */

#include "binding_session.h"

#include "component/controls/slider/themed_slider.h"

#include <QWidget>

class QLabel;
class QSpinBox;
class QDoubleSpinBox;

namespace aoi {

class RecipeConfigViewModel;

/** @brief 相机参数：光源、曝光、图像尺寸、采集精度 */
class RecipeConfigCameraView : public QWidget
{
    Q_OBJECT

public:
    explicit RecipeConfigCameraView(RecipeConfigViewModel *viewModel, QWidget *parent = nullptr);

    void setupBindings();
    void updateAllSources() { m_bindings.updateAllSources(); }
    void beginSuspendUpdates() { m_bindings.beginSuspendUpdates(); }
    void endSuspendUpdates() { m_bindings.endSuspendUpdates(); }
    void retranslatePanel();

private:
    void buildLayout();

    RecipeConfigViewModel *m_viewModel = nullptr;
    BindingSession         m_bindings;

    ui::ThemedSlider   *m_lightSlider         = nullptr;
    QLabel         *m_lightValueLabel     = nullptr;
    QSpinBox       *m_exposureSpin        = nullptr;
    QSpinBox       *m_imageWidthSpin      = nullptr;
    QSpinBox       *m_imageHeightSpin     = nullptr;
    QDoubleSpinBox *m_captureAccuracySpin = nullptr;
};

} // namespace aoi
