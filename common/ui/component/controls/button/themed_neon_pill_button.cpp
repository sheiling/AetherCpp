#include "controls/button/themed_neon_pill_button.h"

#include "theme/theme_color_math.h"
#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include <QEvent>
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QtMath>

#include <functional>

namespace ui {

namespace {

constexpr int kGlowMarginH = 12;
constexpr int kGlowMarginV = 10;

struct NeonPillPaintOptions
{
    QRectF  bodyRect;
    QString text;
    QColor  accent;
    QColor  bodyColor;
    QColor  surfaceElevated;
    QColor  textPrimary;
    bool    lightTheme    = false;
    bool    haloEnabled   = true;
    bool    shadowEnabled = true;
    bool    enabled       = true;
    bool    hover         = false;
    bool    pressed       = false;
    int     fontPixelSize = 15;
};

qreal pillCornerRadius(const QRectF &bodyRect)
{
    return bodyRect.height() * 0.5;
}

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

QColor resolveBodyColor(const NeonPillPaintOptions &opt)
{
    if (opt.bodyColor.isValid())
        return opt.bodyColor;

    const QColor base = opt.surfaceElevated.isValid() ? opt.surfaceElevated : QColor(18, 22, 28);
    const qreal  mix  = opt.hover ? 0.22 : (opt.pressed ? 0.10 : 0.16);
    return themeBlend(base, opt.accent, mix);
}

QPainterPath neonPillPath(const QRectF &bodyRect)
{
    QPainterPath path;
    path.addRoundedRect(bodyRect, pillCornerRadius(bodyRect), pillCornerRadius(bodyRect));
    return path;
}

/** 依据表面亮度判断亮色主题（用于文字对比度策略分支） */
bool themeIsLightSurface(const QColor &surface)
{
    if (!surface.isValid())
        return false;
    const qreal lum =
        0.2126 * surface.redF() + 0.7152 * surface.greenF() + 0.0722 * surface.blueF();
    return lum > 0.45;
}

struct PillLabelColors
{
    QColor foreground;
    QColor glowStroke;
    int    radialGlowPeak = 0;
    bool   radialPlusMode = false;
};

PillLabelColors resolvePillLabelColors(const QColor &accent, const QColor &textPrimary,
                                       bool lightTheme, bool enabled, bool hover)
{
    PillLabelColors c;
    if (!enabled) {
        c.foreground = themeWithAlpha(textPrimary, 130);
        return c;
    }
    if (!accent.isValid()) {
        c.foreground = textPrimary;
        return c;
    }

    if (lightTheme) {
        // 亮色主题：深 accent + textPrimary，避免高比例混白导致发灰
        const QColor inkBase = themeDarken(accent, 0.42);
        c.foreground         = themeBlend(textPrimary, inkBase, hover ? 0.38 : 0.28);
        c.glowStroke         = themeWithAlpha(themeDarken(accent, 0.18), hover ? 85 : 65);
        c.radialGlowPeak     = hover ? 48 : 34;
        c.radialPlusMode     = false;
    } else {
        const qreal whiteMix = hover ? 0.82 : 0.74;
        c.foreground         = themeBlend(themeLighten(accent, 0.40), Qt::white, whiteMix);
        c.glowStroke         = themeWithAlpha(themeLighten(accent, 0.55), hover ? 155 : 125);
        c.radialGlowPeak     = hover ? 105 : 78;
        c.radialPlusMode     = true;
    }
    return c;
}

void drawOuterAmbient(QPainter &p, const QRectF &bodyRect, const QColor &accent, bool enabled,
                      bool hover, bool haloEnabled)
{
    if (!haloEnabled || !accent.isValid() || !enabled)
        return;

    const QPointF glowCenter(bodyRect.center().x(), bodyRect.bottom() + bodyRect.height() * 0.04);
    const qreal   rx = bodyRect.width() * 0.55;
    const qreal   ry = bodyRect.height() * (hover ? 1.35 : 1.15);
    const QRectF  bloom(glowCenter.x() - rx, glowCenter.y() - ry * 0.55, rx * 2.0, ry * 1.35);

    drawEllipseRadialFill(p, bloom,
                          [&](QRadialGradient &g) {
                              const int peak = hover ? 58 : 42;
                              g.setColorAt(0.0, themeWithAlpha(accent, peak / 2));
                              g.setColorAt(0.35, themeWithAlpha(accent, peak / 3));
                              g.setColorAt(0.68, themeWithAlpha(accent, peak / 5));
                              g.setColorAt(1.0, QColor(0, 0, 0, 0));
                          },
                          QPainter::CompositionMode_Plus);

    const QRectF wide = bodyRect.adjusted(-bodyRect.height() * 0.35, -bodyRect.height() * 0.15,
                                          bodyRect.height() * 0.35, bodyRect.height() * 0.55);
    drawEllipseRadialFill(p, wide,
                          [&](QRadialGradient &g) {
                              const int peak = hover ? 32 : 22;
                              g.setColorAt(0.0, QColor(0, 0, 0, 0));
                              g.setColorAt(0.55, themeWithAlpha(accent, peak / 4));
                              g.setColorAt(0.82, themeWithAlpha(accent, peak / 2));
                              g.setColorAt(1.0, QColor(0, 0, 0, 0));
                          },
                          QPainter::CompositionMode_Plus);
}

void drawDropShadow(QPainter &p, const QRectF &bodyRect, const QColor &accent, bool enabled,
                    bool shadowEnabled)
{
    if (!shadowEnabled)
        return;

    const QPointF center(bodyRect.center().x(), bodyRect.bottom() + bodyRect.height() * 0.08);
    const qreal   rx = bodyRect.width() * 0.46;
    const qreal   ry = bodyRect.height() * 0.38;
    const QRectF  shadowRect(center.x() - rx, center.y() - ry, rx * 2.0, ry * 2.0);

    if (enabled && accent.isValid()) {
        drawEllipseRadialFill(p, shadowRect, [&](QRadialGradient &g) {
            g.setColorAt(0.0, themeWithAlpha(accent, 52));
            g.setColorAt(0.50, themeWithAlpha(accent, 20));
            g.setColorAt(1.0, QColor(0, 0, 0, 0));
        });
    }

    drawEllipseRadialFill(p, shadowRect.translated(0.0, 2.0), [&](QRadialGradient &g) {
        const int peak = enabled ? 42 : 18;
        g.setColorAt(0.0, QColor(0, 0, 0, peak));
        g.setColorAt(0.60, QColor(0, 0, 0, peak / 4));
        g.setColorAt(1.0, QColor(0, 0, 0, 0));
    });
}

void drawGlassBody(QPainter &p, const QRectF &bodyRect, const QPainterPath &pillPath,
                   const QColor &body, const QColor &accent, bool enabled, bool hover, bool pressed)
{
    QRectF disc = bodyRect;
    if (pressed)
        disc.translate(0.0, 1.0);

    QLinearGradient bodyGrad(disc.topLeft(), disc.bottomLeft());
    bodyGrad.setColorAt(0.0, themeWithAlpha(themeLighten(body, 0.10), enabled ? 210 : 150));
    bodyGrad.setColorAt(0.46, themeWithAlpha(body, enabled ? 225 : 165));
    bodyGrad.setColorAt(0.54, themeWithAlpha(themeDarken(body, 0.06), enabled ? 220 : 160));
    bodyGrad.setColorAt(1.0, themeWithAlpha(themeDarken(body, 0.18), enabled ? 235 : 175));

    p.setPen(Qt::NoPen);
    p.setBrush(bodyGrad);
    p.drawPath(pillPath);

    const qreal   r = pillCornerRadius(disc);
    const QPointF midL(disc.left() + r * 0.55, disc.center().y());
    const QPointF midR(disc.right() - r * 0.55, disc.center().y());
    QPen          seamPen(themeWithAlpha(accent, enabled ? (hover ? 55 : 38) : 18));
    seamPen.setWidthF(0.65);
    p.setPen(seamPen);
    p.drawLine(midL, midR);

    QPen seamHi(themeWithAlpha(Qt::white, enabled ? 22 : 8));
    seamHi.setWidthF(0.45);
    p.setPen(seamHi);
    p.drawLine(midL + QPointF(0.0, -0.6), midR + QPointF(0.0, -0.6));
}

void drawInnerHalo(QPainter &p, const QRectF &bodyRect, const QPainterPath &pillPath,
                   const QColor &accent, bool enabled, bool hover, bool haloEnabled)
{
    if (!haloEnabled || !accent.isValid())
        return;

    p.save();
    p.setClipPath(pillPath);

    const QPointF origin(bodyRect.center().x(), bodyRect.bottom() - bodyRect.height() * 0.06);
    const qreal   rx = bodyRect.width() * 0.42;
    const qreal   ry = bodyRect.height() * (hover ? 0.92 : 0.78);
    const QRectF  inner(origin.x() - rx, origin.y() - ry, rx * 2.0, ry * 1.05);

    drawEllipseRadialFill(p, inner,
                          [&](QRadialGradient &g) {
                              const int peak = enabled ? (hover ? 130 : 95) : 35;
                              g.setColorAt(0.0, themeWithAlpha(accent, peak));
                              g.setColorAt(0.38, themeWithAlpha(accent, peak / 2));
                              g.setColorAt(0.72, themeWithAlpha(accent, peak / 6));
                              g.setColorAt(1.0, QColor(0, 0, 0, 0));
                          },
                          enabled ? QPainter::CompositionMode_Plus
                                  : QPainter::CompositionMode_SourceOver);

    p.restore();
}

void drawCapHighlights(QPainter &p, const QRectF &bodyRect, const QColor &accent, bool enabled,
                       bool hover)
{
    if (!accent.isValid())
        return;

    const qreal capSize = bodyRect.height() * 0.92;
    const qreal cy      = bodyRect.center().y();

    const QRectF leftCap(bodyRect.left() - capSize * 0.08, cy - capSize * 0.5, capSize, capSize);
    drawEllipseRadialFill(p, leftCap,
                          [&](QRadialGradient &g) {
                              const int peak = enabled ? (hover ? 165 : 120) : 40;
                              g.setColorAt(0.0, themeWithAlpha(themeLighten(accent, 0.42), peak));
                              g.setColorAt(0.42, themeWithAlpha(accent, peak / 2));
                              g.setColorAt(1.0, QColor(0, 0, 0, 0));
                          },
                          enabled ? QPainter::CompositionMode_Plus
                                  : QPainter::CompositionMode_SourceOver);

    const QRectF rightCap(bodyRect.right() - capSize * 0.92, cy - capSize * 0.5, capSize,
                          capSize);
    drawEllipseRadialFill(p, rightCap,
                          [&](QRadialGradient &g) {
                              const int peak = enabled ? (hover ? 165 : 120) : 40;
                              g.setColorAt(0.0, themeWithAlpha(themeLighten(accent, 0.42), peak));
                              g.setColorAt(0.42, themeWithAlpha(accent, peak / 2));
                              g.setColorAt(1.0, QColor(0, 0, 0, 0));
                          },
                          enabled ? QPainter::CompositionMode_Plus
                                  : QPainter::CompositionMode_SourceOver);
}

void drawNeonBorder(QPainter &p, const QRectF &bodyRect, const QPainterPath &pillPath,
                    const QColor &accent, bool enabled, bool hover)
{
    if (!accent.isValid())
        return;

    struct StrokePass
    {
        qreal width;
        int   alpha;
    };

    const StrokePass outer[] = {{hover ? 6.0 : 5.0, enabled ? 28 : 12},
                                {hover ? 3.2 : 2.6, enabled ? 55 : 22},
                                {hover ? 1.8 : 1.4, enabled ? 110 : 45}};
    for (const StrokePass &pass : outer) {
        QPen pen(themeWithAlpha(accent, pass.alpha));
        pen.setWidthF(pass.width);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawPath(pillPath);
    }

    QPen corePen(themeWithAlpha(themeLighten(accent, 0.28), enabled ? (hover ? 235 : 205) : 85));
    corePen.setWidthF(1.0);
    corePen.setJoinStyle(Qt::RoundJoin);
    corePen.setCapStyle(Qt::RoundCap);
    p.setPen(corePen);
    p.drawPath(pillPath);

    const qreal  flareW = bodyRect.width() * 0.34;
    const QRectF flare(bodyRect.center().x() - flareW * 0.5, bodyRect.bottom() - 1.6, flareW, 2.8);
    QLinearGradient flareGrad(flare.topLeft(), flare.topRight());
    flareGrad.setColorAt(0.0, QColor(0, 0, 0, 0));
    flareGrad.setColorAt(0.45, themeWithAlpha(themeLighten(accent, 0.35), enabled ? 220 : 80));
    flareGrad.setColorAt(0.55, themeWithAlpha(Qt::white, enabled ? (hover ? 200 : 165) : 60));
    flareGrad.setColorAt(0.65, themeWithAlpha(themeLighten(accent, 0.35), enabled ? 220 : 80));
    flareGrad.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.setPen(Qt::NoPen);
    p.setBrush(flareGrad);
    p.drawRect(flare);
}

void drawLabel(QPainter &p, const QRectF &bodyRect, const QString &text, const QColor &accent,
               const QColor &textPrimary, bool lightTheme, bool enabled, bool hover, bool pressed,
               int fontPixelSize)
{
    if (text.isEmpty())
        return;

    QRectF textRect = bodyRect;
    if (pressed)
        textRect.translate(0.0, 1.0);

    QFont font = p.font();
    font.setPixelSize(fontPixelSize);
    font.setBold(true);
    p.setFont(font);

    const PillLabelColors colors =
        resolvePillLabelColors(accent, textPrimary, lightTheme, enabled, hover);

    if (enabled && accent.isValid() && colors.radialGlowPeak > 0) {
        const QPointF c = textRect.center();
        const qreal   r = bodyRect.height() * 0.40;
        drawEllipseRadialFill(
            p, QRectF(c.x() - r, c.y() - r, r * 2.0, r * 2.0),
            [&](QRadialGradient &g) {
                const int peak = colors.radialGlowPeak;
                g.setColorAt(0.0, themeWithAlpha(accent, peak));
                g.setColorAt(0.55, themeWithAlpha(accent, peak / 3));
                g.setColorAt(1.0, QColor(0, 0, 0, 0));
            },
            colors.radialPlusMode ? QPainter::CompositionMode_Plus
                                  : QPainter::CompositionMode_SourceOver);
    }

    if (enabled && accent.isValid() && colors.glowStroke.isValid()) {
        p.setPen(colors.glowStroke);
        p.drawText(textRect, Qt::AlignCenter, text);
    }

    p.setPen(colors.foreground);
    p.drawText(textRect, Qt::AlignCenter, text);
}

void paintNeonPillButton(QPainter &p, const NeonPillPaintOptions &opt)
{
    if (!opt.bodyRect.isValid())
        return;

    QRectF bodyRect = opt.bodyRect;
    if (opt.pressed)
        bodyRect.translate(0.0, 1.0);

    const QPainterPath pillPath = neonPillPath(bodyRect);
    const QColor       body     = resolveBodyColor(opt);

    drawOuterAmbient(p, bodyRect, opt.accent, opt.enabled, opt.hover, opt.haloEnabled);
    drawDropShadow(p, bodyRect, opt.accent, opt.enabled, opt.shadowEnabled);

    if (opt.haloEnabled)
        drawCapHighlights(p, bodyRect, opt.accent, opt.enabled, opt.hover);

    drawGlassBody(p, bodyRect, pillPath, body, opt.accent, opt.enabled, opt.hover, opt.pressed);
    drawInnerHalo(p, bodyRect, pillPath, opt.accent, opt.enabled, opt.hover, opt.haloEnabled);
    drawNeonBorder(p, bodyRect, pillPath, opt.accent, opt.enabled, opt.hover);
    drawLabel(p, bodyRect, opt.text, opt.accent, opt.textPrimary, opt.lightTheme, opt.enabled,
              opt.hover, opt.pressed, opt.fontPixelSize);
}

} // namespace

ThemedNeonPillButton::ThemedNeonPillButton(QWidget *parent)
    : QAbstractButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    bindTheme();
    updateSizeFromContent();
}

