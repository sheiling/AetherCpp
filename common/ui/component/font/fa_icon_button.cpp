/**
 * @file fa_icon_button.cpp
 * @brief Font Awesome 与 ThemedIconButton 桥接
 */

#include "font/fa_icon_button.h"

#include "uibase/languagemanager.h"

namespace ui {

FaIcon faIconForTriangleDirection(TriangleDirection dir)
{
    switch (dir) {
    case TriangleDirection::Up:
        return FaIcon::ArrowUp;
    case TriangleDirection::Down:
        return FaIcon::ArrowDown;
    case TriangleDirection::Left:
        return FaIcon::ArrowLeft;
    case TriangleDirection::Right:
        return FaIcon::ArrowRight;
    }
    return FaIcon::ArrowUp;
}

void applyFaIcon(ThemedIconButton *btn, FaIcon icon, int pixelSize)
{
    if (!btn)
        return;
    if (pixelSize > 0)
        btn->setIconPixelSize(pixelSize);
    btn->setIconDrawFont(FontAwesome::font(btn->iconPixelSize()));
    btn->setIconGlyph(FontAwesome::glyph(icon));
}

void applyFaToggleIcons(ThemedIconButton *btn, FaIcon offIcon, FaIcon onIcon, int pixelSize)
{
    if (!btn)
        return;
    if (pixelSize > 0)
        btn->setIconPixelSize(pixelSize);
    btn->setIconDrawFont(FontAwesome::font(btn->iconPixelSize()));
    btn->setToggleIconGlyphs(FontAwesome::glyph(offIcon), FontAwesome::glyph(onIcon));
}

ThemedIconButton *makeFaIconButton(FaIcon icon, QWidget *parent, const QString &tooltipKey)
{
    auto *btn = new ThemedIconButton(parent);
    applyFaIcon(btn, icon);
    if (!tooltipKey.isEmpty())
        btn->setToolTip(TR(tooltipKey));
    return btn;
}

ThemedIconButton *makeFaIconButton(TriangleDirection dir, QWidget *parent,
                                   const QString &tooltipKey)
{
    return makeFaIconButton(faIconForTriangleDirection(dir), parent, tooltipKey);
}

} // namespace ui
