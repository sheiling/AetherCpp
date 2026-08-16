#include "ui/vm/permission_resource_view_model.h"

#include "data/domain/domain_types.h"
#include "service/perm_action_keys.h"
#include "service/perm_system_constants.h"
#include "ui/view/perm_i18n_utils.h"
#include "ui/view/perm_text.h"
#include "uibase/permission/permservice.h"

#include <QMap>
#include <QSet>

PermissionResourceViewModel::PermissionResourceViewModel(
    permission::PermissionResourceModel *model, QObject *parent)
    : ViewModel(parent)
{
    if (model) {
        m_model = model;
    } else {
        m_ownedModel = std::make_unique<permission::PermissionResourceModel>(nullptr);
        m_model      = m_ownedModel.get();
    }

    registerProperty(PROP_SEARCH_KEYWORD, QString());
    registerProperty(PROP_RESOURCE_TYPE_FILTER, QString());
    registerProperty(PROP_ENABLED_FILTER_MODE, QVariant::fromValue(-1));
    registerProperty(PROP_SELECTED_RESOURCE_ID, QVariant::fromValue(qint64(0)));

    bindCommands();
    connect(this, &PermissionResourceViewModel::editorDirtyChanged, this,
            [this]() { updateCommandStates(); });
    connect(this, &PermissionResourceViewModel::selectedResourceIdChanged, this,
            [this]() { updateCommandStates(); });
}

namespace {

bool canResourceSaveOrReset(const QString &actionKey)
{
    if (PS->canAction(actionKey))
        return true;
    return PS->canAction(QStringLiteral("perm.resources.edit"));
}

} // namespace

void PermissionResourceViewModel::bindCommands()
{
    m_saveCommand = new FunctionalCommand(
        [this]() { saveEditor(); },
        [this]() {
            if (permission::kPermissionResourcesCatalogReadOnly)
                return false;
            return canResourceSaveOrReset(permission::PermKeys::ResourcesSave) && isEditorDirty();
        },
        this);

    m_resetCommand = new FunctionalCommand(
        [this]() { resetEditor(); },
        [this]() {
            if (permission::kPermissionResourcesCatalogReadOnly)
                return false;
            return canResourceSaveOrReset(permission::PermKeys::ResourcesReset) && isEditorDirty();
        },
        this);

    updateCommandStates();
}

void PermissionResourceViewModel::refreshCommandStates()
{
    updateCommandStates();
}

void PermissionResourceViewModel::updateCommandStates()
{
    if (m_saveCommand)
        m_saveCommand->updateCanExecute();
    if (m_resetCommand)
        m_resetCommand->updateCanExecute();
}

bool PermissionResourceViewModel::isEditorDirty() const
{
    if (m_creating)
        return true;

    const auto &a = m_editorForm;
    const auto &b = m_editorBaseline;
    return a.displayName != b.displayName || a.description != b.description
           || a.sortOrder != b.sortOrder || a.enabled != b.enabled
           || a.pageKey != b.pageKey;
}

QString PermissionResourceViewModel::validationMessage(const QString &key) const
{
    static const QMap<QString, QString> kMap = {
        {QStringLiteral("key_required"), PTXT("perm.err.key_required")},
        {QStringLiteral("display_name_required"), PTXT("perm.resources.please_enter_display_name")},
        {QStringLiteral("invalid_resource_type"), PTXT("perm.err.invalid_resource_type")},
        {QStringLiteral("key_duplicate"), PTXT("perm.err.key_duplicate")},
        {QStringLiteral("resource_not_found"), PTXT("perm.err.resource_not_found")},
        {QStringLiteral("key_immutable"), PTXT("perm.err.key_immutable")},
        {QStringLiteral("resource_type_immutable"), PTXT("perm.err.resource_type_immutable")},
        {QStringLiteral("nav_parent_must_be_root"), PTXT("perm.err.nav_parent_must_be_root")},
        {QStringLiteral("parent_required"), PTXT("perm.err.parent_required")},
        {QStringLiteral("parent_not_found"), PTXT("perm.err.parent_not_found")},
        {QStringLiteral("sub_page_parent_must_be_nav"), PTXT("perm.err.sub_page_parent_must_be_nav")},
        {QStringLiteral("action_parent_invalid"), PTXT("perm.err.action_parent_invalid")},
        {QStringLiteral("store_unavailable"), PTXT("perm.err.store_unavailable")},
        {QStringLiteral("insert_failed"), PTXT("perm.err.create_failed")},
        {QStringLiteral("update_failed"), PTXT("perm.err.save_failed")},
        {QStringLiteral("delete_failed"), PTXT("perm.err.delete_failed")},
        {QStringLiteral("has_children"), PTXT("perm.err.has_children")},
        {QStringLiteral("has_grant_refs"), PTXT("perm.err.has_grant_refs")},
    };
    return kMap.value(key, key);
}

