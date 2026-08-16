#include "repository/orm/orm_data_store.h"

#include "model/role_model.h"
#include "repository/datastore.h"
#include "repository/orm/data_seeder.h"
#include "service/perm_entity_guard.h"
#include "repository/local/local_db_manager.h"
#include "repository/orm/schema_initializer.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

#include <algorithm>

namespace permission {
namespace {

QString sqlError(const QSqlError &e)
{
    return e.isValid() ? e.text() : QString();
}

// Inline replacements for orm_mapper utilities (no QxOrm entity dependency)
QString nowIso()
{
    return QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
}

QDateTime parseIso(const QString &s)
{
    if (s.isEmpty())
        return {};
    return QDateTime::fromString(s, Qt::ISODateWithMs);
}

// ── User ─────────────────────────────────────────────────────────────────────

class OrmUserRepository final : public IUserRepository
{
public:
    QVector<UserRecord> list(const UserQueryFilter &filter) override
    {
        QVector<UserRecord> out;
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        if (!q.exec(QStringLiteral("SELECT * FROM perm_user")))
            return out;

        while (q.next()) {
            UserRecord u;
            u.id           = q.value(QStringLiteral("id")).toLongLong();
            u.username     = q.value(QStringLiteral("username")).toString();
            u.displayName  = q.value(QStringLiteral("display_name")).toString();
            u.department   = q.value(QStringLiteral("department")).toString();
            u.remark       = q.value(QStringLiteral("remark")).toString();
            u.accountStatus = q.value(QStringLiteral("status")).toInt();
            const QString lla = q.value(QStringLiteral("last_login_at")).toString();
            if (!lla.isEmpty())
                u.lastLoginAt = QDateTime::fromString(lla, Qt::ISODateWithMs);
            u.isHidden     = q.value(QStringLiteral("is_hidden")).toInt() != 0;

            fillRoles(u);
            if (!filter.includeHidden && u.isHidden)
                continue;
            if (!matchFilter(u, filter))
                continue;
            out.append(u);
        }
        return out;
    }

    qint64 count(const UserQueryFilter &filter) override
    {
        return list(filter).size();
    }

    bool findById(qint64 id, UserRecord &out) override
    {
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral("SELECT * FROM perm_user WHERE id = ?"));
        q.addBindValue(id);
        if (!q.exec() || !q.next())
            return false;

        out.id           = q.value(QStringLiteral("id")).toLongLong();
        out.username     = q.value(QStringLiteral("username")).toString();
        out.displayName  = q.value(QStringLiteral("display_name")).toString();
        out.department   = q.value(QStringLiteral("department")).toString();
        out.remark       = q.value(QStringLiteral("remark")).toString();
        out.accountStatus = q.value(QStringLiteral("status")).toInt();
        const QString lla = q.value(QStringLiteral("last_login_at")).toString();
        if (!lla.isEmpty())
            out.lastLoginAt = QDateTime::fromString(lla, Qt::ISODateWithMs);
        out.isHidden     = q.value(QStringLiteral("is_hidden")).toInt() != 0;

        fillRoles(out);
        return true;
    }

    bool findByUsername(const QString &username, UserRecord &out) override
    {
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral("SELECT * FROM perm_user WHERE username = ?"));
        q.addBindValue(username);
        if (!q.exec() || !q.next())
            return false;

        out.id           = q.value(QStringLiteral("id")).toLongLong();
        out.username     = q.value(QStringLiteral("username")).toString();
        out.displayName  = q.value(QStringLiteral("display_name")).toString();
        out.department   = q.value(QStringLiteral("department")).toString();
        out.remark       = q.value(QStringLiteral("remark")).toString();
        out.accountStatus = q.value(QStringLiteral("status")).toInt();
        const QString lla = q.value(QStringLiteral("last_login_at")).toString();
        if (!lla.isEmpty())
            out.lastLoginAt = QDateTime::fromString(lla, Qt::ISODateWithMs);
        out.isHidden     = q.value(QStringLiteral("is_hidden")).toInt() != 0;

        fillRoles(out);
        return true;
    }

