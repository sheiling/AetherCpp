#pragma once

#include "service/param_change/param_change_store.h"

namespace aoi {

/**
 * @brief 参数变更文本存储后端
 *
 * 用途：
 * - 兼容历史“文本审计”模式
 * - 作为 Dual 双写中的一个后端
 * - 便于现场快速查看最近变更
 */
class ParamChangeFileStore : public IParamChangeStore
{
public:
    /**
     * @brief 构造文本存储后端
     * @param logDir 日志目录（绝对路径）；为空时使用默认目录
     */
    explicit ParamChangeFileStore(const QString &logDir = QString());

    void save(const ParamAuditContext &ctx,
              const QVector<parammeta::ParamChangeRecord> &changes) override;

private:
    QString m_logDir;
};

} // namespace aoi
