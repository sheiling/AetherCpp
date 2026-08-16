#pragma once

/**
 * @file recipe_config_position_calib_view.h
 * @brief 配方配置 · 位置标定子面板
 */

#include "binding_session.h"

#include <QWidget>

#include <functional>

class QDoubleSpinBox;

namespace aoi {

class AxisJogPanel;
class RecipeConfigViewModel;

/** @brief 位置标定：轨道位置、扫描起点（可从轴点动面板写入） */
class RecipeConfigPositionCalibView : public QWidget
{
    Q_OBJECT

public:
    explicit RecipeConfigPositionCalibView(RecipeConfigViewModel *viewModel, QWidget *parent = nullptr);

    void setAxisJogPanel(AxisJogPanel *panel);
    void setCameraIdProvider(std::function<QString()> provider);

    void setupBindings();
    void updateAllSources() { m_bindings.updateAllSources(); }
    void beginSuspendUpdates() { m_bindings.beginSuspendUpdates(); }
    void endSuspendUpdates() { m_bindings.endSuspendUpdates(); }
    void retranslatePanel();
    void refreshCompactLayout();

public slots:
    void rememberScanPositionFromAxis();
    void moveToScanPosition();

private:
    void buildLayout();

    RecipeConfigViewModel *m_viewModel     = nullptr;
    AxisJogPanel          *m_axisJogPanel = nullptr;
    std::function<QString()> m_cameraIdProvider;
    BindingSession         m_bindings;

    QDoubleSpinBox *m_trackPosSpin = nullptr;
    QDoubleSpinBox *m_scanPosXSpin = nullptr;
    QDoubleSpinBox *m_scanPosYSpin = nullptr;
    QDoubleSpinBox *m_scanPosZSpin = nullptr;
};

} // namespace aoi
