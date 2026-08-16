/**
 * @file pill_switch.cpp
 * @brief PillSwitch 自绘实现：轨道/滑块 ThemeTokens 与 toggled 信号
 */

#include "controls/switch/pill_switch.h"

#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include <QMouseEvent>
#include <QMarginsF>
#include <QPainter>
#include <QPaintEvent>
#include <QSizePolicy>

namespace ui {

PillSwitch::PillSwitch(bool checked, QWidget *parent)
    : QWidget(parent)
    , m_checked(checked)
{
    setFixedSize(52, 26);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);

    bindThemeRefresh(this, [this]() { update(); });
}

bool PillSwitch::isChecked() const
{
    return m_checked;
}

void PillSwitch::setChecked(bool on)
{
    if (m_checked == on)
        return;
    m_checked = on;
    update();
    emit toggled(m_checked);
}

void PillSwitch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    const ThemeTokens t = ThemeTokens::resolve();

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF outer = QRectF(rect());
    const QRectF r     = outer.adjusted(1.0, 1.0, -1.0, -1.0);
    const qreal  rad   = qMax(1.0, r.height() / 2.0);

    QPen pen(m_checked ? t.switchBorderOn : t.switchBorderOff, 1.0);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.setBrush(m_checked ? t.switchTrackOn : t.switchTrackOff);
    p.drawRoundedRect(r, rad, rad);

    const qreal  strokeW = pen.widthF();
    const QRectF inner =
        r.marginsRemoved(QMarginsF(strokeW * 0.5, strokeW * 0.5, strokeW * 0.5, strokeW * 0.5));
    const qreal padV   = 3.0;
    const qreal thumbD = qMax(8.0, inner.height() - 2.0 * padV);
    const qreal tr     = thumbD * 0.5;
    const qreal cy     = inner.top() + inner.height() * 0.5;
    const qreal padH   = 3.0;
    const qreal cx     = m_checked ? (inner.right() - padH - tr) : (inner.left() + padH + tr);

    p.setPen(Qt::NoPen);
    p.setBrush(t.switchThumb);
    p.drawEllipse(QPointF(cx, cy), tr, tr);
}

void PillSwitch::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && rect().contains(e->pos())) {
        m_checked = !m_checked;
        update();
        emit toggled(m_checked);
    }
    QWidget::mouseReleaseEvent(e);
}

QSize PillSwitch::sizeHint() const
{
    return QSize(52, 26);
}

QSize PillSwitch::minimumSizeHint() const
{
    return QSize(52, 26);
}

PillSwitch *makePillSwitch(bool checked, QWidget *parent)
{
    return new PillSwitch(checked, parent);
}

} // namespace ui