    bool insert(UserRecord &record, QString *errorString) override
    {
        const QString now = nowIso();
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "INSERT INTO perm_user (username, password_hash, display_name, department, "
            "remark, is_hidden, status, created_at, updated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"));
        q.addBindValue(record.username);
        q.addBindValue(QStringLiteral("pending")); // initial password hash
        q.addBindValue(record.displayName);
        q.addBindValue(record.department);
        q.addBindValue(record.remark);
        q.addBindValue(record.isHidden ? 1 : 0);
        q.addBindValue(record.accountStatus);
        q.addBindValue(now);
        q.addBindValue(now);
        if (!q.exec()) {
            if (errorString)
                *errorString = q.lastError().text();
            return false;
        }
        record.id = q.lastInsertId().toLongLong();
        return true;
    }

    bool update(const UserRecord &record, QString *errorString) override
    {
        // Check existence
        QSqlDatabase db = orm::DbManager::instance().database();
        {
            QSqlQuery q(db);
            q.prepare(QStringLiteral("SELECT 1 FROM perm_user WHERE id = ?"));
            q.addBindValue(record.id);
            if (!q.exec() || !q.next()) {
                if (errorString)
                    *errorString = QStringLiteral("User not found.");
                return false;
            }
        }

        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "UPDATE perm_user SET display_name = ?, department = ?, remark = ?, "
            "status = ?, last_login_at = ?, updated_at = ? WHERE id = ?"));
        q.addBindValue(record.displayName);
        q.addBindValue(record.department);
        q.addBindValue(record.remark);
        q.addBindValue(record.accountStatus);
        q.addBindValue(record.lastLoginAt.isValid()
                           ? record.lastLoginAt.toString(Qt::ISODateWithMs)
                           : QVariant(QVariant::String));
        q.addBindValue(nowIso());
        q.addBindValue(record.id);
        if (!q.exec()) {
            if (errorString)
                *errorString = q.lastError().text();
            return false;
        }
        return true;
    }

    bool setAccountStatus(qint64 id, int accountStatus, QString *errorString) override
    {
        UserRecord u;
        if (!findById(id, u))
            return false;
        if (!canChangeUserAccountStatus(u)) {
            if (errorString)
                *errorString = QStringLiteral("protected_user_status");
            return false;
        }
        u.accountStatus = accountStatus;
        return update(u, errorString);
    }

    bool resetPassword(qint64 id, const QString &passwordHash, QString *errorString) override
    {
        QSqlDatabase db = orm::DbManager::instance().database();
        {
            QSqlQuery q(db);
            q.prepare(QStringLiteral("SELECT 1 FROM perm_user WHERE id = ?"));
            q.addBindValue(id);
            if (!q.exec() || !q.next()) {
                if (errorString)
                    *errorString = QStringLiteral("User not found.");
                return false;
            }
        }

        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "UPDATE perm_user SET password_hash = ?, updated_at = ? WHERE id = ?"));
        q.addBindValue(passwordHash);
        q.addBindValue(nowIso());
        q.addBindValue(id);
        if (!q.exec()) {
            if (errorString)
                *errorString = q.lastError().text();
            return false;
        }
        return true;
    }

    bool assignRoles(qint64 userId,
                     const QVector<qint64> &roleIds,
                     qint64 primaryRoleId,
                     QString *errorString) override
    {
        QSqlDatabase db = orm::DbManager::instance().database();
        if (!db.transaction()) {
            if (errorString)
                *errorString = db.lastError().text();
            return false;
        }

        QSqlQuery del(db);
        del.prepare(QStringLiteral("DELETE FROM perm_user_role WHERE user_id = :uid"));
        del.bindValue(QStringLiteral(":uid"), userId);
        if (!del.exec()) {
            db.rollback();
            if (errorString)
                *errorString = del.lastError().text();
            return false;
        }

        const QString now = nowIso();
        for (qint64 roleId : roleIds) {
            QSqlQuery ins(db);
            ins.prepare(QStringLiteral(
                "INSERT INTO perm_user_role (user_id, role_id, is_primary, assigned_at) "
                "VALUES (:uid, :rid, :pri, :at)"));
            ins.bindValue(QStringLiteral(":uid"), userId);
            ins.bindValue(QStringLiteral(":rid"), roleId);
            ins.bindValue(QStringLiteral(":pri"), roleId == primaryRoleId ? 1 : 0);
            ins.bindValue(QStringLiteral(":at"), now);
            if (!ins.exec()) {
                db.rollback();
                if (errorString)
                    *errorString = ins.lastError().text();
                return false;
            }
        }

        if (!db.commit()) {
            if (errorString)
                *errorString = db.lastError().text();
            return false;
        }
        return true;
    }

    QVector<qint64> roleIdsOfUser(qint64 userId) override
    {
        QVector<qint64> ids;
        QSqlQuery q(orm::DbManager::instance().database());
        q.prepare(QStringLiteral("SELECT role_id FROM perm_user_role WHERE user_id = :uid"));
        q.bindValue(QStringLiteral(":uid"), userId);
        if (!q.exec())
            return ids;
        while (q.next())
            ids.append(q.value(0).toLongLong());
        return ids;
    }

