#include "ui/view/user_manage/user_management_view.h"

#include "service/perm_action_keys.h"
#include "service/perm_system_constants.h"
#include "uibase/permission/permservice.h"

#include "model/user_management_model.h"
#include "ui/view/message_popup.h"
#include "ui/view/perm_combobox.h"
#include "ui/view/perm_text.h"
#include "ui/view/permission_ui_utils.h"
#include "ui/view/user_manage/user_list_widget.h"
#include "ui/vm/user_management_view_model.h"

#include "databinding.h"

#include "controls/button/themed_push_button.h"
#include "controls/input/themed_line_edit.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace permission {

UserManagementView::UserManagementView(QWidget *parent)
    : PageWidget(parent)
{
}

void UserManagementView::applyTheme()
{
    permission_ui::applyPermissionTheme(this);
}

void UserManagementView::onLoad()
{
    permission_ui::applyPermissionTheme(this);

    m_viewModel = new UserManagementViewModel(nullptr, nullptr, this);

    buildLayout();
    setupBindings();

    retranslateUi();
    m_viewModel->loadMeta();
    m_viewModel->refreshList();
}

void UserManagementView::buildLayout()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 20);
    root->setSpacing(16);
    root->addWidget(buildSearchArea());
    root->addWidget(buildListArea(), 1);
    root->addWidget(buildStatusArea());
}

QWidget *UserManagementView::buildSearchArea()
{
    auto *toolbar = new QFrame(this);
    permission_ui::initFilterToolbar(toolbar);

    auto *headerRow = new QHBoxLayout(toolbar);
    headerRow->setContentsMargins(0, 2, 0, 4);
    headerRow->setSpacing(10);
    headerRow->setAlignment(Qt::AlignVCenter);

    m_pageTitle = new QLabel(toolbar);
    permission_ui::initPageTitle(m_pageTitle, true);

    m_searchEdit = new ui::ThemedLineEdit(toolbar);
    m_searchEdit->setMinimumWidth(240);
    permission_ui::initSearchField(m_searchEdit);

    m_roleFilter = new permission_ui::PermComboBox(toolbar);
    m_roleFilter->setMinimumWidth(136);
    permission_ui::initComboBox(m_roleFilter);

    m_statusFilter = new permission_ui::PermComboBox(toolbar);
    m_statusFilter->setMinimumWidth(116);
    permission_ui::initComboBox(m_statusFilter);

    m_btnSearch = permission_ui::makePrimaryButton(QString(), toolbar);
    m_btnReset  = permission_ui::makeSecondaryButton(QString(), toolbar);
    m_btnAdd    = permission_ui::makePrimaryButton(QString(), toolbar);

    headerRow->addWidget(m_pageTitle);
    headerRow->addStretch();
    headerRow->addWidget(m_searchEdit);
    headerRow->addWidget(m_roleFilter);
    headerRow->addWidget(m_statusFilter);
    headerRow->addWidget(m_btnSearch);
    headerRow->addWidget(m_btnReset);
    headerRow->addWidget(m_btnAdd);

    return toolbar;
}

QWidget *UserManagementView::buildListArea()
{
    m_userList = new UserListWidget(this);
    return m_userList;
}

QWidget *UserManagementView::buildStatusArea()
{
    auto *footer = new QWidget(this);
    permission_ui::initFooter(footer);

    auto *footerLay = new QHBoxLayout(footer);
    footerLay->setSpacing(12);

    m_selectionLabel = new QLabel(footer);
    permission_ui::initMutedLabel(m_selectionLabel);
    m_btnBulkDisable = permission_ui::makeDangerButton(QString(), footer);
    m_btnBulkEnable  = permission_ui::makeSecondaryButton(QString(), footer);
    m_btnBulkRole    = permission_ui::makeSecondaryButton(QString(), footer);

    m_totalLabel = new QLabel(footer);
    permission_ui::initMutedLabel(m_totalLabel);
    m_pageBtn = permission_ui::makePrimaryButton(QStringLiteral("1"), footer);
    m_pageBtn->setFixedWidth(32);

    footerLay->addWidget(m_selectionLabel);
    footerLay->addWidget(m_btnBulkDisable);
    footerLay->addWidget(m_btnBulkEnable);
    footerLay->addWidget(m_btnBulkRole);
    footerLay->addStretch();
    footerLay->addWidget(m_totalLabel);
    footerLay->addWidget(m_pageBtn);

    return footer;
}