void PermissionResourceViewModel::refreshTree()
{
    m_allRecords = m_model->loadAll();
    applyFilters();

    if (selectedResourceId() > 0) {
        permission::PermissionRecord rec;
        if (m_model->findById(selectedResourceId(), rec))
            loadEditorForId(selectedResourceId());
        else
            selectResource(0);
    }
}

void PermissionResourceViewModel::applyFilters()
{
    rebuildVisibleForest();
    emit resourceTreeChanged();
}

void PermissionResourceViewModel::rebuildVisibleForest()
{
    m_visibleForest = m_model->buildVisibleForest(m_allRecords, searchKeyword(),
                                                  resourceTypeFilter(), enabledFilterMode());
}

void PermissionResourceViewModel::noteTreeExpanded(qint64 id, bool expanded)
{
    if (id <= 0)
        return;
    if (expanded)
        m_expandedResourceIds.insert(id);
    else
        m_expandedResourceIds.remove(id);
}

void PermissionResourceViewModel::ensureExpandedOnPathTo(qint64 id)
{
    if (id <= 0 || !m_model)
        return;

    permission::PermissionRecord rec;
    if (!m_model->findById(id, rec))
        return;

    qint64 pid = rec.parentId;
    while (pid > 0) {
        m_expandedResourceIds.insert(pid);
        permission::PermissionRecord parent;
        if (!m_model->findById(pid, parent))
            break;
        pid = parent.parentId;
    }
}

void PermissionResourceViewModel::selectResource(qint64 id)
{
    setSelectedResourceId(id);
    emit selectedResourceIdChanged();

    if (id <= 0) {
        m_creating = false;
        m_editorForm = {};
        m_editorBaseline = {};
        emit editorFormChanged();
        emit editorDirtyChanged();
        updateCommandStates();
        return;
    }

    loadEditorForId(id);
}

void PermissionResourceViewModel::loadEditorForId(qint64 id)
{
    permission::PermissionRecord rec;
    if (!m_model->findById(id, rec)) {
        m_creating = false;
        m_editorForm = {};
        m_editorBaseline = {};
        emit editorFormChanged();
        emit editorDirtyChanged();
        updateCommandStates();
        return;
    }

    m_creating       = false;
    m_editorForm     = m_model->toEditorForm(rec, parentDisplayName(rec.parentId));
    m_editorForm.grantRefCount = m_model->grantReferenceCount(id);
    m_editorBaseline = m_editorForm;
    emit editorFormChanged();
    emit editorDirtyChanged();
    updateCommandStates();
}

void PermissionResourceViewModel::loadEditorForCreate(
    const permission::PermissionResourceEditorForm &draft)
{
    m_creating       = true;
    m_editorForm     = draft;
    m_editorBaseline = {};
    emit editorFormChanged();
    emit editorDirtyChanged();
    updateCommandStates();
}

QString PermissionResourceViewModel::parentDisplayName(qint64 parentId) const
{
    if (parentId <= 0)
        return {};
    permission::PermissionRecord p;
    if (!m_model->findById(parentId, p))
        return {};
    return permission_ui::permissionResourceDisplayText(p.key, p.displayName);
}

