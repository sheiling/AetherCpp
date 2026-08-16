/**
 * @file panel_navigator_overlay.cpp
 * @brief 全图导航浮层实现
 */

#include "domain/panel_navigator_overlay.h"

#include "recipe_config_view_model.h"
#include "fov_planner/fov_layout_types.h"
#include "fov_planner/recipe_fov_layout_adapter.h"

#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"

#include <QLineEdit>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QtMath>

namespace aoi {

namespace {

constexpr int kOverlayMargin       = 8;
constexpr int kOverlayCornerRadius = 8;
constexpr int kGlassBorder         = 5;
constexpr int kDragHandleHeight    = 16;

constexpr QColor kFovEdgeColor(255, 214, 84, 255);
constexpr QColor kFovGlowColor(255, 236, 160, 180);
constexpr double kMinFovDisplayPx = 10.0;

bool isInsideMarkLogicalBounds(const fov_planner::FovLayoutConfig &cfg,
                               const common::geometry::Vec2d &logical)
{
    const common::geometry::Rect2d bounds = fov_planner::computeMarkNavigateLogicalBounds(cfg);
    return logical.x >= bounds.left() && logical.x <= bounds.right()
           && logical.y >= bounds.bottom() && logical.y <= bounds.top();
}

void drawFovReticle(QPainter &painter, const QPointF &center, const QColor &color)
{
    constexpr double kRadius   = 5.5;
    constexpr double kGap      = 2.0;
    constexpr double kArm      = 4.0;

    painter.setPen(QPen(color, 1.6));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, kRadius, kRadius);

    painter.drawLine(QPointF(center.x() - kRadius - kArm, center.y()),
                     QPointF(center.x() - kGap, center.y()));
    painter.drawLine(QPointF(center.x() + kGap, center.y()),
                     QPointF(center.x() + kRadius + kArm, center.y()));
    painter.drawLine(QPointF(center.x(), center.y() - kRadius - kArm),
                     QPointF(center.x(), center.y() - kGap));
    painter.drawLine(QPointF(center.x(), center.y() + kGap),
                     QPointF(center.x(), center.y() + kRadius + kArm));
}

int statusBarReservePx(const QWidget *hostWidget)
{
    if (!hostWidget)
        return 0;

    const QList<QLineEdit *> statusEdits =
        hostWidget->findChildren<QLineEdit *>(QStringLiteral("graphicsStatusEdit"));
    if (statusEdits.isEmpty())
        return 0;

    return statusEdits.first()->height() + 12;
}

void drawGlassChrome(QPainter &painter, const QRectF &outer, int radius, const ui::ThemeTokens &t)
{
    QPainterPath path;
    path.addRoundedRect(outer, radius, radius);

    QLinearGradient fill(outer.topLeft(), outer.bottomLeft());
    fill.setColorAt(0.0, ui::themeWithAlpha(ui::themeBlend(t.surface, t.accent, 0.28), 96));
    fill.setColorAt(0.55, ui::themeWithAlpha(ui::themeBlend(t.surface, t.accentHover, 0.22), 88));
    fill.setColorAt(1.0, ui::themeWithAlpha(ui::themeBlend(t.surface, t.accent, 0.18), 104));

    painter.setPen(Qt::NoPen);
    painter.setBrush(fill);
    painter.drawPath(path);

    QPen outerBorder(ui::themeWithAlpha(t.accentHover, 175), 1.2);
    painter.setPen(outerBorder);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);

    QPen innerGlow(ui::themeWithAlpha(t.focus, 72), 1.0);
    painter.setPen(innerGlow);
    painter.drawRoundedRect(outer.adjusted(1.0, 1.0, -1.0, -1.0), radius - 1, radius - 1);

    QLinearGradient highlight(outer.topLeft(), outer.topRight());
    highlight.setColorAt(0.0, ui::themeWithAlpha(t.accent, 0));
    highlight.setColorAt(0.5, ui::themeWithAlpha(t.accentHover, 110));
    highlight.setColorAt(1.0, ui::themeWithAlpha(t.accent, 0));
    painter.setPen(Qt::NoPen);
    painter.setBrush(highlight);
    painter.drawRoundedRect(QRectF(outer.left() + 8, outer.top() + 1.5, outer.width() - 16, 3),
                            1.5, 1.5);

    painter.setPen(ui::themeWithAlpha(t.textSecondary, 170));
    const double cx = outer.center().x();
    const double cy = outer.top() + kDragHandleHeight * 0.5;
    for (int i = -1; i <= 1; ++i)
        painter.drawPoint(QPointF(cx + i * 5.0, cy));
}

} // namespace

