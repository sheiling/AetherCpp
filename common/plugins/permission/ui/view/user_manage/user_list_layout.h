#pragma once

#include <QRect>

namespace user_list_layout {

constexpr int kCheckWidth    = 44;
constexpr int kActionsWidth  = 248;
constexpr int kHeaderHeight  = 40;
constexpr int kRowHeight     = 48;

struct ColumnRects {
    QRect check;
    QRect username;
    QRect displayName;
    QRect roles;
    QRect department;
    QRect statusColumn;
    QRect lastLogin;
    QRect actions;
};

inline ColumnRects columnRects(int totalWidth)
{
    ColumnRects c;
    int x = 0;
    c.check = QRect(x, 0, kCheckWidth, 0);
    x += kCheckWidth;

    const int actionsW = qMin(kActionsWidth, qMax(180, totalWidth / 5));
    const int remain   = qMax(0, totalWidth - kCheckWidth - actionsW);
    const int wUser  = remain * 14 / 100;
    const int wName  = remain * 14 / 100;
    const int wRole  = remain * 24 / 100;
    const int wDept  = remain * 16 / 100;
    const int wStat  = remain * 10 / 100;
    const int wLogin = remain - wUser - wName - wRole - wDept - wStat;

    c.username    = QRect(x, 0, wUser, 0);
    x += wUser;
    c.displayName = QRect(x, 0, wName, 0);
    x += wName;
    c.roles       = QRect(x, 0, wRole, 0);
    x += wRole;
    c.department  = QRect(x, 0, wDept, 0);
    x += wDept;
    c.statusColumn = QRect(x, 0, wStat, 0);
    x += wStat;
    c.lastLogin   = QRect(x, 0, wLogin, 0);
    x += wLogin;
    c.actions     = QRect(x, 0, actionsW, 0);
    return c;
}

inline QRect rowRect(const QRect &col, int y, int h)
{
    return QRect(col.x(), y, col.width(), h);
}

} // namespace user_list_layout
