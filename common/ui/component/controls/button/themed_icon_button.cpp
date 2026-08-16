/**
 * @file themed_icon_button.cpp
 * @brief ThemedIconButton：自绘 glyph、三种布局、语义色
 */

#include "controls/button/themed_icon_button.h"

#include "theme/theme_font.h"
#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include "uibase/languagemanager.h"

#include <QApplication>
#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QStyleOptionButton>

namespace ui {

namespace {

constexpr int kIconOnlySize   = 28;
constexpr int kContentPadding = 4;
constexpr int kIconTextGap    = 2;

} // namespace

QColor resolveIconSemanticColor(IconSemanticColor role)
{
    const ThemeTokens t = ThemeTokens::resolve();
    switch (role) {
    case IconSemanticColor::Foreground:
        return t.textPrimary;
    case IconSemanticColor::Red:
        return t.dangerText.isValid() ? t.dangerText : htmlFallbackDanger();
    case IconSemanticColor::Yellow:
        return t.warningText.isValid() ? t.warningText : htmlFallbackWarning();
    case IconSemanticColor::Green:
        return t.successText.isValid() ? t.successText : htmlFallbackSuccess();
    case IconSemanticColor::LightGreen:
        return QColor(QStringLiteral("#7BC67E"));
    case IconSemanticColor::Orange:
        return QColor(QStringLiteral("#FF8C42"));
    }
    return t.textPrimary;
}

ThemedIconButton::ThemedIconButton(QWidget *parent)
    : QPushButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    bindTheme();
    refreshStyle();
    syncButtonChrome();
    connect(this, &QPushButton::toggled, this, [this](bool) { update(); });
}

ThemedIconButton::ThemedIconButton(const QString &glyph, QWidget *parent)
    : ThemedIconButton(parent)
{
    setGlyph(glyph);
}

void ThemedIconButton::setGlyph(const QString &glyph)
{
    setIconDrawFont(QFont());
    setIconGlyph(glyph);
}

void ThemedIconButton::setIconGlyph(const QString &glyph)
{
    m_iconGlyph        = glyph;
    m_iconGlyphChecked = QString();
    m_hasToggleGlyphs  = false;
    syncButtonChrome();
    updateGeometry();
    update();
}

void ThemedIconButton::setToggleIconGlyphs(const QString &offGlyph, const QString &onGlyph)
{
    m_iconGlyph        = offGlyph;
    m_iconGlyphChecked = onGlyph;
    m_hasToggleGlyphs  = true;
    syncButtonChrome();
    updateGeometry();
    update();
}

void ThemedIconButton::setIconDrawFont(const QFont &font)
{
    m_iconDrawFont       = font;
    m_useCustomIconFont  = !font.family().isEmpty();
    update();
}

void ThemedIconButton::setLayout(IconButtonLayout layout)
{
    if (m_layout == layout)
        return;
    m_layout = layout;
    refreshStyle();
    updateGeometry();
    update();
}

void ThemedIconButton::setCaption(const QString &text)
{
    if (m_caption == text)
        return;
    m_caption = text;
    updateGeometry();
    update();
}

void ThemedIconButton::setIconPixelSize(int px)
{
    m_iconPixelSize = qMax(8, px);
    if (m_useCustomIconFont) {
        m_iconDrawFont.setPixelSize(m_iconPixelSize);
    }
    updateGeometry();
    update();
}

void ThemedIconButton::setIconColor(const QColor &color)
{
    m_iconColor          = color;
    m_useCustomIconColor = color.isValid();
    m_iconSemanticColor  = IconSemanticColor::Foreground;
    update();
}

void ThemedIconButton::setIconSemanticColor(IconSemanticColor role)
{
    m_iconSemanticColor  = role;
    m_useCustomIconColor = false;
    m_iconColor          = QColor();
    update();
}

QColor ThemedIconButton::resolvedIconColor() const
{
    if (m_useCustomIconColor && m_iconColor.isValid())
        return m_iconColor;
    if (m_iconSemanticColor != IconSemanticColor::Foreground)
        return resolveIconSemanticColor(m_iconSemanticColor);

    const ThemeTokens t = ThemeTokens::resolve();
    if (t.textPrimary.isValid())
        return t.textPrimary;
    if (qApp)
        return qApp->palette().color(QPalette::ButtonText);
    return QColor(QStringLiteral("#b8cde8"));
}

QString ThemedIconButton::activeIconGlyph() const
{
    if (m_hasToggleGlyphs && isCheckable() && isChecked())
        return m_iconGlyphChecked;
    return m_iconGlyph;
}

QFont ThemedIconButton::iconDrawFont() const
{
    if (m_useCustomIconFont)
        return m_iconDrawFont;
    QFont font = themeUiFont(9);
    font.setPixelSize(m_iconPixelSize);
    return font;
}

void ThemedIconButton::syncButtonChrome()
{
    setText(QString());
    setIcon(QIcon());
}

void ThemedIconButton::bindTheme()
{
    bindThemeRefresh(this, [this]() {
        refreshStyle();
        update();
    });
}

void ThemedIconButton::refreshStyle()
{
    IconButtonStyleMetrics metrics;
    if (m_layout == IconButtonLayout::IconOnly)
        metrics.size = kIconOnlySize;
    else
        metrics.size = qMax(kIconOnlySize, m_iconPixelSize + kContentPadding * 2 + 8);
    metrics.fontSize = qMax(9, m_iconPixelSize - 2);
    setStyleSheet(buildIconButtonStylesheet(metrics));
}