private:
    static bool matchFilter(const UserRecord &u, const UserQueryFilter &f)
    {
        if (f.accountStatus >= 0 && u.accountStatus != f.accountStatus)
            return false;
        if (!f.keyword.isEmpty()) {
            const QString kw = f.keyword.trimmed();
            if (!u.username.contains(kw, Qt::CaseInsensitive)
                && !u.displayName.contains(kw, Qt::CaseInsensitive))
                return false;
        }
        if (!f.roleCode.isEmpty()) {
            bool hit = false;
            for (const QString &c : u.roleCodes) {
                if (c == f.roleCode) {
                    hit = true;
                    break;
                }
            }
            if (!hit)
                return false;
        }
        return true;
    }

    void fillRoles(UserRecord &u)
    {
        u.roleCodes.clear();
        u.primaryRoleId = 0;

        QSqlQuery q(orm::DbManager::instance().database());
        q.prepare(QStringLiteral(
            "SELECT r.code, ur.is_primary, ur.role_id "
            "FROM perm_user_role ur "
            "JOIN perm_role r ON r.id = ur.role_id "
            "WHERE ur.user_id = :uid"));
        q.bindValue(QStringLiteral(":uid"), u.id);
        if (!q.exec())
            return;
        while (q.next()) {
            u.roleCodes.append(q.value(0).toString());
            if (q.value(1).toInt() != 0)
                u.primaryRoleId = q.value(2).toLongLong();
        }
    }
};

// ── Role ─────────────────────────────────────────────────────────────────────

class OrmRoleRepository final : public IRoleRepository
{
public:
    QVector<RoleRecord> list(const RoleQueryFilter &filter) override
    {
        QVector<RoleRecord> out;
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        if (!q.exec(QStringLiteral("SELECT * FROM perm_role")))
            return out;

        while (q.next()) {
            RoleRecord r;
            r.id          = q.value(QStringLiteral("id")).toLongLong();
            r.code        = q.value(QStringLiteral("code")).toString();
            r.name        = q.value(QStringLiteral("name")).toString();
            r.description = q.value(QStringLiteral("description")).toString();
            r.isSystem    = q.value(QStringLiteral("is_system")).toInt() != 0;
            r.isHidden    = q.value(QStringLiteral("is_hidden")).toInt() != 0;
            r.tagColor    = q.value(QStringLiteral("tag_color")).toString();
            r.sortOrder   = q.value(QStringLiteral("sort_order")).toInt();
            r.enabled     = q.value(QStringLiteral("enabled")).toInt() != 0;

            if (!filter.includeHidden && isSuperAdminRole(r))
                continue;
            if (!filter.includeDisabled && !r.enabled)
                continue;
            if (!filter.keyword.isEmpty()
                && !r.name.contains(filter.keyword, Qt::CaseInsensitive)
                && !r.code.contains(filter.keyword, Qt::CaseInsensitive))
                continue;
            out.append(r);
        }
        return out;
    }

    bool findById(qint64 id, RoleRecord &out) override
    {
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral("SELECT * FROM perm_role WHERE id = ?"));
        q.addBindValue(id);
        if (!q.exec() || !q.next())
            return false;

