#pragma once

#include "model/audit_log_management_model.h"

#include <QAbstractTableModel>
#include <QVector>

namespace permission {

class AuditLogTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColTime = 0,
        ColUser,
        ColRoles,
        ColActionType,
        ColActionName,
        ColTarget,
        ColResult,
        ColDetail,
        ColCount
    };

    explicit AuditLogTableModel(QObject *parent = nullptr);

    int           rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int           columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant      data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant      headerData(int section, Qt::Orientation orientation,
                           int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setItems(const QVector<AuditLogListItem> &items);
    qint64 logIdAt(int row) const;

private:
    QVector<AuditLogListItem> m_items;
};

} // namespace permission
