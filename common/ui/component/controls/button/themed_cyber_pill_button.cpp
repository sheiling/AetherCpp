#include "controls/button/themed_cyber_pill_button.h"

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

namespace ui {

namespace {

constexpr int kOuterMarginH = 8;
constexpr int kOuterMarginV = 6;

struct CyberPillPaintOptions
{
    QRectF  bodyRect;
    QString text;
    QColor  accent;
    QColor  bodyColor;
    QColor  textColor;
    QColor  surfaceElevated;
    bool    lightTheme        = false;
    bool    enabled            = true;
    bool    hover              = false;
    bool    pressed            = false;
    bool    decorationsEnabled = true;
    int     fontPixelSize      = 14;
};

bool themeIsLightSurface(const QColor &surface)
{
    if (!surface.isValid())
        return false;
    const qreal lum =
        0.2126 * surface.redF() + 0.7152 * surface.greenF() + 0.0722 * surface.blueF();
    return lum > 0.45;
}

QColor resolveBodyFill(const CyberPillPaintOptions &opt)
{
    if (opt.bodyColor.isValid()) {
        QColor base = opt.bodyColor;
        if (!opt.enabled)
            return themeWithAlpha(themeDarken(base, 0.10), 150);
        if (opt.pressed)
            return themeWithAlpha(themeDarken(base, 0.06), base.alpha());
        if (opt.hover)
            return themeWithAlpha(themeBlend(base, opt.accent, 0.10), base.alpha());
        return base;
    }

    const QColor base =
        opt.surfaceElevated.isValid() ? opt.surfaceElevated : QColor(24, 32, 44);

    if (opt.lightTheme) {
        const qreal mix = opt.hover ? 0.18 : (opt.pressed ? 0.10 : 0.14);
        QColor      body = themeBlend(themeLighten(base, 0.06), opt.accent, mix);
        return themeWithAlpha(body, opt.enabled ? 235 : 175);
    }

    const qreal mix = opt.hover ? 0.22 : (opt.pressed ? 0.10 : 0.16);
    QColor      body = themeBlend(base, opt.accent, mix);
    return themeWithAlpha(body, opt.enabled ? 215 : 165);
}

QColor resolveLabelColor(const CyberPillPaintOptions &opt)
{
    if (opt.textColor.isValid())
        return opt.enabled ? opt.textColor : themeWithAlpha(opt.textColor, 130);

    if (!opt.enabled)
        return themeWithAlpha(opt.lightTheme ? Qt::black : Qt::white, 130);

    if (opt.lightTheme) {
        if (opt.accent.isValid()) {
            const QColor ink = themeDarken(opt.accent, 0.40);
            return themeBlend(ink, Qt::black, opt.hover ? 0.22 : 0.32);
        }
        return themeBlend(Qt::black, Qt::white, 0.08);
    }

    if (opt.accent.isValid()) {
        const qreal whiteMix = opt.hover ? 0.80 : 0.72;
        return themeBlend(themeLighten(opt.accent, 0.35), Qt::white, whiteMix);
    }
    return Qt::white;
}

qreal pillRadius(const QRectF &bodyRect)
{
    return bodyRect.height() * 0.5;
}

QPainterPath pillPath(const QRectF &bodyRect)
{
    QPainterPath path;
    path.addRoundedRect(bodyRect, pillRadius(bodyRect), pillRadius(bodyRect));
    return path;
}

void drawBody(QPainter &p, const QRectF &bodyRect, const QPainterPath &path, const QColor &body,
              bool enabled, bool pressed, bool lightTheme)
{
    QRectF rect = bodyRect;
    if (pressed)
        rect.translate(0.0, 0.5);

    QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
    if (lightTheme) {
        grad.setColorAt(0.0, themeWithAlpha(themeLighten(body, 0.10), enabled ? body.alpha() : 150));
        grad.setColorAt(0.50, themeWithAlpha(body, enabled ? body.alpha() : 140));
        grad.setColorAt(1.0, themeWithAlpha(themeDarken(body, 0.06), enabled ? body.alpha() : 130));
    } else {
        grad.setColorAt(0.0, themeWithAlpha(themeLighten(body, 0.08), enabled ? body.alpha() : 150));
        grad.setColorAt(0.45, themeWithAlpha(body, enabled ? body.alpha() : 140));
        grad.setColorAt(1.0, themeWithAlpha(themeDarken(body, 0.10), enabled ? body.alpha() : 130));
    }

    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawPath(path);

    QLinearGradient sheen(rect.topLeft(), rect.bottomLeft());
    sheen.setColorAt(0.0, themeWithAlpha(Qt::white, enabled ? (lightTheme ? 42 : 32) : 12));
    sheen.setColorAt(0.22, QColor(0, 0, 0, 0));
    sheen.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.setBrush(sheen);
    p.drawPath(path);
}

void drawAccentBorder(QPainter &p, const QRectF &bodyRect, const QPainterPath &path,
                      const QColor &accent, bool enabled, bool hover)
{
    if (!accent.isValid())
        return;

    const struct Pass
    {
        qreal inset;
        qreal width;
        int   alpha;
    } passes[] = {
        {0.0, hover ? 4.8 : 4.2, enabled ? 24 : 10},
        {0.0, hover ? 2.8 : 2.4, enabled ? 52 : 18},
        {0.0, hover ? 1.6 : 1.3, enabled ? 110 : 40},
    };

    for (const Pass &pass : passes) {
        QPen pen(themeWithAlpha(accent, pass.alpha));
        pen.setWidthF(pass.width);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);
    }

