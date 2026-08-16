#include "ui/view/user_manage/user_row_delegate.h"

#include "service/perm_action_keys.h"
#include "ui/view/permission_ui_utils.h"
#include "uibase/permission/permservice.h"

#include "theme/theme_color_math.h"
#include "theme/theme_tokens.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

using namespace user_list_layout;

namespace {

constexpr int kBadgeH     = 22;
constexpr int kBadgePadX  = 10;
constexpr int kBtnH       = 22;
constexpr int kBtnPadX      = 10;
constexpr int kBtnSpacing   = 6;

} // namespace

UserRowDelegate::UserRowDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void UserRowDelegate::setActionLabels(const QString &edit, const QString &resetPassword,
                                      const QString &enable, const QString &disable)
{
    m_lblEdit    = edit;
    m_lblReset   = resetPassword;
    m_lblEnable  = enable;
    m_lblDisable = disable;
}

QSize UserRowDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QSize(option.rect.width(), kRowHeight);
}

void UserRowDelegate::paintCheckbox(QPainter *painter, const QRect &rect, bool checked) const
{
    permission_ui::paintPermissionCheckbox(painter, rect, checked);
}

void UserRowDelegate::paintText(QPainter *painter, const QRect &rect, const QString &text,
                                const QColor &color, Qt::Alignment align) const
{
    if (text.isEmpty())
        return;
    painter->save();
    painter->setPen(color);
    const QString elided =
        painter->fontMetrics().elidedText(text, Qt::ElideRight, qMax(0, rect.width()));
    painter->drawText(rect, static_cast<int>(align) | Qt::TextSingleLine, elided);
    painter->restore();
}

void UserRowDelegate::paintRoleBadges(QPainter *painter, const QRect &rect,
                                      const QVariantList &roles) const
{
    if (roles.isEmpty()) {
        const ui::ThemeTokens t = ui::ThemeTokens::resolve();
        paintText(painter, rect.adjusted(8, 0, -4, 0), QStringLiteral("—"), t.textSecondary,
                  Qt::AlignVCenter | Qt::AlignLeft);
        return;
    }

    QFontMetrics fm(painter->font());
    QVector<QPair<QString, QString>> badges;
    int totalW = 0;
    for (const QVariant &v : roles) {
        const QVariantMap map = v.toMap();
        const QString name  = map.value(QStringLiteral("name")).toString();
        const QString color = map.value(QStringLiteral("color")).toString();
        const int w         = fm.horizontalAdvance(name) + kBadgePadX * 2;
        badges.append({name, color});
        totalW += w + kBtnSpacing;
    }
    if (!badges.isEmpty())
        totalW -= kBtnSpacing;

    int x       = rect.x() + 8;
    const int y = rect.y() + (rect.height() - kBadgeH) / 2;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    for (const auto &badge : badges) {
        const QString bg = badge.second.isEmpty() ? QStringLiteral("#6b7280") : badge.second;
        const int w      = fm.horizontalAdvance(badge.first) + kBadgePadX * 2;
        const QRect pill(x, y, w, kBadgeH);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(bg));
        painter->drawRoundedRect(pill, kBadgeH / 2, kBadgeH / 2);
        painter->setPen(QColor("#ffffff"));
        painter->drawText(pill, Qt::AlignCenter, badge.first);
        x += w + kBtnSpacing;
    }
    painter->restore();
}

QVector<QPair<UserRowAction, QRect>> UserRowDelegate::actionButtonRects(const QRect &rect,
                                                                      bool enabled) const
{
    const QString toggleLbl = enabled ? m_lblDisable : m_lblEnable;
    const QStringList labels  = {m_lblEdit, m_lblReset, toggleLbl};
    const QVector<UserRowAction> actions = {UserRowAction::EditUser, UserRowAction::ResetPassword,
                                            UserRowAction::ToggleEnabled};

    QFont f = QApplication::font();
    f.setPixelSize(11);
    const QFontMetrics fm(f);
    QVector<int> widths;
    int totalW = 0;
    for (const QString &lbl : labels) {
        const int w = fm.horizontalAdvance(lbl) + kBtnPadX * 2;
        widths.append(w);
        totalW += w;
    }
    totalW += kBtnSpacing * (labels.size() - 1);

    int x       = rect.x() + qMax(0, (rect.width() - totalW) / 2);
    const int y = rect.y() + (rect.height() - kBtnH) / 2;

    QVector<QPair<UserRowAction, QRect>> hits;
    for (int i = 0; i < labels.size(); ++i) {
        hits.append({actions.at(i), QRect(x, y, widths.at(i), kBtnH)});
        x += widths.at(i) + kBtnSpacing;
    }
    return hits;
}

void UserRowDelegate::paintActionButtons(QPainter *painter, const QRect &rect, bool enabled,
                                         bool accountStatusMutable) const
{
    const QString toggleLbl = enabled ? m_lblDisable : m_lblEnable;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QFont f = painter->font();
    f.setPixelSize(11);
    painter->setFont(f);

    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    for (const auto &hit : actionButtonRects(rect, enabled)) {
        const QRect         btn = hit.second;
        const UserRowAction act = hit.first;
        const bool          allowed = actionPermitted(act, accountStatusMutable);
        const QColor        border =
            (act == UserRowAction::ToggleEnabled && enabled) ? t.dangerText : t.accent;
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(allowed ? border : t.textSecondary, 1));
        painter->drawRoundedRect(btn, 4, 4);
        painter->setPen(allowed ? t.textPrimary : t.textSecondary);
        const QString lbl = act == UserRowAction::EditUser        ? m_lblEdit
                            : act == UserRowAction::ResetPassword ? m_lblReset
                                                                  : toggleLbl;
        painter->drawText(btn, Qt::AlignCenter, lbl);
    }
    painter->restore();
}

