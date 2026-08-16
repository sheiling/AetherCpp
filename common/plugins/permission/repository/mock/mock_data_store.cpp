#include "repository/mock/mock_data_store.h"

#include "service/perm_entity_guard.h"
#include "service/perm_system_constants.h"

#include <QDateTime>

#include <algorithm>

namespace permission {
namespace {

qint64 nextId(qint64 &seq) { return ++seq; }

PermissionRecord makePerm(qint64 id, qint64 parentId, const char *pageKey, const char *key,
                          const char *resourceType, const char *displayName, int sortOrder = 0,
                          const char *description = "")
{
    PermissionRecord r;
    r.id           = id;
    r.parentId     = parentId;
    r.pageKey      = QString::fromUtf8(pageKey);
    r.key          = QString::fromUtf8(key);
    r.resourceType = QString::fromUtf8(resourceType);
    r.displayName = QString::fromUtf8(displayName);
    if (description && *description)
        r.description = QString::fromUtf8(description);
    r.source    = QStringLiteral("manual");
    r.sortOrder = sortOrder;
    r.enabled   = true;
    return r;
}

class MockUserRepository final : public IUserRepository
{
public:
    MockUserRepository(QVector<UserRecord> *users, QVector<QPair<qint64, qint64>> *userRoles,
                       const QVector<RoleRecord> *roles, qint64 *idSeq)
        : m_users(users)
        , m_userRoles(userRoles)
        , m_roles(roles)
        , m_idSeq(idSeq)
    {
    }

    QVector<UserRecord> list(const UserQueryFilter &filter) override
    {
        QVector<UserRecord> out;
        for (auto u : *m_users) {
            fillRoles(u);
            if (!filter.includeHidden && u.isHidden)
                continue;
            if (match(u, filter))
                out.append(u);
        }
        return out;
    }

    qint64 count(const UserQueryFilter &filter) override { return list(filter).size(); }

    bool findById(qint64 id, UserRecord &out) override
    {
        for (auto u : *m_users) {
            if (u.id == id) {
                fillRoles(u);
                out = u;
                return true;
            }
        }
        return false;
    }

    bool findByUsername(const QString &username, UserRecord &out) override
    {
        for (auto u : *m_users) {
            if (u.username == username) {
                fillRoles(u);
                out = u;
                return true;
            }
        }
        return false;
    }

    bool insert(UserRecord &record, QString *) override
    {
        record.id = nextId(*m_idSeq);
        m_users->append(record);
        return true;
    }

    bool update(const UserRecord &record, QString *) override
    {
        for (auto &u : *m_users) {
            if (u.id == record.id) {
                u = record;
                return true;
            }
        }
        return false;
    }

    bool setAccountStatus(qint64 id, int accountStatus, QString *errorString) override
    {
        UserRecord rec;
        if (!findById(id, rec)) {
            if (errorString)
                *errorString = QStringLiteral("User not found.");
            return false;
        }
        if (!canChangeUserAccountStatus(rec)) {
            if (errorString)
                *errorString = QStringLiteral("protected_user_status");
            return false;
        }
        for (auto &u : *m_users) {
            if (u.id == id) {
                u.accountStatus = accountStatus;
                return true;
            }
        }
        return false;
    }

    bool resetPassword(qint64, const QString &, QString *) override { return true; }

    bool assignRoles(qint64 userId, const QVector<qint64> &roleIds, qint64, QString *) override
    {
        m_userRoles->erase(std::remove_if(m_userRoles->begin(), m_userRoles->end(),
                                          [userId](const QPair<qint64, qint64> &p) {
                                              return p.first == userId;
                                          }),
                           m_userRoles->end());
        for (qint64 rid : roleIds)
            m_userRoles->append({userId, rid});
        return true;
    }

