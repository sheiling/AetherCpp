/**
 * @file panel_virtual_map_widget.cpp
 * @brief 灯板虚拟图绘制与 FOV 滑动动画
 */

#include "home/panel_virtual_map_widget.h"

#include "uibase/languagemanager.h"

#include <QEasingCurve>
#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <QVBoxLayout>

namespace aoi {

namespace {

constexpr int    kAnimIntervalMs  = 16;
constexpr int    kAnimDurationMs  = 260;
constexpr qreal  kFineGridMinPx   = 2.0;
constexpr QColor kBackgroundColor(0x0b, 0x1a, 0x2d);
constexpr QColor kFineGridColor(0x1e, 0x3a, 0x5f, 80);
constexpr QColor kFovLineColor(0x64, 0x96, 0xff, 140);
constexpr QColor kCapturedFill(0x64, 0x96, 0xff, 55);
constexpr QColor kActiveFill(0xd0, 0xd8, 0xe8, 90);
constexpr QColor kActiveBorder(0xff, 0xff, 0xff);

} // namespace

// ── PanelVirtualMapCanvas ──

PanelVirtualMapCanvas::PanelVirtualMapCanvas(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("panelVirtualMapCanvas"));
    setMinimumHeight(220);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(kAnimIntervalMs);
    connect(m_animTimer, &QTimer::timeout, this, &PanelVirtualMapCanvas::onAnimationTick);
}

void PanelVirtualMapCanvas::applyState(const PanelVirtualMapState &state)
{
    const bool geometryChanged = m_state.panelGroupCols != state.panelGroupCols
                                 || m_state.panelGroupRows != state.panelGroupRows
                                 || m_state.fovGridCols != state.fovGridCols
                                 || m_state.fovGridRows != state.fovGridRows;

    const bool focusChanged = m_state.activeFovRectNorm != state.activeFovRectNorm;
    const bool captureChanged =
        m_state.capturedCells != state.capturedCells
        || m_state.currentFovIndex != state.currentFovIndex
        || m_state.totalFovCount != state.totalFovCount;
    m_state = state;

    if (geometryChanged || m_displayFocusNorm.isNull()) {
        m_displayFocusNorm = state.activeFovRectNorm;
        m_animProgress     = 1.0;
        m_animTimer->stop();
        update();
        return;
    }

    if (focusChanged) {
        if (state.inspecting) {
            m_displayFocusNorm = state.activeFovRectNorm;
            m_animProgress     = 1.0;
            m_animTimer->stop();
        } else {
            startFocusAnimation(state.activeFovRectNorm);
        }
    }

    if (focusChanged || captureChanged)
        update();
}

QRectF PanelVirtualMapCanvas::mapRect() const
{
    constexpr int margin = 8;
    return QRectF(margin, margin,
                  qMax<qreal>(1.0, width() - 2 * margin),
                  qMax<qreal>(1.0, height() - 2 * margin));
}

QRectF PanelVirtualMapCanvas::normRectToPixel(const QRectF &normRect) const
{
    const QRectF area = mapRect();
    return QRectF(area.left() + normRect.left() * area.width(),
                  area.top() + normRect.top() * area.height(),
                  normRect.width() * area.width(),
                  normRect.height() * area.height());
}

int PanelVirtualMapCanvas::cellKey(int row, int col) const
{
    return row * 10000 + col;
}

void PanelVirtualMapCanvas::startFocusAnimation(const QRectF &targetNormRect)
{
    if (targetNormRect.isNull()) {
        m_displayFocusNorm = targetNormRect;
        m_animProgress     = 1.0;
        m_animTimer->stop();
        update();
        return;
    }

    m_animFromNorm = m_displayFocusNorm.isNull() ? targetNormRect : m_displayFocusNorm;
    m_animToNorm   = targetNormRect;
    m_animProgress = 0.0;
    if (!m_animTimer->isActive())
        m_animTimer->start();
}

void PanelVirtualMapCanvas::onAnimationTick()
{
    m_animProgress += static_cast<qreal>(kAnimIntervalMs) / static_cast<qreal>(kAnimDurationMs);
    if (m_animProgress >= 1.0) {
        m_animProgress     = 1.0;
        m_displayFocusNorm = m_animToNorm;
        m_animTimer->stop();
    } else {
        const QEasingCurve curve(QEasingCurve::InOutCubic);
        const qreal        t = curve.valueForProgress(m_animProgress);
        m_displayFocusNorm   = QRectF(
            m_animFromNorm.left() + (m_animToNorm.left() - m_animFromNorm.left()) * t,
            m_animFromNorm.top() + (m_animToNorm.top() - m_animFromNorm.top()) * t,
            m_animFromNorm.width() + (m_animToNorm.width() - m_animFromNorm.width()) * t,
            m_animFromNorm.height() + (m_animToNorm.height() - m_animFromNorm.height()) * t);
    }
    update();
}

void PanelVirtualMapCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), kBackgroundColor);

    const QRectF area = mapRect();
    if (area.width() < 4.0 || area.height() < 4.0)
        return;

    const int groupCols = qMax(1, m_state.panelGroupCols);
    const int groupRows = qMax(1, m_state.panelGroupRows);
    const int fovCols   = qMax(1, m_state.fovGridCols);
    const int fovRows   = qMax(1, m_state.fovGridRows);

    const qreal cellW = area.width() / static_cast<qreal>(groupCols);
    const qreal cellH = area.height() / static_cast<qreal>(groupRows);

    if (cellW >= kFineGridMinPx && cellH >= kFineGridMinPx) {
        QPen finePen(kFineGridColor);
        finePen.setWidth(1);
        painter.setPen(finePen);
        for (int c = 0; c <= groupCols; ++c) {
            const qreal x = area.left() + static_cast<qreal>(c) * cellW;
            painter.drawLine(QPointF(x, area.top()), QPointF(x, area.bottom()));
        }
        for (int r = 0; r <= groupRows; ++r) {
            const qreal y = area.top() + static_cast<qreal>(r) * cellH;
            painter.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
        }
    }

    const qreal fovW = area.width() / static_cast<qreal>(fovCols);
    const qreal fovH = area.height() / static_cast<qreal>(fovRows);

    for (int row = 0; row < fovRows; ++row) {
        for (int col = 0; col < fovCols; ++col) {
            if (!m_state.capturedCells.contains(cellKey(row, col)))
                continue;
            const QRectF cellRect(area.left() + col * fovW, area.top() + row * fovH, fovW, fovH);
            painter.fillRect(cellRect, kCapturedFill);
        }
    }

    if (!m_displayFocusNorm.isNull() && m_displayFocusNorm.width() > 0.0
        && m_displayFocusNorm.height() > 0.0) {
        const QRectF activePx = normRectToPixel(m_displayFocusNorm);
        painter.fillRect(activePx, kActiveFill);
        QPen activePen(kActiveBorder);
        activePen.setWidth(2);
        painter.setPen(activePen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(activePx.adjusted(1.0, 1.0, -1.0, -1.0));
    }

    QPen fovPen(kFovLineColor);
    fovPen.setWidth(1);
    painter.setPen(fovPen);
    for (int c = 0; c <= fovCols; ++c) {
        const qreal x = area.left() + static_cast<qreal>(c) * fovW;
        painter.drawLine(QPointF(x, area.top()), QPointF(x, area.bottom()));
    }
    for (int r = 0; r <= fovRows; ++r) {
        const qreal y = area.top() + static_cast<qreal>(r) * fovH;
        painter.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
    }
}

// ── PanelVirtualMapWidget ──

PanelVirtualMapWidget::PanelVirtualMapWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("panelVirtualMapWidget"));
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral("QWidget#panelVirtualMapWidget { background: transparent; }"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    m_headerLabel = new QLabel(this);
    m_headerLabel->setObjectName(QStringLiteral("panelVirtualMapHeader"));
    m_headerLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_headerLabel->setWordWrap(false);
    m_headerLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_canvas = new PanelVirtualMapCanvas(this);

    layout->addWidget(m_headerLabel);
    layout->addWidget(m_canvas, 1);
}

void PanelVirtualMapWidget::applyState(const PanelVirtualMapState &state)
{
    m_lastState = state;
    rebuildHeaderText(state);
    m_canvas->applyState(state);
}

void PanelVirtualMapWidget::retranslateUi()
{
    rebuildHeaderText(m_lastState);
}

void PanelVirtualMapWidget::rebuildHeaderText(const PanelVirtualMapState &state)
{
    QStringList parts;

    if (state.dualTrackMode && state.activeTrack > 0) {
        const char *trackKey = state.activeTrack >= 2 ? "aoi.home.virtual_map.track_2"
                                                      : "aoi.home.virtual_map.track_1";
        parts << TR(trackKey);
    }

    if (state.inspecting)
        parts << TR("aoi.home.virtual_map.inspecting");

    if (state.totalFovCount > 0) {
        parts << TR("aoi.home.virtual_map.scan_progress")
                     .arg(state.currentFovIndex)
                     .arg(state.totalFovCount);
    }

    if (!state.statusText.trimmed().isEmpty())
        parts << state.statusText.trimmed();

    m_headerLabel->setText(parts.join(QStringLiteral("  ")));
}

} // namespace aoi
