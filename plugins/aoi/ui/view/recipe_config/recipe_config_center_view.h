#pragma once

/**
 * @file recipe_config_center_view.h
 * @brief 配方配置 · 中间调试区（预览 + 缺陷汇总表 + 轴点动）
 */

#include "recipe_config_model.h"

#include "fov_planner/fov_coordinate_system.h"
#include "fov_planner/recipe_fov_layout_adapter.h"

#include "component/controls/combo/themed_combo_box.h"

#include <QVector>
#include <QWidget>

class IGraphicsVew;
class QLabel;
class QFrame;
class QTimer;

namespace motion_control_plugin {
class IMotionControlService;
}

namespace ui {

class ProgressOverlay;
class ThemedComboBox;
class ThemedIconButton;
class ThemedPushButton;
class ThemedTableWidget;

} // namespace ui

namespace aoi {

class AxisJogPanel;
class PanelNavigatorOverlay;
class RecipeConfigViewModel;

/**
 * @brief 中间调试区：相机预览 IGraphicsView + 工具栏 + 缺陷汇总表 + 轴点动
 *
 * 虚拟调测明细由复判页展示；本区仅显示各缺陷类型数量。
 */
class RecipeConfigCenterView : public QWidget
{
    Q_OBJECT

public:
    static constexpr int kPreviewWinId = 2;

    explicit RecipeConfigCenterView(RecipeConfigViewModel *viewModel, QWidget *parent = nullptr);

    IGraphicsVew *graphicsView() const { return m_graphicsView; }
    int           previewWinId() const { return kPreviewWinId; }
    QString       currentCameraId() const;

    AxisJogPanel *axisJogPanel() const { return m_axisJogPanel; }
    ui::ThemedPushButton *saveRecipeBtn() const { return m_saveRecipeBtn; }
    ui::ThemedIconButton     *toolbarSnapBtn() const { return m_toolbarSnapBtn; }
    ui::ThemedIconButton     *toolbarPreviewBtn() const { return m_toolbarPreviewBtn; }
    ui::ThemedPushButton *readImageBtn() const { return m_readImageBtn; }
    ui::ThemedPushButton *scanDetectBtn() const { return m_scanDetectBtn; }
    ui::ThemedPushButton *inspectBtn() const { return m_inspectBtn; }
    ui::ThemedPushButton *resultDetailBtn() const { return m_detailBtn; }
    ui::ThemedComboBox   *toolbarCamCombo() const { return m_toolbarCamCombo; }
    ui::ThemedComboBox   *roiShapeCombo() const { return m_roiShapeCombo; }
    const QVector<ui::ThemedPushButton *> &toolbarButtons() const { return m_toolbarButtons; }
    const QVector<ui::ThemedPushButton *> &roiToolButtons() const { return m_roiToolButtons; }
    const QVector<QString>                &toolbarKeys() const { return m_toolbarKeys; }
    const QVector<QString>                &roiToolKeys() const { return m_roiToolKeys; }

    ui::ThemedPushButton *panelNavBtn() const { return m_panelNavBtn; }
    PanelNavigatorOverlay *panelNavigator() const { return m_panelNavigator; }

    void loadPreviewImage(const QString &path);
    void applyPreviewImage(QImage image);
    bool currentPreviewImage(QImage &out) const;
    void tryLoadDefaultPreviewImage();
    /** 从虚拟调测 DefectInfoList 刷新缺陷种类汇总表 */
    void refreshDefectTable();
    void syncPreviewToolbarHeights();
    void refreshToolbarIcons();
    void refreshAuxiliaryUi(const RecipeDocument &doc);
    void retranslatePanel();
    void refreshPresentation();
    void refreshPanelNavigator(const RecipeDocument &doc);
    void setVirtualInspectOverlayVisible(bool visible);
    void updateVirtualInspectProgress(int percent, const QString &stageKey);
    /** 产线运行中时禁用检测按钮 */
    void refreshInspectButtonEnabled();

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void buildLayout();
    void wireAxisJogPanel();
    void refreshAxisJogFromCapture();
    void syncAxisJogPositionsFromService();
    void onAxisJogPressed(const QString &axisName, int direction, double speedRatio);
    void onAxisJogReleased(const QString &axisName);
    void onAxisAbsoluteMoveRequested(const QString &axisName, double targetPos);
    void onAxisRelativeMoveRequested(const QString &axisName, double delta);
    void updateAxisJogAutoRefresh();
    void applyRoiShape(int index);
    void wirePanelNavigator();
    void syncPanelNavigatorFov();
    void syncGraphicsPixelScale(const RecipeDocument &doc);
    fov_planner::FovCoordinateSystem::MarkPhysicalAabb captureMarkPhysicalAabb() const;
    void clampCapturePhysicalXY(double &xMm, double &yMm) const;
    bool moveAxisWithMarkClamp(const QString &axisName, bool absolute, double value);

    RecipeConfigViewModel *m_viewModel = nullptr;

    QWidget              *m_previewHostWidget    = nullptr;
    PanelNavigatorOverlay *m_panelNavigator      = nullptr;
    ui::ThemedPushButton *m_panelNavBtn          = nullptr;

    AxisJogPanel             *m_axisJogPanel       = nullptr;
    QFrame                   *m_debugBlock         = nullptr;
    QLabel                   *m_previewLabel       = nullptr;
    QLabel                   *m_roiInfoLabel       = nullptr;
    ui::ThemedTableWidget    *m_defectTable         = nullptr;
    ui::ThemedComboBox       *m_toolbarCamCombo     = nullptr;
    ui::ThemedComboBox       *m_roiShapeCombo       = nullptr;
    ui::ThemedIconButton     *m_toolbarSnapBtn      = nullptr;
    ui::ThemedIconButton     *m_toolbarPreviewBtn   = nullptr;
    ui::ThemedPushButton     *m_readImageBtn        = nullptr;
    ui::ThemedPushButton     *m_scanDetectBtn       = nullptr;
    ui::ThemedPushButton     *m_inspectBtn          = nullptr;
    ui::ThemedPushButton     *m_saveRecipeBtn        = nullptr;
    ui::ThemedPushButton     *m_detailBtn           = nullptr;
    QFrame                   *m_resultBlock         = nullptr;
    QLabel                   *m_resultSectionTitle  = nullptr;

    ui::ProgressOverlay *m_inspectOverlay = nullptr;

    IGraphicsVew *m_graphicsView = nullptr;

    QVector<ui::ThemedPushButton *> m_roiToolButtons;
    QVector<ui::ThemedPushButton *> m_toolbarButtons;
    QVector<QString>                m_roiToolKeys;
    QVector<QString>                m_roiShapeKeys;
    QVector<QString>                m_toolbarKeys;

    QTimer *m_jogFovRefreshTimer = nullptr;
    int     m_activeJogCount       = 0;
};

} // namespace aoi
