#include "ui/view/role_manage/role_management_view.h"

#include "ui/view/role_manage/role_list_item_delegate.h"
#include "ui/view/role_manage/role_permission_list_model.h"
#include "ui/view/role_manage/role_permission_page_delegate.h"

#include "model/role_management_model.h"
#include "ui/view/message_popup.h"
#include "ui/view/perm_i18n_utils.h"
#include "ui/view/perm_text.h"
#include "ui/view/permission_ui_utils.h"
#include "ui/vm/role_management_view_model.h"

#include "databinding.h"
#include "service/perm_action_keys.h"
#include "uibase/permission/permservice.h"

#include "controls/button/themed_push_button.h"
#include "controls/input/themed_line_edit.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSet>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace permission {

namespace {

void initPermissionList(QListView *list)
{
    list->setObjectName(QStringLiteral("permRolePermList"));
    list->setFrameShape(QFrame::NoFrame);
    list->setSelectionMode(QAbstractItemView::NoSelection);
    list->setFocusPolicy(Qt::NoFocus);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    list->setSpacing(0);
    list->setStyleSheet(QStringLiteral(
        "QListView#permRolePermList{background:transparent;border:none;outline:none;}"
        "QListView#permRolePermList::item{background:transparent;border:none;padding:0;margin:0;}"
        "QListView#permRolePermList::item:selected{background:transparent;}"));
}

} // namespace

RoleManagementView::RoleManagementView(QWidget *parent)
    : PageWidget(parent)
{
}

void RoleManagementView::applyTheme()
{
    permission_ui::applyPermissionTheme(this);
}

void RoleManagementView::onLoad()
{
    permission_ui::applyPermissionTheme(this);

    m_viewModel = new RoleManagementViewModel(nullptr, this);
    m_permModel = new RolePermissionListModel(this);

    buildLayout();
    setupBindings();

    retranslateUi();
    updatePermissionListInteraction();
    m_viewModel->refreshRoleList();
}

void RoleManagementView::onShow()
{
    if (m_viewModel)
        m_viewModel->reloadPermissionTree();
    relayoutPermissionList();
}

void RoleManagementView::buildLayout()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(16);

    auto *leftPanel = new QWidget(this);
    leftPanel->setFixedWidth(300);
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(10);

    auto *leftHeader = new QHBoxLayout;
    m_listTitleLabel = new QLabel(leftPanel);
    permission_ui::initPageTitle(m_listTitleLabel);
    m_btnAddRole = permission_ui::makePrimaryButton(QString(), leftPanel);
    leftHeader->addWidget(m_listTitleLabel);
    leftHeader->addStretch();
    leftHeader->addWidget(m_btnAddRole);

    m_roleList = new QListWidget(leftPanel);
    permission_ui::initRoleList(m_roleList);
    m_roleList->setItemDelegate(new RoleListItemDelegate(m_roleList));

    leftLayout->addLayout(leftHeader);
    leftLayout->addWidget(m_roleList, 1);

    auto *rightPanel = new QWidget(this);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    auto *rightHeader = new QHBoxLayout;
    m_permTitle = new QLabel(rightPanel);
    permission_ui::initPageTitle(m_permTitle);
    m_btnEditRole    = permission_ui::makeSecondaryButton(QString(), rightPanel);
    m_btnSelectAll   = permission_ui::makeSecondaryButton(QString(), rightPanel);
    m_btnClear       = permission_ui::makeSecondaryButton(QString(), rightPanel);
    m_btnReset       = permission_ui::makeSecondaryButton(QString(), rightPanel);
    m_btnSave        = permission_ui::makePrimaryButton(QString(), rightPanel);
    rightHeader->addWidget(m_permTitle);
    rightHeader->addStretch();
    rightHeader->addWidget(m_btnEditRole);
    rightHeader->addWidget(m_btnSelectAll);
    rightHeader->addWidget(m_btnClear);
    rightHeader->addWidget(m_btnReset);
    rightHeader->addWidget(m_btnSave);

    m_permDesc = new QLabel(rightPanel);
    m_permDesc->setWordWrap(true);
    permission_ui::initDescLabel(m_permDesc);

    m_permList = new QListView(rightPanel);
    initPermissionList(m_permList);
    m_permList->setModel(m_permModel);
    m_permDelegate = new RolePermissionPageDelegate(m_permModel, m_permList);
    m_permList->setItemDelegate(m_permDelegate);

    rightLayout->addLayout(rightHeader);
    rightLayout->addWidget(m_permDesc);
    rightLayout->addWidget(m_permList, 1);

    root->addWidget(leftPanel);
    root->addWidget(rightPanel, 1);
}