void UserManagementView::setupBindings()
{
    DataBinding::bind(m_viewModel, UserManagementViewModel::PROP_SEARCH_KEYWORD, m_searchEdit);
    DataBinding::bindCommand(m_viewModel->searchCommand(), m_btnSearch);
    DataBinding::bindCommand(m_viewModel->resetFilterCommand(), m_btnReset);
    DataBinding::bindCommand(m_viewModel->bulkDisableCommand(), m_btnBulkDisable);
    DataBinding::bindCommand(m_viewModel->bulkEnableCommand(), m_btnBulkEnable);
    DataBinding::bindCommand(m_viewModel->addUserCommand(), m_btnAdd);
    DataBinding::bindCommand(m_viewModel->bulkAssignRolesCommand(), m_btnBulkRole);

    connect(PS, &PermService::permissionsChanged, m_viewModel,
            &UserManagementViewModel::refreshCommandStates);

    connect(m_viewModel, &UserManagementViewModel::requestAddUser, this,
            [this]() { showUserEditor(0); });

    if (m_statusFilter->count() == 0) {
        m_statusFilter->addItem(PTXT("perm.common.all_statuses"), -1);
        m_statusFilter->addItem(PTXT("perm.common.enabled"), 1);
        m_statusFilter->addItem(PTXT("perm.common.disabled"), 0);
    }

    connect(m_viewModel, &UserManagementViewModel::roleFilterCodeChanged, this, [this]() {
        if (!m_roleFilter || !m_viewModel)
            return;
        m_roleFilter->blockSignals(true);
        const int idx = m_roleFilter->findData(m_viewModel->roleFilterCode());
        m_roleFilter->setCurrentIndex(idx >= 0 ? idx : 0);
        m_roleFilter->blockSignals(false);
    });
    connect(m_viewModel, &UserManagementViewModel::filterStatusChanged, this, [this]() {
        if (!m_statusFilter || !m_viewModel)
            return;
        const int st = m_viewModel->filterStatus();
        m_statusFilter->blockSignals(true);
        m_statusFilter->setCurrentIndex(st < 0 ? 0 : (st == 1 ? 1 : 2));
        m_statusFilter->blockSignals(false);
    });

    connect(m_roleFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        if (!m_viewModel || !m_roleFilter)
            return;
        m_viewModel->setRoleFilterCode(m_roleFilter->currentData().toString());
    });
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        if (!m_viewModel || !m_statusFilter)
            return;
        m_viewModel->setFilterStatus(m_statusFilter->currentData().toInt());
    });

    connect(m_viewModel, &UserManagementViewModel::userListChanged, this,
            &UserManagementView::syncUserList);
    connect(m_viewModel, &UserManagementViewModel::rolesMetaChanged, this,
            &UserManagementView::syncRoleFilterCombo);
    connect(m_viewModel, &UserManagementViewModel::totalCountChanged, this, [this]() {
        if (m_totalLabel)
            m_totalLabel->setText(PTXT("perm.common.total_count").arg(m_viewModel->totalCount()));
    });
    connect(m_viewModel, &UserManagementViewModel::selectedCountChanged, this, [this]() {
        if (m_selectionLabel)
            m_selectionLabel->setText(PTXT("perm.common.selected_count").arg(m_viewModel->selectedCount()));
    });
    connect(m_viewModel, &UserManagementViewModel::showWarning, this,
            [this](const QString &title, const QString &message) {
                const QString text = validationMessage(message);
                permission_ui::MessagePopup::show(this, title.isEmpty() ? PTXT("perm.common.notice") : title, text,
                                                 QDialogButtonBox::Ok);
            });

    connect(m_userList, &UserListWidget::selectionChanged, this, [this]() {
        if (!m_viewModel)
            return;
        m_viewModel->updateSelectedIds(m_userList->selectedUserIds());
    });
    connect(m_userList, &UserListWidget::editRequested, this, [this](qint64 userId) {
        if (!PS->canAction(permission::PermKeys::UsersEdit))
            return;
        showUserEditor(userId);
    });
    connect(m_userList, &UserListWidget::resetPasswordRequested, this, [this](qint64 userId) {
        if (!m_viewModel || !PS->canAction(permission::PermKeys::UsersResetPassword))
            return;
        if (m_viewModel->resetPasswordToDefault(userId)) {
            permission_ui::MessagePopup::show(this, PTXT("perm.common.notice"), PTXT("perm.users.password_reset_done"),
                                              QDialogButtonBox::Ok);
        }
    });
    connect(m_userList, &UserListWidget::toggleStatusRequested, this,
            [this](qint64 userId, bool enabled) {
                if (!m_viewModel || !PS->canAction(permission::PermKeys::UsersToggleStatus))
                    return;
                m_viewModel->toggleUserEnabled(userId, enabled);
            });

    connect(m_viewModel, &UserManagementViewModel::requestBulkAssignRoles, this, [this]() {
        if (!m_viewModel)
            return;
        const QVector<qint64> ids = m_userList->selectedUserIds();
        if (ids.isEmpty())
            return;

        auto *dlg = new QWidget(this, Qt::Dialog | Qt::FramelessWindowHint);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setWindowModality(Qt::ApplicationModal);
        dlg->setFixedSize(360, 200);
        permission_ui::applyPermissionTheme(dlg);

        auto *root = new QVBoxLayout(dlg);
        root->setContentsMargins(24, 20, 24, 20);
        root->setSpacing(16);

        auto *title = new QLabel(PTXT("perm.users.bulk_assign_roles"), dlg);
        permission_ui::initPageTitle(title, true);

        auto *combo = new permission_ui::PermComboBox(dlg);
        permission_ui::initComboBox(combo);
        for (const auto &role : m_viewModel->rolesByCode())
            combo->addItem(role.name, role.id);

        auto *line = new QFrame(dlg);
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet(permission_ui::permissionDialogDividerStyleSheet());

        auto *btnRow = new QHBoxLayout;
        btnRow->addStretch();
        auto *btnCancel = permission_ui::makeSecondaryButton(PTXT("perm.common.cancel"), dlg);
        auto *btnOk     = permission_ui::makeSecondaryButton(PTXT("perm.common.confirm"), dlg);
        btnRow->addWidget(btnCancel);
        btnRow->addWidget(btnOk);

        root->addWidget(title);
        root->addWidget(combo);
        root->addWidget(line);
        root->addLayout(btnRow);

        connect(btnCancel, &ui::ThemedPushButton::clicked, dlg, &QWidget::close);
        connect(btnOk, &ui::ThemedPushButton::clicked, dlg, [this, dlg, ids, combo]() {
            const int idx = combo->currentIndex();
            if (idx < 0)
                return;
            const qint64 roleId = combo->currentData().toLongLong();
            m_viewModel->bulkAssignRoles(ids, roleId);
            dlg->close();
        });

        if (QWidget *host = window()) {
            const QPoint c = host->geometry().center();
            dlg->move(c.x() - dlg->width() / 2, c.y() - dlg->height() / 2);
        }
        dlg->show();
    });
}