    QPen core(themeWithAlpha(themeLighten(accent, 0.22), enabled ? (hover ? 230 : 200) : 70));
    core.setWidthF(0.9);
    p.setPen(core);
    p.drawPath(path);
}

void drawOuterStroke(QPainter &p, const QRectF &bodyRect, const QColor &accent, bool enabled)
{
    const QRectF outer = bodyRect.adjusted(-0.5, -0.5, 0.5, 0.5);
    QPen         pen(themeWithAlpha(accent.isValid() ? themeLighten(accent, 0.55) : Qt::white,
                                  enabled ? 55 : 22));
    pen.setWidthF(0.8);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(outer, pillRadius(outer), pillRadius(outer));
}

void drawCenterGlow(QPainter &p, const QRectF &bodyRect, const QPainterPath &clipPath,
                    const QColor &accent, bool enabled, bool hover, bool lightTheme)
{
    if (!accent.isValid() || !enabled)
        return;

    p.save();
    p.setClipPath(clipPath);
    const QPointF c = bodyRect.center();
    const qreal   r = bodyRect.height() * 0.55;
    QRadialGradient grad(c, r);
    const int peak = lightTheme ? (hover ? 22 : 14) : (hover ? 42 : 28);
    grad.setColorAt(0.0, themeWithAlpha(accent, peak));
    grad.setColorAt(0.55, themeWithAlpha(accent, peak / 4));
    grad.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawEllipse(c, r, r * 0.65);
    p.restore();
}

void drawChevron(QPainter &p, const QPointF &tip, qreal halfHeight, bool pointRight, const QPen &pen)
{
    p.setPen(pen);
    if (pointRight) {
        const QPointF back(tip.x() - halfHeight * 1.15, tip.y());
        p.drawLine(QPointF(back.x(), back.y() - halfHeight), tip);
        p.drawLine(tip, QPointF(back.x(), back.y() + halfHeight));
    } else {
        const QPointF back(tip.x() + halfHeight * 1.15, tip.y());
        p.drawLine(QPointF(back.x(), back.y() - halfHeight), tip);
        p.drawLine(tip, QPointF(back.x(), back.y() + halfHeight));
    }
}

void drawDecorations(QPainter &p, const QRectF &bodyRect, const QRectF &textRect,
                     const QColor &accent, bool enabled, bool hover, bool lightTheme)
{
    if (!accent.isValid())
        return;

    const QColor lineColor =
        lightTheme ? themeDarken(accent, 0.22) : themeLighten(accent, 0.08);
    const qreal cy    = bodyRect.center().y();
    const int   alpha = enabled ? (hover ? 215 : 185) : 70;
    QPen        linePen(themeWithAlpha(lineColor, alpha));
    linePen.setWidthF(1.1);
    linePen.setCapStyle(Qt::FlatCap);
    p.setPen(linePen);

    const qreal chevronHalf = bodyRect.height() * 0.11;
    const qreal gap         = bodyRect.height() * 0.20;
    const qreal edgeInset   = bodyRect.height() * 0.30;

    const qreal leftLineEnd   = textRect.left() - gap;
    const qreal leftChevronX  = bodyRect.left() + edgeInset;
    const qreal rightLineStart = textRect.right() + gap;
    const qreal rightChevronX = bodyRect.right() - edgeInset;

    if (leftLineEnd > leftChevronX + chevronHalf * 2.5) {
        const QPointF tip(leftChevronX + chevronHalf * 1.15, cy);
        drawChevron(p, tip, chevronHalf, true, linePen);
        p.drawLine(QPointF(tip.x(), cy), QPointF(leftLineEnd, cy));
    }

    if (rightChevronX > rightLineStart + chevronHalf * 2.5) {
        const QPointF tip(rightChevronX - chevronHalf * 1.15, cy);
        drawChevron(p, tip, chevronHalf, false, linePen);
        p.drawLine(QPointF(rightLineStart, cy), QPointF(tip.x(), cy));
    }
}

void drawLabel(QPainter &p, const QRectF &bodyRect, const QString &text,
               const CyberPillPaintOptions &opt)
{
    if (text.isEmpty())
        return;

    QRectF textRect = bodyRect;
    if (opt.pressed)
        textRect.translate(0.0, 0.5);

    QFont font = p.font();
    font.setPixelSize(opt.fontPixelSize);
    font.setBold(false);
    p.setFont(font);

    p.setPen(resolveLabelColor(opt));
    p.drawText(textRect, Qt::AlignCenter, text);
}

void paintCyberPillButton(QPainter &p, const CyberPillPaintOptions &opt)
{
    if (!opt.bodyRect.isValid())
        return;

    QRectF bodyRect = opt.bodyRect;
    if (opt.pressed)
        bodyRect.translate(0.0, 0.5);

    const QPainterPath path = pillPath(bodyRect);
    const QColor       body = resolveBodyFill(opt);

    drawBody(p, bodyRect, path, body, opt.enabled, opt.pressed, opt.lightTheme);
    drawCenterGlow(p, bodyRect, path, opt.accent, opt.enabled, opt.hover, opt.lightTheme);
    drawAccentBorder(p, bodyRect, path, opt.accent, opt.enabled, opt.hover);
    drawOuterStroke(p, bodyRect, opt.accent, opt.enabled);

    QFont font = p.font();
    font.setPixelSize(opt.fontPixelSize);
    font.setBold(false);
    p.setFont(font);
    const QFontMetricsF fm(font);
    const qreal         textW = opt.text.isEmpty() ? 0.0 : fm.horizontalAdvance(opt.text);
    const QRectF        textRect(bodyRect.center().x() - textW * 0.5, bodyRect.top(), textW,
                                 bodyRect.height());

    if (opt.decorationsEnabled)
        drawDecorations(p, bodyRect, textRect, opt.accent, opt.enabled, opt.hover, opt.lightTheme);

    drawLabel(p, bodyRect, opt.text, opt);
}

} // namespace

