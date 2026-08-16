/**
 * @file panel_overview_painter.cpp
 * @brief 灯板示意缩略图绘制
 */

#include "domain/panel_overview_painter.h"

#include "component/theme/theme_tokens.h"
#include "fov_planner/fov_layout_types.h"
#include "uibase/languagemanager.h"

#include <QPainter>
#include <QPen>
#include <QColor>
#include <QtMath>

namespace aoi {

using namespace ui;

namespace {

constexpr double kContentPaddingRatio = 0.92;

QRectF logicalRectToWidget(const PanelOverviewViewMapping &map, const common::geometry::Rect2d &rect)
{
    const QPointF bl = map.logicalToWidget({rect.left(), rect.bottom()});
    const QPointF tr = map.logicalToWidget({rect.right(), rect.top()});
    return QRectF(bl, tr).normalized();
}

} // namespace

QPointF PanelOverviewViewMapping::logicalToWidget(const common::geometry::Vec2d &logical) const
{
    const double px = originPx.x() + (logical.x - logicalBounds.left()) * scalePxPerMm;
    const double py = originPx.y() - (logical.y - logicalBounds.bottom()) * scalePxPerMm;
    return {px, py};
}

std::optional<common::geometry::Vec2d> PanelOverviewViewMapping::widgetToLogical(
    const QPointF &widgetPx) const
{
    if (scalePxPerMm <= 0.0)
        return std::nullopt;

    const double lx = logicalBounds.left() + (widgetPx.x() - originPx.x()) / scalePxPerMm;
    const double ly = logicalBounds.bottom() + (originPx.y() - widgetPx.y()) / scalePxPerMm;
    return common::geometry::Vec2d{lx, ly};
}

PanelOverviewPaintResult PanelOverviewPainter::paint(const fov_planner::FovCoordinateSystem &coords,
                                                     const QSize &targetSize)
{
    PanelOverviewPaintResult result;
    if (targetSize.width() <= 0 || targetSize.height() <= 0)
        return result;

    if (!coords.isValid()) {
        result.invalidReasonKey = QStringLiteral("aoi.recipe.nav.mark_required");
        QPixmap placeholder(targetSize);
        placeholder.fill(QColor(24, 28, 36, 180));
        QPainter p(&placeholder);
        p.setPen(QColor(160, 168, 180));
        p.drawText(placeholder.rect(), Qt::AlignCenter | Qt::TextWordWrap,
                   TR(result.invalidReasonKey));
        result.background = placeholder;
        return result;
    }

    const common::geometry::Rect2d panelRect = coords.panelLogicalBounds();
    const common::geometry::Rect2d viewBounds =
        fov_planner::computeNavigatorViewBounds(coords.config());
    if (panelRect.width <= 0.0 || panelRect.height <= 0.0 || viewBounds.width <= 0.0
        || viewBounds.height <= 0.0) {
        result.invalidReasonKey = QStringLiteral("aoi.recipe.nav.invalid_layout");
        QPixmap placeholder(targetSize);
        placeholder.fill(QColor(24, 28, 36, 180));
        QPainter p(&placeholder);
        p.setPen(QColor(160, 168, 180));
        p.drawText(placeholder.rect(), Qt::AlignCenter | Qt::TextWordWrap,
                   TR(result.invalidReasonKey));
        result.background = placeholder;
        return result;
    }

    const double scaleX =
        (targetSize.width() * kContentPaddingRatio) / qMax(viewBounds.width, 1e-6);
    const double scaleY =
        (targetSize.height() * kContentPaddingRatio) / qMax(viewBounds.height, 1e-6);
    const double scale = qMin(scaleX, scaleY);

    const double contentW = viewBounds.width * scale;
    const double contentH = viewBounds.height * scale;
    const double offsetX  = (targetSize.width() - contentW) * 0.5;
    const double offsetY  = (targetSize.height() - contentH) * 0.5;

    PanelOverviewViewMapping map;
    map.logicalBounds = viewBounds;
    map.scalePxPerMm  = scale;
    map.originPx      = QPointF(offsetX, targetSize.height() - offsetY);
    result.mapping    = map;

    const ThemeTokens theme = ThemeTokens::resolve();

    QPixmap pixmap(targetSize);
    pixmap.fill(QColor(16, 28, 42, 48));

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF panelWidgetRect = logicalRectToWidget(map, panelRect);
    painter.setPen(QPen(themeWithAlpha(theme.accentHover, 220), 1.6));
    painter.setBrush(themeWithAlpha(theme.accent, 72));
    painter.drawRect(panelWidgetRect);

    const auto &cfg = coords.config();
    const auto drawMark = [&](const common::geometry::Vec2d &pos, const QColor &color,
                              const QString &label, bool labelBelowLeft = false) {
        const QPointF pt = map.logicalToWidget(pos);
        painter.setPen(QPen(color, 1.0));
        painter.setBrush(color);
        painter.drawEllipse(pt, 3.0, 3.0);
        painter.setPen(theme.highlightText);
        if (labelBelowLeft) {
            if (pt.x() >= 26.0) {
                painter.drawText(QRectF(pt.x() - 26.0, pt.y() + 5.0, 24.0, 14.0),
                                 Qt::AlignRight | Qt::AlignTop, label);
            } else {
                painter.drawText(QRectF(pt.x() + 5.0, pt.y() + 5.0, 24.0, 14.0),
                                 Qt::AlignLeft | Qt::AlignTop, label);
            }
        } else {
            painter.drawText(QRectF(pt.x() + 4.0, pt.y() - 10.0, 24.0, 14.0), label);
        }
    };
    drawMark(cfg.logicalMarkerBottomLeft, theme.warningText, QStringLiteral("M1"), true);
    drawMark(cfg.logicalMarkerTopRight, theme.accentHover, QStringLiteral("M2"));

    result.background = pixmap;
    result.valid      = true;
    return result;
}

} // namespace aoi
