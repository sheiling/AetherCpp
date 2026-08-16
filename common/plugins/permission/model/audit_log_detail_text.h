#pragma once

#include "model/audit_log_management_model.h"

namespace permission {

/// 表格「详情」列摘要（可截断）
QString auditDetailSummary(const AuditLogListItem &item, int maxChars = 96);

/// 审计详情弹窗全文（结构化 + 原始 JSON）
QString auditDetailFullText(const AuditLogListItem &item);

} // namespace permission