ThemedCyberPillButton::ThemedCyberPillButton(QWidget *parent)
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

ThemedCyberPillButton::ThemedCyberPillButton(const QString &text, QWidget *parent)
    : ThemedCyberPillButton(parent)
{
    setText(text);
}

void ThemedCyberPillButton::setPillHeight(int height)
{
    const int h = qMax(24, height);
    if (m_pillHeight == h)
        return;
    m_pillHeight = h;
    updateSizeFromContent();
    update();
}

void ThemedCyberPillButton::setMinPillWidth(int width)
{
    const int w = qMax(48, width);
    if (m_minPillWidth == w)
        return;
    m_minPillWidth = w;
    updateSizeFromContent();
    update();
}

void ThemedCyberPillButton::setHorizontalPadding(int padding)
{
    const int pad = qMax(8, padding);
    if (m_horizontalPadding == pad)
        return;
    m_horizontalPadding = pad;
    updateSizeFromContent();
    update();
}

void ThemedCyberPillButton::setAccentColor(const QColor &color)
{
    if (m_accentColor == color)
        return;
    m_accentColor = color;
    update();
}

void ThemedCyberPillButton::setBodyColor(const QColor &color)
{
    if (m_bodyColor == color)
        return;
    m_bodyColor = color;
    update();
}

