#pragma once

#include <QStyledItemDelegate>

namespace permission {

/// 「结果」列：success / denied 等状态使用不同颜色与字重
class AuditLogResultDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit AuditLogResultDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

} // namespace permission