void UserManagementView::syncUserList()
{
    if (!m_viewModel || !m_userList)
        return;

    QVector<UserListDisplayRow> rows;
    rows.reserve(m_viewModel->userListItems().size());
    for (const auto &item : m_viewModel->userListItems()) {
        UserListDisplayRow row;
        row.id            = item.id;
        row.checked       = item.checked;
        row.username      = item.username;
        row.displayName   = item.displayName;
        row.department    = item.department;
        row.roles         = item.roles;
        row.accountStatus     = item.accountStatus;
        row.accountStatusText = item.accountStatus == 1 ? PTXT("perm.common.enabled") : PTXT("perm.common.disabled");
        row.accountEnabled         = item.accountEnabled;
        row.accountStatusMutable   = item.accountStatusMutable;
        row.lastLoginText = item.lastLoginAt.isValid()
                                ? item.lastLoginAt.toString(QStringLiteral("yyyy-MM-dd HH:mm"))
                                : QStringLiteral("—");
        rows.append(row);
    }
    m_userList->setRows(rows);

    if (m_totalLabel)
        m_totalLabel->setText(PTXT("perm.common.total_count").arg(m_viewModel->totalCount()));
    if (m_pageBtn)
        m_pageBtn->setText(QStringLiteral("1"));
}

void UserManagementView::syncRoleFilterCombo()
{
    if (!m_viewModel || !m_roleFilter)
        return;

    const QString roleData = m_viewModel->roleFilterCode();
    m_roleFilter->blockSignals(true);
    m_roleFilter->clear();
    m_roleFilter->addItem(PTXT("perm.users.filter_all_roles"), QString());
    for (auto it = m_viewModel->rolesByCode().constBegin();
         it != m_viewModel->rolesByCode().constEnd(); ++it)
        m_roleFilter->addItem(it.value().name, it.key());
    const int idx = m_roleFilter->findData(roleData);
    m_roleFilter->setCurrentIndex(idx >= 0 ? idx : 0);
    m_roleFilter->blockSignals(false);
}