    QVector<qint64> roleIdsOfUser(qint64 userId) override
    {
        QVector<qint64> ids;
        for (const auto &p : *m_userRoles) {
            if (p.first == userId)
                ids.append(p.second);
        }
        return ids;
    }

private:
    static bool match(const UserRecord &u, const UserQueryFilter &f)
    {
        if (f.accountStatus >= 0 && u.accountStatus != f.accountStatus)
            return false;
        if (!f.keyword.isEmpty()) {
            const QString kw = f.keyword.trimmed();
            if (!u.username.contains(kw, Qt::CaseInsensitive)
                && !u.displayName.contains(kw, Qt::CaseInsensitive))
                return false;
        }
        if (!f.roleCode.isEmpty() && !u.roleCodes.contains(f.roleCode))
            return false;
        return true;
    }

    void fillRoles(UserRecord &u)
    {
        u.roleCodes.clear();
        u.primaryRoleId = 0;
        for (const auto &p : *m_userRoles) {
            if (p.first != u.id)
                continue;
            for (const auto &r : *m_roles) {
                if (r.id == p.second) {
                    u.roleCodes.append(r.code);
                    if (u.primaryRoleId == 0)
                        u.primaryRoleId = p.second;
                }
            }
        }
    }

    QVector<UserRecord>            *m_users;
    QVector<QPair<qint64, qint64>> *m_userRoles;
    const QVector<RoleRecord>      *m_roles;
    qint64                         *m_idSeq;
};

class MockRoleRepository final : public IRoleRepository
{
public:
    MockRoleRepository(QVector<RoleRecord> *roles, QVector<QPair<qint64, qint64>> *rolePerms,
                       qint64 *idSeq)
        : m_roles(roles)
        , m_rolePerms(rolePerms)
        , m_idSeq(idSeq)
    {
    }

    QVector<RoleRecord> list(const RoleQueryFilter &filter) override
    {
        QVector<RoleRecord> out;
        for (const auto &r : *m_roles) {
            if (!filter.includeHidden && r.isHidden)
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
        for (const auto &r : *m_roles) {
            if (r.id == id) {
                out = r;
                return true;
            }
        }
        return false;
    }

    bool findByCode(const QString &code, RoleRecord &out) override
    {
        for (const auto &r : *m_roles) {
            if (r.code == code) {
                out = r;
                return true;
            }
        }
        return false;
    }

    bool insert(RoleRecord &record, QString *) override
    {
        record.id = nextId(*m_idSeq);
        m_roles->append(record);
        return true;
    }

    bool update(const RoleRecord &record, QString *) override
    {
        for (auto &r : *m_roles) {
            if (r.id == record.id) {
                r = record;
                return true;
            }
        }
        return false;
    }

    bool remove(qint64 id, QString *errorString) override
    {
        RoleRecord rec;
        if (!findById(id, rec)) {
            if (errorString)
                *errorString = QStringLiteral("Role not found.");
            return false;
        }
        if (!canDeleteRole(rec)) {
            if (errorString)
                *errorString = QStringLiteral("protected_role_delete");
            return false;
        }
        const int n = m_roles->size();
        m_roles->erase(std::remove_if(m_roles->begin(), m_roles->end(),
                                      [id](const RoleRecord &r) { return r.id == id; }),
                       m_roles->end());
        return m_roles->size() < n;
    }

    bool assignPermissions(qint64 roleId, const QVector<qint64> &permissionIds, QString *) override
    {
        m_rolePerms->erase(std::remove_if(m_rolePerms->begin(), m_rolePerms->end(),
                                          [roleId](const QPair<qint64, qint64> &p) {
                                              return p.first == roleId;
                                          }),
                           m_rolePerms->end());
        for (qint64 pid : permissionIds)
            m_rolePerms->append({roleId, pid});
        return true;
    }

    QVector<qint64> permissionIdsOfRole(qint64 roleId) override
    {
        QVector<qint64> ids;
        for (const auto &p : *m_rolePerms) {
            if (p.first == roleId)
                ids.append(p.second);
        }
        return ids;
    }

private:
    QVector<RoleRecord>            *m_roles;
    QVector<QPair<qint64, qint64>> *m_rolePerms;
    qint64                         *m_idSeq;
};

class MockPermissionRepository final : public IPermissionResourceRepository
{
public:
    explicit MockPermissionRepository(QVector<PermissionRecord> *data, qint64 *idSeq)
        : m_data(data)
        , m_idSeq(idSeq)
    {
    }

