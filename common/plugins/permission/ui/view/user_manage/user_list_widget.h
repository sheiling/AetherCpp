#pragma once

#include "ui/view/user_manage/user_list_item_model.h"

#include <QWidget>

class QListView;
class UserListHeaderWidget;
class UserListItemModel;
class UserRowDelegate;

class UserListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserListWidget(QWidget *parent = nullptr);

    void setColumnHeaders(const QStringList &headers);
    void setActionLabels(const QString &edit, const QString &resetPassword,
                         const QString &enable, const QString &disable);
    void setRows(const QVector<UserListDisplayRow> &rows);

    QVector<qint64> selectedUserIds() const;
    UserListItemModel *itemModel() const { return m_model; }

signals:
    void selectionChanged();
    void editRequested(qint64 userId);
    void resetPasswordRequested(qint64 userId);
    void toggleStatusRequested(qint64 userId, bool currentlyEnabled);

private:
    UserListHeaderWidget *m_header   = nullptr;
    QListView         *m_list     = nullptr;
    UserListItemModel *m_model    = nullptr;
    UserRowDelegate   *m_delegate = nullptr;
};