void UserManagementView::syncListHeaders()
{
    if (!m_userList)
        return;
    m_userList->setColumnHeaders({
        QString(),
        PTXT("perm.users.col_username"),
        PTXT("perm.resources.label_display_name"),
        PTXT("perm.users.col_role"),
        PTXT("perm.users.label_department"),
        PTXT("perm.users.col_status"),
        PTXT("perm.users.col_last_login"),
        PTXT("perm.resources.type_action"),
    });
    m_userList->setActionLabels(PTXT("perm.common.edit"), PTXT("perm.users.reset_password"), PTXT("perm.common.enabled"), PTXT("perm.common.disabled"));
}

void UserManagementView::onShow()
{
    if (m_viewModel) {
        m_viewModel->refreshCommandStates();
        m_viewModel->loadMeta();
        m_viewModel->refreshList();
    }
}

void UserManagementView::retranslateUi()
{
    syncListHeaders();

    if (m_pageTitle)
        m_pageTitle->setText(PTXT("perm.users.page_title"));
    if (m_searchEdit)
        m_searchEdit->setPlaceholderText(PTXT("perm.users.search_placeholder"));
    if (m_btnSearch)
        m_btnSearch->setText(PTXT("perm.common.search"));
    if (m_btnReset)
        m_btnReset->setText(PTXT("perm.common.reset"));
    if (m_btnAdd)
        m_btnAdd->setText(PTXT("perm.users.add"));
    if (m_btnBulkDisable)
        m_btnBulkDisable->setText(PTXT("perm.users.bulk_disable"));
    if (m_btnBulkEnable)
        m_btnBulkEnable->setText(PTXT("perm.users.bulk_enable"));
    if (m_btnBulkRole)
        m_btnBulkRole->setText(PTXT("perm.users.bulk_assign_roles"));

    if (m_statusFilter && m_statusFilter->count() >= 3) {
        const int st = m_statusFilter->currentData().toInt();
        m_statusFilter->blockSignals(true);
        m_statusFilter->setItemText(0, PTXT("perm.common.all_statuses"));
        m_statusFilter->setItemText(1, PTXT("perm.common.enabled"));
        m_statusFilter->setItemText(2, PTXT("perm.common.disabled"));
        m_statusFilter->setCurrentIndex(st < 0 ? 0 : (st == 1 ? 1 : 2));
        m_statusFilter->blockSignals(false);
    }

    syncRoleFilterCombo();
    syncUserList();
}