bool UserRowDelegate::actionPermitted(UserRowAction action, bool accountStatusMutable) const
{
    switch (action) {
    case UserRowAction::EditUser:
        return PS->canAction(permission::PermKeys::UsersEdit);
    case UserRowAction::ToggleEnabled:
        return accountStatusMutable
               && PS->canAction(permission::PermKeys::UsersToggleStatus);
    case UserRowAction::ResetPassword:
        return PS->canAction(permission::PermKeys::UsersResetPassword);
    }
    return false;
}

bool UserRowDelegate::hitAction(const QPoint &pos, const QRect &actionsRect, bool enabled,
                                bool accountStatusMutable, UserRowAction *action) const
{
    for (const auto &hit : actionButtonRects(actionsRect, enabled)) {
        if (hit.second.contains(pos) && actionPermitted(hit.first, accountStatusMutable)) {
            *action = hit.first;
            return true;
        }
    }
    return false;
}

void UserRowDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    if (opt.state & QStyle::State_Selected)
        painter->fillRect(opt.rect, ui::themeWithAlpha(t.accent, 90));
    else if (index.row() % 2 == 1)
        painter->fillRect(opt.rect, t.surfaceElevated);

    painter->setPen(QPen(t.divider, 1));
    painter->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());

    const auto cols = columnRects(opt.rect.width());
    const int y     = opt.rect.y();
    const int h     = opt.rect.height();

    const bool checked = index.data(Qt::CheckStateRole).toInt() == Qt::Checked;
    paintCheckbox(painter, rowRect(cols.check, y, h), checked);

    paintText(painter, rowRect(cols.username, y, h).adjusted(8, 0, -4, 0),
              index.data(UserListItemModel::UsernameRole).toString(), t.textPrimary,
              Qt::AlignVCenter | Qt::AlignLeft);
    paintText(painter, rowRect(cols.displayName, y, h).adjusted(8, 0, -4, 0),
              index.data(UserListItemModel::DisplayNameRole).toString(), t.textPrimary,
              Qt::AlignVCenter | Qt::AlignLeft);
    paintRoleBadges(painter, rowRect(cols.roles, y, h),
                    index.data(UserListItemModel::RolesRole).toList());
    paintText(painter, rowRect(cols.department, y, h).adjusted(8, 0, -4, 0),
              index.data(UserListItemModel::DepartmentRole).toString(), t.textPrimary,
              Qt::AlignVCenter | Qt::AlignLeft);

    const int accountStatus = index.data(UserListItemModel::AccountStatusRole).toInt();
    const QColor statusColor = accountStatus == 1 ? t.successText : t.dangerText;
    paintText(painter, rowRect(cols.statusColumn, y, h),
              index.data(UserListItemModel::AccountStatusTextRole).toString(), statusColor,
              Qt::AlignCenter);
    paintText(painter, rowRect(cols.lastLogin, y, h).adjusted(8, 0, -4, 0),
              index.data(UserListItemModel::LastLoginRole).toString(), t.textPrimary,
              Qt::AlignVCenter | Qt::AlignLeft);

    const bool enabled = index.data(UserListItemModel::AccountEnabledRole).toBool();
    const bool statusMutable =
        index.data(UserListItemModel::AccountStatusMutableRole).toBool();
    paintActionButtons(painter, rowRect(cols.actions, y, h), enabled, statusMutable);

    painter->restore();
}

bool UserRowDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index)
{
    if (!index.isValid())
        return false;

    const auto cols      = columnRects(option.rect.width());
    const QRect rowCheck =
rowRect(cols.check, option.rect.y(), option.rect.height());
    const QRect rowActions =
rowRect(cols.actions, option.rect.y(), option.rect.height());

    if (event->type() == QEvent::MouseButtonRelease) {
        const auto *mouse = static_cast<QMouseEvent *>(event);
        if (mouse->button() != Qt::LeftButton)
            return false;

        const QPoint pos = mouse->pos();

        if (rowCheck.contains(pos)) {
            const int raw = index.data(Qt::CheckStateRole).toInt();
            const Qt::CheckState next =
                raw == static_cast<int>(Qt::Checked) ? Qt::Unchecked : Qt::Checked;
            model->setData(index, next, Qt::CheckStateRole);
            return true;
        }

        const bool enabled = index.data(UserListItemModel::AccountEnabledRole).toBool();
        const bool statusMutable =
            index.data(UserListItemModel::AccountStatusMutableRole).toBool();
        UserRowAction action;
        if (hitAction(pos, rowActions, enabled, statusMutable, &action)) {
            const qint64 userId = index.data(UserListItemModel::UserIdRole).toLongLong();
            emit rowActionTriggered(action, userId);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