        out.id          = q.value(QStringLiteral("id")).toLongLong();
        out.code        = q.value(QStringLiteral("code")).toString();
        out.name        = q.value(QStringLiteral("name")).toString();
        out.description = q.value(QStringLiteral("description")).toString();
        out.isSystem    = q.value(QStringLiteral("is_system")).toInt() != 0;
        out.isHidden    = q.value(QStringLiteral("is_hidden")).toInt() != 0;
        out.tagColor    = q.value(QStringLiteral("tag_color")).toString();
        out.sortOrder   = q.value(QStringLiteral("sort_order")).toInt();
        out.enabled     = q.value(QStringLiteral("enabled")).toInt() != 0;
        return true;
    }

    bool findByCode(const QString &code, RoleRecord &out) override
    {
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral("SELECT * FROM perm_role WHERE code = ?"));
        q.addBindValue(code);
        if (!q.exec() || !q.next())
            return false;

        out.id          = q.value(QStringLiteral("id")).toLongLong();
        out.code        = q.value(QStringLiteral("code")).toString();
        out.name        = q.value(QStringLiteral("name")).toString();
        out.description = q.value(QStringLiteral("description")).toString();
        out.isSystem    = q.value(QStringLiteral("is_system")).toInt() != 0;
        out.isHidden    = q.value(QStringLiteral("is_hidden")).toInt() != 0;
        out.tagColor    = q.value(QStringLiteral("tag_color")).toString();
        out.sortOrder   = q.value(QStringLiteral("sort_order")).toInt();
        out.enabled     = q.value(QStringLiteral("enabled")).toInt() != 0;
        return true;
    }

    bool insert(RoleRecord &record, QString *errorString) override
    {
        const QString now = nowIso();
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "INSERT INTO perm_role (code, name, description, is_system, is_hidden, "
            "tag_color, sort_order, enabled, created_at, updated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
        q.addBindValue(record.code);
        q.addBindValue(record.name);
        q.addBindValue(record.description);
        q.addBindValue(record.isSystem ? 1 : 0);
        q.addBindValue(record.isHidden ? 1 : 0);
        q.addBindValue(record.tagColor);
        q.addBindValue(record.sortOrder);
        q.addBindValue(record.enabled ? 1 : 0);
        q.addBindValue(now);
        q.addBindValue(now);
        if (!q.exec()) {
            if (errorString)
                *errorString = q.lastError().text();
            return false;
        }
        record.id = q.lastInsertId().toLongLong();
        return true;
    }

    bool update(const RoleRecord &record, QString *errorString) override
    {
        // Check existence
        QSqlDatabase db = orm::DbManager::instance().database();
        {
            QSqlQuery q(db);
            q.prepare(QStringLiteral("SELECT 1 FROM perm_role WHERE id = ?"));
            q.addBindValue(record.id);
            if (!q.exec() || !q.next()) {
                if (errorString)
                    *errorString = QStringLiteral("Role not found.");
                return false;
            }
        }

        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "UPDATE perm_role SET name = ?, description = ?, tag_color = ?, "
            "sort_order = ?, enabled = ?, updated_at = ? WHERE id = ?"));
        q.addBindValue(record.name);
        q.addBindValue(record.description);
        q.addBindValue(record.tagColor);
        q.addBindValue(record.sortOrder);
        q.addBindValue(record.enabled ? 1 : 0);
        q.addBindValue(nowIso());
        q.addBindValue(record.id);
        if (!q.exec()) {
            if (errorString)
                *errorString = q.lastError().text();
            return false;
        }
        return true;
    }

    bool remove(qint64 id, QString *errorString) override
    {
        // Load role for existence check + guard validation
        RoleRecord rec;
        {
            QSqlDatabase db = orm::DbManager::instance().database();
            QSqlQuery q(db);
            q.prepare(QStringLiteral("SELECT * FROM perm_role WHERE id = ?"));
            q.addBindValue(id);
            if (!q.exec() || !q.next()) {
                if (errorString)
                    *errorString = QStringLiteral("Role not found.");
                return false;
            }
            rec.id          = q.value(QStringLiteral("id")).toLongLong();
            rec.code        = q.value(QStringLiteral("code")).toString();
            rec.name        = q.value(QStringLiteral("name")).toString();
            rec.description = q.value(QStringLiteral("description")).toString();
            rec.isSystem    = q.value(QStringLiteral("is_system")).toInt() != 0;
            rec.isHidden    = q.value(QStringLiteral("is_hidden")).toInt() != 0;
            rec.tagColor    = q.value(QStringLiteral("tag_color")).toString();
            rec.sortOrder   = q.value(QStringLiteral("sort_order")).toInt();
            rec.enabled     = q.value(QStringLiteral("enabled")).toInt() != 0;
        }

        if (!canDeleteRole(rec)) {
            if (errorString)
                *errorString = QStringLiteral("protected_role_delete");
            return false;
        }

        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral("DELETE FROM perm_role WHERE id = ?"));
        q.addBindValue(id);
        if (!q.exec()) {
            if (errorString)
                *errorString = q.lastError().text();
            return false;
        }
        return true;
    }

    bool assignPermissions(qint64 roleId,
                           const QVector<qint64> &permissionIds,
                           QString *errorString) override
    {
        QSqlDatabase db = orm::DbManager::instance().database();
        if (!db.transaction())
            return false;

        QSqlQuery del(db);
        del.prepare(QStringLiteral("DELETE FROM perm_role_permission WHERE role_id = :rid"));
        del.bindValue(QStringLiteral(":rid"), roleId);
        if (!del.exec()) {
            db.rollback();
            if (errorString)
                *errorString = del.lastError().text();
            return false;
        }

        const QString now = nowIso();
        for (qint64 pid : permissionIds) {
            QSqlQuery ins(db);
            ins.prepare(QStringLiteral(
                "INSERT INTO perm_role_permission (role_id, permission_id, granted_at) "
                "VALUES (:rid, :pid, :gt)"));
            ins.bindValue(QStringLiteral(":rid"), roleId);
            ins.bindValue(QStringLiteral(":pid"), pid);
            ins.bindValue(QStringLiteral(":gt"), now);
            if (!ins.exec()) {
                db.rollback();
                if (errorString)
                    *errorString = ins.lastError().text();
                return false;
            }
        }
        return db.commit();
    }

    QVector<qint64> permissionIdsOfRole(qint64 roleId) override
    {
        QVector<qint64> ids;
        QSqlQuery q(orm::DbManager::instance().database());
        q.prepare(QStringLiteral(
            "SELECT permission_id FROM perm_role_permission WHERE role_id = :rid"));
        q.bindValue(QStringLiteral(":rid"), roleId);
        if (!q.exec())
            return ids;
        while (q.next())
            ids.append(q.value(0).toLongLong());
        return ids;
    }
};

