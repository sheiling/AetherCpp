#pragma once

#include "data/domain/domain_types.h"

#include <QHash>
#include <QSet>
#include <QString>
#include <QVector>

namespace permission {

/// 角色授权归一化结果（阶段 4 / plan 1.3）
struct GrantNormalizeResult {
    QVector<qint64> normalized; ///< 可落库的 grant id 集合（有序稳定：nav → sub_page → action）
    QVector<qint64> removedIds; ///< 从输入中剔除的 id
};

/// 授权 diff（审计日志用）
struct GrantPermissionDiff {
    QVector<qint64> added;
    QVector<qint64> removed;
};

class PermissionGrantService
{
public:
    /// 按 resource_type 父子链与 enabled 规则归一化勾选集合
    static GrantNormalizeResult normalize(const QVector<PermissionRecord> &all,
                                          const QVector<qint64>           &selectedIds);

    static GrantPermissionDiff diff(const QVector<qint64> &before,
                                    const QVector<qint64> &after);

    /// 生成审计 detail_json
    static QString auditDetailJson(qint64 roleId, const QString &roleName,
                                   const GrantPermissionDiff &diff, int normalizedCount);
};

} // namespace permission
