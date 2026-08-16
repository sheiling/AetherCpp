#pragma once

#include "repository/interface/idatastore.h"
#include "service/iplugin_availability.h"

namespace permission {

struct CatalogReconcileResult {
    int hardDeleted  = 0;
    int softDisabled = 0;
    int reEnabled    = 0;
};

/**
 * @brief 按插件可用性对齐 source=plugin 的权限目录
 *
 * - owner 不在 deployed → 硬删子树（action → sub_page → nav）并级联 perm_role_permission
 * - owner 在 deployed 但不在 active（启动失败）→ 保留资源与角色授权，不修改 enabled
 * - owner 在 active 且在 disabled → 软下线 enabled=false（Phase 2）
 * - owner 在 active 且未 disabled → enabled=true
 */
CatalogReconcileResult reconcilePermissionCatalog(IDataStore *store,
                                                  const IPluginAvailability &availability);

} // namespace permission
