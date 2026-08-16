/**
 * @file themed_glass_indicator_dot.cpp
 * @brief ThemedGlassIndicatorDot 玻璃质感与光晕绘制
 */

#include "controls/indicator/themed_glass_indicator_dot.h"

#include "theme/theme_color_math.h"
#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include <QPainter>
#include <QRadialGradient>
#include <functional>

namespace ui {

namespace {

void drawEllipseRadialFill(QPainter &p, const QRectF &bounds,
                           const std::function<void(QRadialGradient &)> &setupGradient,
                           QPainter::CompositionMode mode = QPainter::CompositionMode_SourceOver)
{
    if (bounds.width() < 1.0 || bounds.height() < 1.0)
        return;

    const QPointF c  = bounds.center();
    const qreal   rx = bounds.width() * 0.5;
    const qreal   ry = bounds.height() * 0.5;

    QRadialGradient grad(QPointF(0.0, 0.0), 1.0);
    setupGradient(grad);

    p.save();
    if (mode != QPainter::CompositionMode_SourceOver)
        p.setCompositionMode(mode);
    p.translate(c);
    p.scale(rx, ry);
    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawEllipse(QRectF(-1.0, -1.0, 2.0, 2.0));
    p.restore();
}

int glowPaddingForDiameter(int diameter)
{
    return qMax(6, diameter / 2 + 3);
}

} // namespace

ThemedGlassIndicatorDot::ThemedGlassIndicatorDot(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("themedGlassIndicatorDot"));
    setAttribute(Qt::WA_TranslucentBackground, true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    refreshAccentFromTheme();
    bindTheme();
    setDiameter(m_diameter);
}

void ThemedGlassIndicatorDot::setDiameter(int pixels)
{
    m_diameter = qMax(6, pixels);
    const int pad = glowPaddingForDiameter(m_diameter);
    setFixedSize(pad * 2, pad * 2);
    update();
}

void ThemedGlassIndicatorDot::setAccentColor(const QColor &color)
{
    m_accentColor      = color;
    m_useThemeFallback = !color.isValid();
    if (m_useThemeFallback)
        refreshAccentFromTheme();
    update();
}

QSize ThemedGlassIndicatorDot::sizeHint() const
{
    const int pad = glowPaddingForDiameter(m_diameter);
    return QSize(pad * 2, pad * 2);
}

void ThemedGlassIndicatorDot::bindTheme()
{
    bindThemeRefresh(this, [this]() {
        if (m_useThemeFallback)
            refreshAccentFromTheme();
        update();
    });
}

void ThemedGlassIndicatorDot::refreshAccentFromTheme()
{
    m_accentColor = ThemeTokens::resolve().textSecondary;
}

void ThemedGlassIndicatorDot::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const ThemeTokens t = ThemeTokens::resolve();
    const QColor      accent =
        m_accentColor.isValid() ? m_accentColor : t.textSecondary;

    const QRectF bounds = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const qreal  d      = static_cast<qreal>(m_diameter);
    const QRectF orbRect(bounds.center().x() - d * 0.5, bounds.center().y() - d * 0.5, d, d);

    const QRectF bloomRect = orbRect.adjusted(-d * 0.42, -d * 0.42, d * 0.42, d * 0.42);
    drawEllipseRadialFill(p, bloomRect, [&](QRadialGradient &g) {
        g.setCoordinateMode(QGradient::ObjectBoundingMode);
        g.setColorAt(0.0, themeWithAlpha(accent, 150));
        g.setColorAt(0.45, themeWithAlpha(accent, 55));
        g.setColorAt(1.0, themeWithAlpha(accent, 0));
    });

    const QColor shellBase = themeBlend(t.surface, accent, 0.28);
    const QColor shellDark = themeDarken(shellBase, 0.22);
    p.setPen(QPen(themeWithAlpha(themeLighten(accent, 0.35), 150), 1.0));
    p.setBrush(themeBlend(shellDark, accent, 0.18));
    p.drawEllipse(orbRect);

    const QRectF innerGlow = orbRect.adjusted(d * 0.12, d * 0.12, -d * 0.12, -d * 0.12);
    drawEllipseRadialFill(p, innerGlow, [&](QRadialGradient &g) {
        g.setCoordinateMode(QGradient::ObjectBoundingMode);
        g.setColorAt(0.0, themeLighten(accent, 0.62));
        g.setColorAt(0.42, accent);
        g.setColorAt(1.0, themeDarken(accent, 0.38));
    });

    const QRectF core = orbRect.adjusted(d * 0.28, d * 0.28, -d * 0.28, -d * 0.28);
    drawEllipseRadialFill(p, core, [&](QRadialGradient &g) {
        g.setCoordinateMode(QGradient::ObjectBoundingMode);
        g.setColorAt(0.0, themeLighten(accent, 0.45));
        g.setColorAt(0.65, accent);
        g.setColorAt(1.0, themeDarken(accent, 0.25));
    });

    const QRectF spec(
        orbRect.left() + d * 0.16, orbRect.top() + d * 0.10, d * 0.38, d * 0.30);
    drawEllipseRadialFill(p, spec, [&](QRadialGradient &g) {
        g.setCoordinateMode(QGradient::ObjectBoundingMode);
        g.setColorAt(0.0, themeWithAlpha(Qt::white, 210));
        g.setColorAt(0.55, themeWithAlpha(Qt::white, 70));
        g.setColorAt(1.0, themeWithAlpha(Qt::white, 0));
    });
}

} // namespace ui