void ThemedCyberPillButton::setTextColor(const QColor &color)
{
    if (m_textColor == color)
        return;
    m_textColor = color;
    update();
}

void ThemedCyberPillButton::setDecorationsEnabled(bool enabled)
{
    if (m_decorationsEnabled == enabled)
        return;
    m_decorationsEnabled = enabled;
    update();
}

void ThemedCyberPillButton::setText(const QString &text)
{
    if (this->text() == text)
        return;
    QAbstractButton::setText(text);
    updateSizeFromContent();
    update();
}

int ThemedCyberPillButton::pillContentWidth() const
{
    QFont font;
    font.setPixelSize(qMax(11, qRound(m_pillHeight * 0.39)));
    QFontMetricsF fm(font);
    const int     textW = text().isEmpty() ? 0 : qCeil(fm.horizontalAdvance(text()));
    return qMax(m_minPillWidth, textW + m_horizontalPadding * 2);
}

QSize ThemedCyberPillButton::sizeHint() const
{
    return QSize(pillContentWidth() + kOuterMarginH * 2, m_pillHeight + kOuterMarginV * 2);
}

QSize ThemedCyberPillButton::minimumSizeHint() const
{
    return sizeHint();
}

void ThemedCyberPillButton::bindTheme()
{
    bindThemeRefresh(this, [this]() { update(); });
}

void ThemedCyberPillButton::updateSizeFromContent()
{
    setMinimumSize(sizeHint());
}

QRectF ThemedCyberPillButton::pillRectInWidget() const
{
    const int contentW = pillContentWidth();
    const int availW   = width() - 2 * kOuterMarginH;
    const int pillW    = qMax(contentW, availW);
    const int x        = (width() - pillW) / 2;
    return QRectF(x, kOuterMarginV, pillW, m_pillHeight);
}

void ThemedCyberPillButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    const ThemeTokens tokens = ThemeTokens::resolve();
    const bool        enabled = isEnabled();
    const bool        hover   = enabled && underMouse();
    const bool        pressed = enabled && isDown();

    QColor accent = m_accentColor.isValid() ? m_accentColor : tokens.accent;
    if (hover && enabled)
        accent = themeLighten(accent, 0.08);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    CyberPillPaintOptions opt;
    opt.bodyRect           = pillRectInWidget();
    opt.text               = text();
    opt.accent             = accent;
    opt.bodyColor          = m_bodyColor;
    opt.textColor          = m_textColor;
    opt.surfaceElevated    = tokens.surfaceElevated;
    opt.lightTheme         = themeIsLightSurface(tokens.surfaceElevated);
    opt.enabled            = enabled;
    opt.hover              = hover;
    opt.pressed            = pressed;
    opt.decorationsEnabled = m_decorationsEnabled;
    opt.fontPixelSize      = qMax(11, qRound(m_pillHeight * 0.39));

    paintCyberPillButton(p, opt);
}

void ThemedCyberPillButton::changeEvent(QEvent *event)
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
void ThemedCyberPillButton::enterEvent(QEnterEvent *event)
#else
void ThemedCyberPillButton::enterEvent(QEvent *event)
#endif
{
    QAbstractButton::enterEvent(event);
    update();
}

void ThemedCyberPillButton::leaveEvent(QEvent *event)
{
    QAbstractButton::leaveEvent(event);
    update();
}

ThemedCyberPillButton *makeCyberPillButton(const QString &text, QWidget *parent,
                                           const QColor &accentColor)
{
    auto *btn = new ThemedCyberPillButton(text, parent);
    if (accentColor.isValid())
        btn->setAccentColor(accentColor);
    return btn;
}

} // namespace ui
