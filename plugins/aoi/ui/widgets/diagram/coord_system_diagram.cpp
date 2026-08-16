/**
 * @file coord_system_diagram.cpp
 * @brief CoordSystemDiagram 绘制逻辑：输入区画布、双轴箭头与原点标记
 *
 * ThemeTokens 用量：inputBackground / inputBorder（画布）、focus（轴线和箭头）、
 * inputBorderHover（原点圆）、highlightText（X/Y 标签）。
 */

#include "diagram/coord_system_diagram.h"

#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QtMath>

namespace aoi {

using namespace ui;

CoordSystemDiagram::CoordSystemDiagram(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("coordSystemDiagram"));
    setMinimumWidth(0);
    setMinimumHeight(80);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    bindTheme();
}

void CoordSystemDiagram::bindTheme()
{
    bindThemeRefresh(this, [this]() { update(); });
}

void CoordSystemDiagram::setCoordIndex(int idx)
{
    idx = qBound(0, idx, 3);
    if (m_idx == idx)
        return;
    m_idx = idx;
    update();
}

void CoordSystemDiagram::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

void CoordSystemDiagram::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF canvas = QRectF(rect());
    p.fillRect(canvas, t.inputBackground);
    p.setPen(QPen(t.inputBorder, 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(canvas.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);

    const QRectF r = canvas.adjusted(16, 14, -16, -14);
    if (r.width() < 8.0 || r.height() < 8.0)
        return;

    const qreal len = qMax(28.0, qMin(r.width(), r.height()) * 0.48);

    struct Spec
    {
        Qt::Corner corner;
        QPointF    xDir;
        QPointF    yDir;
    };
    static const Spec kSpecs[4] = {
        {Qt::BottomLeftCorner,  {1, 0}, {0, -1}},
        {Qt::BottomRightCorner, {-1, 0}, {0, -1}},
        {Qt::TopLeftCorner,     {1, 0}, {0, 1}},
        {Qt::TopRightCorner,    {-1, 0}, {0, 1}},
    };

    auto cornerPt = [&r](Qt::Corner c) -> QPointF {
        switch (c) {
        case Qt::TopLeftCorner:
            return r.topLeft();
        case Qt::TopRightCorner:
            return r.topRight();
        case Qt::BottomLeftCorner:
            return r.bottomLeft();
        case Qt::BottomRightCorner:
            return r.bottomRight();
        default:
            return r.center();
        }
    };

    auto drawAxis = [&](const QPointF &o, QPointF dir, qreal L, const QString &tag) {
        const qreal dl = QLineF(QPointF(), dir).length();
        if (dl < 1e-6)
            return;
        dir /= dl;
        const QPointF e = o + dir * L;

        QPen axisPen(t.focus, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(axisPen);
        p.drawLine(o, e);

        const qreal   ah = qBound(5.0, L * 0.13, 11.0);
        const QPointF back = -dir * ah;
        QPointF       perp(-dir.y(), dir.x());
        perp *= ah * 0.45;
        p.setBrush(t.focus);
        p.setPen(Qt::NoPen);
        p.drawPolygon(QPolygonF() << e << (e + back + perp) << (e + back - perp));

        QFont f = p.font();
        f.setPixelSize(12);
        f.setBold(true);
        p.setFont(f);
        p.setPen(t.highlightText);
        p.setBrush(Qt::NoBrush);

        QPointF mid = o + dir * (L * 0.55);
        QPointF tOff(-dir.y(), dir.x());
        const qreal tl = QLineF(QPointF(), tOff).length();
        if (tl > 1e-3)
            tOff = tOff / tl * 12.0;
        p.drawText(QRectF(mid + tOff - QPointF(8, 8), QSizeF(16, 16)), Qt::AlignCenter, tag);
    };

    const Spec &sp = kSpecs[m_idx];
    QPointF      xd = sp.xDir;
    QPointF      yd = sp.yDir;
    const qreal  lx = QLineF(QPointF(), xd).length();
    const qreal  ly = QLineF(QPointF(), yd).length();
    if (lx > 1e-6)
        xd /= lx;
    if (ly > 1e-6)
        yd /= ly;

    const QPointF O  = cornerPt(sp.corner);
    const QPointF P1 = O + xd * len;
    const QPointF P2 = O + yd * len;
    const qreal   minx = qMin(O.x(), qMin(P1.x(), P2.x()));
    const qreal   maxx = qMax(O.x(), qMax(P1.x(), P2.x()));
    const qreal   miny = qMin(O.y(), qMin(P1.y(), P2.y()));
    const qreal   maxy = qMax(O.y(), qMax(P1.y(), P2.y()));
    QRectF        bbox(minx, miny, maxx - minx, maxy - miny);
    bbox.adjust(-14, -14, 14, 14);
    const QPointF delta = r.center() - bbox.center();
    const QPointF Odraw = O + delta;

    p.setPen(QPen(t.inputBorderHover, 1.5));
    p.setBrush(t.inputBackground);
    p.drawEllipse(Odraw, 3.5, 3.5);

    drawAxis(Odraw, sp.xDir, len, QStringLiteral("X"));
    drawAxis(Odraw, sp.yDir, len, QStringLiteral("Y"));
}

} // namespace aoi
