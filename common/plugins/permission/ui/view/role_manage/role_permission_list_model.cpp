#include "ui/view/role_manage/role_permission_list_model.h"

#include "ui/view/perm_i18n_utils.h"

#include <algorithm>

namespace permission {

namespace {

bool isNav(const PermissionRecord &p) { return p.resourceType == PermResourceType::Nav; }
bool isSubPage(const PermissionRecord &p) { return p.resourceType == PermResourceType::SubPage; }
bool isAction(const PermissionRecord &p) { return p.resourceType == PermResourceType::Action; }

QVector<PermissionRecord> sortedChildren(
    const QMap<qint64, QVector<PermissionRecord>> &children, qint64 parentId,
    const std::function<bool(const PermissionRecord &)> &pred)
{
    auto list = children.value(parentId);
    std::sort(list.begin(), list.end(),
              [](const PermissionRecord &a, const PermissionRecord &b) {
                  return a.sortOrder < b.sortOrder
                         || (a.sortOrder == b.sortOrder && a.id < b.id);
              });
    QVector<PermissionRecord> out;
    for (const auto &p : list) {
        if (pred(p))
            out.append(p);
    }
    return out;
}

} // namespace

RolePermissionListModel::RolePermissionListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int RolePermissionListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_rows.size();
}

QVariant RolePermissionListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return {};

    const RolePermListRow &row = m_rows.at(index.row());
    if (role == Qt::DisplayRole)
        return row.nav.displayName;
    return {};
}

Qt::ItemFlags RolePermissionListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void RolePermissionListModel::setPermissionTree(const QVector<PermissionRecord> &all,
                                                const QVector<qint64>           &grantedIds)
{
    beginResetModel();
    m_granted.clear();
    for (qint64 id : grantedIds)
        m_granted.insert(id);
    rebuildRows(all);
    endResetModel();
    emit grantsChanged();
}

QVector<qint64> RolePermissionListModel::grantedIds() const
{
    QVector<qint64> ids;
    ids.reserve(m_granted.size());
    for (qint64 id : m_granted)
        ids.append(id);
    return ids;
}

const RolePermListRow &RolePermissionListModel::rowAt(int row) const { return m_rows.at(row); }

bool RolePermissionListModel::isGranted(qint64 resourceId) const
{
    return m_granted.contains(resourceId);
}

bool RolePermissionListModel::isNavGranted(qint64 navId) const { return m_granted.contains(navId); }

bool RolePermissionListModel::isSubPageGranted(qint64 subPageId) const
{
    return m_granted.contains(subPageId);
}

bool RolePermissionListModel::isNavSubtreeFullyGranted(qint64 navId) const
{
    const int row = findNavRow(navId);
    if (row < 0 || !isNavGranted(navId))
        return false;

    const auto &navRow = m_rows.at(row);
    for (const auto &a : navRow.navActions) {
        if (!isGranted(a.id))
            return false;
    }
    for (const auto &block : navRow.subPages) {
        if (!isSubPageGranted(block.subPage.id))
            return false;
        for (const auto &a : block.actions) {
            if (!isGranted(a.id))
                return false;
        }
    }
    return true;
}

bool RolePermissionListModel::isSubPageFullyGranted(qint64 subPageId) const
{
    if (!isSubPageGranted(subPageId))
        return false;
    const RolePermSubPageBlock *block = subPageBlock(subPageId, nullptr);
    if (!block)
        return true;
    for (const auto &a : block->actions) {
        if (!isGranted(a.id))
            return false;
    }
    return true;
}

void RolePermissionListModel::setNavGranted(qint64 navId, bool granted)
{
    if (granted) {
        m_granted.insert(navId);
        const int row = findNavRow(navId);
        if (row >= 0)
            emitRowRefresh(row);
    } else {
        m_granted.remove(navId);
        refreshNavSubtree(navId);
    }
    emit grantsChanged();
}

void RolePermissionListModel::setSubPageGranted(qint64 subPageId, bool granted)
{
    const int navRow = findNavRowForSubPage(subPageId);
    if (navRow < 0)
        return;

    if (granted) {
        m_granted.insert(subPageId);
    } else {
        m_granted.remove(subPageId);
        const RolePermSubPageBlock *block = subPageBlock(subPageId, nullptr);
        if (block) {
            for (const auto &act : block->actions)
                m_granted.remove(act.id);
        }
    }

    emitRowRefresh(navRow);
    emit grantsChanged();
}

void RolePermissionListModel::setActionGranted(qint64 actionId, bool granted)
{
    for (int r = 0; r < m_rows.size(); ++r) {
        const auto &row = m_rows.at(r);
        for (const auto &act : row.navActions) {
            if (act.id != actionId)
                continue;
            if (granted && !isNavGranted(row.nav.id))
                return;
            if (granted)
                m_granted.insert(actionId);
            else
                m_granted.remove(actionId);
            emitRowRefresh(r);
            emit grantsChanged();
            return;
        }

        for (const auto &block : row.subPages) {
            for (const auto &act : block.actions) {
                if (act.id != actionId)
                    continue;
                if (granted && !isSubPageGranted(block.subPage.id))
                    return;
                if (granted)
                    m_granted.insert(actionId);
                else
                    m_granted.remove(actionId);
                emitRowRefresh(r);
                emit grantsChanged();
                return;
            }
        }
    }
}

