#include "recipe_pill_switch.h"

#include <QMouseEvent>
#include <QPainter>

namespace motion {

RecipePillSwitch::RecipePillSwitch(bool checked, QWidget *parent, int trackHeight)
    : QWidget(parent)
    , m_checked(checked)
    , m_trackH(qMax(22, trackHeight))
{
    const int w = m_trackH * 2 + 8;
    setFixedSize(w, m_trackH);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
}

bool RecipePillSwitch::isChecked() const { return m_checked; }

void RecipePillSwitch::setChecked(bool on)
{
    if (m_checked == on)
        return;
    m_checked = on;
    update();
}

void RecipePillSwitch::setOnToggled(std::function<void(bool)> handler)
{
    m_onToggled = std::move(handler);
}

QSize RecipePillSwitch::sizeHint() const { return QSize(m_trackH * 2 + 8, m_trackH); }

QSize RecipePillSwitch::minimumSizeHint() const { return sizeHint(); }

void RecipePillSwitch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF outer = QRectF(rect());
    const QRectF r     = outer.adjusted(1.0, 1.0, -1.0, -1.0);
    const qreal  rad   = qMax(1.0, r.height() / 2.0);

    const QColor borderOff(0x4a, 0x60, 0x78);
    const QColor borderOn(0x64, 0xb5, 0xf6);
    const QColor offTrack(0x1e, 0x2d, 0x40);
    const QColor onTrack(0x1a, 0x4a, 0x8c);

    QPen pen(m_checked ? borderOn : borderOff, 1.0);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.setBrush(m_checked ? onTrack : offTrack);
    p.drawRoundedRect(r, rad, rad);

    const qreal  strokeW = pen.widthF();
    const QRectF inner   = r.marginsRemoved(QMarginsF(strokeW * 0.5, strokeW * 0.5, strokeW * 0.5, strokeW * 0.5));
    const qreal  padV    = (m_trackH <= 24) ? 0.5 : 3.0;
    const qreal  padH    = (m_trackH <= 24) ? 1.0 : 3.0;
    const qreal  thumbD  = qMax(3.0, inner.height() - 2.0 * padV);
    const qreal  tr      = thumbD * 0.5;
    const qreal  cy      = inner.top() + inner.height() * 0.5;
    const qreal  cx      = m_checked ? (inner.right() - padH - tr) : (inner.left() + padH + tr);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0xf5, 0xf8, 0xfc));
    p.drawEllipse(QPointF(cx, cy), tr, tr);
}

void RecipePillSwitch::mouseReleaseEvent(QMouseEvent *e)
{
    if (!isEnabled()) {
        QWidget::mouseReleaseEvent(e);
        return;
    }
    if (e->button() == Qt::LeftButton && rect().contains(e->pos())) {
        m_checked = !m_checked;
        update();
        if (m_onToggled)
            m_onToggled(m_checked);
    }
    QWidget::mouseReleaseEvent(e);
}

} // namespace motion