ThemedNeonPillButton::ThemedNeonPillButton(const QString &text, QWidget *parent)
    : ThemedNeonPillButton(parent)
{
    setText(text);
}

void ThemedNeonPillButton::setPillHeight(int height)
{
    const int h = qMax(28, height);
    if (m_pillHeight == h)
        return;
    m_pillHeight = h;
    updateSizeFromContent();
    update();
}

void ThemedNeonPillButton::setMinPillWidth(int width)
{
    const int w = qMax(48, width);
    if (m_minPillWidth == w)
        return;
    m_minPillWidth = w;
    updateSizeFromContent();
    update();
}

void ThemedNeonPillButton::setHorizontalPadding(int padding)
{
    const int pad = qMax(8, padding);
    if (m_horizontalPadding == pad)
        return;
    m_horizontalPadding = pad;
    updateSizeFromContent();
    update();
}

void ThemedNeonPillButton::setAccentColor(const QColor &color)
{
    if (m_accentColor == color)
        return;
    m_accentColor = color;
    update();
}

void ThemedNeonPillButton::setBodyColor(const QColor &color)
{
    if (m_bodyColor == color)
        return;
    m_bodyColor = color;
    update();
}

void ThemedNeonPillButton::setHaloEnabled(bool enabled)
{
    if (m_haloEnabled == enabled)
        return;
    m_haloEnabled = enabled;
    update();
}