void RolePermissionListModel::setNavSelectAll(qint64 navId, bool selectAll)
{
    const int row = findNavRow(navId);
    if (row < 0)
        return;

    const auto &navRow = m_rows.at(row);
    if (selectAll)
        m_granted.insert(navRow.nav.id);
    else
        m_granted.remove(navRow.nav.id);

    for (const auto &a : navRow.navActions) {
        if (selectAll)
            m_granted.insert(a.id);
        else
            m_granted.remove(a.id);
    }
    for (const auto &block : navRow.subPages) {
        if (selectAll)
            m_granted.insert(block.subPage.id);
        else
            m_granted.remove(block.subPage.id);
        for (const auto &a : block.actions) {
            if (selectAll)
                m_granted.insert(a.id);
            else
                m_granted.remove(a.id);
        }
    }

    emitRowRefresh(row);
    emit grantsChanged();
}

void RolePermissionListModel::setSubPageSelectAll(qint64 subPageId, bool selectAll)
{
    const int navRow = findNavRowForSubPage(subPageId);
    const RolePermSubPageBlock *block = subPageBlock(subPageId, nullptr);
    if (navRow < 0 || !block)
        return;

    if (selectAll)
        m_granted.insert(block->subPage.id);
    else
        m_granted.remove(block->subPage.id);

    for (const auto &act : block->actions) {
        if (selectAll)
            m_granted.insert(act.id);
        else
            m_granted.remove(act.id);
    }

    emitRowRefresh(navRow);
    emit grantsChanged();
}

void RolePermissionListModel::selectAll()
{
    for (const auto &row : m_rows) {
        m_granted.insert(row.nav.id);
        for (const auto &a : row.navActions)
            m_granted.insert(a.id);
        for (const auto &block : row.subPages) {
            m_granted.insert(block.subPage.id);
            for (const auto &a : block.actions)
                m_granted.insert(a.id);
        }
    }
    if (!m_rows.isEmpty())
        emit dataChanged(index(0), index(m_rows.size() - 1));
    emit grantsChanged();
}

void RolePermissionListModel::clearAll()
{
    m_granted.clear();
    if (!m_rows.isEmpty())
        emit dataChanged(index(0), index(m_rows.size() - 1));
    emit grantsChanged();
}

void RolePermissionListModel::restoreGrantedIds(const QVector<qint64> &grantedIds)
{
    QSet<qint64> next;
    for (qint64 id : grantedIds)
        next.insert(id);
    if (next == m_granted)
        return;

    m_granted = next;
    if (!m_rows.isEmpty())
        emit dataChanged(index(0), index(m_rows.size() - 1));
    emit grantsChanged();
}

void RolePermissionListModel::rebuildRows(const QVector<PermissionRecord> &all)
{
    m_rows.clear();

    QMap<qint64, QVector<PermissionRecord>> children;
    QVector<PermissionRecord>              navs;

    for (const auto &p : all) {
        if (!p.enabled)
            continue;
        if (p.parentId == 0 && isNav(p))
            navs.append(p);
        else if (p.parentId > 0)
            children[p.parentId].append(p);
    }

    std::sort(navs.begin(), navs.end(),
              [](const PermissionRecord &a, const PermissionRecord &b) {
                  return a.sortOrder < b.sortOrder
                         || (a.sortOrder == b.sortOrder && a.id < b.id);
              });

    for (const auto &nav : navs) {
        RolePermListRow navRow;
        navRow.nav = nav;
        navRow.nav.displayName =
            permission_ui::permissionResourceDisplayText(nav.key, nav.displayName);
        navRow.navActions = sortedChildren(children, nav.id,
                                           [](const PermissionRecord &p) { return isAction(p); });
        for (auto &action : navRow.navActions) {
            action.displayName =
                permission_ui::permissionResourceDisplayText(action.key, action.displayName);
        }

        const auto subPages =
            sortedChildren(children, nav.id, [](const PermissionRecord &p) { return isSubPage(p); });
        for (const auto &sub : subPages) {
            RolePermSubPageBlock block;
            block.subPage = sub;
            block.subPage.displayName =
                permission_ui::permissionResourceDisplayText(block.subPage.key,
                                                             block.subPage.displayName);
            block.actions = sortedChildren(
                children, sub.id, [](const PermissionRecord &p) { return isAction(p); });
            for (auto &action : block.actions) {
                action.displayName =
                    permission_ui::permissionResourceDisplayText(action.key, action.displayName);
            }
            navRow.subPages.append(block);
        }

        m_rows.append(navRow);
    }
}

int RolePermissionListModel::findNavRow(qint64 navId) const
{
    for (int i = 0; i < m_rows.size(); ++i) {
        if (m_rows.at(i).nav.id == navId)
            return i;
    }
    return -1;
}

int RolePermissionListModel::findNavRowForSubPage(qint64 subPageId) const
{
    for (int i = 0; i < m_rows.size(); ++i) {
        for (const auto &block : m_rows.at(i).subPages) {
            if (block.subPage.id == subPageId)
                return i;
        }
    }
    return -1;
}

const RolePermSubPageBlock *RolePermissionListModel::subPageBlock(qint64 subPageId,
                                                                  int *navRowOut) const
{
    const int navRow = findNavRowForSubPage(subPageId);
    if (navRowOut)
        *navRowOut = navRow;
    if (navRow < 0)
        return nullptr;
    for (const auto &block : m_rows.at(navRow).subPages) {
        if (block.subPage.id == subPageId)
            return &block;
    }
    return nullptr;
}

void RolePermissionListModel::refreshNavSubtree(qint64 navId)
{
    const int row = findNavRow(navId);
    if (row < 0)
        return;

    const auto &navRow = m_rows.at(row);
    for (const auto &a : navRow.navActions)
        m_granted.remove(a.id);
    for (const auto &block : navRow.subPages) {
        m_granted.remove(block.subPage.id);
        for (const auto &a : block.actions)
            m_granted.remove(a.id);
    }
    emitRowRefresh(row);
}

void RolePermissionListModel::emitRowRefresh(int row)
{
    if (row < 0 || row >= m_rows.size())
        return;
    emit dataChanged(index(row), index(row));
}

} // namespace permission
