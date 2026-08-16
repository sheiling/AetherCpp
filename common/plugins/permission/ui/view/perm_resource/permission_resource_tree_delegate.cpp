#include "ui/view/perm_resource/permission_resource_tree_delegate.h"

#include "data/domain/domain_types.h"
#include "ui/view/permission_ui_utils.h"

#include "theme/theme_color_math.h"
#include "theme/theme_tokens.h"

#include <QAbstractItemView>
#include <QFontMetrics>
#include <QPainter>
#include <QTreeView>

namespace {

constexpr int kRowHeight    = 32;
constexpr int kExpandBtnW   = PermissionResourceTreeDelegate::kExpandToggleW;
constexpr int kExpandGap    = 8;
constexpr int kTextGap      = 10;
constexpr int kPadRight     = 8;
constexpr int kSelectRadius = 4;

} // namespace

PermissionResourceTreeDelegate::PermissionResourceTreeDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QRect PermissionResourceTreeDelegate::expandToggleRect(const QRect &visualRect, bool hasChildren)
{
    if (!hasChildren)
        return QRect();

    const QRect rowRect = visualRect.adjusted(0, 1, -2, -1);
    const int   x       = rowRect.left() + 4;
    return QRect(x, rowRect.top(), kExpandBtnW, rowRect.height());
}

void PermissionResourceTreeDelegate::applyTreeStyle(QTreeView *tree)
{
    if (!tree)
        return;

    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    tree->setObjectName(QStringLiteral("permResourceTree"));
    tree->setIndentation(24);
    tree->setRootIsDecorated(false);
    tree->setStyleSheet(QStringLiteral(
        "QTreeView#permResourceTree{background:transparent;border:none;outline:none;}"
        "QTreeView#permResourceTree::item{border:none;padding:0;margin:0;}"
        "QTreeView#permResourceTree::item:selected{background:transparent;color:%1;}"
        "QTreeView#permResourceTree::item:hover{background:transparent;}"
        "QTreeView#permResourceTree::branch{background:transparent;border:none;}"
        "QTreeView#permResourceTree::branch:has-siblings:!adjoins-item{"
        "border-image:none;image:none;}"
        "QTreeView#permResourceTree::branch:has-siblings:adjoins-item{"
        "border-image:none;image:none;}"
        "QTreeView#permResourceTree::branch:!has-children{"
        "border-image:none;image:none;}"
        "QTreeView#permResourceTree::branch:has-children:!has-siblings{"
        "border-image:none;image:none;}"
        "QTreeView#permResourceTree::branch:closed:has-children:has-siblings{"
        "border-image:none;image:none;}"
        "QTreeView#permResourceTree::branch:open:has-children:has-siblings{"
        "border-image:none;image:none;}"
        "QTreeView#permResourceTree::branch:closed:has-children{"
        "border-image:none;image:none;}"
        "QTreeView#permResourceTree::branch:open:has-children{"
        "border-image:none;image:none;}")
                            .arg(t.textPrimary.name()));
}

void PermissionResourceTreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    const QString displayName = index.data(Qt::DisplayRole).toString();
    const QString key         = index.data(ResourceKeyRole).toString();
    const bool hasChildren = index.data(HasChildrenRole).toBool();
    const bool enabled =
        index.data(ResourceEnabledRole).isValid()
            ? index.data(ResourceEnabledRole).toBool()
            : true;

    const bool selected = option.state.testFlag(QStyle::State_Selected);
    const bool expanded = option.state.testFlag(QStyle::State_Open);

    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QRect rowRect = option.rect.adjusted(0, 1, -2, -1);
    if (selected) {
        QPainterPath bg;
        bg.addRoundedRect(rowRect, kSelectRadius, kSelectRadius);
        painter->fillPath(bg, ui::themeWithAlpha(t.accent, 70));
    }

    int x = rowRect.left() + 4;

    if (hasChildren) {
        const QRect expandRect = expandToggleRect(option.rect, true);
        permission_ui::paintPermissionExpandToggle(painter, expandRect, expanded, t);
        x += kExpandBtnW + kExpandGap;
    }

    QFont primaryFont = option.font;
    primaryFont.setPixelSize(13);
    QFont secondaryFont = primaryFont;
    secondaryFont.setPixelSize(12);

    const QColor primaryColor = enabled ? t.textPrimary : t.mutedText;
    const QColor secondaryColor = t.textSecondary;

    const QString leftText  = displayName;
    const QString rightText = key;

    painter->setFont(primaryFont);
    const int availW = rowRect.right() - x - kPadRight;
    const QFontMetrics fmPrimary(primaryFont);
    const int leftW = qMin(fmPrimary.horizontalAdvance(leftText), availW);

    painter->setPen(primaryColor);
    painter->drawText(x, rowRect.top(), leftW, rowRect.height(), Qt::AlignLeft | Qt::AlignVCenter,
                      fmPrimary.elidedText(leftText, Qt::ElideRight, availW));

    if (!rightText.isEmpty()) {
        painter->setFont(secondaryFont);
        const QFontMetrics fmSecondary(secondaryFont);
        const int rightX = x + leftW + (leftW > 0 ? kTextGap : 0);
        const int rightAvail = rowRect.right() - rightX - kPadRight;
        if (rightAvail > 20) {
            painter->setPen(secondaryColor);
            painter->drawText(rightX, rowRect.top(), rightAvail, rowRect.height(),
                              Qt::AlignLeft | Qt::AlignVCenter,
                              fmSecondary.elidedText(rightText, Qt::ElideRight, rightAvail));
        }
    }

    painter->restore();
}

QSize PermissionResourceTreeDelegate::sizeHint(const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(0, kRowHeight);
}