// ── Permission ───────────────────────────────────────────────────────────────

class OrmPermissionRepository final : public IPermissionResourceRepository
{
public:
    QVector<PermissionRecord> list(const PermissionQueryFilter &filter) override
    {
        return treeFiltered(filter);
    }

    QVector<PermissionRecord> tree() override
    {
        return treeFiltered({});
    }

    bool findById(qint64 id, PermissionRecord &out) override
    {
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral("SELECT * FROM perm_permission WHERE id = ?"));
        q.addBindValue(id);
        if (!q.exec() || !q.next())
            return false;

        out.id           = q.value(QStringLiteral("id")).toLongLong();
        out.parentId     = q.value(QStringLiteral("parent_id")).toLongLong();
        if (out.parentId < 0)
            out.parentId = 0;
        out.pageKey      = q.value(QStringLiteral("page_key")).toString();
        out.resourceType = q.value(QStringLiteral("resource_type")).toString();
        out.key          = q.value(QStringLiteral("key")).toString();
        out.displayName  = q.value(QStringLiteral("display_name")).toString();
        out.description  = q.value(QStringLiteral("description")).toString();
        out.source       = q.value(QStringLiteral("source")).toString();
        out.ownerPlugin  = q.value(QStringLiteral("owner_plugin")).toString();
        out.sortOrder    = q.value(QStringLiteral("sort_order")).toInt();
        out.enabled      = q.value(QStringLiteral("enabled")).toInt() != 0;
        return true;
    }

    bool findByKey(const QString &key, PermissionRecord &out) override
    {
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral("SELECT * FROM perm_permission WHERE key = ?"));
        q.addBindValue(key);
        if (!q.exec() || !q.next())
            return false;

        out.id           = q.value(QStringLiteral("id")).toLongLong();
        out.parentId     = q.value(QStringLiteral("parent_id")).toLongLong();
        if (out.parentId < 0)
            out.parentId = 0;
        out.pageKey      = q.value(QStringLiteral("page_key")).toString();
        out.resourceType = q.value(QStringLiteral("resource_type")).toString();
        out.key          = q.value(QStringLiteral("key")).toString();
        out.displayName  = q.value(QStringLiteral("display_name")).toString();
        out.description  = q.value(QStringLiteral("description")).toString();
        out.source       = q.value(QStringLiteral("source")).toString();
        out.ownerPlugin  = q.value(QStringLiteral("owner_plugin")).toString();
        out.sortOrder    = q.value(QStringLiteral("sort_order")).toInt();
        out.enabled      = q.value(QStringLiteral("enabled")).toInt() != 0;
        return true;
    }

    bool insert(PermissionRecord &record, QString *errorString) override
    {
        const QString now = nowIso();
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "INSERT INTO perm_permission (parent_id, page_key, resource_type, key, "
            "display_name, description, source, owner_plugin, sort_order, enabled, "
            "created_at, updated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
        q.addBindValue(record.parentId);
        q.addBindValue(record.pageKey);
        q.addBindValue(record.resourceType);
        q.addBindValue(record.key);
        q.addBindValue(record.displayName);
        q.addBindValue(record.description);
        q.addBindValue(record.source.isEmpty() ? QStringLiteral("manual") : record.source);
        q.addBindValue(record.ownerPlugin);
        q.addBindValue(record.sortOrder);
        q.addBindValue(record.enabled ? 1 : 0);
        q.addBindValue(now);
        q.addBindValue(now);
        if (!q.exec()) {
            if (errorString)
                *errorString = q.lastError().text();
            return false;
        }
        record.id = q.lastInsertId().toLongLong();
        return true;
    }

    bool update(const PermissionRecord &record, QString *errorString) override
    {
        // Check existence
        QSqlDatabase db = orm::DbManager::instance().database();
        {
            QSqlQuery q(db);
            q.prepare(QStringLiteral("SELECT 1 FROM perm_permission WHERE id = ?"));
            q.addBindValue(record.id);
            if (!q.exec() || !q.next()) {
                if (errorString)
                    *errorString = QStringLiteral("Permission not found.");
                return false;
            }
        }

        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "UPDATE perm_permission SET parent_id = ?, key = ?, resource_type = ?, "
            "page_key = ?, display_name = ?, description = ?, source = ?, "
            "owner_plugin = ?, sort_order = ?, enabled = ?, updated_at = ? "
            "WHERE id = ?"));
        q.addBindValue(record.parentId);
        q.addBindValue(record.key);
        q.addBindValue(record.resourceType);
        q.addBindValue(record.pageKey);
        q.addBindValue(record.displayName);
        q.addBindValue(record.description);
        q.addBindValue(record.source);
        q.addBindValue(record.ownerPlugin);
        q.addBindValue(record.sortOrder);
        q.addBindValue(record.enabled ? 1 : 0);
        q.addBindValue(nowIso());
        q.addBindValue(record.id);
        if (!q.exec()) {
            if (errorString)
                *errorString = q.lastError().text();
            return false;
        }
        return true;
    }

    bool remove(qint64 id, QString *errorString) override
    {
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral("DELETE FROM perm_permission WHERE id = ?"));
        q.addBindValue(id);
        if (!q.exec()) {
            if (errorString)
                *errorString = q.lastError().text();
            return false;
        }
        return true;
    }