QSize ThemedIconButton::sizeHint() const
{
    if (m_layout == IconButtonLayout::IconOnly)
        return QSize(kIconOnlySize, kIconOnlySize);

    const QFont captionFont = themeUiFont(9);
    QFontMetrics fm(captionFont);
    const int iconSide = m_iconPixelSize + kContentPadding;

    if (m_layout == IconButtonLayout::IconTopTextBottom) {
        const int textW = m_caption.isEmpty() ? 0 : fm.horizontalAdvance(m_caption);
        const int textH = m_caption.isEmpty() ? 0 : fm.height();
        const int w     = qMax(iconSide, textW) + kContentPadding * 2;
        const int h     = iconSide + (textH > 0 ? kIconTextGap + textH : 0) + kContentPadding * 2;
        return QSize(qMax(w, kIconOnlySize), qMax(h, kIconOnlySize));
    }

    const int textW = m_caption.isEmpty() ? 0 : fm.horizontalAdvance(m_caption);
    const int textH = m_caption.isEmpty() ? 0 : fm.height();
    const int w     = iconSide + (textW > 0 ? kIconTextGap + textW : 0) + kContentPadding * 2;
    const int h     = qMax(iconSide, textH) + kContentPadding * 2;
    return QSize(qMax(w, kIconOnlySize), qMax(h, kIconOnlySize));
}

QSize ThemedIconButton::minimumSizeHint() const
{
    return sizeHint();
}

void ThemedIconButton::paintContent(QPainter &p, const QRect &contentRect, const ThemeTokens &tokens,
                                    bool enabled) const
{
    const QString glyphText = activeIconGlyph();
    if (glyphText.isEmpty())
        return;

    const QColor iconColor =
        enabled ? resolvedIconColor()
                : (tokens.textSecondary.isValid() ? tokens.textSecondary : resolvedIconColor());

    const QFont iconFont = iconDrawFont();
    const QFont captionFont = themeUiFont(9);
    QFontMetrics iconFm(iconFont);
    QFontMetrics captionFm(captionFont);

    if (m_layout == IconButtonLayout::IconOnly) {
        p.setFont(iconFont);
        p.setPen(iconColor);
        p.drawText(contentRect, Qt::AlignCenter, glyphText);
        return;
    }

    if (m_layout == IconButtonLayout::IconTopTextBottom) {
        const int iconH = iconFm.height();
        const int textH = m_caption.isEmpty() ? 0 : captionFm.height();
        const int totalH = iconH + (textH > 0 ? kIconTextGap + textH : 0);
        int y = contentRect.y() + (contentRect.height() - totalH) / 2;

        QRect iconRect(contentRect.x(), y, contentRect.width(), iconH);
        p.setFont(iconFont);
        p.setPen(iconColor);
        p.drawText(iconRect, Qt::AlignHCenter | Qt::AlignVCenter, glyphText);
        y += iconH;

        if (!m_caption.isEmpty()) {
            y += kIconTextGap;
            QRect textRect(contentRect.x(), y, contentRect.width(), textH);
            p.setFont(captionFont);
            p.setPen(enabled ? tokens.textPrimary : tokens.textSecondary);
            p.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter, m_caption);
        }
        return;
    }

    const int iconW = iconFm.horizontalAdvance(glyphText) + 2;
    const int iconH = iconFm.height();
    const int textW = m_caption.isEmpty() ? 0 : captionFm.horizontalAdvance(m_caption);
    const int textH = m_caption.isEmpty() ? 0 : captionFm.height();
    const int totalW = iconW + (textW > 0 ? kIconTextGap + textW : 0);
    const int totalH = qMax(iconH, textH);
    int x = contentRect.x() + (contentRect.width() - totalW) / 2;
    const int y = contentRect.y() + (contentRect.height() - totalH) / 2;

    p.setFont(iconFont);
    p.setPen(iconColor);
    p.drawText(QRect(x, y, iconW, totalH), Qt::AlignLeft | Qt::AlignVCenter, glyphText);
    x += iconW;

    if (!m_caption.isEmpty()) {
        x += kIconTextGap;
        p.setFont(captionFont);
        p.setPen(enabled ? tokens.textPrimary : tokens.textSecondary);
        p.drawText(QRect(x, y, textW, totalH), Qt::AlignLeft | Qt::AlignVCenter, m_caption);
    }
}

void ThemedIconButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOptionButton opt;
    initStyleOption(&opt);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    style()->drawControl(QStyle::CE_PushButton, &opt, &p, this);

    const ThemeTokens tokens = ThemeTokens::resolve();
    paintContent(p, rect().adjusted(kContentPadding, kContentPadding, -kContentPadding,
                                    -kContentPadding),
                 tokens, isEnabled());
}

void ThemedIconButton::changeEvent(QEvent *event)
{
    QPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
        refreshStyle();
    if (event->type() == QEvent::LanguageChange)
        updateGeometry();
}

ThemedIconButton *makeIconButton(const QString &glyph, QWidget *parent,
                                 const QString &tooltipKey)
{
    auto *btn = new ThemedIconButton(glyph, parent);
    if (!tooltipKey.isEmpty())
        btn->setToolTip(TR(tooltipKey));
    return btn;
}

} // namespace ui
