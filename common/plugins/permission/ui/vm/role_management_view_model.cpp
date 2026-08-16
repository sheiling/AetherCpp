#include "ui/vm/role_management_view_model.h"

#include "model/permission_grant_service.h"
#include "service/perm_action_keys.h"
#include "service/perm_entity_guard.h"
#include "ui/view/perm_text.h"

#include <algorithm>

RoleManagementViewModel::RoleManagementViewModel(permission::RoleModel *roleModel,
                                                 QObject *parent)
    : ViewModel(parent)
{
    qRegisterMetaType<permission::RoleEditorForm>("permission::RoleEditorForm");
    if (roleModel) {
        m_roleModel = roleModel;
    } else {
        m_ownedRoleModel = std::make_unique<permission::RoleModel>(nullptr);
        m_roleModel      = m_ownedRoleModel.get();
    }

    registerProperty(PROP_SELECTED_ROLE_ID, QVariant::fromValue(qint64(0)));

    bindCommands();
    connect(this, &ViewModel::propertyChanged, this, [this]() { updateCommandStates(); });
}

void RoleManagementViewModel::bindCommands()
{
    m_createRoleCommand = new FunctionalCommand([this]() { emit requestCreateRole(); }, nullptr, this);
    m_createRoleCommand->setName(permission::PermKeys::RolesCreate);

    m_editRoleCommand = new FunctionalCommand(
        [this]() { emit requestEditRole(); },
        [this]() {
            if (selectedRoleId() <= 0 || !m_roleModel)
                return false;
            permission::RoleRecord role;
            return m_roleModel->findById(selectedRoleId(), role)
                   && permission::canEditRoleMeta(role);
        },
        this);
    m_editRoleCommand->setName(permission::PermKeys::RolesEdit);

    m_savePermissionsCommand =
        new FunctionalCommand([this]() { emit requestSavePermissions(); },
                              [this]() { return selectedRoleId() > 0; }, this);
    m_savePermissionsCommand->setName(permission::PermKeys::RolesSave);

    m_resetPermissionsCommand =
        new FunctionalCommand([this]() { emit requestResetPermissions(); },
                              [this]() { return selectedRoleId() > 0; }, this);
    m_resetPermissionsCommand->setName(permission::PermKeys::RolesReset);

    m_selectAllPermissionsCommand =
        new FunctionalCommand([this]() { emit requestSelectAllPermissions(); },
                              [this]() { return selectedRoleId() > 0; }, this);
    m_selectAllPermissionsCommand->setName(permission::PermKeys::RolesSelectAll);

    m_clearPermissionsCommand =
        new FunctionalCommand([this]() { emit requestClearPermissions(); },
                              [this]() { return selectedRoleId() > 0; }, this);
    m_clearPermissionsCommand->setName(permission::PermKeys::RolesClear);

    updateCommandStates();
}

void RoleManagementViewModel::refreshCommandStates()
{
    updateCommandStates();
}

void RoleManagementViewModel::updateCommandStates()
{
    if (m_createRoleCommand)
        m_createRoleCommand->updateCanExecute();
    if (m_editRoleCommand)
        m_editRoleCommand->updateCanExecute();
    if (m_savePermissionsCommand)
        m_savePermissionsCommand->updateCanExecute();
    if (m_resetPermissionsCommand)
        m_resetPermissionsCommand->updateCanExecute();
    if (m_selectAllPermissionsCommand)
        m_selectAllPermissionsCommand->updateCanExecute();
    if (m_clearPermissionsCommand)
        m_clearPermissionsCommand->updateCanExecute();
}

permission::RoleListItem RoleManagementViewModel::toListItem(
    const permission::RoleRecord &record) const
{
    permission::RoleListItem item;
    item.id          = record.id;
    item.code        = record.code;
    item.name        = record.name;
    item.description = record.description;
    return item;
}

void RoleManagementViewModel::refreshRoleList()
{
    const qint64 prevId = selectedRoleId();

    m_roleListItems.clear();
    const auto roles = m_roleModel->listRoles();
    m_roleListItems.reserve(roles.size());
    for (const auto &r : roles)
        m_roleListItems.append(toListItem(r));

    emit roleListChanged();

    if (m_roleListItems.isEmpty()) {
        setSelectedRoleId(0);
        m_selectedRole = {};
        m_allPermissions.clear();
        m_grantedPermissionIds.clear();
        emit roleDetailChanged();
        emit permissionPanelChanged();
        return;
    }

    qint64 nextId = prevId;
    if (nextId <= 0 || !std::any_of(m_roleListItems.begin(), m_roleListItems.end(),
                                    [nextId](const permission::RoleListItem &item) {
                                        return item.id == nextId;
                                    })) {
        nextId = m_roleListItems.first().id;
    }
    selectRole(nextId);
}

