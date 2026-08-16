#include "ui/view/audit_log/audit_log_table_model.h"

#include "model/audit_log_detail_text.h"
#include "ui/view/perm_text.h"

#include <QColor>
#include <QFont>

namespace permission {
namespace {

QColor resultForeground(const QString &result)
{
    const QString key = result.trimmed().toLower();
    if (key == QStringLiteral("success"))
        return QColor(QStringLiteral("#52c41a"));
    if (key == QStringLiteral("denied"))
        return QColor(QStringLiteral("#fa8c16"));
    return {};
}

} // namespace

AuditLogTableModel::AuditLogTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int AuditLogTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

int AuditLogTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ColCount;
}

QVariant AuditLogTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};

    const AuditLogListItem &item = m_items.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColTime:
            return item.occurredAt.isValid()
                       ? item.occurredAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
                       : QStringLiteral("—");
        case ColUser:
            return item.username;
        case ColRoles:
            return item.roleNames;
        case ColActionType:
            return item.actionType;
        case ColActionName:
            return item.actionName;
        case ColTarget:
            return item.target;
        case ColResult:
            return item.result;
        case ColDetail:
            return auditDetailSummary(item);
        default:
            break;
        }
    }

    if (role == Qt::ToolTipRole && index.column() == ColDetail) {
        const QString summary = auditDetailSummary(item, 200);
        const QString full    = auditDetailFullText(item);
        if (full != summary && !full.isEmpty())
            return full;
    }

    if (role == Qt::ForegroundRole && index.column() == ColResult) {
        const QColor c = resultForeground(item.result);
        if (c.isValid())
            return c;
    }

    if (role == Qt::FontRole && index.column() == ColResult) {
        const QString key = item.result.trimmed().toLower();
        if (key == QStringLiteral("success") || key == QStringLiteral("denied")) {
            QFont font;
            font.setBold(true);
            return font;
        }
    }

    if (role == Qt::UserRole)
        return item.id;

    return {};
}

QVariant AuditLogTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch (section) {
    case ColTime:
        return PTXT("perm.audit.col_time");
    case ColUser:
        return PTXT("perm.audit.col_user");
    case ColRoles:
        return PTXT("perm.users.col_role");
    case ColActionType:
        return PTXT("perm.audit.col_action_type");
    case ColActionName:
        return PTXT("perm.audit.col_action_name");
    case ColTarget:
        return PTXT("perm.audit.col_target");
    case ColResult:
        return PTXT("perm.audit.col_result");
    case ColDetail:
        return PTXT("perm.audit.col_details");
    default:
        return {};
    }
}

Qt::ItemFlags AuditLogTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void AuditLogTableModel::setItems(const QVector<AuditLogListItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

qint64 AuditLogTableModel::logIdAt(int row) const
{
    if (row < 0 || row >= m_items.size())
        return 0;
    return m_items.at(row).id;
}

} // namespace permission
