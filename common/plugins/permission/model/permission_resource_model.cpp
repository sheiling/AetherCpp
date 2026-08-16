#include "model/permission_resource_model.h"

#include "perm_text.h"
#include "ui/view/perm_i18n_utils.h"

#include "model/role_model.h"
#include "service/perm_entity_guard.h"
#include "service/perm_system_constants.h"
#include "repository/datastore.h"
#include "repository/interface/idatastore.h"

#include <algorithm>

namespace permission {
namespace {

bool isNav(const PermissionRecord &p) { return p.resourceType == PermResourceType::Nav; }
bool isSubPage(const PermissionRecord &p) { return p.resourceType == PermResourceType::SubPage; }
bool isAction(const PermissionRecord &p) { return p.resourceType == PermResourceType::Action; }

bool recordMatchesKeyword(const PermissionRecord &p, const QString &kw)
{
    const QString localized =
        permission_ui::permissionResourceDisplayText(p.key, p.displayName);
    return p.key.contains(kw, Qt::CaseInsensitive)
           || p.displayName.contains(kw, Qt::CaseInsensitive)
           || p.pageKey.contains(kw, Qt::CaseInsensitive)
           || localized.contains(kw, Qt::CaseInsensitive);
}

QVector<PermissionRecord> sortedChildren(
    const QMap<qint64, QVector<PermissionRecord>> &children, qint64 parentId)
{
    auto list = children.value(parentId);
    std::sort(list.begin(), list.end(),
              [](const PermissionRecord &a, const PermissionRecord &b) {
                  return a.sortOrder < b.sortOrder
                         || (a.sortOrder == b.sortOrder && a.id < b.id);
              });
    return list;
}

} // namespace

PermissionResourceModel::PermissionResourceModel(IDataStore *store)
    : m_store(store ? store : DataStore::instance())
{
}

QVector<PermissionRecord> PermissionResourceModel::loadAll() const
{
    if (!m_store)
        return {};
    PermissionQueryFilter filter;
    filter.includeDisabled = true;
    return m_store->permissions().list(filter);
}

bool PermissionResourceModel::findById(qint64 id, PermissionRecord &out) const
{
    if (!m_store || id <= 0)
        return false;
    return m_store->permissions().findById(id, out);
}

bool PermissionResourceModel::findByKey(const QString &key, PermissionRecord &out) const
{
    if (!m_store || key.trimmed().isEmpty())
        return false;
    return m_store->permissions().findByKey(key.trimmed(), out);
}

int PermissionResourceModel::childCount(qint64 parentId) const
{
    int n = 0;
    for (const auto &p : loadAll()) {
        if (p.parentId == parentId)
            ++n;
    }
    return n;
}

int PermissionResourceModel::grantReferenceCount(qint64 permissionId) const
{
    if (!m_store || permissionId <= 0)
        return 0;

    int count = 0;
    for (const auto &role : m_store->roles().list({})) {
        if (isSuperAdminRole(role))
            continue;
        const QVector<qint64> ids = m_store->roles().permissionIdsOfRole(role.id);
        if (ids.contains(permissionId))
            ++count;
    }
    return count;
}

int PermissionResourceModel::nextSortOrder(qint64 parentId) const
{
    int maxOrder = 0;
    for (const auto &p : loadAll()) {
        if (p.parentId == parentId)
            maxOrder = qMax(maxOrder, p.sortOrder);
    }
    return maxOrder + 10;
}

OperationResult PermissionResourceModel::validateParentChain(
    const PermissionResourceEditorForm &form) const
{
    OperationResult r;

    if (form.resourceType == PermResourceType::Nav) {
        if (form.parentId != 0) {
            r.message = QStringLiteral("nav_parent_must_be_root");
            return r;
        }
        r.ok = true;
        return r;
    }

    if (form.parentId <= 0) {
        r.message = QStringLiteral("parent_required");
        return r;
    }

    PermissionRecord parent;
    if (!findById(form.parentId, parent)) {
        r.message = QStringLiteral("parent_not_found");
        return r;
    }

    if (form.resourceType == PermResourceType::SubPage && !isNav(parent)) {
        r.message = QStringLiteral("sub_page_parent_must_be_nav");
        return r;
    }

    if (form.resourceType == PermResourceType::Action && !isNav(parent)
        && !isSubPage(parent)) {
        r.message = QStringLiteral("action_parent_invalid");
        return r;
    }

    r.ok = true;
    return r;
}

OperationResult PermissionResourceModel::validateEditorForm(
    const PermissionResourceEditorForm &form) const
{
    OperationResult r;

    const QString key = form.key.trimmed();
    if (key.isEmpty()) {
        r.message = QStringLiteral("key_required");
        return r;
    }

    if (form.displayName.trimmed().isEmpty()) {
        r.message = QStringLiteral("display_name_required");
        return r;
    }

    if (form.resourceType != PermResourceType::Nav
        && form.resourceType != PermResourceType::SubPage
        && form.resourceType != PermResourceType::Action) {
        r.message = QStringLiteral("invalid_resource_type");
        return r;
    }

    PermissionRecord existing;
    if (findByKey(key, existing) && existing.id != form.editingId) {
        r.message = QStringLiteral("key_duplicate");
        return r;
    }

    if (form.editingId > 0) {
        PermissionRecord self;
        if (!findById(form.editingId, self)) {
            r.message = QStringLiteral("resource_not_found");
            return r;
        }
        if (self.key != key) {
            r.message = QStringLiteral("key_immutable");
            return r;
        }
        if (self.resourceType != form.resourceType) {
            r.message = QStringLiteral("resource_type_immutable");
            return r;
        }
    }

    const OperationResult parentCheck = validateParentChain(form);
    if (!parentCheck.ok)
        return parentCheck;

    r.ok = true;
    return r;
}

PermissionResourceEditorForm PermissionResourceModel::toEditorForm(
    const PermissionRecord &record, const QString &parentDisplayName) const
{
    PermissionResourceEditorForm form;
    form.editingId          = record.id;
    form.resourceType       = record.resourceType;
    form.parentId           = record.parentId;
    form.key                = record.key;
    form.pageKey            = record.pageKey;
    form.displayName        = record.displayName;
    form.description        = record.description;
    form.sortOrder          = record.sortOrder;
    form.enabled            = record.enabled;
    form.source             = record.source;
    form.parentDisplayName  = parentDisplayName;
    return form;
}

QVector<qint64> PermissionResourceModel::collectAncestorIds(qint64 id) const
{
    QVector<qint64> out;
    PermissionRecord cur;
    if (!findById(id, cur))
        return out;

    qint64 pid = cur.parentId;
    while (pid > 0) {
        out.append(pid);
        PermissionRecord parent;
        if (!findById(pid, parent))
            break;
        pid = parent.parentId;
    }
    return out;
}

QVector<qint64> PermissionResourceModel::collectDescendantIds(qint64 id) const
{
    QVector<qint64> out;
    if (id <= 0)
        return out;

    QMap<qint64, QVector<qint64>> childrenByParent;
    for (const auto &p : loadAll()) {
        if (p.parentId > 0)
            childrenByParent[p.parentId].append(p.id);
    }

    std::function<void(qint64)> dfs = [&](qint64 parentId) {
        for (qint64 childId : childrenByParent.value(parentId)) {
            out.append(childId);
            dfs(childId);
        }
    };
    dfs(id);
    return out;
}

void PermissionResourceModel::enableAncestors(qint64 id, QStringList *touchedNames)
{
    if (!m_store || !touchedNames)
        return;

    for (qint64 ancestorId : collectAncestorIds(id)) {
        PermissionRecord rec;
        if (!findById(ancestorId, rec) || rec.enabled)
            continue;
        rec.enabled = true;
        QString err;
        if (m_store->permissions().update(rec, &err))
            touchedNames->append(rec.displayName);
    }
}

void PermissionResourceModel::disableDescendants(qint64 id, QStringList *touchedNames)
{
    if (!m_store || !touchedNames)
        return;

    for (qint64 childId : collectDescendantIds(id)) {
        PermissionRecord rec;
        if (!findById(childId, rec) || !rec.enabled)
            continue;
        rec.enabled = false;
        QString err;
        if (m_store->permissions().update(rec, &err))
            touchedNames->append(rec.displayName);
    }
}

OperationResult PermissionResourceModel::applyEnabledCascade(qint64 id, bool enabled,
                                                             QString *cascadeInfo)
{
    OperationResult r;
    if (!m_store || id <= 0) {
        r.message = QStringLiteral("resource_not_found");
        return r;
    }

    PermissionRecord rec;
    if (!findById(id, rec)) {
        r.message = QStringLiteral("resource_not_found");
        return r;
    }

    QStringList touched;
    if (enabled) {
        enableAncestors(id, &touched);
        if (!rec.enabled) {
            rec.enabled = true;
            QString err;
            if (!m_store->permissions().update(rec, &err)) {
                r.message = err.isEmpty() ? QStringLiteral("update_failed") : err;
                return r;
            }
        }
        if (!touched.isEmpty() && cascadeInfo) {
            *cascadeInfo =
                PTXT("perm.resources.cascade_enabled_parents").arg(touched.join(QStringLiteral("、")));
        }
    } else {
        if (rec.enabled) {
            rec.enabled = false;
            QString err;
            if (!m_store->permissions().update(rec, &err)) {
                r.message = err.isEmpty() ? QStringLiteral("update_failed") : err;
                return r;
            }
        }
        disableDescendants(id, &touched);
        if (!touched.isEmpty() && cascadeInfo) {
            *cascadeInfo =
                PTXT("perm.resources.cascade_disabled_children").arg(touched.join(QStringLiteral("、")));
        }
    }

    r.ok = true;
    return r;
}

OperationResult PermissionResourceModel::createResource(
    const PermissionResourceEditorForm &form, PermissionRecord *createdOut)
{
    OperationResult r = validateEditorForm(form);
    if (!r.ok)
        return r;

    if (!m_store) {
        r.message = QStringLiteral("store_unavailable");
        return r;
    }

    PermissionRecord rec;
    rec.parentId     = form.resourceType == PermResourceType::Nav ? 0 : form.parentId;
    rec.key          = form.key.trimmed();
    rec.resourceType = form.resourceType;
    rec.pageKey      = form.resourceType == PermResourceType::Nav
                           ? QString::fromUtf8(kPermPageKeyRoot)
                           : form.pageKey.trimmed();
    rec.displayName  = form.displayName.trimmed();
    rec.description  = form.description.trimmed();
    rec.source       = form.source.isEmpty() ? QStringLiteral("manual") : form.source;
    rec.sortOrder    = form.sortOrder > 0 ? form.sortOrder : nextSortOrder(rec.parentId);
    rec.enabled      = form.enabled;

    QString err;
    if (!m_store->permissions().insert(rec, &err)) {
        if (err == QStringLiteral("key_duplicate"))
            r.message = QStringLiteral("key_duplicate");
        else
            r.message = err.isEmpty() ? QStringLiteral("insert_failed") : err;
        return r;
    }

    if (form.enabled) {
        QString cascadeInfo;
        const OperationResult cascade = applyEnabledCascade(rec.id, true, &cascadeInfo);
        if (!cascade.ok)
            return cascade;
        r.info = cascadeInfo;
        findById(rec.id, rec);
    } else {
        QString cascadeInfo;
        applyEnabledCascade(rec.id, false, &cascadeInfo);
        r.info = cascadeInfo;
        findById(rec.id, rec);
    }

    if (createdOut)
        *createdOut = rec;

    r.ok = true;
    syncSuperAdminGrantsFromCatalog(m_store);
    return r;
}

OperationResult PermissionResourceModel::updateResource(
    const PermissionResourceEditorForm &form)
{
    OperationResult r = validateEditorForm(form);
    if (!r.ok)
        return r;

    if (!m_store || form.editingId <= 0) {
        r.message = QStringLiteral("resource_not_found");
        return r;
    }

    PermissionRecord rec;
    if (!findById(form.editingId, rec)) {
        r.message = QStringLiteral("resource_not_found");
        return r;
    }

    rec.displayName = form.displayName.trimmed();
    rec.description = form.description.trimmed();
    rec.sortOrder   = form.sortOrder;
    if (rec.source == QStringLiteral("manual") || rec.source.isEmpty())
        rec.pageKey = form.pageKey.trimmed();

    QString err;
    if (!m_store->permissions().update(rec, &err)) {
        r.message = err.isEmpty() ? QStringLiteral("update_failed") : err;
        return r;
    }

    QString cascadeInfo;
    const OperationResult cascade = applyEnabledCascade(form.editingId, form.enabled, &cascadeInfo);
    if (!cascade.ok)
        return cascade;
    r.info = cascadeInfo;

    r.ok = true;
    syncSuperAdminGrantsFromCatalog(m_store);
    return r;
}

OperationResult PermissionResourceModel::deleteResource(qint64 id)
{
    OperationResult r;
    if (!m_store || id <= 0) {
        r.message = QStringLiteral("resource_not_found");
        return r;
    }

    if (childCount(id) > 0) {
        r.message = QStringLiteral("has_children");
        return r;
    }

    if (grantReferenceCount(id) > 0) {
        r.message = QStringLiteral("has_grant_refs");
        return r;
    }

    QString err;
    if (!m_store->permissions().remove(id, &err)) {
        r.message = err.isEmpty() ? QStringLiteral("delete_failed") : err;
        return r;
    }

    r.ok = true;
    syncSuperAdminGrantsFromCatalog(m_store);
    return r;
}

OperationResult PermissionResourceModel::setEnabled(qint64 id, bool enabled)
{
    PermissionRecord rec;
    if (!findById(id, rec)) {
        OperationResult r;
        r.message = QStringLiteral("resource_not_found");
        return r;
    }

    if (rec.enabled == enabled) {
        OperationResult r;
        QString cascadeInfo;
        applyEnabledCascade(id, enabled, &cascadeInfo);
        r.ok   = true;
        r.info = cascadeInfo;
        if (r.ok)
            syncSuperAdminGrantsFromCatalog(m_store);
        return r;
    }

    OperationResult r = applyEnabledCascade(id, enabled);
    if (r.ok)
        syncSuperAdminGrantsFromCatalog(m_store);
    return r;
}

QVector<PermissionResourceTreeBranch> PermissionResourceModel::buildVisibleForest(
    const QVector<PermissionRecord> &all, const QString &keyword,
    const QString &resourceTypeFilter, int enabledFilterMode) const
{
    const QString kw = keyword.trimmed();

    const bool filterAll = resourceTypeFilter.isEmpty();
    const bool filterNav = resourceTypeFilter == PermResourceType::Nav;
    const bool filterSub = resourceTypeFilter == PermResourceType::SubPage;
    const bool filterAct = resourceTypeFilter == PermResourceType::Action;

    QMap<qint64, QVector<PermissionRecord>> children;
    QVector<PermissionRecord>              navs;

    for (const auto &p : all) {
        if (p.parentId == 0 && isNav(p))
            navs.append(p);
        else if (p.parentId > 0)
            children[p.parentId].append(p);
    }
    std::sort(navs.begin(), navs.end(),
              [](const PermissionRecord &a, const PermissionRecord &b) {
                  return a.sortOrder < b.sortOrder;
              });

    auto makeNode = [&](const PermissionRecord &p) {
        PermissionResourceTreeNode node;
        node.id             = p.id;
        node.parentId       = p.parentId;
        node.resourceType   = p.resourceType;
        node.displayName    = p.displayName;
        node.key            = p.key;
        node.enabled        = p.enabled;
        node.sortOrder      = p.sortOrder;
        node.childCount     = childCount(p.id);
        node.grantRefCount  = grantReferenceCount(p.id);
        return node;
    };

    auto leafBranch = [&](const PermissionRecord &p) {
        PermissionResourceTreeBranch branch;
        branch.node = makeNode(p);
        return branch;
    };

    auto enabledVisible = [&](const PermissionRecord &p) {
        if (enabledFilterMode < 0)
            return true;
        return enabledFilterMode == 1 ? p.enabled : !p.enabled;
    };

    auto keywordMatches = [&](const PermissionRecord &p) {
        return kw.isEmpty() || recordMatchesKeyword(p, kw);
    };

    auto subPageVisibleUnderNav = [&](const PermissionRecord &sub,
                                      const PermissionRecord &nav) -> bool {
        if (!isSubPage(sub))
            return false;
        if (!enabledVisible(sub))
            return false;
        if (keywordMatches(sub))
            return true;
        if (keywordMatches(nav))
            return true;
        return false;
    };

    auto actionVisibleUnder = [&](const PermissionRecord &act,
                                  const PermissionRecord &directParent,
                                  const PermissionRecord &navAncestor) -> bool {
        if (!isAction(act) || !enabledVisible(act))
            return false;
        if (keywordMatches(act))
            return true;
        if (keywordMatches(directParent))
            return true;
        if (keywordMatches(navAncestor))
            return true;
        return false;
    };

    // —— 全部类型：完整三级树 ——
    if (filterAll) {
        std::function<bool(qint64)> subtreeMatch;
        subtreeMatch = [&](qint64 parentId) -> bool {
            for (const auto &c : sortedChildren(children, parentId)) {
                if (keywordMatches(c) && enabledVisible(c))
                    return true;
                if (subtreeMatch(c.id))
                    return true;
            }
            return false;
        };

        std::function<PermissionResourceTreeBranch(const PermissionRecord &)> buildFullBranch;
        buildFullBranch = [&](const PermissionRecord &record) -> PermissionResourceTreeBranch {
            PermissionResourceTreeBranch branch;
            branch.node = makeNode(record);

            const auto kids = sortedChildren(children, record.id);
            QVector<PermissionRecord> navActions;
            QVector<PermissionRecord> subPages;
            for (const auto &c : kids) {
                if (isAction(c))
                    navActions.append(c);
                else if (isSubPage(c))
                    subPages.append(c);
            }

            for (const auto &act : navActions) {
                if (!enabledVisible(act))
                    continue;
                if (!kw.isEmpty() && !keywordMatches(act) && !keywordMatches(record)
                    && !subtreeMatch(record.id))
                    continue;
                branch.children.append(buildFullBranch(act));
            }
            for (const auto &sub : subPages) {
                if (!enabledVisible(sub) && !subtreeMatch(sub.id))
                    continue;
                if (!kw.isEmpty() && !keywordMatches(sub) && !keywordMatches(record)
                    && !subtreeMatch(sub.id))
                    continue;
                branch.children.append(buildFullBranch(sub));
            }
            return branch;
        };

        QVector<PermissionResourceTreeBranch> forest;
        for (const auto &nav : navs) {
            bool navKwOk = keywordMatches(nav);
            if (!navKwOk) {
                for (const auto &c : sortedChildren(children, nav.id)) {
                    if (keywordMatches(c) || subtreeMatch(c.id)) {
                        navKwOk = true;
                        break;
                    }
                }
            }
            if (!navKwOk)
                continue;
            if (!enabledVisible(nav) && !subtreeMatch(nav.id))
                continue;
            forest.append(buildFullBranch(nav));
        }
        return forest;
    }

    QVector<PermissionResourceTreeBranch> forest;

    // —— 仅导航页：扁平列表，无子节点 ——
    if (filterNav) {
        for (const auto &nav : navs) {
            if (!enabledVisible(nav))
                continue;
            if (!keywordMatches(nav))
                continue;
            forest.append(leafBranch(nav));
        }
        return forest;
    }

    // —— 仅子页面：导航（父） + 子页面，不含操作 ——
    if (filterSub) {
        for (const auto &nav : navs) {
            QVector<PermissionResourceTreeBranch> subBranches;
            for (const auto &sub : sortedChildren(children, nav.id)) {
                if (!subPageVisibleUnderNav(sub, nav))
                    continue;
                subBranches.append(leafBranch(sub));
            }
            if (subBranches.isEmpty())
                continue;

            PermissionResourceTreeBranch navBranch;
            navBranch.node     = makeNode(nav);
            navBranch.children = subBranches;
            forest.append(navBranch);
        }
        return forest;
    }

    // —— 仅操作：导航 / 子页面（容器） + 操作叶子，不含无关子页面 ——
    if (filterAct) {
        for (const auto &nav : navs) {
            QVector<PermissionResourceTreeBranch> navChildren;

            for (const auto &act : sortedChildren(children, nav.id)) {
                if (!isAction(act))
                    continue;
                if (!actionVisibleUnder(act, nav, nav))
                    continue;
                navChildren.append(leafBranch(act));
            }

            for (const auto &sub : sortedChildren(children, nav.id)) {
                if (!isSubPage(sub))
                    continue;

                QVector<PermissionResourceTreeBranch> actBranches;
                for (const auto &act : sortedChildren(children, sub.id)) {
                    if (!isAction(act))
                        continue;
                    if (!actionVisibleUnder(act, sub, nav))
                        continue;
                    actBranches.append(leafBranch(act));
                }
                if (actBranches.isEmpty())
                    continue;

                PermissionResourceTreeBranch subBranch;
                subBranch.node     = makeNode(sub);
                subBranch.children = actBranches;
                navChildren.append(subBranch);
            }

            if (navChildren.isEmpty())
                continue;

            PermissionResourceTreeBranch navBranch;
            navBranch.node     = makeNode(nav);
            navBranch.children = navChildren;
            forest.append(navBranch);
        }
        return forest;
    }

    return forest;
}

} // namespace permission
