/**
 * @file defect_map_canvas.cpp
 */

#include "review/defect_map_canvas.h"

#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"
#include "uibase/languagemanager.h"

#include <QMouseEvent>
#include <QPainter>
#include <QSet>
#include <QToolTip>
#include <QWheelEvent>
#include <QtMath>

#include <cmath>

namespace aoi::review {
namespace {

bool recordIsRejudgeOk(const DefectReviewLampRecord &rec)
{
    return rec.isRejudge && rec.rejudgeDefectType == 0;
}

bool recordShowsOkMarker(const DefectReviewLampRecord &rec,
                         inspection::InspectionDefectTypeMask visibleTypeMask)
{
    const auto mask = (rec.defectType | rec.rejudgeDefectType) & visibleTypeMask;
    if (mask != 0)
        return false;
    if (rec.isRejudge)
        return rec.rejudgeDefectType == 0;
    return rec.defectType == 0;
}

} // namespace

DefectMapCanvas::DefectMapCanvas(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("defectMapCanvas"));
    setMinimumHeight(200);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setMouseTracking(true);
    setContextMenuPolicy(Qt::NoContextMenu);

    ui::bindThemeRefresh(this, [this]() { update(); });
}

QColor DefectMapCanvas::mapBackgroundColor() const
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    return t.inputBackground.isValid() ? t.inputBackground : t.surface;
}

QColor DefectMapCanvas::mapGridLineColor() const
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    QColor line             = t.divider.isValid() ? t.divider : t.surfaceBorder;
    if (!line.isValid())
        line = QColor(0x94, 0xa3, 0xb8);
    // 暗色主题下略提亮网格线，保证可见
    const int gray = (line.red() + line.green() + line.blue()) / 3;
    if (gray < 96)
        line = QColor(qMin(255, line.red() + 48), qMin(255, line.green() + 52), qMin(255, line.blue() + 58));
    line.setAlpha(200);
    return line;
}

void DefectMapCanvas::applyState(const DefectMapCanvasState &state)
{
    m_state = state;
    if (m_fitPending)
        fitView();
    update();
}

void DefectMapCanvas::applyZoomAtViewportCenter(double factor)
{
    const QRectF content = mapContentRect();
    if (content.isEmpty() || width() <= 0 || height() <= 0)
        return;

    const double oldZoom = m_zoom;
    const double newZoom = qBound(0.2, oldZoom * factor, 40.0);
    if (qFuzzyCompare(newZoom, oldZoom))
        return;

    // 以当前视口中心下的地图点为锚点缩放，拖动后仍围绕可见区域中心缩放
    const QPointF viewCenter(width() * 0.5, height() * 0.5);
    const QPointF mapAnchor = widgetToMap(viewCenter);

    m_zoom       = newZoom;
    m_fitPending = false;

    const double baseScale = qMin(width() / content.width(), height() / content.height());
    const double scale     = baseScale * m_zoom;
    const QPointF contentCenter(content.width() * 0.5, content.height() * 0.5);
    const QPointF panOffset(-scale * (mapAnchor.x() - contentCenter.x()),
                            -scale * (mapAnchor.y() - contentCenter.y()));

    m_pan.setX(panOffset.x() / width() + 0.5);
    m_pan.setY(panOffset.y() / height() + 0.5);

    clampPan();
    update();
}

void DefectMapCanvas::zoomIn()
{
    applyZoomAtViewportCenter(1.25);
}

void DefectMapCanvas::zoomOut()
{
    applyZoomAtViewportCenter(1.0 / 1.25);
}

void DefectMapCanvas::fitView()
{
    m_zoom       = 1.0;
    m_pan        = QPointF(0.5, 0.5);
    m_fitPending = false;
    clampPan();
    update();
}

void DefectMapCanvas::resetOneToOne()
{
    const QRectF content = mapContentRect();
    if (content.isEmpty())
        return;
    const double sx = width() / content.width();
    const double sy = height() / content.height();
    m_zoom          = qMin(sx, sy);
    m_pan           = QPointF(0.5, 0.5);
    m_fitPending    = false;
    clampPan();
    update();
}

QRectF DefectMapCanvas::mapContentRect() const
{
    if (!m_state.gridValid)
        return {};
    return QRectF(0, 0, m_state.dieGrid.colCount, m_state.dieGrid.rowCount);
}

QRectF DefectMapCanvas::visibleMapRect() const
{
    const QRectF content = mapContentRect();
    if (content.isEmpty() || width() <= 0 || height() <= 0)
        return {};

    const QPointF tl = widgetToMap(QPointF(0, 0));
    const QPointF br = widgetToMap(QPointF(width(), height()));
    return QRectF(tl, br).normalized().intersected(content);
}