    QVector<PermissionRecord> list(const PermissionQueryFilter &filter) override
    {
        return filtered(filter);
    }

    QVector<PermissionRecord> tree() override { return filtered({}); }

    bool findById(qint64 id, PermissionRecord &out) override
    {
        for (const auto &p : *m_data) {
            if (p.id == id) {
                out = p;
                return true;
            }
        }
        return false;
    }

    bool findByKey(const QString &key, PermissionRecord &out) override
    {
        for (const auto &p : *m_data) {
            if (p.key == key) {
                out = p;
                return true;
            }
        }
        return false;
    }

    bool insert(PermissionRecord &record, QString *errorString) override
    {
        PermissionRecord existing;
        if (findByKey(record.key, existing)) {
            if (errorString)
                *errorString = QStringLiteral("key_duplicate");
            return false;
        }
        record.id = nextId(*m_idSeq);
        m_data->append(record);
        return true;
    }

    bool update(const PermissionRecord &record, QString *) override
    {
        for (auto &p : *m_data) {
            if (p.id == record.id) {
                p = record;
                return true;
            }
        }
        return false;
    }

    bool remove(qint64 id, QString *) override
    {
        const int n = m_data->size();
        m_data->erase(std::remove_if(m_data->begin(), m_data->end(),
                                     [id](const PermissionRecord &p) { return p.id == id; }),
                      m_data->end());
        return m_data->size() < n;
    }

private:
    QVector<PermissionRecord> filtered(const PermissionQueryFilter &filter)
    {
        QVector<PermissionRecord> out;
        for (const auto &p : *m_data) {
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
            return a.sortOrder < b.sortOrder;
        });
        return out;
    }

    QVector<PermissionRecord> *m_data;
    qint64                    *m_idSeq;
};

class MockAuditRepository final : public IAuditRepository
{
public:
    explicit MockAuditRepository(QVector<AuditLogRecord> *data, qint64 *idSeq)
        : m_data(data)
        , m_idSeq(idSeq)
    {
    }

