/**
 * @file spinbox_paint_utils.cpp
 * @brief paintTriangleArrow / paintThemedSpinArrows 实现
 */

#include "controls/spinbox/spinbox_paint_utils.h"

#include "theme/theme_tokens.h"

#include <QAbstractSpinBox>
#include <QPainter>
#include <QPolygon>
#include <QStyle>
#include <QStyleOptionSpinBox>

namespace ui {

void paintTriangleArrow(QPainter &p, const QRect &rect, TriangleDirection dir, bool enabled)
{
    if (!rect.isValid())
        return;

    const ThemeTokens t     = ThemeTokens::resolve();
    const QColor      arrow = enabled ? t.arrowActive : t.arrowDisabled;
    const int         cx    = rect.center().x();
    const int         cy    = rect.center().y();

    // 上下箭头镜像对称：半宽 4px、半高 3px（总约 8×6），避免 Up 比 Down 矮一像素
    QPolygon tri;
    switch (dir) {
    case TriangleDirection::Up:
        tri << QPoint(cx, cy - 3) << QPoint(cx - 4, cy + 3) << QPoint(cx + 4, cy + 3);
        break;
    case TriangleDirection::Down:
        tri << QPoint(cx - 4, cy - 3) << QPoint(cx + 4, cy - 3) << QPoint(cx, cy + 3);
        break;
    case TriangleDirection::Left:
        tri << QPoint(cx - 3, cy) << QPoint(cx + 3, cy - 4) << QPoint(cx + 3, cy + 4);
        break;
    case TriangleDirection::Right:
        tri << QPoint(cx + 3, cy) << QPoint(cx - 3, cy - 4) << QPoint(cx - 3, cy + 4);
        break;
    }

    p.setPen(Qt::NoPen);
    p.setBrush(arrow);
    p.drawPolygon(tri);
}

void paintThemedSpinArrows(QAbstractSpinBox *spin, const QStyleOptionSpinBox &opt)
{
    if (!spin || spin->buttonSymbols() == QAbstractSpinBox::NoButtons)
        return;

    const ThemeTokens t = ThemeTokens::resolve();

    QPainter p(spin);
    p.setRenderHint(QPainter::Antialiasing, true);

    const auto drawOne = [&](QStyle::SubControl sc, TriangleDirection dir) {
        const QRect r = spin->style()->subControlRect(QStyle::CC_SpinBox, &opt, sc, spin);
        if (!r.isValid())
            return;
        const QRect fill = r.adjusted(1, 1, -1, -1);
        if (fill.width() <= 0 || fill.height() <= 0)
            return;
        p.fillRect(fill, t.spinButtonBg);
        paintTriangleArrow(p, r, dir, spin->isEnabled());
    };

    drawOne(QStyle::SC_SpinBoxUp, TriangleDirection::Up);
    drawOne(QStyle::SC_SpinBoxDown, TriangleDirection::Down);
}

} // namespace ui
