#pragma once

#include "pagewidget.h"

#include <QVector>

class QLabel;
class QListView;
class QListWidget;
class QWidget;

namespace ui {
class ThemedPushButton;
}

class RoleManagementViewModel;

namespace permission {

class RolePermissionListModel;
class RolePermissionPageDelegate;

class RoleManagementView : public PageWidget
{
    Q_OBJECT

public:
    explicit RoleManagementView(QWidget *parent = nullptr);

protected:
    void onLoad() override;
    void applyTheme() override;
    void retranslateUi() override;
    void onShow() override;

private:
    void buildLayout();
    void setupBindings();
    void syncRoleList();
    void syncRoleDetail();
    void rebuildPermissionPanel();
    void collectAndSavePermissions();
    void resetPermissionGrants();
    void updatePermissionActionButtons();
    void updatePermissionListInteraction();
    void relayoutPermissionList();
    void showRoleEditor(qint64 roleId = 0);
    QString validationMessage(const QString &key) const;
    bool isPermissionGrantsDirty() const;
    int  roleListRowForId(qint64 roleId) const;
    void selectRoleListRow(qint64 roleId);
    bool confirmDiscardPermissionEdits(const QString &message);

    ::RoleManagementViewModel *m_viewModel = nullptr;

    QLabel      *m_listTitleLabel = nullptr;
    QListWidget *m_roleList       = nullptr;
    ui::ThemedPushButton *m_btnAddRole     = nullptr;
    QLabel               *m_permTitle      = nullptr;
    QLabel               *m_permDesc       = nullptr;
    ui::ThemedPushButton *m_btnEditRole    = nullptr;
    ui::ThemedPushButton *m_btnSelectAll   = nullptr;
    ui::ThemedPushButton *m_btnClear       = nullptr;
    ui::ThemedPushButton *m_btnReset       = nullptr;
    ui::ThemedPushButton *m_btnSave        = nullptr;
    QListView   *m_permList       = nullptr;

    RolePermissionListModel       *m_permModel    = nullptr;
    RolePermissionPageDelegate *m_permDelegate = nullptr;

    bool m_blockRoleListSelection = false;
};

} // namespace permission
