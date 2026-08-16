#include "ui/vm/user_management_view_model.h"

#include "service/perm_action_keys.h"
#include "service/perm_entity_guard.h"
#include "ui/view/perm_text.h"

#include <QMetaType>

UserManagementViewModel::UserManagementViewModel(permission::UserModel *userModel,
                                                 permission::RoleModel *roleModel,
                                                 QObject *parent)
    : ViewModel(parent)
{
    if (userModel) {
        m_userModel = userModel;
    } else {
        m_ownedUserModel = std::make_unique<permission::UserModel>(nullptr);
        m_userModel      = m_ownedUserModel.get();
    }
    if (roleModel) {
        m_roleModel = roleModel;
    } else {
        m_ownedRoleModel = std::make_unique<permission::RoleModel>(nullptr);
        m_roleModel      = m_ownedRoleModel.get();
    }

    qRegisterMetaType<permission::UserEditorForm>("permission::UserEditorForm");

    registerProperty(PROP_SEARCH_KEYWORD, QString());
    registerProperty(PROP_ROLE_FILTER, QString());
    registerProperty(PROP_FILTER_STATUS, -1);
    registerProperty(PROP_TOTAL_COUNT, 0);
    registerProperty(PROP_SELECTED_COUNT, 0);

    bindCommands();

    connect(this, &ViewModel::propertyChanged, this, [this]() { updateCommandStates(); });
    loadMeta();
    refreshList();
}

void UserManagementViewModel::bindCommands()
{
    m_searchCommand = new FunctionalCommand([this]() { refreshList(); }, nullptr, this);
    m_searchCommand->setName(permission::PermKeys::UsersQuery);

    m_resetFilterCommand = new FunctionalCommand([this]() { resetFilters(); }, nullptr, this);
    m_resetFilterCommand->setName(permission::PermKeys::UsersResetFilter);

    m_addUserCommand = new FunctionalCommand([this]() { emit requestAddUser(); }, nullptr, this);
    m_addUserCommand->setName(permission::PermKeys::UsersCreate);

    m_bulkAssignRolesCommand =
        new FunctionalCommand([this]() { emit requestBulkAssignRoles(); },
                              [this]() { return selectedCount() > 0; },
                              this);
    m_bulkAssignRolesCommand->setName(permission::PermKeys::UsersBulkAssignRoles);

    m_bulkDisableCommand =
        new FunctionalCommand([this]() { bulkApplyAccountStatus(0); },
                              [this]() { return selectedCount() > 0; },
                              this);
    m_bulkDisableCommand->setName(permission::PermKeys::UsersBulkDisable);

    m_bulkEnableCommand =
        new FunctionalCommand([this]() { bulkApplyAccountStatus(1); },
                              [this]() { return selectedCount() > 0; },
                              this);
    m_bulkEnableCommand->setName(permission::PermKeys::UsersBulkEnable);

    updateCommandStates();
}

void UserManagementViewModel::refreshCommandStates()
{
    updateCommandStates();
}

void UserManagementViewModel::updateCommandStates()
{
    if (m_searchCommand)
        m_searchCommand->updateCanExecute();
    if (m_resetFilterCommand)
        m_resetFilterCommand->updateCanExecute();
    if (m_addUserCommand)
        m_addUserCommand->updateCanExecute();
    if (m_bulkAssignRolesCommand)
        m_bulkAssignRolesCommand->updateCanExecute();
    if (m_bulkDisableCommand)
        m_bulkDisableCommand->updateCanExecute();
    if (m_bulkEnableCommand)
        m_bulkEnableCommand->updateCanExecute();
}

const QMap<QString, permission::RoleRecord> &UserManagementViewModel::rolesByCode() const
{
    return m_roleModel->rolesByCode();
}

void UserManagementViewModel::loadMeta()
{
    m_roleModel->reload();
    emit rolesMetaChanged();
}

permission::UserQueryFilter UserManagementViewModel::currentFilter() const
{
    permission::UserQueryFilter filter;
    filter.keyword   = searchKeyword().trimmed();
    filter.roleCode  = roleFilterCode();
    filter.accountStatus = filterStatus();
    return filter;
}

permission::UserListItem UserManagementViewModel::toListItem(
    const permission::UserRecord &record) const
{
    permission::UserListItem item;
    item.id            = record.id;
    item.username      = record.username;
    item.displayName   = record.displayName;
    item.department    = record.department.isEmpty() ? QStringLiteral("—") : record.department;
    item.accountStatus  = record.accountStatus;
    item.accountEnabled = (record.accountStatus == 1);
    item.accountStatusMutable = permission::canChangeUserAccountStatus(record);
    item.lastLoginAt   = record.lastLoginAt;
    for (const QString &code : record.roleCodes) {
        item.roles.append(
            permission::UserListRoleBadge{m_roleModel->displayName(code),
                                            m_roleModel->tagColor(code)});
    }
    return item;
}

void UserManagementViewModel::refreshList()
{
    const permission::UserQueryFilter filter = currentFilter();
    const auto            users  = m_userModel->list(filter);

    m_userListItems.clear();
    m_userListItems.reserve(users.size());
    for (const auto &u : users)
        m_userListItems.append(toListItem(u));

    setTotalCount(static_cast<int>(m_userModel->count(filter)));
    emit userListChanged();
}