    static QVector<AuditLogRecord> matched(const AuditLogQueryFilter &filter,
                                             const QVector<AuditLogRecord> *data)
    {
        QVector<AuditLogRecord> out;
        for (const auto &a : *data) {
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
        return out;
    }

    QVector<AuditLogRecord> list(const AuditLogQueryFilter &filter) override
    {
        QVector<AuditLogRecord> out = matched(filter, m_data);
        const int start           = qMax(0, filter.offset);
        if (start >= out.size())
            return {};
        if (filter.limit <= 0)
            return out.mid(start);
        return out.mid(start, qMin(filter.limit, out.size() - start));
    }

    qint64 count(const AuditLogQueryFilter &filter) override
    {
        AuditLogQueryFilter f = filter;
        f.offset              = 0;
        f.limit               = 0;
        return matched(f, m_data).size();
    }

    bool append(const AuditLogRecord &record, QString *) override
    {
        AuditLogRecord r = record;
        r.id = nextId(*m_idSeq);
        if (!r.occurredAt.isValid())
            r.occurredAt = QDateTime::currentDateTime();
        m_data->append(r);
        return true;
    }

private:
    QVector<AuditLogRecord> *m_data;
    qint64                  *m_idSeq;
};

} // namespace

MockDataStore::MockDataStore(StorageConfig config)
    : m_config(std::move(config))
{
    seedData();

    m_usersRepo = std::make_unique<MockUserRepository>(&m_users, &m_userRoles, &m_roles, &m_userIdSeq);
    m_rolesRepo = std::make_unique<MockRoleRepository>(&m_roles, &m_rolePerms, &m_roleIdSeq);
    m_permissionsRepo = std::make_unique<MockPermissionRepository>(&m_permissionResources, &m_permIdSeq);
    m_auditLogsRepo = std::make_unique<MockAuditRepository>(&m_auditLogs, &m_auditIdSeq);
}

bool MockDataStore::open(QString *)
{
    m_open = true;
    return true;
}

void MockDataStore::close() { m_open = false; }

IUserRepository &MockDataStore::users() { return *m_usersRepo; }
IRoleRepository &MockDataStore::roles() { return *m_rolesRepo; }
IPermissionResourceRepository &MockDataStore::permissions() { return *m_permissionsRepo; }
IAuditRepository &MockDataStore::auditLogs() { return *m_auditLogsRepo; }

void MockDataStore::seedData()
{
    m_roles = {
        {1,
         QStringLiteral("super_admin"),
         QStringLiteral("超级管理员"),
         QStringLiteral("系统内置，不可删除"),
         true,
         true,
         QStringLiteral("#ef4444"),
         0,
         true},
        {2,
         QStringLiteral("engineer"),
         QStringLiteral("工程师"),
         QStringLiteral("负责全厂配方管理、参数调优、检测流程配置、设备联机策略与异常恢复方案设计，"
                         "并参与版本发布评审与跨部门技术支持，确保产线稳定运行与质量追溯。"),
         false,
         false,
         QStringLiteral("#3b82f6"),
         1,
         true},
        {3,
         QStringLiteral("operator"),
         QStringLiteral("操作员"),
         QStringLiteral("日常检测与设备操作"),
         false,
         false,
         QStringLiteral("#22c55e"),
         2,
         true},
        {4,
         QStringLiteral("guest"),
         QStringLiteral("访客"),
         QStringLiteral("只读查看"),
         false,
         false,
         QStringLiteral("#9ca3af"),
         3,
         true},
        {5,
         QStringLiteral("minimal"),
         QStringLiteral("最小权限"),
         QStringLiteral("未登录缺省身份；可在角色管理中调整 grants"),
         true,
         false,
         QStringLiteral("#64748b"),
         4,
         true},
    };

    const auto dt = [](const char *s) {
        return QDateTime::fromString(QString::fromUtf8(s), QStringLiteral("yyyy-MM-dd HH:mm"));
    };

    m_users = {
        {1,
         QStringLiteral("admin"),
         QStringLiteral("系统管理员"),
         QStringLiteral("IT"),
         QString(),
         1,
         dt("2026-05-26 09:12"),
         {},
         0,
         true},
        {2,
         QStringLiteral("zhang.san"),
         QStringLiteral("张三"),
         QStringLiteral("工艺"),
         QString(),
         1,
         dt("2026-05-26 08:55")},
        {3,
         QStringLiteral("lisi"),
         QStringLiteral("李四"),
         QStringLiteral("产线 A"),
         QString(),
         1,
         dt("2026-05-25 17:40")},
        {4,
         QStringLiteral("wang.wu"),
         QStringLiteral("王五"),
         QStringLiteral("质检"),
         QString(),
         0,
         dt("2026-05-20 11:05")},
        {5,
         QStringLiteral("minimal"),
         QStringLiteral("最小权限用户"),
         QStringLiteral("系统"),
         QString(),
         1,
         dt("2026-05-26 07:00")},
    };
    m_userRoles = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};