void RoleManagementView::setupBindings()
{
    connect(m_viewModel, &RoleManagementViewModel::selectedRoleIdChanged, this, [this]() {
        updatePermissionListInteraction();
        updatePermissionActionButtons();
        relayoutPermissionList();
    });

    connect(PS, &PermService::permissionsChanged, this, [this]() {
        if (m_viewModel)
            m_viewModel->refreshCommandStates();
        updatePermissionListInteraction();
        updatePermissionActionButtons();
    });

    connect(m_roleList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *previous) {
                if (m_blockRoleListSelection || !m_viewModel || !current)
                    return;

                const qint64 roleId = current->data(Qt::UserRole).toLongLong();
                if (roleId == m_viewModel->selectedRoleId())
                    return;

                if (isPermissionGrantsDirty()
                    && !confirmDiscardPermissionEdits(PTXT(
                           "perm.roles.discard_unsaved_switch"))) {
                    QListWidgetItem *revertTo = previous;
                    if (!revertTo) {
                        const int row = roleListRowForId(m_viewModel->selectedRoleId());
                        revertTo = row >= 0 ? m_roleList->item(row) : nullptr;
                    }
                    if (revertTo) {
                        QTimer::singleShot(0, this, [this, revertTo]() {
                            if (m_blockRoleListSelection || !m_roleList)
                                return;
                            m_blockRoleListSelection = true;
                            m_roleList->setCurrentItem(revertTo,
                                                       QItemSelectionModel::ClearAndSelect);
                            m_roleList->scrollToItem(revertTo);
                            m_blockRoleListSelection = false;
                            m_roleList->viewport()->update();
                        });
                    }
                    return;
                }

                m_viewModel->selectRole(roleId);
            });

    connect(m_viewModel, &RoleManagementViewModel::roleListChanged, this,
            &RoleManagementView::syncRoleList);
    connect(m_viewModel, &RoleManagementViewModel::roleDetailChanged, this,
            &RoleManagementView::syncRoleDetail);
    connect(m_viewModel, &RoleManagementViewModel::permissionPanelChanged, this,
            &RoleManagementView::rebuildPermissionPanel);

    connect(m_viewModel, &RoleManagementViewModel::showInfo, this,
            [this](const QString &title, const QString &message) {
                permission_ui::MessagePopup::show(this, title, message, QDialogButtonBox::Ok);
            });
    connect(m_viewModel, &RoleManagementViewModel::showWarning, this,
            [this](const QString &title, const QString &message) {
                permission_ui::MessagePopup::show(this, title.isEmpty() ? PTXT("perm.common.notice") : title,
                                                 message, QDialogButtonBox::Ok);
            });

    DataBinding::bindCommand(m_viewModel->createRoleCommand(), m_btnAddRole);
    DataBinding::bindCommand(m_viewModel->editRoleCommand(), m_btnEditRole);
    DataBinding::bindCommand(m_viewModel->selectAllPermissionsCommand(), m_btnSelectAll);
    DataBinding::bindCommand(m_viewModel->clearPermissionsCommand(), m_btnClear);
    // 重置/保存：可用状态由 updatePermissionActionButtons（须 dirty）控制，仅绑定点击到 Command
    connect(m_btnReset, &ui::ThemedPushButton::clicked, m_viewModel->resetPermissionsCommand(),
            &Command::run);
    connect(m_btnSave, &ui::ThemedPushButton::clicked, m_viewModel->savePermissionsCommand(),
            &Command::run);

    connect(m_viewModel, &RoleManagementViewModel::requestCreateRole, this,
            [this]() { showRoleEditor(0); });

    connect(m_viewModel, &RoleManagementViewModel::requestEditRole, this, [this]() {
        if (!m_viewModel || m_viewModel->selectedRoleId() <= 0)
            return;
        showRoleEditor(m_viewModel->selectedRoleId());
    });

    connect(m_viewModel, &RoleManagementViewModel::requestSelectAllPermissions, this, [this] {
        if (m_permModel)
            m_permModel->selectAll();
    });
    connect(m_viewModel, &RoleManagementViewModel::requestClearPermissions, this, [this] {
        if (m_permModel)
            m_permModel->clearAll();
    });
    connect(m_viewModel, &RoleManagementViewModel::requestResetPermissions, this,
            &RoleManagementView::resetPermissionGrants);
    connect(m_viewModel, &RoleManagementViewModel::requestSavePermissions, this,
            &RoleManagementView::collectAndSavePermissions);

    connect(m_permModel, &RolePermissionListModel::grantsChanged, this,
            &RoleManagementView::updatePermissionActionButtons);
}

