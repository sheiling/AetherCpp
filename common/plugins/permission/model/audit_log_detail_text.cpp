#include "model/audit_log_detail_text.h"

#include "perm_text.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace permission {
namespace {

QString joinIdList(const QJsonArray &arr)
{
    QStringList parts;
    for (const auto &v : arr) {
        if (v.isDouble())
            parts << QString::number(static_cast<qint64>(v.toDouble()));
        else
            parts << v.toString();
    }
    return parts.join(QStringLiteral(", "));
}

QString formatJsonDetail(const QString &actionType, const QString &detailJson)
{
    if (detailJson.trimmed().isEmpty())
        return {};

    const QJsonDocument doc = QJsonDocument::fromJson(detailJson.toUtf8());
    if (!doc.isObject())
        return detailJson;

    const QJsonObject obj = doc.object();

    if (actionType == QStringLiteral("role_permission")) {
        QStringList lines;
        const QString roleName = obj.value(QStringLiteral("roleName")).toString();
        if (!roleName.isEmpty())
            lines << PTXT("perm.audit.field_role").arg(roleName);
        if (obj.contains(QStringLiteral("roleId")))
            lines << PTXT("perm.audit.field_role_id")
                         .arg(obj.value(QStringLiteral("roleId")).toVariant().toString());
        if (obj.contains(QStringLiteral("normalizedCount")))
            lines << PTXT("perm.audit.field_normalized_count")
                         .arg(obj.value(QStringLiteral("normalizedCount")).toInt());
        const QJsonArray added   = obj.value(QStringLiteral("added")).toArray();
        const QJsonArray removed = obj.value(QStringLiteral("removed")).toArray();
        if (!added.isEmpty())
            lines << PTXT("perm.audit.field_added_ids").arg(joinIdList(added));
        if (!removed.isEmpty())
            lines << PTXT("perm.audit.field_removed_ids").arg(joinIdList(removed));
        if (!lines.isEmpty())
            return lines.join(QStringLiteral("\n"));
    }

    QStringList lines;
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        const QJsonValue v = it.value();
        if (v.isArray())
            lines << QStringLiteral("%1: [%2]").arg(it.key(), joinIdList(v.toArray()));
        else if (v.isObject())
            lines << QStringLiteral("%1: %2").arg(it.key(),
                                                    QString::fromUtf8(
                                                        QJsonDocument(v.toObject()).toJson(
                                                            QJsonDocument::Compact)));
        else
            lines << QStringLiteral("%1: %2").arg(it.key(), v.toVariant().toString());
    }
    return lines.join(QStringLiteral("\n"));
}

QString truncateText(const QString &text, int maxChars)
{
    if (maxChars <= 0 || text.size() <= maxChars)
        return text;
    return text.left(maxChars - 1) + QChar(0x2026);
}

} // namespace

QString auditDetailSummary(const AuditLogListItem &item, int maxChars)
{
    const QString formatted = formatJsonDetail(item.actionType, item.detailJson);
    if (!formatted.isEmpty()) {
        QString oneLine = formatted;
        oneLine.replace(QLatin1Char('\n'), QStringLiteral("；"));
        return truncateText(oneLine, maxChars);
    }

    if (!item.target.isEmpty() && item.target != QStringLiteral("—"))
        return truncateText(item.target, maxChars);

    if (!item.actionName.isEmpty())
        return truncateText(item.actionName, maxChars);

    if (!item.ip.isEmpty())
        return truncateText(PTXT("perm.audit.field_ip").arg(item.ip), maxChars);

    return QStringLiteral("—");
}

QString auditDetailFullText(const AuditLogListItem &item)
{
    QStringList lines;
    lines << PTXT("perm.audit.field_time")
                 .arg(item.occurredAt.isValid()
                          ? item.occurredAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
                          : QStringLiteral("—"));
    lines << PTXT("perm.audit.field_user")
                 .arg(item.username.isEmpty() ? QStringLiteral("—") : item.username);
    if (!item.roleNames.isEmpty())
        lines << PTXT("perm.audit.field_role").arg(item.roleNames);
    lines << PTXT("perm.audit.field_action_type").arg(item.actionType);
    lines << PTXT("perm.audit.field_action_name").arg(item.actionName);
    lines << PTXT("perm.audit.field_target")
                 .arg(item.target.isEmpty() ? QStringLiteral("—") : item.target);
    lines << PTXT("perm.audit.field_result").arg(item.result);
    if (!item.ip.isEmpty())
        lines << PTXT("perm.audit.field_ip").arg(item.ip);
    if (!item.moduleCode.isEmpty())
        lines << PTXT("perm.audit.field_module").arg(item.moduleCode);

    const QString body = formatJsonDetail(item.actionType, item.detailJson);
    if (!body.isEmpty()) {
        lines << QString();
        lines << PTXT("perm.audit.field_details");
        lines << body;
    } else if (!item.detailJson.trimmed().isEmpty()) {
        lines << QString();
        lines << PTXT("perm.audit.field_details");
        lines << item.detailJson;
    } else if (!item.target.isEmpty() && item.target != QStringLiteral("—")) {
        lines << QString();
        lines << PTXT("perm.audit.field_details");
        lines << item.target;
    }

    if (!item.detailJson.trimmed().isEmpty()) {
        lines << QString();
        lines << PTXT("perm.audit.field_raw_data");
        const QJsonDocument doc = QJsonDocument::fromJson(item.detailJson.toUtf8());
        if (doc.isObject() || doc.isArray())
            lines << QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
        else
            lines << item.detailJson;
    }

    return lines.join(QStringLiteral("\n"));
}

} // namespace permission
