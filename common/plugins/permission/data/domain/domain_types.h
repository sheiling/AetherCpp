#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVector>

namespace permission {

/// 数据存储介质（由外部配置决定具体实现）
enum class StorageBackend {
    Mock   = 0,
    Sqlite = 1,
    MySql  = 2,
    File   = 3
};

inline QString storageBackendName(StorageBackend b)
{
    switch (b) {
    case StorageBackend::Mock:   return QStringLiteral("mock");
    case StorageBackend::Sqlite: return QStringLiteral("sqlite");
    case StorageBackend::MySql:  return QStringLiteral("mysql");
    case StorageBackend::File:   return QStringLiteral("file");
    }
    return QStringLiteral("unknown");
}

/// perm_permission.resource_type 取值
namespace PermResourceType {
inline constexpr const char *Nav     = "nav";
inline constexpr const char *SubPage = "sub_page";
inline constexpr const char *Action  = "action";
} // namespace PermResourceType

struct RoleRecord {
    qint64  id          = 0;
    QString code;
    QString name;
    QString description;
    bool    isSystem    = false;
    bool    isHidden    = false; ///< 系统内置且不在管理界面展示（如超级管理员）
    QString tagColor;
    int     sortOrder   = 0;
    bool    enabled     = true;
};

struct PermissionRecord {
    qint64  id           = 0;
    qint64  parentId     = 0;
    QString pageKey;      ///< 配置期：树形归属/筛选（如 nav 下挂 root），不参与运行时判定
    QString resourceType; ///< 配置期：nav | sub_page | action（类型名，非 key 前缀）
    QString key;          ///< 全局唯一；应对齐代码里 titleKey 或 Command::name()，无 nav/sub/act 前缀要求
    QString displayName;
    QString description;
    QString source;
    /// 注册来源插件名（PluginSpec::name()）；source=plugin 时由 Catalog 注册写入
    QString ownerPlugin;
    int     sortOrder    = 0;
    bool    enabled      = true;
};

struct UserRecord {
    qint64      id           = 0;
    QString     username;
    QString     displayName;
    QString     department;
    QString     remark;
    int         accountStatus = 1;
    QDateTime   lastLoginAt;
    QStringList roleCodes;
    qint64      primaryRoleId = 0;
    bool        isHidden      = false; ///< 系统内置且不在管理界面展示（如超级用户）
};

struct AuditLogRecord {
    qint64    id          = 0;
    QDateTime occurredAt;
    qint64    userId      = 0;
    QString   username;
    QString   roleNames;
    QString   actionType;
    QString   actionName;
    QString   target;
    QString   result;
    QString   ip;
    QString   moduleCode;
    QString   detailJson;
};

struct UserQueryFilter {
    QString keyword;
    QString roleCode;
    int     accountStatus = -1;
    int     offset   = 0;
    int     limit    = 100;
    bool    includeHidden = false; ///< 管理列表默认 false，排除超级用户等
};

struct RoleQueryFilter {
    QString keyword;
    bool    includeDisabled = false;
    bool    includeHidden   = false; ///< 管理列表默认 false，排除超级管理员角色
};

struct PermissionQueryFilter {
    QString keyword;
    QString resourceType;
    bool    includeDisabled = false;
};

struct AuditLogQueryFilter {
    QString     operatorName;
    QString     actionType;
    QDateTime   timeFrom;
    QDateTime   timeTo;
    int         offset = 0;
    int         limit  = 200;
};

} // namespace permission