void RoleManagementView::retranslateUi()
{
    if (m_listTitleLabel)
        m_listTitleLabel->setText(PTXT("perm.roles.list_title"));
    if (m_btnAddRole)
        m_btnAddRole->setText(PTXT("perm.roles.add"));
    if (m_btnEditRole)
        m_btnEditRole->setText(PTXT("perm.common.edit"));
    if (m_btnSelectAll)
        m_btnSelectAll->setText(PTXT("perm.common.select_all"));
    if (m_btnClear)
        m_btnClear->setText(PTXT("perm.common.clear"));
    if (m_btnReset)
        m_btnReset->setText(PTXT("perm.common.reset"));
    if (m_btnSave)
        m_btnSave->setText(PTXT("perm.roles.save_permissions"));

    syncRoleList();
    syncRoleDetail();
    if (m_permModel && m_viewModel)
        m_permModel->setPermissionTree(m_viewModel->allPermissions(),
                                       m_viewModel->grantedPermissionIds());
}

void RoleManagementView::syncRoleList()
{
    if (!m_viewModel || !m_roleList)
        return;

    const qint64 selectedId = m_viewModel->selectedRoleId();
    m_roleList->blockSignals(true);
    m_roleList->clear();

    for (const auto &item : m_viewModel->roleListItems()) {
        auto *listItem = new QListWidgetItem(
            permission_ui::builtInRoleName(item), m_roleList);
        listItem->setData(Qt::UserRole, item.id);
        listItem->setData(RoleListItemDelegate::RoleDescriptionRole,
                          permission_ui::builtInRoleDescription(item));
        if (item.id == selectedId)
            m_roleList->setCurrentItem(listItem);
    }

    if (m_roleList->currentRow() < 0 && m_roleList->count() > 0)
        m_roleList->setCurrentRow(0);
    m_roleList->blockSignals(false);
    relayoutPermissionList();
}

void RoleManagementView::syncRoleDetail()
{
    if (!m_viewModel || !m_permTitle || !m_permDesc)
        return;

    const qint64 roleId = m_viewModel->selectedRoleId();
    if (roleId <= 0) {
        m_permTitle->setText(PTXT("perm.roles.permissions_title"));
        m_permDesc->clear();
        return;
    }

    const auto &role = m_viewModel->selectedRole();
    m_permTitle->setText(PTXT("perm.roles.permissions_title_with_name").arg(permission_ui::builtInRoleName(role)));
    m_permDesc->setText(permission_ui::builtInRoleDescription(role));
}

void RoleManagementView::rebuildPermissionPanel()
{
    if (!m_viewModel || !m_permModel)
        return;

    if (m_permDelegate)
        m_permDelegate->resetNavExpandState();

    m_permModel->setPermissionTree(m_viewModel->allPermissions(),
                                   m_viewModel->grantedPermissionIds());
    updatePermissionListInteraction();
    updatePermissionActionButtons();
    relayoutPermissionList();
}

void RoleManagementView::updatePermissionListInteraction()
{
    const bool hasRole = m_viewModel && m_viewModel->selectedRoleId() > 0;
    const bool canEdit =
        hasRole
        && (PS->canAction(permission::PermKeys::RolesSave)
            || PS->canAction(permission::PermKeys::RolesSelectAll)
            || PS->canAction(permission::PermKeys::RolesClear));

    if (m_permList)
        m_permList->setEnabled(hasRole);
    if (m_permDelegate)
        m_permDelegate->setInteractionEnabled(canEdit);
}

void RoleManagementView::relayoutPermissionList()
{
    if (!m_permList)
        return;
    QTimer::singleShot(0, m_permList, [list = m_permList]() {
        if (!list)
            return;
        list->doItemsLayout();
        list->viewport()->update();
    });
}

void RoleManagementView::collectAndSavePermissions()
{
    if (!m_viewModel || !m_permModel || !isPermissionGrantsDirty())
        return;

    const auto result = m_viewModel->saveRolePermissions(m_permModel->grantedIds());
    if (result.ok)
        updatePermissionActionButtons();
}

void RoleManagementView::resetPermissionGrants()
{
    if (!m_viewModel || !m_permModel || !isPermissionGrantsDirty())
        return;

    m_permModel->restoreGrantedIds(m_viewModel->grantedPermissionIds());
}

void RoleManagementView::updatePermissionActionButtons()
{
    const bool hasRole  = m_viewModel && m_viewModel->selectedRoleId() > 0;
    const bool dirty    = hasRole && isPermissionGrantsDirty();
    const bool canReset = PS->canAction(permission::PermKeys::RolesReset);
    const bool canSave  = PS->canAction(permission::PermKeys::RolesSave);

    if (m_btnReset)
        m_btnReset->setEnabled(dirty && canReset);
    if (m_btnSave)
        m_btnSave->setEnabled(dirty && canSave);
}

bool RoleManagementView::isPermissionGrantsDirty() const
{
    if (!m_viewModel || !m_permModel || m_viewModel->selectedRoleId() <= 0)
        return false;

    QSet<qint64> saved;
    for (qint64 id : m_viewModel->grantedPermissionIds())
        saved.insert(id);

    QSet<qint64> current;
    for (qint64 id : m_permModel->grantedIds())
        current.insert(id);

    return saved != current;
}

