#pragma once

#include "service/param_change/param_change_store.h"

namespace aoi {

/**
 * @brief 参数变更数据库存储后端（SQLite）
 *
 * 关键点：
 * - 使用独立连接，不依赖/污染全局 orm::DbManager 连接状态
 * - 支持外部传入 DB 文件路径
 * - 默认路径为 {exe}/data/record.db（与 permission.db 同目录）
 */
class ParamChangeDbStore : public IParamChangeStore
{
public:
    /**
     * @brief 构造数据库存储后端
     * @param dbPath SQLite 文件路径（绝对路径）；为空时使用默认路径
     */
    explicit ParamChangeDbStore(const QString &dbPath = QString());
    ~ParamChangeDbStore() override;

    void save(const ParamAuditContext &ctx,
              const QVector<parammeta::ParamChangeRecord> &changes) override;

private:
    bool ensureConnection();
    bool ensureSchema();

    QString m_dbPath;
    QString m_connName;
    bool    m_schemaReady = false;
};

} // namespace aoi
