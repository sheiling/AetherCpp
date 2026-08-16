#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

#ifdef Edit
#undef Edit
#endif

#include "model/user_management_model.h"

struct UserListDisplayRow {
    qint64                     id = 0;
    bool                       checked = false;
    QString                    username;
    QString                    displayName;
    QString                    department;
    QVector<permission::UserListRoleBadge> roles;
    int                        accountStatus = 1;
    QString                    accountStatusText;
    QString                    lastLoginText;
    bool                       accountEnabled = true;
    bool                       accountStatusMutable = true;
};

enum class UserRowAction {
    EditUser       = 0,
    ResetPassword  = 1,
    ToggleEnabled  = 2,
};

Q_DECLARE_METATYPE(UserRowAction)

class UserListItemModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ItemRoles {
        UserIdRole = Qt::UserRole + 1,
        UsernameRole,
        DisplayNameRole,
        DepartmentRole,
        RolesRole,
        AccountStatusRole,
        AccountStatusTextRole,
        LastLoginRole,
        AccountEnabledRole,
        AccountStatusMutableRole,
    };

    explicit UserListItemModel(QObject *parent = nullptr);

    int           rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant      data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool          setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setRows(const QVector<UserListDisplayRow> &rows);
    const QVector<UserListDisplayRow> &rows() const { return m_rows; }

    QVector<qint64>    selectedUserIds() const;
    UserListDisplayRow rowAt(int row) const;

signals:
    void selectionChanged();

private:
    QVector<UserListDisplayRow> m_rows;
};