void ThemedNeonPillButton::setShadowEnabled(bool enabled)
{
    if (m_shadowEnabled == enabled)
        return;
    m_shadowEnabled = enabled;
    update();
}

void ThemedNeonPillButton::setText(const QString &text)
{
    if (this->text() == text)
        return;
    QAbstractButton::setText(text);
    updateSizeFromContent();
    update();
}

int ThemedNeonPillButton::pillContentWidth() const
{
    QFont font;
    font.setPixelSize(qMax(12, qRound(m_pillHeight * 0.38)));
    font.setBold(true);
    QFontMetricsF fm(font);
    const int     textW = text().isEmpty() ? 0 : qCeil(fm.horizontalAdvance(text()));
    return qMax(m_minPillWidth, textW + m_horizontalPadding * 2);
}

QSize ThemedNeonPillButton::sizeHint() const
{
    return QSize(pillContentWidth() + kGlowMarginH * 2, m_pillHeight + kGlowMarginV * 2);
}

QSize ThemedNeonPillButton::minimumSizeHint() const
{
    return sizeHint();
}

void ThemedNeonPillButton::bindTheme()
{
    bindThemeRefresh(this, [this]() { update(); });
}

void ThemedNeonPillButton::updateSizeFromContent()
{
    setMinimumSize(sizeHint());
}

