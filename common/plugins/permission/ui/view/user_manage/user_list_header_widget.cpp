#include "ui/view/user_manage/user_list_header_widget.h"

#include "ui/view/permission_ui_utils.h"

#include "theme/theme_tokens.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

UserListHeaderWidget::UserListHeaderWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(user_list_layout::kHeaderHeight);
    setAttribute(Qt::WA_StyledBackground, true);
}

void UserListHeaderWidget::setHeaders(const QStringList &headers)
{
    m_headers = headers;
    update();
}

void UserListHeaderWidget::setChecked(bool checked)
{
    if (m_checked == checked)
        return;
    m_checked = checked;
    update();
}

void UserListHeaderWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!event || event->button() != Qt::LeftButton)
        return;

    const auto cols       = user_list_layout::columnRects(width());
    const QRect checkRect = user_list_layout::rowRect(cols.check, 0, height());
    if (!checkRect.contains(event->pos()))
        return;

    m_checked = !m_checked;
    update();
    emit checkToggled(m_checked);
}

void UserListHeaderWidget::paintEvent(QPaintEvent *)
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    QPainter              p(this);
    p.fillRect(rect(), t.headerBg);

    const auto cols = user_list_layout::columnRects(width());
    const int y     = 0;
    const int h     = height();

    paintCheckbox(&p, user_list_layout::rowRect(cols.check, y, h), m_checked);

    QFont f = font();
    f.setPixelSize(13);
    f.setWeight(QFont::DemiBold);
    p.setFont(f);
    p.setPen(t.textSecondary);

    const QVector<QRect> rects = {
        user_list_layout::rowRect(cols.check, y, h),
        user_list_layout::rowRect(cols.username, y, h),
        user_list_layout::rowRect(cols.displayName, y, h),
        user_list_layout::rowRect(cols.roles, y, h),
        user_list_layout::rowRect(cols.department, y, h),
        user_list_layout::rowRect(cols.statusColumn, y, h),
        user_list_layout::rowRect(cols.lastLogin, y, h),
        user_list_layout::rowRect(cols.actions, y, h),
    };

    for (int i = 0; i < m_headers.size() && i < rects.size(); ++i) {
        if (m_headers.at(i).isEmpty())
            continue;
        const Qt::Alignment align =
            (i == static_cast<int>(rects.size()) - 1 || i == 5) ? Qt::AlignCenter
                                                                  : Qt::AlignVCenter | Qt::AlignLeft;
        const QRect r = rects.at(i).adjusted(8, 0, -4, 0);
        p.drawText(r, static_cast<int>(align), m_headers.at(i));
    }

    p.setPen(t.divider);
    p.drawLine(0, height() - 1, width(), height() - 1);
}

void UserListHeaderWidget::paintCheckbox(QPainter *painter, const QRect &rect, bool checked) const
{
    permission_ui::paintPermissionCheckbox(painter, rect, checked);
}
