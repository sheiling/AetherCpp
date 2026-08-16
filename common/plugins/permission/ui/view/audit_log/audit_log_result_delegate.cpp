#include "ui/view/audit_log/audit_log_result_delegate.h"

#include "table/table_style.h"
#include "theme/theme_tokens.h"

#include <QApplication>
#include <QColor>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>

namespace permission {
namespace {

QColor resultColor(const QString &result)
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    const QString         key = result.trimmed().toLower();
    if (key == QStringLiteral("success"))
        return t.successText;
    if (key == QStringLiteral("denied"))
        return t.warningText;
    return t.textPrimary;
}

bool resultBold(const QString &result)
{
    const QString key = result.trimmed().toLower();
    return key == QStringLiteral("success") || key == QStringLiteral("denied");
}

} // namespace

AuditLogResultDelegate::AuditLogResultDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void AuditLogResultDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QWidget *widget = opt.widget;
    QStyle          *style = widget ? widget->style() : QApplication::style();

    painter->save();

    // 仅绘制背景/选中态；文字不用 QStyledItemDelegate，避免全局 QSS 覆盖 palette 颜色
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

    const QString text = index.data(Qt::DisplayRole).toString();
    QFont         font = opt.font;
    if (resultBold(text))
        font.setBold(true);

    painter->setFont(font);
    painter->setPen(resultColor(text));

    const QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
    painter->drawText(textRect, static_cast<int>(opt.displayAlignment), text);

    painter->restore();
}

} // namespace permission