PanelNavigatorOverlay::PanelNavigatorOverlay(QWidget *hostWidget,
                                             RecipeConfigViewModel *viewModel,
                                             std::function<QString()> cameraIdProvider,
                                             QWidget *parent)
    : QWidget(parent)
    , m_hostWidget(hostWidget)
    , m_viewModel(viewModel)
    , m_cameraIdProvider(std::move(cameraIdProvider))
{
    setObjectName(QStringLiteral("recipePanelNavigatorOverlay"));
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    setMouseTracking(true);
    hide();

    if (m_hostWidget)
        m_hostWidget->installEventFilter(this);

    ui::bindThemeRefresh(this, [this]() {
        m_paintResult = {};
        ensurePaintResult();
        updateFovRect();
        update();
    });
}

QRect PanelNavigatorOverlay::contentRect() const
{
    return rect().adjusted(kGlassBorder, kGlassBorder + kDragHandleHeight, -kGlassBorder,
                           -kGlassBorder);
}

void PanelNavigatorOverlay::setActive(bool active)
{
    m_active = active;
    if (!active) {
        m_dragging       = false;
        m_userPositioned = false;
        hide();
        return;
    }

    if (!m_userPositioned)
        repositionOnHost();
    setVisible(true);
    ensurePaintResult();
    updateFovRect();
    update();
}

void PanelNavigatorOverlay::rebuildFromDocument(const RecipeDocument &doc)
{
    const fov_planner::FovLayoutConfig cfg = fov_planner::buildFovLayoutConfig(doc);
    m_coords.build(cfg);
    m_paintResult = {};
    ensurePaintResult();
    updateFovRect();
    update();
}

void PanelNavigatorOverlay::ensurePaintResult()
{
    const QSize innerSize = contentRect().size();
    if (innerSize.width() <= 0 || innerSize.height() <= 0)
        return;

    if (m_paintResult.background.isNull()
        || m_paintResult.background.size() != innerSize
        || (!m_coords.isValid() && m_paintResult.valid)
        || (m_coords.isValid() && !m_paintResult.valid)) {
        m_paintResult = PanelOverviewPainter::paint(m_coords, innerSize);
    }
}

void PanelNavigatorOverlay::updateFovRect()
{
    m_fovWidgetRect      = QRectF();
    m_fovCenterInContent = QPointF();
    if (!m_hasFovCenter || !m_coords.isValid() || !m_paintResult.valid)
        return;

    double fovWidthMm  = m_coords.config().fovWidthMm;
    double fovHeightMm = m_coords.config().fovHeightMm;
    if (m_viewModel) {
        const fov_planner::FovSizeMm fovSize =
            fov_planner::computeFovSizeMm(m_viewModel->document());
        fovWidthMm  = fovSize.widthMm;
        fovHeightMm = fovSize.heightMm;
    }

    const PanelOverviewViewMapping &map = m_paintResult.mapping;
    if (map.scalePxPerMm <= 0.0)
        return;

    const common::geometry::Vec2d centerLogical =
        m_coords.physicalToLogical({m_fovCenterXMm, m_fovCenterYMm});
    const QPointF centerWidget = map.logicalToWidget(centerLogical);
    m_fovCenterInContent       = centerWidget;

    const double halfW = fovWidthMm * 0.5 * map.scalePxPerMm;
    const double halfH = fovHeightMm * 0.5 * map.scalePxPerMm;
    if (halfW <= 0.0 || halfH <= 0.0)
        return;

    QRectF fovRect(centerWidget.x() - halfW, centerWidget.y() - halfH, halfW * 2.0, halfH * 2.0);
    if (fovRect.width() < kMinFovDisplayPx || fovRect.height() < kMinFovDisplayPx) {
        const double scaleUp =
            qMax(kMinFovDisplayPx / qMax(fovRect.width(), 1e-6),
                 kMinFovDisplayPx / qMax(fovRect.height(), 1e-6));
        fovRect.setWidth(fovRect.width() * scaleUp);
        fovRect.setHeight(fovRect.height() * scaleUp);
        fovRect.moveCenter(centerWidget);
    }

    m_fovWidgetRect = fovRect;
}

void PanelNavigatorOverlay::syncFovFromMotion(const QStringList &axisNames, double xMm,
                                              double yMm)
{
    Q_UNUSED(axisNames);
    if (!m_active)
        return;

    m_fovCenterXMm = xMm;
    m_fovCenterYMm = yMm;
    m_hasFovCenter = true;
    ensurePaintResult();
    updateFovRect();
    update();
}