private:
    QVector<PermissionRecord> treeFiltered(const PermissionQueryFilter &filter)
    {
        QVector<PermissionRecord> out;
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        if (!q.exec(QStringLiteral("SELECT * FROM perm_permission")))
            return out;

        while (q.next()) {
            PermissionRecord p;
            p.id           = q.value(QStringLiteral("id")).toLongLong();
            p.parentId     = q.value(QStringLiteral("parent_id")).toLongLong();
            if (p.parentId < 0)
                p.parentId = 0;
            p.pageKey      = q.value(QStringLiteral("page_key")).toString();
            p.resourceType = q.value(QStringLiteral("resource_type")).toString();
            p.key          = q.value(QStringLiteral("key")).toString();
            p.displayName  = q.value(QStringLiteral("display_name")).toString();
            p.description  = q.value(QStringLiteral("description")).toString();
            p.source       = q.value(QStringLiteral("source")).toString();
            p.ownerPlugin  = q.value(QStringLiteral("owner_plugin")).toString();
            p.sortOrder    = q.value(QStringLiteral("sort_order")).toInt();
            p.enabled      = q.value(QStringLiteral("enabled")).toInt() != 0;

            if (!filter.includeDisabled && !p.enabled)
                continue;
            if (!filter.resourceType.isEmpty() && p.resourceType != filter.resourceType)
                continue;
            if (!filter.keyword.isEmpty()
                && !p.key.contains(filter.keyword, Qt::CaseInsensitive)
                && !p.displayName.contains(filter.keyword, Qt::CaseInsensitive))
                continue;
            out.append(p);
        }
        std::sort(out.begin(), out.end(), [](const PermissionRecord &a, const PermissionRecord &b) {
            return a.sortOrder < b.sortOrder || (a.sortOrder == b.sortOrder && a.id < b.id);
        });
        return out;
    }
};

