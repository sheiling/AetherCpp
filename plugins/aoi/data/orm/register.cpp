#include "entities/RevisionHistory.h"

// 注册 RevisionHistory 到 QxOrm 反射系统，供 orm::Repository<T> 使用。
QX_REGISTER_CPP(RevisionHistory)

namespace qx {

template <>
void register_class(QxClass<RevisionHistory> &t)
{
    // 表映射：参数修订历史
    t.setName(QStringLiteral("aoi_param_change_log"));
    t.id(&RevisionHistory::id, QStringLiteral("id"));
    t.data(&RevisionHistory::occurredAt, QStringLiteral("occurred_at"));
    t.data(&RevisionHistory::moduleCode, QStringLiteral("module_code"));
    t.data(&RevisionHistory::documentType, QStringLiteral("document_type"));
    t.data(&RevisionHistory::documentId, QStringLiteral("document_id"));
    t.data(&RevisionHistory::sourcePage, QStringLiteral("source_page"));
    t.data(&RevisionHistory::fullLabel, QStringLiteral("full_label"));
    t.data(&RevisionHistory::labelPattern, QStringLiteral("label_pattern"));
    t.data(&RevisionHistory::beforeValue, QStringLiteral("before_value"));
    t.data(&RevisionHistory::afterValue, QStringLiteral("after_value"));
    t.data(&RevisionHistory::valueType, QStringLiteral("value_type"));
    t.data(&RevisionHistory::detailJson, QStringLiteral("detail_json"));
}

} // namespace qx