permission::PermissionResourceEditorForm PermissionResourceViewModel::defaultCreateForm(
    const QString &resourceType, qint64 parentId) const
{
    permission::PermissionResourceEditorForm form;
    form.editingId    = 0;
    form.resourceType = resourceType;
    form.parentId     = resourceType == permission::PermResourceType::Nav ? 0 : parentId;
    form.enabled      = true;
    form.source       = QStringLiteral("manual");
    form.sortOrder    = m_model->nextSortOrder(form.parentId);
    form.parentDisplayName = parentDisplayName(parentId);

    if (resourceType == permission::PermResourceType::Nav) {
        form.pageKey = QString::fromUtf8(permission::kPermPageKeyRoot);
        form.key     = QString();
    } else if (resourceType == permission::PermResourceType::SubPage) {
        permission::PermissionRecord parent;
        if (m_model->findById(parentId, parent))
            form.pageKey = parent.resourceType == permission::PermResourceType::Nav
                               ? parent.key
                               : parent.pageKey;
        form.key = QString();
    } else {
        permission::PermissionRecord parent;
        if (m_model->findById(parentId, parent))
            form.pageKey = parent.resourceType == permission::PermResourceType::SubPage
                               ? parent.key
                               : parent.pageKey;
        form.key = QString();
    }
    return form;
}

void PermissionResourceViewModel::beginCreateNav()
{
    if (permission::kPermissionResourcesCatalogReadOnly)
        return;
    loadEditorForCreate(defaultCreateForm(permission::PermResourceType::Nav, 0));
    setSelectedResourceId(0);
    emit selectedResourceIdChanged();
}

void PermissionResourceViewModel::beginCreateSubPage()
{
    if (permission::kPermissionResourcesCatalogReadOnly)
        return;
    const qint64 parentId = selectedResourceId() > 0 ? selectedResourceId() : 0;
    permission::PermissionRecord parent;
    qint64 navId = parentId;
    if (m_model->findById(parentId, parent)) {
        if (parent.resourceType == permission::PermResourceType::SubPage)
            navId = parent.parentId;
        else if (parent.resourceType != permission::PermResourceType::Nav)
            navId = 0;
    } else {
        navId = 0;
    }
    if (navId <= 0) {
        emit showWarning(PTXT("perm.common.notice"), PTXT("perm.resources.please_select_nav"));
        return;
    }
    loadEditorForCreate(
        defaultCreateForm(permission::PermResourceType::SubPage, navId));
    setSelectedResourceId(0);
    emit selectedResourceIdChanged();
}

void PermissionResourceViewModel::beginCreateAction()
{
    if (permission::kPermissionResourcesCatalogReadOnly)
        return;
    qint64 parentId = selectedResourceId();
    if (parentId <= 0) {
        emit showWarning(PTXT("perm.common.notice"), PTXT("perm.resources.please_select_nav_or_sub"));
        return;
    }
    permission::PermissionRecord parent;
    if (!m_model->findById(parentId, parent)) {
        emit showWarning(PTXT("perm.common.notice"), PTXT("perm.err.parent_not_found"));
        return;
    }
    if (parent.resourceType == permission::PermResourceType::Action) {
        emit showWarning(PTXT("perm.common.notice"), PTXT("perm.err.action_as_parent"));
        return;
    }
    loadEditorForCreate(defaultCreateForm(permission::PermResourceType::Action, parentId));
    setSelectedResourceId(0);
    emit selectedResourceIdChanged();
}

void PermissionResourceViewModel::updateEditorForm(
    const permission::PermissionResourceEditorForm &form)
{
    m_editorForm = form;
    emit editorFormChanged();
    emit editorDirtyChanged();
    updateCommandStates();
}