void RoleManagementViewModel::reloadPermissionTree()
{
    const qint64 roleId = selectedRoleId();
    if (roleId <= 0 || !m_roleModel)
        return;

    m_allPermissions = m_roleModel->permissionTree();
    const QVector<qint64> rawGrants = m_roleModel->grantedPermissionIds(roleId);
    m_grantedPermissionIds =
        permission::PermissionGrantService::normalize(m_allPermissions, rawGrants).normalized;
    emit permissionPanelChanged();
}

void RoleManagementViewModel::selectRole(qint64 roleId)
{
    if (roleId <= 0) {
        setSelectedRoleId(0);
        m_selectedRole = {};
        m_allPermissions.clear();
        m_grantedPermissionIds.clear();
        emit roleDetailChanged();
        emit permissionPanelChanged();
        return;
    }

    setSelectedRoleId(roleId);
    loadRoleDetail(roleId);
}

void RoleManagementViewModel::loadRoleDetail(qint64 roleId)
{
    permission::RoleRecord role;
    if (!m_roleModel->findById(roleId, role)) {
        m_selectedRole = {};
        m_allPermissions.clear();
        m_grantedPermissionIds.clear();
        emit roleDetailChanged();
        emit permissionPanelChanged();
        return;
    }

    m_selectedRole = role;

    m_allPermissions = m_roleModel->permissionTree();
    const QVector<qint64> rawGrants = m_roleModel->grantedPermissionIds(role.id);
    m_grantedPermissionIds =
        permission::PermissionGrantService::normalize(m_allPermissions, rawGrants).normalized;
    emit roleDetailChanged();
    emit permissionPanelChanged();
}

permission::RoleEditorForm RoleManagementViewModel::loadEditorForm(qint64 roleId) const
{
    permission::RoleEditorForm form;
    form.editingRoleId = roleId;
    if (roleId <= 0 || !m_roleModel)
        return form;

    permission::RoleRecord rec;
    if (!m_roleModel->findById(roleId, rec))
        return form;

    form.name        = rec.name;
    form.description = rec.description;
    return form;
}

permission::OperationResult RoleManagementViewModel::validateEditorForm(
    const permission::RoleEditorForm &form) const
{
    permission::OperationResult r;
    r.ok = true;

    if (form.name.trimmed().isEmpty()) {
        r.ok      = false;
        r.message = QStringLiteral("role_name_required");
        return r;
    }

    if (m_roleModel
        && m_roleModel->isRoleNameTaken(form.name.trimmed(), form.editingRoleId)) {
        r.ok      = false;
        r.message = QStringLiteral("role_name_duplicate");
        return r;
    }

    if (form.editingRoleId > 0 && m_roleModel) {
        permission::RoleRecord rec;
        if (!m_roleModel->findById(form.editingRoleId, rec)) {
            r.ok      = false;
            r.message = QStringLiteral("role_not_found");
            return r;
        }
        if (!permission::canEditRoleMeta(rec)) {
            r.ok      = false;
            r.message = QStringLiteral("protected_role_edit");
            return r;
        }
    }
    return r;
}

permission::OperationResult RoleManagementViewModel::saveEditorForm(
    const permission::RoleEditorForm &form)
{
    const permission::OperationResult validation = validateEditorForm(form);
    if (!validation.ok)
        return validation;

    if (form.editingRoleId > 0) {
        permission::RoleRecord rec;
        if (!m_roleModel->findById(form.editingRoleId, rec)) {
            permission::OperationResult r;
            r.message = QStringLiteral("role_not_found");
            return r;
        }
        rec.name        = form.name.trimmed();
        rec.description = form.description.trimmed();
        const permission::OperationResult r = m_roleModel->updateRole(rec);
        if (!r.ok)
            return r;
        refreshRoleList();
        selectRole(form.editingRoleId);
        return r;
    }

    permission::RoleCreateForm createForm;
    createForm.name        = form.name.trimmed();
    createForm.description = form.description.trimmed();
    qint64 newId = 0;
    const permission::OperationResult r = m_roleModel->createRole(createForm, &newId);
    if (!r.ok)
        return r;
    refreshRoleList();
    if (newId > 0)
        selectRole(newId);
    return r;
}

permission::OperationResult RoleManagementViewModel::saveRolePermissions(
    const QVector<qint64> &permissionIds)
{
    const qint64 roleId = selectedRoleId();
    if (roleId <= 0) {
        permission::OperationResult r;
        r.message = QStringLiteral("No role selected");
        return r;
    }

    const permission::OperationResult r = m_roleModel->saveRolePermissions(roleId, permissionIds);
    if (!r.ok) {
        emit showWarning(PTXT("perm.common.error"), r.message);
        return r;
    }

    m_grantedPermissionIds = m_roleModel->grantedPermissionIds(roleId);
    emit permissionPanelChanged();

    QString msg = PTXT("perm.roles.permissions_saved");
    if (r.info.startsWith(QStringLiteral("pruned_invalid_grants:"))) {
        const int n = r.info.mid(QStringLiteral("pruned_invalid_grants:").size()).toInt();
        if (n > 0)
            msg = PTXT("perm.roles.permissions_saved_pruned").arg(n);
    }
    emit showInfo(PTXT("perm.common.notice"), msg);
    return r;
}