bool PanelNavigatorOverlay::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_hostWidget && event->type() == QEvent::Resize) {
        if (!m_userPositioned)
            repositionOnHost();
        else
            clampToHost(m_customTopLeft);
        if (m_active && m_viewModel)
            rebuildFromDocument(m_viewModel->document());
    }
    return QWidget::eventFilter(watched, event);
}

void PanelNavigatorOverlay::clampToHost(QPoint &topLeft) const
{
    if (!m_hostWidget)
        return;

    const int maxX = qMax(0, m_hostWidget->width() - width());
    const int maxY = qMax(0, m_hostWidget->height() - height());
    topLeft.setX(qBound(0, topLeft.x(), maxX));
    topLeft.setY(qBound(0, topLeft.y(), maxY));
}

void PanelNavigatorOverlay::repositionOnHost()
{
    if (!m_hostWidget)
        return;

    const QSize hostSize = m_hostWidget->size();
    const int   w        = qMax(96, hostSize.width() / 3);
    const int   h        = qMax(72, hostSize.height() / 3);
    const int   bottomGap = kOverlayMargin + statusBarReservePx(m_hostWidget);
    const QPoint topLeft(kOverlayMargin, hostSize.height() - h - bottomGap);
    setGeometry(topLeft.x(), topLeft.y(), w, h);
    m_customTopLeft = topLeft;
    raise();
}

bool PanelNavigatorOverlay::isDragZone(const QPoint &pos) const
{
    if (pos.y() <= kDragHandleHeight)
        return true;
    const QRect inner = contentRect();
    return !inner.contains(pos);
}

void PanelNavigatorOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const ui::ThemeTokens theme = ui::ThemeTokens::resolve();
    const QRectF          outer = rect().adjusted(0.5, 0.5, -1.5, -1.5);

    painter.setPen(Qt::NoPen);
    painter.setBrush(ui::themeWithAlpha(theme.accent, 36));
    painter.drawRoundedRect(outer.translated(0, 2), kOverlayCornerRadius, kOverlayCornerRadius);

    drawGlassChrome(painter, outer, kOverlayCornerRadius, theme);

    ensurePaintResult();
    updateFovRect();

    const QRect inner = contentRect();
    if (m_paintResult.background.isNull() || inner.width() <= 0 || inner.height() <= 0)
        return;

    QPainterPath clipPath;
    clipPath.addRoundedRect(inner, kOverlayCornerRadius - 2, kOverlayCornerRadius - 2);
    painter.setClipPath(clipPath);
    painter.drawPixmap(inner.topLeft(), m_paintResult.background);
    painter.setClipping(false);

    if (!m_fovWidgetRect.isEmpty()) {
        const QRectF fov     = m_fovWidgetRect.translated(inner.topLeft());
        const QPointF center = m_fovCenterInContent + inner.topLeft();

        painter.setPen(QPen(kFovGlowColor, 1.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(fov.adjusted(-0.5, -0.5, 0.5, 0.5));

        painter.setPen(QPen(kFovEdgeColor, 1.8));
        painter.drawRect(fov);

        drawFovReticle(painter, center, kFovEdgeColor);
    }
}

void PanelNavigatorOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    if (isDragZone(event->pos())) {
        m_dragging         = true;
        m_dragPressGlobal  = event->globalPos();
        m_dragWidgetOrigin = pos();
        m_userPositioned   = true;
        event->accept();
        return;
    }

    if (!m_paintResult.valid || !m_coords.isValid()) {
        QWidget::mousePressEvent(event);
        return;
    }

    const QRect inner = contentRect();
    const auto  logical =
        m_paintResult.mapping.widgetToLogical(event->pos() - inner.topLeft());
    if (!logical)
        return;

    if (!isInsideMarkLogicalBounds(m_coords.config(), *logical))
        return;

    navigateToLogical(*logical);
}

void PanelNavigatorOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging || !m_hostWidget) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    QPoint topLeft = m_dragWidgetOrigin + (event->globalPos() - m_dragPressGlobal);
    clampToHost(topLeft);
    move(topLeft);
    m_customTopLeft = topLeft;
    event->accept();
}

void PanelNavigatorOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}

void PanelNavigatorOverlay::navigateToLogical(const common::geometry::Vec2d &logical)
{
    if (!m_viewModel)
        return;

    const common::geometry::Vec2d physical = m_coords.logicalToPhysical(logical);
    const QString cameraId =
        m_cameraIdProvider ? m_cameraIdProvider() : QStringLiteral("main");
    m_viewModel->moveCaptureToPhysicalXY(cameraId, physical.x, physical.y);
}

} // namespace aoi
