#pragma once

/**
 * @file panel_overview_painter.h
 * @brief 由配方 panel 几何参数拟合灯板示意缩略图（逻辑坐标 → QPixmap）
 */

#include "fov_planner/fov_coordinate_system.h"

#include <QPixmap>
#include <QPointF>
#include <QSize>
#include <QString>

#include <optional>

namespace aoi {

struct PanelOverviewViewMapping
{
    common::geometry::Rect2d logicalBounds;
    double                   scalePxPerMm = 1.0;
    QPointF                  originPx;

    QPointF logicalToWidget(const common::geometry::Vec2d &logical) const;
    std::optional<common::geometry::Vec2d> widgetToLogical(const QPointF &widgetPx) const;
};

struct PanelOverviewPaintResult
{
    QPixmap                  background;
    PanelOverviewViewMapping mapping;
    bool                     valid = false;
    QString                  invalidReasonKey;
};

class PanelOverviewPainter
{
public:
    static PanelOverviewPaintResult paint(const fov_planner::FovCoordinateSystem &coords,
                                          const QSize &targetSize);
};

} // namespace aoi
