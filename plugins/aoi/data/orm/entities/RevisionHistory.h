#pragma once

#include <QxOrm.h>
#include <QString>

/**
 * @brief 参数修订历史 ORM 实体（映射 aoi_param_change_log 表）
 *
 * 设计说明：
 * - 该实体只承载“参数改动明细”的持久化字段，不包含业务行为。
 * - 字段命名统一使用 camelCase，具体数据库列名在 register.cpp 中映射。
 * - 本实体可被 DB store 复用，不绑定 AOI 的某个具体页面或 ViewModel。
 */
struct RevisionHistory
{
    long    id = 0;            ///< 自增主键
    QString occurredAt;        ///< 记录时间（ISO8601）
    QString moduleCode;        ///< 模块编码（如 aoi.system_config / aoi.recipe）
    QString documentType;      ///< 文档类型（由模板类型自动推导）
    QString documentId;        ///< 文档 ID（配方等场景使用，可为空）
    QString sourcePage;        ///< 来源页面（UI 侧传入，可为空）
    QString fullLabel;         ///< 完整标签路径（含索引/key）
    QString labelPattern;      ///< 归一化标签路径（[*] / {*}）
    QString beforeValue;       ///< 修改前值（字符串化）
    QString afterValue;        ///< 修改后值（字符串化）
    QString valueType;         ///< 值类型（string/number/bool/object/array...）
    QString detailJson;        ///< 冗余扩展字段（便于后续扩展）
};

QX_REGISTER_HPP(RevisionHistory, qx::trait::no_base_class_defined, 0)

namespace qx {
template <>
void register_class(QxClass<RevisionHistory> &t);
}