void DefectMapCanvas::centerOnLamp(const LampKey &key)
{
    if (!m_state.gridValid || width() <= 0 || height() <= 0)
        return;
    if (key.dieRow < 0 || key.dieCol < 0 || key.dieRow >= m_state.dieGrid.rowCount
        || key.dieCol >= m_state.dieGrid.colCount)
        return;

    const QRectF content = mapContentRect();
    if (content.isEmpty())
        return;

    const QPointF mapCenter(key.dieCol + 0.5, key.dieRow + 0.5);
    const double  baseScale = qMin(width() / content.width(), height() / content.height());
    const double  scale     = baseScale * m_zoom;
    const QPointF mapped(mapCenter.x() * scale, mapCenter.y() * scale);
    const QPointF contentCenter(content.width() * 0.5 * scale, content.height() * 0.5 * scale);
    const QPointF panOffset   = contentCenter - mapped;

    m_pan.setX(panOffset.x() / width() + 0.5);
    m_pan.setY(panOffset.y() / height() + 0.5);
    m_fitPending = false;

    clampPan();
    update();
}

QRectF DefectMapCanvas::cellRect(int dieRow, int dieCol) const
{
    return QRectF(dieCol, dieRow, 1.0, 1.0);
}

QPointF DefectMapCanvas::mapToWidget(const QPointF &mapPt) const
{
    const QRectF content = mapContentRect();
    if (content.isEmpty())
        return {};

    const double baseScale = qMin(width() / content.width(), height() / content.height());
    const double scale     = baseScale * m_zoom;
    const QPointF center(width() * 0.5, height() * 0.5);
    const QPointF mapped(mapPt.x() * scale, mapPt.y() * scale);
    const QPointF contentCenter(content.width() * 0.5 * scale, content.height() * 0.5 * scale);
    const QPointF panOffset((m_pan.x() - 0.5) * width(), (m_pan.y() - 0.5) * height());
    return center - contentCenter + mapped + panOffset;
}

QPointF DefectMapCanvas::widgetToMap(const QPointF &widgetPt) const
{
    const QRectF content = mapContentRect();
    if (content.isEmpty())
        return {};

    const double baseScale = qMin(width() / content.width(), height() / content.height());
    const double scale     = baseScale * m_zoom;
    const QPointF center(width() * 0.5, height() * 0.5);
    const QPointF contentCenter(content.width() * 0.5 * scale, content.height() * 0.5 * scale);
    const QPointF panOffset((m_pan.x() - 0.5) * width(), (m_pan.y() - 0.5) * height());
    const QPointF local = widgetPt - center + contentCenter - panOffset;
    return QPointF(local.x() / scale, local.y() / scale);
}

QVector<inspection::InspectionDefectType> DefectMapCanvas::visibleTypesForRecord(
    const DefectReviewLampRecord &rec) const
{
    return markerTypesForMapDraw(rec, m_state.visibleTypeMask, kMaxMapMarkerCount);
}

void DefectMapCanvas::drawMarkerSquares(QPainter &p, const QRectF &pxRect,
                                        const QVector<inspection::InspectionDefectType> &types,
                                        bool okOnly) const
{
    const QPointF center = pxRect.center();
    const double  half   = qMax(3.0, qMin(pxRect.width(), pxRect.height()) * 0.38);
    const double  step   = half * 0.24;
    const double  radius = 3.0;
    const double  minHalf = 2.0;

    if (okOnly) {
        const QRectF sq(center.x() - half, center.y() - half, half * 2.0, half * 2.0);
        p.setBrush(okSwatchColor());
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(sq, radius, radius);
        return;
    }

    for (int i = 0; i < types.size(); ++i) {
        const double h = qMax(minHalf, half - i * step);
        const QRectF sq(center.x() - h, center.y() - h, h * 2.0, h * 2.0);
        p.setBrush(defectTypeColor(types[i]));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(sq, radius, radius);
    }
}

void DefectMapCanvas::drawSelectionFrame(QPainter &p, const QRectF &pxRect) const
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
  QColor glow = t.accent.isValid() ? t.accent : QColor(0x3b, 0x82, 0xf6);
    glow.setAlpha(90);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(glow, 4));
    p.drawRoundedRect(pxRect.adjusted(-3, -3, 3, 3), 5, 5);
    p.setPen(QPen(Qt::white, 2));
    p.drawRoundedRect(pxRect.adjusted(0, 0, -1, -1), 4, 4);
}

void DefectMapCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), mapBackgroundColor());

    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    if (!m_state.gridValid) {
        p.setPen(t.textSecondary.isValid() ? t.textSecondary : QColor(0x94, 0xa3, 0xb8));
        p.drawText(rect(), Qt::AlignCenter, TR("aoi.review.die_grid_missing"));
        return;
    }

    const int rows = m_state.dieGrid.rowCount;
    const int cols = m_state.dieGrid.colCount;

    const QPointF cellBr = mapToWidget(QPointF(1, 1));
    const double    cellPx = qMin(qAbs(cellBr.x()), qAbs(cellBr.y()));

    p.setPen(QPen(mapGridLineColor(), 1));
    if (cellPx >= 2.0) {
        for (int r = 0; r <= rows; ++r) {
            const QPointF a = mapToWidget(QPointF(0, r));
            const QPointF b = mapToWidget(QPointF(cols, r));
            p.drawLine(a, b);
        }
        for (int c = 0; c <= cols; ++c) {
            const QPointF a = mapToWidget(QPointF(c, 0));
            const QPointF b = mapToWidget(QPointF(c, rows));
            p.drawLine(a, b);
        }
    } else {
        // 大地图缩略视图：灯珠行边界（每 3 晶粒行）+ 稀疏列线
        constexpr int kLampRowStride = 3;
        constexpr int kCoarseColStep = 30;
        for (int r = 0; r <= rows; r += kLampRowStride) {
            const QPointF a = mapToWidget(QPointF(0, r));
            const QPointF b = mapToWidget(QPointF(cols, r));
            p.drawLine(a, b);
        }
        for (int c = 0; c <= cols; c += kCoarseColStep) {
            const QPointF a = mapToWidget(QPointF(c, 0));
            const QPointF b = mapToWidget(QPointF(c, rows));
            p.drawLine(a, b);
        }
    }

    if (!m_state.visibleTypeMask && !m_state.showOkFilter)
        return;

    constexpr double kOkPerCellMinPx = 2.0;
    if (m_state.showOkFilter && cellPx < kOkPerCellMinPx) {
        const QPointF tl = mapToWidget(QPointF(0, 0));
        const QPointF br = mapToWidget(QPointF(cols, rows));
        const QRectF  mapPx = QRectF(tl, br).normalized().intersected(rect());
        if (!mapPx.isEmpty())
            p.fillRect(mapPx, okSwatchColor());
    }

    const auto paintCellAt = [&](int dieRow, int dieCol, bool okOnly,
                                 const QVector<inspection::InspectionDefectType> &types) {
        const QRectF cell     = cellRect(dieRow, dieCol);
        const QPointF tl      = mapToWidget(cell.topLeft());
        const QPointF br      = mapToWidget(cell.bottomRight());
        const QRectF  pxRect  = QRectF(tl, br).normalized();
        drawMarkerSquares(p, pxRect, types, okOnly);

        const LampKey key{dieRow, dieCol};
        if (m_state.selectedKey.has_value() && *m_state.selectedKey == key)
            drawSelectionFrame(p, pxRect);
    };

    QSet<LampKey> painted;
    for (const LampKey &key : m_state.visibleKeys) {
        const DefectReviewLampRecord &rec = m_state.records.value(key);
        if (!rec.inDieGridBounds)
            continue;

        if (recordIsRejudgeOk(rec)) {
            paintCellAt(key.dieRow, key.dieCol, true, {});
            painted.insert(key);
            continue;
        }

        const QVector<inspection::InspectionDefectType> types = visibleTypesForRecord(rec);
        const bool okOnly =
            m_state.showOkFilter && types.isEmpty() && recordShowsOkMarker(rec, m_state.visibleTypeMask);
        if (types.isEmpty() && !okOnly)
            continue;

        paintCellAt(key.dieRow, key.dieCol, okOnly, types);
        painted.insert(key);
    }

    if (m_state.showOkFilter && cellPx >= kOkPerCellMinPx) {
        const QRectF vis = visibleMapRect();
        if (!vis.isEmpty()) {
            const int r0 = qMax(0, static_cast<int>(std::floor(vis.top())));
            const int r1 =
                qMin(rows - 1, static_cast<int>(std::ceil(vis.bottom())) - 1);
            const int c0 = qMax(0, static_cast<int>(std::floor(vis.left())));
            const int c1 =
                qMin(cols - 1, static_cast<int>(std::ceil(vis.right())) - 1);

            for (int r = r0; r <= r1; ++r) {
                for (int c = c0; c <= c1; ++c) {
                    const LampKey key{r, c};
                    if (painted.contains(key))
                        continue;
                    if (!isOkCell(key))
                        continue;
                    paintCellAt(r, c, true, {});
                }
            }
        }
    }
}

void DefectMapCanvas::wheelEvent(QWheelEvent *event)
{
    const QPoint angle = event->angleDelta();
    if (angle.y() == 0)
        return;

    const double factor = angle.y() > 0 ? 1.25 : 1.0 / 1.25;
    applyZoomAtViewportCenter(factor);
    event->accept();
}