int RoleManagementView::roleListRowForId(qint64 roleId) const
{
    if (!m_roleList || roleId <= 0)
        return -1;

    for (int i = 0; i < m_roleList->count(); ++i) {
        const QListWidgetItem *item = m_roleList->item(i);
        if (item && item->data(Qt::UserRole).toLongLong() == roleId)
            return i;
    }
    return -1;
}

void RoleManagementView::selectRoleListRow(qint64 roleId)
{
    if (!m_roleList)
        return;

    const int row = roleListRowForId(roleId);
    if (row < 0)
        return;

    QListWidgetItem *item = m_roleList->item(row);
    if (!item)
        return;

    m_blockRoleListSelection = true;
    m_roleList->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
    m_roleList->scrollToItem(item);
    m_blockRoleListSelection = false;
    m_roleList->viewport()->update();
}

bool RoleManagementView::confirmDiscardPermissionEdits(const QString &message)
{
    return permission_ui::MessagePopup::show(
               this, PTXT("perm.common.notice"), message, QDialogButtonBox::Yes | QDialogButtonBox::No,
               QDialogButtonBox::No)
           == QDialogButtonBox::Yes;
}

QString RoleManagementView::validationMessage(const QString &key) const
{
    if (key == QStringLiteral("role_name_required"))
        return PTXT("perm.roles.please_enter_name");
    if (key == QStringLiteral("role_name_duplicate"))
        return PTXT("perm.roles.name_duplicate");
    if (key == QStringLiteral("role_not_found"))
        return PTXT("perm.roles.not_found");
    if (key == QStringLiteral("protected_role_edit"))
        return PTXT("perm.roles.not_editable");
    if (key == QStringLiteral("copy_minimal_grants_failed"))
        return PTXT("perm.roles.copy_minimal_grants_failed");
    if (key == QStringLiteral("insert_failed"))
        return PTXT("perm.err.save_failed");
    return key;
}

void RoleManagementView::showRoleEditor(qint64 roleId)
{
    if (!m_viewModel)
        return;

    const bool                  editing = roleId > 0;
    const permission::RoleEditorForm loaded = m_viewModel->loadEditorForm(roleId);

    auto *dlg = new QWidget(this, Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowModality(Qt::ApplicationModal);
    dlg->setFixedSize(520, 340);
    permission_ui::bindPermissionDialogTheme(dlg);
    dlg->setStyleSheet(dlg->styleSheet() + permission_ui::permissionDialogPanelStyleSheet());

    auto *root = new QVBoxLayout(dlg);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);

    auto *title = new QLabel(editing ? PTXT("perm.roles.edit") : PTXT("perm.roles.add"), dlg);
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

    auto *nameEdit = new ui::ThemedLineEdit(dlg);
    auto *descEdit = new QTextEdit(dlg);
    nameEdit->setPlaceholderText(PTXT("perm.roles.label_name"));
    descEdit->setPlaceholderText(PTXT("perm.roles.description_placeholder"));
    permission_ui::initFormLineEdit(nameEdit);
    permission_ui::initResourceFormTextEdit(descEdit);

    nameEdit->setText(loaded.name);
    descEdit->setPlainText(loaded.description);

    form->addRow(reqLabel(PTXT("perm.roles.label_name")), nameEdit);
    form->addRow(PTXT("perm.common.description"), descEdit);

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
            [this, dlg, editing, roleId, nameEdit, descEdit]() {
                const QString savePerm = editing ? permission::PermKeys::RolesEdit
                                                 : permission::PermKeys::RolesCreate;
                if (!PS->canAction(savePerm))
                    return;

                permission::RoleEditorForm form;
                form.editingRoleId = editing ? roleId : 0;
                form.name          = nameEdit->text().trimmed();
                form.description   = descEdit->toPlainText().trimmed();

                const permission::OperationResult validation =
                    m_viewModel->validateEditorForm(form);
                if (!validation.ok) {
                    permission_ui::MessagePopup::show(
                        this, PTXT("perm.common.notice"), validationMessage(validation.message),
                        QDialogButtonBox::Ok);
                    return;
                }

                const permission::OperationResult saved = m_viewModel->saveEditorForm(form);
                if (!saved.ok) {
                    const QString msg =
                        saved.message.isEmpty()
                            ? PTXT("perm.err.save_failed")
                            : validationMessage(saved.message);
                    permission_ui::MessagePopup::show(this, PTXT("perm.common.error"), msg,
                                                      QDialogButtonBox::Ok);
                    return;
                }

                dlg->close();
            });

    if (QWidget *host = window()) {
        const QPoint c = host->geometry().center();
        dlg->move(c.x() - dlg->width() / 2, c.y() - dlg->height() / 2);
    }
    dlg->show();
}

} // namespace permission
