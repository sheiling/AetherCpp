#pragma once

#include <QString>

namespace permission {

/// 顶栏 nav 资源固定 page_key（与代码 addPage titleKey 的 key 字段配对）
inline constexpr const char *kPermPageKeyRoot = "root";

inline constexpr const char *kRoleCodeSuperAdmin = "super_admin";
inline constexpr const char *kRoleCodeMinimal    = "minimal";
inline constexpr const char *kMinimalUsername    = "minimal";

/// 空闲超时（写死，后续可改为配置项）
inline constexpr int kSessionIdleTimeoutMs = 30 * 60 * 1000; // 30 分钟

/// 权限资源页仅展示代码/种子注册的目录，禁止 UI 新建/编辑/删除/启停
inline constexpr bool kPermissionResourcesCatalogReadOnly = true;

} // namespace permission