// ── Audit ────────────────────────────────────────────────────────────────────

class OrmAuditRepository final : public IAuditRepository
{
public:
    QVector<AuditLogRecord> list(const AuditLogQueryFilter &filter) override
    {
        QVector<AuditLogRecord> out;
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        if (!q.exec(QStringLiteral("SELECT * FROM perm_audit_log")))
            return out;

        while (q.next()) {
            AuditLogRecord a;
            a.id = q.value(QStringLiteral("id")).toLongLong();
            const QString oa = q.value(QStringLiteral("occurred_at")).toString();
            if (!oa.isEmpty())
                a.occurredAt = QDateTime::fromString(oa, Qt::ISODateWithMs);
            a.userId     = q.value(QStringLiteral("user_id")).toLongLong();
            a.username   = q.value(QStringLiteral("username")).toString();
            a.roleNames  = q.value(QStringLiteral("role_names")).toString();
            a.actionType = q.value(QStringLiteral("action_type")).toString();
            a.actionName = q.value(QStringLiteral("action_name")).toString();
            a.target     = q.value(QStringLiteral("target")).toString();
            a.result     = q.value(QStringLiteral("result")).toString();
            a.ip         = q.value(QStringLiteral("ip")).toString();
            a.moduleCode = q.value(QStringLiteral("module_code")).toString();
            a.detailJson = q.value(QStringLiteral("detail_json")).toString();

            if (!filter.operatorName.isEmpty()
                && !a.username.contains(filter.operatorName, Qt::CaseInsensitive))
                continue;
            if (!filter.actionType.isEmpty() && a.actionType != filter.actionType)
                continue;
            if (filter.timeFrom.isValid() && a.occurredAt < filter.timeFrom)
                continue;
            if (filter.timeTo.isValid() && a.occurredAt > filter.timeTo)
                continue;
            out.append(a);
        }
        std::sort(out.begin(), out.end(), [](const AuditLogRecord &a, const AuditLogRecord &b) {
            return a.occurredAt > b.occurredAt;
        });
        const int start = qMax(0, filter.offset);
        if (start >= out.size())
            return {};
        if (filter.limit <= 0)
            return out.mid(start);
        return out.mid(start, qMin(filter.limit, out.size() - start));
    }