std::optional<LampKey> DefectMapCanvas::hitTest(const QPoint &pos) const
{
    const QPointF mapPt = widgetToMap(pos);
    const int     col   = static_cast<int>(std::floor(mapPt.x()));
    const int     row   = static_cast<int>(std::floor(mapPt.y()));
    const LampKey key{row, col};

    if (!m_state.gridValid)
        return std::nullopt;
    if (row < 0 || col < 0 || row >= m_state.dieGrid.rowCount || col >= m_state.dieGrid.colCount)
        return std::nullopt;

    if (m_state.visibleKeys.contains(key))
        return key;

    if (m_state.showOkFilter && isOkCell(key))
        return key;

    return std::nullopt;
}

bool DefectMapCanvas::isOkCell(const LampKey &key) const
{
    if (!m_state.gridValid || !m_state.showOkFilter)
        return false;
    if (key.dieRow < 0 || key.dieCol < 0 || key.dieRow >= m_state.dieGrid.rowCount
        || key.dieCol >= m_state.dieGrid.colCount)
        return false;

    if (m_state.records.contains(key)) {
        const DefectReviewLampRecord &rec = m_state.records.value(key);
        return recordShowsOkMarker(rec, m_state.visibleTypeMask);
    }

    return true;
}

std::optional<int> DefectMapCanvas::chipTypeForCell(const LampKey &key) const
{
    if (m_state.dieChipLayout.contains(key))
        return m_state.dieChipLayout.value(key).chipType;
    if (!m_state.gridValid || key.dieRow < 0 || key.dieCol < 0
        || key.dieRow >= m_state.dieGrid.rowCount || key.dieCol >= m_state.dieGrid.colCount)
        return std::nullopt;
    return key.dieRow % kChipsPerLampRow;
}

QString DefectMapCanvas::okCellTooltip(const LampKey &key) const
{
    const std::optional<int> chip = chipTypeForCell(key);
    if (!chip.has_value())
        return {};

    return TR("aoi.review.map.ok_tooltip")
        .arg(key.dieRow)
        .arg(key.dieCol)
        .arg(chipTypeLabel(*chip));
}

void DefectMapCanvas::updateOkTooltip(const QPoint &pos)
{
    if (auto key = hitTest(pos); key.has_value() && isOkCell(*key)) {
        const QString tip = okCellTooltip(*key);
        if (!tip.isEmpty()) {
            QToolTip::showText(mapToGlobal(pos + QPoint(12, 16)), tip, this);
            return;
        }
    }
    QToolTip::hideText();
}

void DefectMapCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (auto key = hitTest(event->pos()))
            emit lampClicked(*key);
        event->accept();
        return;
    }

    if (event->button() == Qt::RightButton) {
        m_panDragging = true;
        m_lastDragPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        grabMouse();
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void DefectMapCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panDragging && (event->buttons() & Qt::RightButton)) {
        const QPoint step = event->pos() - m_lastDragPos;
        m_lastDragPos     = event->pos();

        if (width() > 0)
            m_pan.rx() += static_cast<double>(step.x()) / width();
        if (height() > 0)
            m_pan.ry() += static_cast<double>(step.y()) / height();

        clampPan();
        update();
        QToolTip::hideText();
        event->accept();
        return;
    }

    updateOkTooltip(event->pos());
    event->accept();
}

void DefectMapCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton && m_panDragging) {
        m_panDragging = false;
        releaseMouse();
        unsetCursor();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void DefectMapCanvas::leaveEvent(QEvent *event)
{
    QToolTip::hideText();
    QWidget::leaveEvent(event);
}

void DefectMapCanvas::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_fitPending)
        fitView();
}

void DefectMapCanvas::clampPan()
{
    const QRectF content = mapContentRect();
    if (content.isEmpty() || width() <= 0 || height() <= 0) {
        m_pan = QPointF(0.5, 0.5);
        return;
    }

    const double baseScale   = qMin(width() / content.width(), height() / content.height());
    const double scale       = baseScale * m_zoom;
    const double contentWpx  = content.width() * scale;
    const double contentHpx  = content.height() * scale;

    auto clampAxis = [](double contentPx, int viewportPx, double panCoord) {
        if (contentPx <= viewportPx)
            return 0.5;
        const double minV = 1.0 - contentPx / (2.0 * viewportPx);
        const double maxV = contentPx / (2.0 * viewportPx);
        return qBound(minV, panCoord, maxV);
    };

    m_pan.setX(clampAxis(contentWpx, width(), m_pan.x()));
    m_pan.setY(clampAxis(contentHpx, height(), m_pan.y()));
}

} // namespace aoi::review