permission::OperationResult PermissionResourceViewModel::saveEditor()
{
    if (permission::kPermissionResourcesCatalogReadOnly) {
        permission::OperationResult r;
        return r;
    }
    if (!isEditorDirty() && !m_creating) {
        permission::OperationResult r;
        r.ok = true;
        return r;
    }

    permission::OperationResult r;
    if (m_creating || m_editorForm.editingId <= 0) {
        permission::PermissionRecord created;
        r = m_model->createResource(m_editorForm, &created);
        if (!r.ok) {
            if (r.message == QStringLiteral("key_duplicate")) {
                permission::PermissionRecord existing;
                if (m_model->findByKey(m_editorForm.key, existing))
                    emit keyConflictFound(existing.id, existing.enabled);
            } else {
                emit showWarning(PTXT("perm.common.error"), validationMessage(r.message));
            }
            return r;
        }
        m_creating = false;
        ensureExpandedOnPathTo(created.id);
        refreshTree();
        selectResource(created.id);
        const QString msg = r.info.isEmpty() ? PTXT("perm.resources.created")
                                             : PTXT("perm.resources.created_with_info").arg(r.info);
        emit showInfo(PTXT("perm.common.notice"), msg);
        return r;
    }

    r = m_model->updateResource(m_editorForm);
    if (!r.ok) {
        if (r.message == QStringLiteral("key_duplicate")) {
            permission::PermissionRecord existing;
            if (m_model->findByKey(m_editorForm.key, existing))
                emit keyConflictFound(existing.id, existing.enabled);
        } else {
            emit showWarning(PTXT("perm.common.error"), validationMessage(r.message));
        }
        return r;
    }

    ensureExpandedOnPathTo(m_editorForm.editingId);
    refreshTree();
    selectResource(m_editorForm.editingId);
    const QString msg =
        r.info.isEmpty() ? PTXT("perm.resources.saved") : PTXT("perm.resources.saved_with_info").arg(r.info);
    emit showInfo(PTXT("perm.common.notice"), msg);
    return r;
}

void PermissionResourceViewModel::resetEditor()
{
    if (permission::kPermissionResourcesCatalogReadOnly)
        return;
    if (m_creating) {
        m_creating = false;
        m_editorForm = {};
        m_editorBaseline = {};
        emit editorFormChanged();
        emit editorDirtyChanged();
        updateCommandStates();
        return;
    }
    loadEditorForId(m_editorForm.editingId);
}

permission::OperationResult PermissionResourceViewModel::deleteSelected()
{
    if (permission::kPermissionResourcesCatalogReadOnly) {
        permission::OperationResult r;
        return r;
    }
    const qint64 id = selectedResourceId();
    permission::OperationResult r;
    if (id <= 0 || m_creating) {
        r.message = QStringLiteral("resource_not_found");
        emit showWarning(PTXT("perm.common.error"), validationMessage(r.message));
        return r;
    }

    r = m_model->deleteResource(id);
    if (!r.ok) {
        emit showWarning(PTXT("perm.common.error"), validationMessage(r.message));
        return r;
    }

    refreshTree();
    selectResource(0);
    emit showInfo(PTXT("perm.common.notice"), PTXT("perm.resources.deleted"));
    return r;
}

permission::OperationResult PermissionResourceViewModel::toggleSelectedEnabled()
{
    if (permission::kPermissionResourcesCatalogReadOnly) {
        permission::OperationResult r;
        return r;
    }
    const qint64 id = m_editorForm.editingId;
    permission::OperationResult r;
    if (id <= 0) {
        r.message = QStringLiteral("resource_not_found");
        emit showWarning(PTXT("perm.common.error"), validationMessage(r.message));
        return r;
    }

    r = m_model->setEnabled(id, !m_editorForm.enabled);
    if (!r.ok) {
        emit showWarning(PTXT("perm.common.error"), validationMessage(r.message));
        return r;
    }

    ensureExpandedOnPathTo(id);
    refreshTree();
    // 选中与编辑区由 View::syncTree → syncTreeSelectionToViewModel 按树实际选中同步
    if (r.ok && !r.info.isEmpty())
        emit showInfo(PTXT("perm.common.notice"), r.info);
    return r;
}

