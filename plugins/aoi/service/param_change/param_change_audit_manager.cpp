/**
 * @file param_change_audit_manager.cpp
 * @brief ParamChangeAuditManager 单例：configure / audit / persist
 */

#include "param_change_audit_manager.h"

#if COMMON_ORM_ENABLED
#include "repository/param_change/param_change_db_store.h"
#include "repository/param_change/param_change_file_store.h"
#endif

namespace aoi {

ParamChangeAuditManager &ParamChangeAuditManager::instance()
{
    static ParamChangeAuditManager inst;
    return inst;
}

ParamChangeAuditManager::ParamChangeAuditManager()
{
    // 默认策略：仅写数据库；真正运行时可在插件启动阶段覆盖。
    configure(ParamStoreConfig{});
}

void ParamChangeAuditManager::configure(const ParamStoreConfig &config)
{
    // 启动注入：每次调用都重建后端列表，确保策略即时生效。
    m_config = config;
    m_stores.clear();

    switch (m_config.mode) {
    case ParamStoreMode::None:
        break;
    case ParamStoreMode::Database:
#if COMMON_ORM_ENABLED
        m_stores.append(std::make_shared<ParamChangeDbStore>(m_config.dbPath));
#endif
        break;
    case ParamStoreMode::File:
#if COMMON_ORM_ENABLED
        m_stores.append(std::make_shared<ParamChangeFileStore>(m_config.fileDir));
#endif
        break;
    case ParamStoreMode::Dual:
#if COMMON_ORM_ENABLED
        m_stores.append(std::make_shared<ParamChangeDbStore>(m_config.dbPath));
        m_stores.append(std::make_shared<ParamChangeFileStore>(m_config.fileDir));
#endif
        break;
    }
}

void ParamChangeAuditManager::setStores(const QVector<std::shared_ptr<IParamChangeStore>> &stores)
{
    m_stores = stores;
}

void ParamChangeAuditManager::persist(const ParamAuditContext &ctx,
                                      const QVector<parammeta::ParamChangeRecord> &changes)
{
    if (changes.isEmpty())
        return;
    for (const std::shared_ptr<IParamChangeStore> &store : m_stores) {
        if (store)
            store->save(ctx, changes);
    }
}

} // namespace aoi
