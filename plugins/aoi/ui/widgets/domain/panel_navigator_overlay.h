#pragma once

/**
 * @file panel_navigator_overlay.h
 * @brief 配方预览区全图导航浮层：示意板 + FOV 框 + 点击跳转（可拖动玻璃浮窗）
 */

#include "data/dataModel/recipe_config_model.h"
#include "domain/panel_overview_painter.h"
#include "fov_planner/fov_coordinate_system.h"

#include <QPoint>
#include <QRectF>
#include <QWidget>

#include <functional>

namespace aoi {

class RecipeConfigViewModel;

class PanelNavigatorOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit PanelNavigatorOverlay(QWidget *hostWidget, RecipeConfigViewModel *viewModel,
                                   std::function<QString()> cameraIdProvider,
                                   QWidget *parent = nullptr);

    void setActive(bool active);
    bool isActive() const { return m_active; }

    void rebuildFromDocument(const RecipeDocument &doc);
    void syncFovFromMotion(const QStringList &axisNames, double xMm, double yMm);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QRect  contentRect() const;
    void   repositionOnHost();
    void   ensurePaintResult();
    void   updateFovRect();
    void   navigateToLogical(const common::geometry::Vec2d &logical);
    bool   isDragZone(const QPoint &pos) const;
    void   clampToHost(QPoint &topLeft) const;

    QWidget                 *m_hostWidget = nullptr;
    RecipeConfigViewModel   *m_viewModel  = nullptr;
    std::function<QString()> m_cameraIdProvider;

    bool                      m_active           = false;
    bool                      m_hasFovCenter     = false;
    bool                      m_userPositioned   = false;
    bool                      m_dragging         = false;
    double                    m_fovCenterXMm     = 0.0;
    double                    m_fovCenterYMm     = 0.0;
    QPoint                    m_customTopLeft;
    QPoint                    m_dragPressGlobal;
    QPoint                    m_dragWidgetOrigin;

    fov_planner::FovCoordinateSystem m_coords;
    PanelOverviewPaintResult        m_paintResult;
    QRectF                          m_fovWidgetRect;
    QPointF                         m_fovCenterInContent;
};

} // namespace aoi
