#pragma once

#include "pagewidget.h"

class QLabel;

class UserListWidget;
class UserManagementViewModel;

namespace permission_ui {
class PermComboBox;
}

namespace ui {
class ThemedLineEdit;
class ThemedPushButton;
}

namespace permission {

class UserManagementView : public PageWidget
{
    Q_OBJECT

public:
    explicit UserManagementView(QWidget *parent = nullptr);

protected:
    void onLoad() override;
    void applyTheme() override;
    void retranslateUi() override;
    void onShow() override;

private:
    void buildLayout();
    QWidget *buildSearchArea();
    QWidget *buildListArea();
    QWidget *buildStatusArea();
    void setupBindings();
    void syncUserList();
    void syncRoleFilterCombo();
    void syncListHeaders();
    void showUserEditor(qint64 userId = 0);
    QString validationMessage(const QString &key) const;

    ::UserManagementViewModel      *m_viewModel = nullptr;
    QLabel                         *m_pageTitle   = nullptr;
    ui::ThemedLineEdit             *m_searchEdit  = nullptr;
    permission_ui::PermComboBox    *m_roleFilter  = nullptr;
    permission_ui::PermComboBox    *m_statusFilter = nullptr;
    ui::ThemedPushButton           *m_btnSearch   = nullptr;
    ui::ThemedPushButton           *m_btnReset    = nullptr;
    ui::ThemedPushButton           *m_btnAdd      = nullptr;
    ::UserListWidget               *m_userList    = nullptr;
    QLabel                         *m_selectionLabel = nullptr;
    ui::ThemedPushButton           *m_btnBulkDisable = nullptr;
    ui::ThemedPushButton           *m_btnBulkEnable  = nullptr;
    ui::ThemedPushButton           *m_btnBulkRole    = nullptr;
    QLabel                         *m_totalLabel    = nullptr;
    ui::ThemedPushButton           *m_pageBtn       = nullptr;
};

} // namespace permission