void UserManagementViewModel::resetFilters()
{
    beginBatchUpdate();
    setSearchKeyword(QString());
    setRoleFilterCode(QString());
    setFilterStatus(-1);
    endBatchUpdate();
    refreshList();
}

void UserManagementViewModel::updateSelectedIds(const QVector<qint64> &ids)
{
    m_selectedIds = ids;
    setSelectedCount(ids.size());
}

bool UserManagementViewModel::resetPasswordToDefault(qint64 userId)
{
    const permission::OperationResult r =
        m_userModel->resetPassword(userId, QStringLiteral("123456"));
    if (!r.ok) {
        emit showWarning(PTXT("perm.common.error"), r.message);
        return false;
    }
    return true;
}

void UserManagementViewModel::toggleUserEnabled(qint64 userId, bool currentlyEnabled)
{
    permission::UserRecord rec;
    if (m_userModel->findById(userId, rec) && !permission::canChangeUserAccountStatus(rec)) {
        emit showWarning(PTXT("perm.common.notice"), QStringLiteral("protected_user_status"));
        return;
    }
    const permission::OperationResult r =
        m_userModel->setAccountStatus(userId, currentlyEnabled ? 0 : 1);
    if (r.ok)
        refreshList();
    else
        emit showWarning(QString(), r.message);
}

void UserManagementViewModel::bulkApplyAccountStatus(int statusValue)
{
    if (m_selectedIds.isEmpty())
        return;
    for (qint64 id : m_selectedIds) {
        permission::UserRecord rec;
        if (m_userModel->findById(id, rec) && !permission::canChangeUserAccountStatus(rec))
            continue;
        const permission::OperationResult r = m_userModel->setAccountStatus(id, statusValue);
        if (!r.ok) {
            emit showWarning(QString(), r.message);
            return;
        }
    }
    refreshList();
}

bool UserManagementViewModel::bulkAssignRoles(const QVector<qint64> &userIds, qint64 roleId)
{
    for (qint64 uid : userIds) {
        const permission::OperationResult r = m_userModel->assignRoles(uid, roleId);
        if (!r.ok) {
            emit showWarning(QString(), r.message);
            return false;
        }
    }
    refreshList();
    return true;
}

permission::UserEditorForm UserManagementViewModel::loadEditorForm(qint64 userId) const
{
    permission::UserEditorForm form;
    form.editingUserId = userId;
    if (userId <= 0)
        return form;

    permission::UserRecord rec;
    if (!m_userModel->findById(userId, rec))
        return form;

    form.username     = rec.username;
    form.password     = QLatin1String(permission::kPasswordPlaceholder);
    form.displayName  = rec.displayName;
    form.department   = rec.department;
    form.remark       = rec.remark;
    form.accountStatus = rec.accountStatus;
    form.roleCode     = rec.roleCodes.isEmpty() ? QString() : rec.roleCodes.constFirst();
    return form;
}

permission::OperationResult UserManagementViewModel::validateEditorForm(
    const permission::UserEditorForm &form) const
{
    permission::OperationResult r;
    r.ok = true;

    if (form.username.trimmed().isEmpty()) {
        r.ok      = false;
        r.message = QStringLiteral("username_required");
        return r;
    }
    if (form.editingUserId <= 0 && form.password.isEmpty()) {
        r.ok      = false;
        r.message = QStringLiteral("password_required");
        return r;
    }
    if (form.displayName.trimmed().isEmpty()) {
        r.ok      = false;
        r.message = QStringLiteral("display_name_required");
        return r;
    }
    if (form.department.trimmed().isEmpty()) {
        r.ok      = false;
        r.message = QStringLiteral("department_required");
        return r;
    }
    if (form.roleCode.isEmpty()) {
        r.ok      = false;
        r.message = QStringLiteral("role_required");
        return r;
    }
    permission::RoleRecord role;
    if (!m_roleModel->findByCode(form.roleCode, role)) {
        r.ok      = false;
        r.message = QStringLiteral("role_required");
        return r;
    }
    if (!permission::canAssignRoleCode(role.code)) {
        r.ok      = false;
        r.message = QStringLiteral("role_not_assignable");
        return r;
    }
    if (form.editingUserId > 0) {
        permission::UserRecord existing;
        if (m_userModel->findById(form.editingUserId, existing)
            && !permission::canChangeUserAccountStatus(existing)
            && form.accountStatus != existing.accountStatus) {
            r.ok      = false;
            r.message = QStringLiteral("protected_user_status");
            return r;
        }
    }
    return r;
}

permission::OperationResult UserManagementViewModel::saveEditorForm(
    const permission::UserEditorForm &form)
{
    const permission::OperationResult validation = validateEditorForm(form);
    if (!validation.ok)
        return validation;

    permission::RoleRecord role;
    if (!m_roleModel->findByCode(form.roleCode, role)) {
        permission::OperationResult r;
        r.message = QStringLiteral("role_required");
        return r;
    }

    permission::UserEditorForm normalized = form;
    normalized.username       = form.username.trimmed();
    normalized.displayName    = form.displayName.trimmed();
    normalized.department     = form.department.trimmed();
    normalized.remark         = form.remark.trimmed();

    if (form.editingUserId > 0)
        return m_userModel->updateUser(normalized, role);
    return m_userModel->createUser(normalized, role);
}

