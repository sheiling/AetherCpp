#include "model/permission_grant_service.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

namespace permission {
namespace {

bool isNav(const PermissionRecord &p) { return p.resourceType == PermResourceType::Nav; }
bool isSubPage(const PermissionRecord &p) { return p.resourceType == PermResourceType::SubPage; }
bool isAction(const PermissionRecord &p) { return p.resourceType == PermResourceType::Action; }

qint64 rootNavId(qint64 id, const QHash<qint64, PermissionRecord> &byId)
{
    const PermissionRecord rec = byId.value(id);
    if (rec.id <= 0)
        return 0;
    if (isNav(rec))
        return rec.id;
    if (rec.parentId <= 0)
        return 0;
    return rootNavId(rec.parentId, byId);
}

QVector<qint64> sortedGrantIds(const QSet<qint64> &ids,
                               const QHash<qint64, PermissionRecord> &byId)
{
    QVector<qint64> navs;
    QVector<qint64> subs;
    QVector<qint64> acts;
    for (qint64 id : ids) {
        const PermissionRecord rec = byId.value(id);
        if (rec.id <= 0)
            continue;
        if (isNav(rec))
            navs.append(id);
        else if (isSubPage(rec))
            subs.append(id);
        else if (isAction(rec))
            acts.append(id);
    }
    auto bySort = [&](qint64 a, qint64 b) {
        const auto &ra = byId.value(a);
        const auto &rb = byId.value(b);
        if (ra.sortOrder != rb.sortOrder)
            return ra.sortOrder < rb.sortOrder;
        return a < b;
    };
    std::sort(navs.begin(), navs.end(), bySort);
    std::sort(subs.begin(), subs.end(), bySort);
    std::sort(acts.begin(), acts.end(), bySort);
    QVector<qint64> out;
    out.reserve(navs.size() + subs.size() + acts.size());
    out += navs;
    out += subs;
    out += acts;
    return out;
}

} // namespace

GrantNormalizeResult PermissionGrantService::normalize(const QVector<PermissionRecord> &all,
                                                       const QVector<qint64> &selectedIds)
{
    GrantNormalizeResult result;

    QHash<qint64, PermissionRecord> byId;
    byId.reserve(all.size());
    for (const auto &p : all)
        byId.insert(p.id, p);

    QSet<qint64> working;
    for (qint64 id : selectedIds) {
        if (id > 0)
            working.insert(id);
    }

  auto prune = [&](qint64 id) {
        if (working.remove(id))
            result.removedIds.append(id);
    };

    // 1) 未知资源 / 已禁用
    for (qint64 id : QVector<qint64>(working.begin(), working.end())) {
        const PermissionRecord rec = byId.value(id);
        if (rec.id <= 0 || !rec.enabled)
            prune(id);
    }

    // 2) 非 nav：其根 nav 必须已授予
    for (qint64 id : QVector<qint64>(working.begin(), working.end())) {
        const PermissionRecord rec = byId.value(id);
        if (rec.id <= 0 || isNav(rec))
            continue;
        const qint64 navId = rootNavId(id, byId);
        if (navId <= 0 || !working.contains(navId))
            prune(id);
    }

    // 3) action：直接 parent 必须已授予
    for (qint64 id : QVector<qint64>(working.begin(), working.end())) {
        const PermissionRecord rec = byId.value(id);
        if (!isAction(rec))
            continue;
        if (rec.parentId <= 0 || !working.contains(rec.parentId))
            prune(id);
    }

    // 4) sub_page：parent 必须是 nav（已在步骤 2 保证 nav 授予）
    for (qint64 id : QVector<qint64>(working.begin(), working.end())) {
        const PermissionRecord rec = byId.value(id);
        if (!isSubPage(rec))
            continue;
        const PermissionRecord parent = byId.value(rec.parentId);
        if (!isNav(parent))
            prune(id);
    }

    result.normalized = sortedGrantIds(working, byId);
    std::sort(result.removedIds.begin(), result.removedIds.end());
    return result;
}

GrantPermissionDiff PermissionGrantService::diff(const QVector<qint64> &before,
                                                 const QVector<qint64> &after)
{
    QSet<qint64> a;
    QSet<qint64> b;
    for (qint64 id : before)
        a.insert(id);
    for (qint64 id : after)
        b.insert(id);

    GrantPermissionDiff d;
    for (qint64 id : b) {
        if (!a.contains(id))
            d.added.append(id);
    }
    for (qint64 id : a) {
        if (!b.contains(id))
            d.removed.append(id);
    }
    std::sort(d.added.begin(), d.added.end());
    std::sort(d.removed.begin(), d.removed.end());
    return d;
}

QString PermissionGrantService::auditDetailJson(qint64 roleId, const QString &roleName,
                                                const GrantPermissionDiff &diff,
                                                int normalizedCount)
{
    auto toJsonArray = [](const QVector<qint64> &ids) {
        QJsonArray arr;
        for (qint64 id : ids)
            arr.append(id);
        return arr;
    };

    QJsonObject obj;
    obj.insert(QStringLiteral("roleId"), roleId);
    obj.insert(QStringLiteral("roleName"), roleName);
    obj.insert(QStringLiteral("normalizedCount"), normalizedCount);
    obj.insert(QStringLiteral("added"), toJsonArray(diff.added));
    obj.insert(QStringLiteral("removed"), toJsonArray(diff.removed));

    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

} // namespace permission