    qint64 count(const AuditLogQueryFilter &filter) override
    {
        QVector<AuditLogRecord> out;
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        if (!q.exec(QStringLiteral("SELECT * FROM perm_audit_log")))
            return 0;

        while (q.next()) {
            AuditLogRecord a;
            a.id = q.value(QStringLiteral("id")).toLongLong();
            const QString oa = q.value(QStringLiteral("occurred_at")).toString();
            if (!oa.isEmpty())
                a.occurredAt = QDateTime::fromString(oa, Qt::ISODateWithMs);
            a.userId     = q.value(QStringLiteral("user_id")).toLongLong();
            a.username   = q.value(QStringLiteral("username")).toString();
            a.roleNames  = q.value(QStringLiteral("role_names")).toString();
            a.actionType = q.value(QStringLiteral("action_type")).toString();
            a.actionName = q.value(QStringLiteral("action_name")).toString();
            a.target     = q.value(QStringLiteral("target")).toString();
            a.result     = q.value(QStringLiteral("result")).toString();
            a.ip         = q.value(QStringLiteral("ip")).toString();
            a.moduleCode = q.value(QStringLiteral("module_code")).toString();
            a.detailJson = q.value(QStringLiteral("detail_json")).toString();

            if (!filter.operatorName.isEmpty()
                && !a.username.contains(filter.operatorName, Qt::CaseInsensitive))
                continue;
            if (!filter.actionType.isEmpty() && a.actionType != filter.actionType)
                continue;
            if (filter.timeFrom.isValid() && a.occurredAt < filter.timeFrom)
                continue;
            if (filter.timeTo.isValid() && a.occurredAt > filter.timeTo)
                continue;
            out.append(a);
        }
        return out.size();
    }

    bool append(const AuditLogRecord &record, QString *errorString) override
    {
        const QString now = nowIso();
        QSqlDatabase db = orm::DbManager::instance().database();
        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "INSERT INTO perm_audit_log (occurred_at, user_id, username, role_names, "
            "action_type, action_name, target, result, ip, module_code, detail_json) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
        q.addBindValue(record.occurredAt.isValid()
                           ? record.occurredAt.toString(Qt::ISODateWithMs)
                           : now);
        q.addBindValue(record.userId);
        q.addBindValue(record.username);
        q.addBindValue(record.roleNames);
        q.addBindValue(record.actionType);
        q.addBindValue(record.actionName);
        q.addBindValue(record.target);
        q.addBindValue(record.result);
        q.addBindValue(record.ip);
        q.addBindValue(record.moduleCode);
        q.addBindValue(record.detailJson);
        if (!q.exec()) {
            if (errorString)
                *errorString = q.lastError().text();
            return false;
        }
        return true;
    }
};

} // namespace

OrmDataStore::OrmDataStore(StorageConfig config, StorageBackend backend)
    : m_config(std::move(config))
    , m_backend(backend)
    , m_users(std::make_unique<OrmUserRepository>())
    , m_roles(std::make_unique<OrmRoleRepository>())
    , m_permissions(std::make_unique<OrmPermissionRepository>())
    , m_auditLogs(std::make_unique<OrmAuditRepository>())
{
}

bool OrmDataStore::open(QString *errorString)
{
    if (m_open)
        return true;

    if (!connectDatabase(errorString))
        return false;

    if (m_config.createSchemaOnOpen) {
        if (!createSchema(errorString))
            return false;
    }

    if (m_config.seedUsersOnEmpty) {
        if (!DataSeeder::applyUsersAndRolesIfEmpty(m_backend, errorString))
            return false;
    }

    if (m_config.seedPermissionsOnEmpty) {
        if (!DataSeeder::applyPermissionsIfEmpty(m_backend, errorString))
            return false;
    }

    if (m_backend == StorageBackend::Sqlite || m_backend == StorageBackend::MySql) {
        const OperationResult syncResult = syncSuperAdminGrantsFromCatalog(this);
        if (!syncResult.ok) {
            qWarning() << "[OrmDataStore] syncSuperAdminGrantsFromCatalog failed:"
                       << syncResult.message;
            const qint64 permCount =
                DataSeeder::tableRowCount(QStringLiteral("perm_permission"), errorString);
            if (permCount > 0) {
                if (errorString && errorString->isEmpty())
                    *errorString = syncResult.message;
                return false;
            }
        } else {
            qDebug() << "[OrmDataStore] super_admin grants synced with permission catalog.";
        }
    }

    m_open = true;
    return true;
}

void OrmDataStore::close()
{
    if (!m_open)
        return;
    orm::DbManager::instance().closeAll();
    m_open = false;
}

} // namespace permission