    m_permissionResources = {
        makePerm(70, 0, kPermPageKeyRoot, "nav.home", PermResourceType::Nav, "首页", 10,
                 "生产看板与快捷入口"),
        makePerm(71, 70, "nav.home", "sub.home.dashboard", PermResourceType::SubPage, "生产看板", 11),
        makePerm(72, 70, "nav.home", "sub.home.alerts", PermResourceType::SubPage, "告警中心", 12),
        makePerm(73, 71, "sub.home.dashboard", "home.dashboard.view", PermResourceType::Action,
                 "查看看板", 20),
        makePerm(74, 71, "sub.home.dashboard", "home.dashboard.export", PermResourceType::Action,
                 "导出报表", 21),
        makePerm(75, 72, "sub.home.alerts", "home.alerts.view", PermResourceType::Action,
                 "查看告警", 22),
        makePerm(76, 72, "sub.home.alerts", "home.alerts.ack", PermResourceType::Action,
                 "确认告警", 23),

        makePerm(20, 0, kPermPageKeyRoot, "nav.permission", PermResourceType::Nav, "权限管理", 90,
                 "权限资源与角色授权配置"),
        makePerm(25, 20, "nav.permission", "perm.nav.exportConfig", PermResourceType::Action,
                 "导出权限配置", 95),
        makePerm(26, 20, "nav.permission", "perm.nav.quickSync", PermResourceType::Action,
                 "一键同步全部菜单与代码权限定义到资源树", 96),
        makePerm(21, 20, "nav.permission", "sub.perm.users", PermResourceType::SubPage, "用户管理",
                 91),
        makePerm(22, 20, "nav.permission", "sub.perm.roles", PermResourceType::SubPage, "角色管理",
                 92),
        makePerm(23, 20, "nav.permission", "sub.perm.resources", PermResourceType::SubPage,
                 "权限资源", 93),
        makePerm(24, 20, "nav.permission", "sub.perm.audit", PermResourceType::SubPage, "审计日志",
                 94),
        makePerm(29, 21, "sub.perm.users", "perm.users.query", PermResourceType::Action, "查询", 99),
        makePerm(30, 21, "sub.perm.users", "perm.users.create", PermResourceType::Action,
                 "新增用户", 100),
        makePerm(31, 21, "sub.perm.users", "perm.users.resetPassword", PermResourceType::Action,
                 "重置密码", 101),
        makePerm(32, 21, "sub.perm.users", "perm.users.resetFilter", PermResourceType::Action,
                 "重置", 102),
        makePerm(33, 21, "sub.perm.users", "perm.users.edit", PermResourceType::Action, "编辑", 103),
        makePerm(34, 21, "sub.perm.users", "perm.users.toggleStatus", PermResourceType::Action,
                 "启用/禁用", 104),
        makePerm(35, 21, "sub.perm.users", "perm.users.bulkEnable", PermResourceType::Action,
                 "批量启用", 105),
        makePerm(36, 21, "sub.perm.users", "perm.users.bulkDisable", PermResourceType::Action,
                 "批量禁用", 106),
        makePerm(37, 21, "sub.perm.users", "perm.users.bulkAssignRoles", PermResourceType::Action,
                 "批量分配角色", 107),
        makePerm(40, 22, "sub.perm.roles", "perm.roles.save", PermResourceType::Action,
                 "保存权限", 110),
        makePerm(41, 22, "sub.perm.roles", "perm.roles.create", PermResourceType::Action,
                 "新增角色", 111),
        makePerm(42, 22, "sub.perm.roles", "perm.roles.edit", PermResourceType::Action,
                 "编辑角色", 112),
        makePerm(43, 22, "sub.perm.roles", "perm.roles.selectAll", PermResourceType::Action,
                 "全选", 113),
        makePerm(44, 22, "sub.perm.roles", "perm.roles.clear", PermResourceType::Action, "清空",
                 114),
        makePerm(45, 22, "sub.perm.roles", "perm.roles.reset", PermResourceType::Action, "重置",
                 115),
        makePerm(46, 23, "sub.perm.resources", "perm.resources.createNav", PermResourceType::Action,
                 "新建导航", 115),
        makePerm(47, 23, "sub.perm.resources", "perm.resources.createSubPage",
                 PermResourceType::Action, "新建子页", 116),
        makePerm(48, 23, "sub.perm.resources", "perm.resources.createAction",
                 PermResourceType::Action, "新建操作", 117),
        makePerm(49, 23, "sub.perm.resources", "perm.resources.toggleEnabled",
                 PermResourceType::Action, "禁用", 118),
        makePerm(51, 23, "sub.perm.resources", "perm.resources.reset", PermResourceType::Action,
                 "重置", 119),
        makePerm(52, 23, "sub.perm.resources", "perm.resources.save", PermResourceType::Action,
                 "保存", 120),
        makePerm(53, 23, "sub.perm.resources", "perm.resources.delete", PermResourceType::Action,
                 "删除", 121),
        makePerm(59, 24, "sub.perm.audit", "perm.audit.query", PermResourceType::Action, "查询",
                 129),
        makePerm(62, 24, "sub.perm.audit", "perm.audit.resetFilter", PermResourceType::Action,
                 "重置", 130),
        makePerm(60, 24, "sub.perm.audit", "perm.audit.export", PermResourceType::Action, "导出",
                 131),
        makePerm(63, 24, "sub.perm.audit", "perm.audit.prevPage", PermResourceType::Action,
                 "上一页", 132),
        makePerm(64, 24, "sub.perm.audit", "perm.audit.nextPage", PermResourceType::Action,
                 "下一页", 133),
        makePerm(61, 24, "sub.perm.audit", "perm.audit.view", PermResourceType::Action, "查看详情",
                 134),

        makePerm(100, 0, kPermPageKeyRoot, "nav.settings", PermResourceType::Nav, "设置", 30),
        makePerm(101, 0, kPermPageKeyRoot, "nav.mvvm", PermResourceType::Nav, "MVVM 示例", 40),
        makePerm(102, 0, kPermPageKeyRoot, "nav.mvvm_device", PermResourceType::Nav, "设备 MVVM", 41),
        makePerm(103, 0, kPermPageKeyRoot, "nav.middleware", PermResourceType::Nav, "中间件", 50),
        makePerm(104, 0, kPermPageKeyRoot, "nav.invoker", PermResourceType::Nav, "调用器", 60),
        makePerm(105, 0, kPermPageKeyRoot, "nav.container", PermResourceType::Nav, "容器", 70),
        makePerm(106, 0, kPermPageKeyRoot, "nav.lighten.home", PermResourceType::Nav, "点灯-首页", 15),
        makePerm(107, 0, kPermPageKeyRoot, "nav.lighten.system", PermResourceType::Nav, "点灯-系统",
                 16),
        makePerm(108, 0, kPermPageKeyRoot, "nav.lighten.recipe", PermResourceType::Nav, "点灯-配方",
                 17),
        makePerm(109, 0, kPermPageKeyRoot, "nav.lighten.results", PermResourceType::Nav, "点灯-结果",
                 18),
        makePerm(110, 0, kPermPageKeyRoot, "nav.lighten.virtualpx", PermResourceType::Nav,
                 "点灯-虚拟像素", 19),
        makePerm(121, 0, kPermPageKeyRoot, "nav.lighten.motion_debug", PermResourceType::Nav,
                 "运控调试", 20),
        makePerm(111, 0, kPermPageKeyRoot, "nav.lighten.help", PermResourceType::Nav, "点灯-帮助", 19),

        makePerm(112, 100, "nav.settings", "sub.general", PermResourceType::SubPage, "常规", 1),
        makePerm(113, 100, "nav.settings", "sub.account", PermResourceType::SubPage, "账户", 2),
        makePerm(114, 100, "nav.settings", "sub.privacy", PermResourceType::SubPage, "隐私", 3),
        makePerm(115, 100, "nav.settings", "sub.about", PermResourceType::SubPage, "关于", 4),
        makePerm(117, 104, "nav.invoker", "sub.invoker_basic", PermResourceType::SubPage,
                 "基础调用", 1),
        makePerm(118, 104, "nav.invoker", "sub.invoker_args", PermResourceType::SubPage,
                 "参数调用", 2),
        makePerm(119, 105, "nav.container", "sub.container_type", PermResourceType::SubPage,
                 "类型容器", 1),
        makePerm(120, 105, "nav.container", "sub.container_named", PermResourceType::SubPage,
                 "命名容器", 2),

        makePerm(401, 107, "nav.lighten.system", "systemConfig.save", PermResourceType::Action,
                 "保存配置", 1),
        makePerm(402, 107, "nav.lighten.system", "systemConfig.reset", PermResourceType::Action,
                 "恢复默认", 2),
        makePerm(403, 107, "nav.lighten.system", "systemConfig.clearUseCount",
                 PermResourceType::Action, "使用次数清零", 3),
        makePerm(404, 108, "nav.lighten.recipe", "recipe.save", PermResourceType::Action,
                 "保存配方", 1),
        makePerm(405, 108, "nav.lighten.recipe", "recipe.setDefault", PermResourceType::Action,
                 "设为默认配方", 2),
        makePerm(406, 110, "nav.lighten.virtualpx", "virtualPixel.save", PermResourceType::Action,
                 "保存虚拟像素", 1),
        makePerm(407, 121, "nav.lighten.motion_debug", "motionDebug.ioRead", PermResourceType::Action,
                 "读入IO", 1),
        makePerm(408, 121, "nav.lighten.motion_debug", "motionDebug.ioWrite", PermResourceType::Action,
                 "写出IO", 2),
    };

