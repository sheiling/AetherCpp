/**
 * @file font_awesome.cpp
 * @brief Font Awesome 5 Solid：QFontDatabase + QPainter 生成 QIcon
 */

#include "font/font_awesome.h"

#include "uibase/thememanager.h"

#include <QApplication>
#include <QDebug>
#include <QFontDatabase>
#include <QPalette>
#include <QPainter>
#include <QPixmap>

namespace ui {

namespace {

int g_fontId = -1;

QPixmap renderGlyph(char16_t codepoint, const QSize &size, const QColor &color)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QFont font = FontAwesome::font(size.height());
    painter.setFont(font);
    painter.setPen(color);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QString(QChar(codepoint)));
    return pixmap;
}

} // namespace

void FontAwesome::ensureLoaded()
{
    if (g_fontId >= 0)
        return;

    g_fontId = QFontDatabase::addApplicationFont(
        QStringLiteral(":/resources/fonts/fa-solid-900.ttf"));
    if (g_fontId < 0)
        qWarning() << "[FontAwesome] failed to load :/resources/fonts/fa-solid-900.ttf";
}

QString FontAwesome::familyName()
{
    ensureLoaded();
    if (g_fontId < 0)
        return QString();

    const QStringList families = QFontDatabase::applicationFontFamilies(g_fontId);
    return families.isEmpty() ? QString() : families.first();
}

QFont FontAwesome::font(int pixelSize)
{
    QFont font(familyName());
    font.setPixelSize(qMax(8, pixelSize));
    return font;
}

QString FontAwesome::glyph(FaIcon icon)
{
    if (icon == FaIcon::None)
        return QString();
    return QString(QChar(static_cast<char16_t>(icon)));
}

QColor FontAwesome::defaultIconColor()
{
    if (ThemeManager *tm = ThemeManager::instance(); tm && tm->isInitialized()) {
        const QColor themed = tm->themeColor(QStringLiteral("secondaryTextColor"));
        if (themed.isValid())
            return themed;
    }
    if (qApp)
        return qApp->palette().color(QPalette::ButtonText);
    return QColor(QStringLiteral("#b8cde8"));
}

QIcon FontAwesome::icon(FaIcon glyphIcon, const QSize &size, const QColor &color)
{
    const QColor pen = color.isValid() ? color : defaultIconColor();
    return QIcon(renderGlyph(static_cast<char16_t>(glyphIcon), size, pen));
}

QIcon FontAwesome::toggleIcon(FaIcon offGlyph, FaIcon onGlyph, const QSize &size,
                              const QColor &color)
{
    const QColor pen = color.isValid() ? color : defaultIconColor();
    QIcon icon;
    icon.addPixmap(renderGlyph(static_cast<char16_t>(offGlyph), size, pen), QIcon::Normal,
                   QIcon::Off);
    icon.addPixmap(renderGlyph(static_cast<char16_t>(onGlyph), size, pen), QIcon::Normal,
                   QIcon::On);
    return icon;
}

} // namespace ui
