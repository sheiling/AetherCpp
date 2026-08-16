#include "ui/view/user_manage/user_list_item_model.h"

UserListItemModel::UserListItemModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int UserListItemModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

QVariant UserListItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return {};

    const UserListDisplayRow &row = m_rows.at(index.row());

    switch (role) {
    case Qt::CheckStateRole:
        return row.checked ? Qt::Checked : Qt::Unchecked;
    case UserIdRole:
        return row.id;
    case UsernameRole:
        return row.username;
    case DisplayNameRole:
        return row.displayName;
    case DepartmentRole:
        return row.department;
    case RolesRole: {
        QVariantList list;
        for (const auto &badge : row.roles) {
            list.append(QVariantMap{
                {QStringLiteral("name"), badge.name},
                {QStringLiteral("color"), badge.color},
            });
        }
        return list;
    }
    case AccountStatusRole:
        return row.accountStatus;
    case AccountStatusTextRole:
        return row.accountStatusText;
    case LastLoginRole:
        return row.lastLoginText;
    case AccountEnabledRole:
        return row.accountEnabled;
    case AccountStatusMutableRole:
        return row.accountStatusMutable;
    default:
        return {};
    }
}

bool UserListItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return false;

    if (role == Qt::CheckStateRole) {
        const bool checked = value.toInt() == static_cast<int>(Qt::Checked);
        if (m_rows[index.row()].checked == checked)
            return true;
        m_rows[index.row()].checked = checked;
        emit dataChanged(index, index, {Qt::CheckStateRole});
        emit selectionChanged();
        return true;
    }

    return false;
}

Qt::ItemFlags UserListItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

void UserListItemModel::setRows(const QVector<UserListDisplayRow> &rows)
{
    beginResetModel();
    m_rows = rows;
    endResetModel();
    emit selectionChanged();
}

QVector<qint64> UserListItemModel::selectedUserIds() const
{
    QVector<qint64> ids;
    ids.reserve(m_rows.size());
    for (const auto &row : m_rows) {
        if (row.checked)
            ids.append(row.id);
    }
    return ids;
}

UserListDisplayRow UserListItemModel::rowAt(int row) const
{
    if (row < 0 || row >= m_rows.size())
        return {};
    return m_rows.at(row);
}