    // T-action-01：用户管理下 15+ 操作项（含长短文案）
    static const char *kUserActionNames[] = {
        "编辑用户资料",
        "禁用/启用账号",
        "分配角色",
        "批量导入用户并校验组织架构权限",
        "导出用户列表为 Excel 文件",
        "查看登录历史与安全审计记录",
        "发送账号激活邮件",
        "强制下线所有会话",
        "复制用户权限模板到多名成员",
        "删除用户（需二次确认）",
        "同步 LDAP/AD 目录用户",
        "重置二次验证设备绑定",
        "调整用户所属部门与产线",
        "查看用户操作轨迹时间线",
        "审批待激活的自助注册申请",
    };
    int userActSort = 200;
    for (int i = 0; i < 15; ++i) {
        const qint64 id = 301 + i;
        m_permissionResources.append(makePerm(
            id, 21, "sub.perm.users",
            QString("perm.users.extra%1").arg(i + 1).toUtf8().constData(), PermResourceType::Action,
            kUserActionNames[i], userActSort++));
    }

    m_rolePerms.clear();
    for (const auto &p : m_permissionResources)
        m_rolePerms.append({1, p.id});

    const auto grant = [this](qint64 roleId, std::initializer_list<qint64> ids) {
        for (qint64 id : ids)
            m_rolePerms.append({roleId, id});
    };