QRectF ThemedNeonPillButton::pillRectInWidget() const
{
    const int contentW = pillContentWidth();
    const int availW   = width() - 2 * kGlowMarginH;
    const int pillW    = qMax(contentW, availW);
    const int x        = (width() - pillW) / 2;
    return QRectF(x, kGlowMarginV, pillW, m_pillHeight);
}

void ThemedNeonPillButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    const ThemeTokens tokens = ThemeTokens::resolve();
    const bool        enabled = isEnabled();
    const bool        hover   = enabled && underMouse();
    const bool        pressed = enabled && isDown();

    QColor accent = m_accentColor.isValid() ? m_accentColor : tokens.accent;
    if (hover && enabled)
        accent = themeLighten(accent, 0.10);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    NeonPillPaintOptions paintOpt;
    paintOpt.bodyRect        = pillRectInWidget();
    paintOpt.text            = text();
    paintOpt.accent          = accent;
    paintOpt.bodyColor       = m_bodyColor;
    paintOpt.surfaceElevated = tokens.surfaceElevated;
    paintOpt.textPrimary     = tokens.textPrimary;
    paintOpt.lightTheme      = themeIsLightSurface(tokens.surfaceElevated);
    paintOpt.haloEnabled     = m_haloEnabled;
    paintOpt.shadowEnabled   = m_shadowEnabled;
    paintOpt.enabled         = enabled;
    paintOpt.hover           = hover;
    paintOpt.pressed         = pressed;
    paintOpt.fontPixelSize   = qMax(12, qRound(m_pillHeight * 0.38));

    paintNeonPillButton(p, paintOpt);
}

void ThemedNeonPillButton::changeEvent(QEvent *event)
{
    QAbstractButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        update();
        return;
    }
    if (event->type() == QEvent::FontChange || event->type() == QEvent::LanguageChange) {
        updateSizeFromContent();
        update();
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ThemedNeonPillButton::enterEvent(QEnterEvent *event)
#else
void ThemedNeonPillButton::enterEvent(QEvent *event)
#endif
{
    QAbstractButton::enterEvent(event);
    update();
}

void ThemedNeonPillButton::leaveEvent(QEvent *event)
{
    QAbstractButton::leaveEvent(event);
    update();
}

ThemedNeonPillButton *makeNeonPillButton(const QString &text, QWidget *parent,
                                         const QColor &accentColor)
{
    auto *btn = new ThemedNeonPillButton(text, parent);
    if (accentColor.isValid())
        btn->setAccentColor(accentColor);
    return btn;
}

} // namespace ui
