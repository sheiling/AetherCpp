#pragma once

#ifdef interface
#undef interface
#endif

#include "command.h"
#include "model/role_management_model.h"
#include "model/role_model.h"
#include "viewmodel.h"

#include <QVector>

#include <memory>

class RoleManagementViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_SELECTED_ROLE_ID = "selectedRoleId";

    VIEWMODEL_PROPERTY(qint64, selectedRoleId, SelectedRoleId)

signals:
    void selectedRoleIdChanged();
    void roleListChanged();
    void roleDetailChanged();
    void permissionPanelChanged();

    void showInfo(const QString &title, const QString &message);
    void showWarning(const QString &title, const QString &message);

    void requestCreateRole();
    void requestEditRole();
    void requestSavePermissions();
    void requestResetPermissions();
    void requestSelectAllPermissions();
    void requestClearPermissions();

public:
    explicit RoleManagementViewModel(permission::RoleModel *roleModel = nullptr,
                                     QObject *parent                   = nullptr);

    const QVector<permission::RoleListItem>      &roleListItems() const { return m_roleListItems; }
    const permission::RoleRecord                 &selectedRole() const { return m_selectedRole; }
    const QVector<permission::PermissionRecord>    &allPermissions() const { return m_allPermissions; }
    const QVector<qint64>                        &grantedPermissionIds() const
    {
        return m_grantedPermissionIds;
    }

    permission::RoleEditorForm  loadEditorForm(qint64 roleId) const;
    permission::OperationResult validateEditorForm(const permission::RoleEditorForm &form) const;
    permission::OperationResult saveEditorForm(const permission::RoleEditorForm &form);
    permission::OperationResult saveRolePermissions(const QVector<qint64> &permissionIds);

    Command *createRoleCommand() const { return m_createRoleCommand; }
    Command *editRoleCommand() const { return m_editRoleCommand; }
    Command *savePermissionsCommand() const { return m_savePermissionsCommand; }
    Command *resetPermissionsCommand() const { return m_resetPermissionsCommand; }
    Command *selectAllPermissionsCommand() const { return m_selectAllPermissionsCommand; }
    Command *clearPermissionsCommand() const { return m_clearPermissionsCommand; }

public slots:
    void refreshRoleList();
    void reloadPermissionTree();
    void selectRole(qint64 roleId);
    void refreshCommandStates();

private:
    void bindCommands();
    void updateCommandStates();

    permission::RoleListItem toListItem(const permission::RoleRecord &record) const;
    void loadRoleDetail(qint64 roleId);

    std::unique_ptr<permission::RoleModel> m_ownedRoleModel;
    permission::RoleModel                 *m_roleModel = nullptr;

    QVector<permission::RoleListItem>      m_roleListItems;
    permission::RoleRecord                 m_selectedRole;
    QVector<permission::PermissionRecord>  m_allPermissions;
    QVector<qint64> m_grantedPermissionIds;

    FunctionalCommand *m_createRoleCommand            = nullptr;
    FunctionalCommand *m_editRoleCommand              = nullptr;
    FunctionalCommand *m_savePermissionsCommand         = nullptr;
    FunctionalCommand *m_resetPermissionsCommand        = nullptr;
    FunctionalCommand *m_selectAllPermissionsCommand    = nullptr;
    FunctionalCommand *m_clearPermissionsCommand        = nullptr;
};