    // engineer：权限管理 + 首页/产品配置部分权限，不含重置密码(31)与 quickSync(26)
    grant(2, {70, 71, 72, 73, 74, 75, 20, 21, 22, 23, 24, 25, 29, 30, 40,
              41, 42, 43, 44, 45, 46, 47, 48, 49, 51, 52, 53, 59, 60, 62, 63, 64, 100, 101, 102,
              103, 104,
              105, 106, 107, 108,
              109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
              401, 402, 403, 404, 405, 406, 407, 408});
    for (const auto &p : m_permissionResources) {
        if (p.id == 31 || p.id == 26)
            continue;
        if (p.resourceType == PermResourceType::Action && p.parentId == 21 && p.id >= 301)
            m_rolePerms.append({2, p.id});
    }

    grant(3, {70, 71, 73, 20, 21, 30, 100, 106});
    grant(4, {20, 24, 61});
    grant(5, {20, 21, 22, 23, 24, 29, 32, 59, 62, 63, 64});

    // 审计样例：时间相对「今天」，保证默认 30 天筛选内可见
    const auto auditAt = [](int daysAgo, int hour, int minute, int second = 0) {
        return QDateTime(QDate::currentDate().addDays(-daysAgo), QTime(hour, minute, second));
    };
    const auto auditRow = [&](qint64 id, const QDateTime &when, qint64 userId,
                              const QString &username, const QString &roleNames,
                              const QString &actionType, const QString &actionName,
                              const QString &target, const QString &result,
                              const QString &ip = QString(), const QString &moduleCode = QString(),
                              const QString &detailJson = QString()) {
        AuditLogRecord r;
        r.id          = id;
        r.occurredAt  = when;
        r.userId      = userId;
        r.username    = username;
        r.roleNames   = roleNames;
        r.actionType  = actionType;
        r.actionName  = actionName;
        r.target      = target;
        r.result      = result;
        r.ip          = ip;
        r.moduleCode  = moduleCode;
        r.detailJson  = detailJson;
        return r;
    };