QString UserManagementView::validationMessage(const QString &key) const
{
    if (key == QStringLiteral("username_required"))
        return PTXT("perm.users.please_enter_username");
    if (key == QStringLiteral("password_required"))
        return PTXT("perm.users.please_enter_password");
    if (key == QStringLiteral("display_name_required"))
        return PTXT("perm.resources.please_enter_display_name");
    if (key == QStringLiteral("department_required"))
        return PTXT("perm.users.please_enter_department");
    if (key == QStringLiteral("role_required"))
        return PTXT("perm.users.please_select_role");
    if (key == QStringLiteral("protected_user_status"))
        return PTXT("perm.users.cannot_toggle_status");
    if (key == QStringLiteral("role_not_assignable"))
        return PTXT("perm.users.cannot_assign_super_admin");
    return key;
}

void UserManagementView::showUserEditor(qint64 userId)
{
    if (!m_viewModel)
        return;

    const bool         editing = userId > 0;
    const UserEditorForm loaded = m_viewModel->loadEditorForm(userId);

    auto *dlg = new QWidget(this, Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowModality(Qt::ApplicationModal);
    dlg->setFixedSize(520, 460);
    permission_ui::applyPermissionTheme(dlg);
    dlg->setStyleSheet(dlg->styleSheet() + permission_ui::permissionDialogPanelStyleSheet());

    auto *root = new QVBoxLayout(dlg);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);

    auto *title = new QLabel(editing ? PTXT("perm.users.edit_user") : PTXT("perm.users.add"), dlg);
    permission_ui::initPageTitle(title, true);

    auto *titleLine = new QFrame(dlg);
    titleLine->setFrameShape(QFrame::HLine);
    titleLine->setStyleSheet(permission_ui::permissionDialogDividerStyleSheet());

    auto *form = new QFormLayout;
    form->setSpacing(12);
    form->setHorizontalSpacing(16);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    auto reqLabel = [dlg](const QString &text) {
        auto *lbl = new QLabel(dlg);
        lbl->setText(QStringLiteral("%1 <font color='#ef4444'>*</font>").arg(text));
        lbl->setTextFormat(Qt::RichText);
        return lbl;
    };

    auto *username    = new ui::ThemedLineEdit(dlg);
    auto *password    = new ui::ThemedLineEdit(dlg);
    auto *displayName = new ui::ThemedLineEdit(dlg);
    auto *department  = new ui::ThemedLineEdit(dlg);
    auto *roleCombo   = new permission_ui::PermComboBox(dlg);
    auto *statusCombo = new permission_ui::PermComboBox(dlg);
    auto *remark      = new ui::ThemedLineEdit(dlg);

    username->setPlaceholderText(PTXT("perm.users.placeholder_account"));
    password->setEchoMode(QLineEdit::Password);
    displayName->setPlaceholderText(PTXT("perm.users.label_display_name"));
    department->setPlaceholderText(PTXT("perm.users.col_department"));
    remark->setPlaceholderText(PTXT("perm.common.optional"));
    permission_ui::initFormLineEdit(username);
    permission_ui::initFormLineEdit(password);
    permission_ui::initFormLineEdit(displayName);
    permission_ui::initFormLineEdit(department);
    permission_ui::initFormLineEdit(remark);
    permission_ui::initComboBox(roleCombo);
    permission_ui::initComboBox(statusCombo);

    for (auto it = m_viewModel->rolesByCode().constBegin(); it != m_viewModel->rolesByCode().constEnd();
         ++it)
        roleCombo->addItem(it.value().name, it.key());
    statusCombo->addItem(PTXT("perm.common.enabled"), 1);
    statusCombo->addItem(PTXT("perm.common.disabled"), 0);
    constexpr int kComboWidth = 240;
    roleCombo->setFixedWidth(kComboWidth);
    statusCombo->setFixedWidth(kComboWidth);

    username->setText(loaded.username);
    password->setText(loaded.password);
    displayName->setText(loaded.displayName);
    department->setText(loaded.department);
    remark->setText(loaded.remark);
    if (editing)
        username->setReadOnly(true);
    else
        password->setPlaceholderText(PTXT("perm.users.placeholder_password"));

    const int roleIdx = roleCombo->findData(loaded.roleCode);
    if (roleIdx >= 0)
        roleCombo->setCurrentIndex(roleIdx);
    const int statusIdx = statusCombo->findData(loaded.accountStatus);
    if (statusIdx >= 0)
        statusCombo->setCurrentIndex(statusIdx);
    if (editing && loaded.username == QLatin1String(permission::kMinimalUsername))
        statusCombo->setEnabled(false);

    form->addRow(reqLabel(PTXT("perm.users.col_username")), username);
    form->addRow(reqLabel(PTXT("perm.users.label_password")), password);
    form->addRow(reqLabel(PTXT("perm.resources.label_display_name")), displayName);
    form->addRow(reqLabel(PTXT("perm.users.label_department")), department);
    form->addRow(reqLabel(PTXT("perm.users.col_role")), roleCombo);
    form->addRow(reqLabel(PTXT("perm.users.col_status")), statusCombo);
    form->addRow(PTXT("perm.users.label_remarks"), remark);

    auto *line = new QFrame(dlg);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(permission_ui::permissionDialogDividerStyleSheet());

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *btnCancel = permission_ui::makeSecondaryButton(PTXT("perm.common.cancel"), dlg);
    auto *btnOk     = permission_ui::makeSecondaryButton(PTXT("perm.common.confirm"), dlg);
    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnOk);

    root->addWidget(title);
    root->addWidget(titleLine);
    root->addLayout(form, 1);
    root->addWidget(line);
    root->addLayout(btnRow);

    connect(btnCancel, &ui::ThemedPushButton::clicked, dlg, &QWidget::close);
    connect(btnOk, &ui::ThemedPushButton::clicked, dlg,
            [this, dlg, editing, userId, username, password, displayName, department, remark,
             roleCombo, statusCombo]() {
                const QString savePerm =
                    editing ? permission::PermKeys::UsersEdit : permission::PermKeys::UsersCreate;
                if (!PS->canAction(savePerm))
                    return;

                UserEditorForm form;
                form.editingUserId = editing ? userId : 0;
                form.username      = username->text().trimmed();
                form.password      = password->text();
                form.displayName   = displayName->text().trimmed();
                form.department    = department->text().trimmed();
                form.remark        = remark->text().trimmed();
                form.roleCode      = roleCombo->currentData().toString();
                form.accountStatus = statusCombo->currentData().toInt();

                const OperationResult validation = m_viewModel->validateEditorForm(form);
                if (!validation.ok) {
                    permission_ui::MessagePopup::show(
                        this, PTXT("perm.common.notice"), validationMessage(validation.message),
                        QDialogButtonBox::Ok);
                    return;
                }

                const OperationResult saved = m_viewModel->saveEditorForm(form);
                if (!saved.ok) {
                    const QString msg =
                        saved.message.isEmpty() ? PTXT("perm.err.save_failed") : validationMessage(saved.message);
                    permission_ui::MessagePopup::show(this, PTXT("perm.common.error"), msg,
                                                      QDialogButtonBox::Ok);
                    return;
                }

                m_viewModel->refreshList();
                dlg->close();
            });

    if (QWidget *host = window()) {
        const QPoint c = host->geometry().center();
        dlg->move(c.x() - dlg->width() / 2, c.y() - dlg->height() / 2);
    }
    dlg->show();
}

} // namespace permission
