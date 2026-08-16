#pragma once

#ifdef interface
#undef interface
#endif

#include "command.h"
#include "model/role_model.h"
#include "model/user_management_model.h"
#include "model/user_model.h"
#include "viewmodel.h"

#include <QMap>
#include <QVector>

#include <memory>

class UserManagementViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_SEARCH_KEYWORD  = "searchKeyword";
    static constexpr const char *PROP_ROLE_FILTER     = "roleFilterCode";
    static constexpr const char *PROP_FILTER_STATUS   = "filterStatus";
    static constexpr const char *PROP_TOTAL_COUNT     = "totalCount";
    static constexpr const char *PROP_SELECTED_COUNT  = "selectedCount";

    VIEWMODEL_PROPERTY(QString, searchKeyword, SearchKeyword)
    VIEWMODEL_PROPERTY(QString, roleFilterCode, RoleFilterCode)
    VIEWMODEL_PROPERTY(int,     filterStatus, FilterStatus)
    VIEWMODEL_PROPERTY(int,     totalCount, TotalCount)
    VIEWMODEL_PROPERTY(int,     selectedCount, SelectedCount)

public:
signals:
    void searchKeywordChanged();
    void roleFilterCodeChanged();
    void filterStatusChanged();
    void totalCountChanged();
    void selectedCountChanged();

    void userListChanged();
    void rolesMetaChanged();

    void showInfo(const QString &title, const QString &message);
    void showWarning(const QString &title, const QString &message);

    void requestAddUser();
    void requestBulkAssignRoles();

public:
    explicit UserManagementViewModel(permission::UserModel *userModel = nullptr,
                                     permission::RoleModel *roleModel = nullptr,
                                     QObject *parent                   = nullptr);

    const QVector<permission::UserListItem>         &userListItems() const { return m_userListItems; }
    const QMap<QString, permission::RoleRecord> &rolesByCode() const;

    Command *searchCommand() const { return m_searchCommand; }
    Command *resetFilterCommand() const { return m_resetFilterCommand; }
    Command *addUserCommand() const { return m_addUserCommand; }
    Command *bulkAssignRolesCommand() const { return m_bulkAssignRolesCommand; }
    Command *bulkDisableCommand() const { return m_bulkDisableCommand; }
    Command *bulkEnableCommand() const { return m_bulkEnableCommand; }

    permission::UserEditorForm  loadEditorForm(qint64 userId) const;
    permission::OperationResult validateEditorForm(const permission::UserEditorForm &form) const;
    permission::OperationResult saveEditorForm(const permission::UserEditorForm &form);
    bool            resetPasswordToDefault(qint64 userId);
    bool            bulkAssignRoles(const QVector<qint64> &userIds, qint64 roleId);
    void            updateSelectedIds(const QVector<qint64> &ids);

public slots:
    void loadMeta();
    void refreshList();
    void refreshCommandStates();
    void resetFilters();
    void toggleUserEnabled(qint64 userId, bool currentlyEnabled);
    void bulkApplyAccountStatus(int statusValue);

private:
    void bindCommands();
    void updateCommandStates();
    permission::UserQueryFilter currentFilter() const;
    permission::UserListItem    toListItem(const permission::UserRecord &record) const;

    std::unique_ptr<permission::UserModel> m_ownedUserModel;
    std::unique_ptr<permission::RoleModel> m_ownedRoleModel;
    permission::UserModel                 *m_userModel = nullptr;
    permission::RoleModel                 *m_roleModel = nullptr;

    QVector<permission::UserListItem> m_userListItems;
    QVector<qint64>       m_selectedIds;

    FunctionalCommand *m_searchCommand       = nullptr;
    FunctionalCommand *m_resetFilterCommand  = nullptr;
    FunctionalCommand *m_addUserCommand         = nullptr;
    FunctionalCommand *m_bulkAssignRolesCommand = nullptr;
    FunctionalCommand *m_bulkDisableCommand  = nullptr;
    FunctionalCommand *m_bulkEnableCommand   = nullptr;
};