    m_auditLogs = {
        auditRow(1, auditAt(0, 8, 30, 5), 1, QStringLiteral("admin"),
                 QStringLiteral("超级管理员"), QStringLiteral("login"), QStringLiteral("登录"),
                 QStringLiteral("—"), QStringLiteral("success"), QStringLiteral("192.168.1.10")),
        auditRow(2, auditAt(0, 10, 15, 0), 0, QStringLiteral("system"), QString(),
                 QStringLiteral("role_permission"), QStringLiteral("save_role_grants"),
                 QStringLiteral("role:1"), QStringLiteral("success"), QString(),
                 QStringLiteral("perm"),
                 QStringLiteral("{\"roleId\":1,\"roleName\":\"超级管理员\",\"normalizedCount\":"
                                "42,\"added\":[70,71],\"removed\":[26]}")),
        auditRow(3, auditAt(0, 14, 22, 18), 2, QStringLiteral("zhang.san"),
                 QStringLiteral("工程师"), QStringLiteral("recipe_edit"),
                 QStringLiteral("配方修改"), QStringLiteral("PANEL A01 / 亮度曲线"),
                 QStringLiteral("success"), QStringLiteral("192.168.1.23"), QString(),
                 QStringLiteral("{\"panel\":\"PANEL A01\",\"brightness\":[120,135,150],"
                                "\"gamma\":2.2,\"savedBy\":\"zhang.san\"}")),
        auditRow(4, auditAt(1, 9, 18, 1), 1, QStringLiteral("admin"),
                 QStringLiteral("超级管理员"), QStringLiteral("permission_change"),
                 QStringLiteral("权限变更"), QStringLiteral("角色: 操作员"),
                 QStringLiteral("success"), QStringLiteral("192.168.1.10"), QString(),
                 QStringLiteral("{\"roleName\":\"操作员\",\"changes\":[\"perm.user.view\","
                                "\"perm.user.edit\"]}")),
        auditRow(5, auditAt(1, 16, 40, 44), 3, QStringLiteral("lisi"),
                 QStringLiteral("操作员"), QStringLiteral("system_config"),
                 QStringLiteral("系统配置"), QStringLiteral("网络设置 / DHCP"),
                 QStringLiteral("denied"), QStringLiteral("192.168.1.55"), QString(),
                 QStringLiteral("{\"section\":\"network\",\"dhcp\":true,"
                                "\"reason\":\"insufficient_permission\"}")),
        auditRow(6, auditAt(3, 11, 5, 0), 2, QStringLiteral("zhang.san"),
                 QStringLiteral("工程师"), QStringLiteral("role_permission"),
                 QStringLiteral("save_role_grants"), QStringLiteral("role:2"),
                 QStringLiteral("success"), QString(),
                 QStringLiteral("perm"),
                 QStringLiteral("{\"roleId\":2,\"roleName\":\"工程师\",\"normalizedCount\":"
                                "28,\"added\":[301,302],\"removed\":[]}")),
        auditRow(7, auditAt(5, 7, 50, 12), 3, QStringLiteral("lisi"),
                 QStringLiteral("操作员"), QStringLiteral("login"), QStringLiteral("登录"),
                 QStringLiteral("—"), QStringLiteral("success"), QStringLiteral("192.168.1.55")),
        auditRow(8, auditAt(7, 13, 20, 0), 1, QStringLiteral("admin"),
                 QStringLiteral("超级管理员"), QStringLiteral("permission_change"),
                 QStringLiteral("资源维护"), QStringLiteral("perm:nav:perm / 用户管理"),
                 QStringLiteral("success"), QStringLiteral("192.168.1.10"),
                 QStringLiteral("perm"),
                 QStringLiteral("{\"resourceKey\":\"perm.user.create\",\"displayName\":"
                                "\"新增用户\",\"enabled\":true}")),
        auditRow(9, auditAt(12, 9, 0, 0), 4, QStringLiteral("guest"), QStringLiteral("访客"),
                 QStringLiteral("login"), QStringLiteral("登录"), QStringLiteral("—"),
                 QStringLiteral("success"), QStringLiteral("192.168.1.88")),
        auditRow(10, auditAt(20, 15, 30, 0), 1, QStringLiteral("admin"),
                 QStringLiteral("超级管理员"), QStringLiteral("system_config"),
                 QStringLiteral("系统配置"), QStringLiteral("检测阈值 / NG 上限"),
                 QStringLiteral("success"), QStringLiteral("192.168.1.10"), QString(),
                 QStringLiteral("{\"ngLimit\":5,\"confidence\":0.92,\"line\":\"L3\"}")),
    };
    m_auditIdSeq = 100;
}

} // namespace permission
