#include "ui/view/role_manage/role_permission_page_delegate.h"

#include "ui/view/role_manage/role_permission_list_model.h"
#include "ui/view/permission_ui_utils.h"
#include "ui/view/perm_text.h"

#include "theme/theme_tokens.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace {

constexpr int kCardMarginV     = 6;
constexpr int kCardPad         = 16;
constexpr int kCardRadius      = 8;
constexpr int kNavTitleFontPx  = 16;
constexpr int kSubTitleFontPx  = 15;
constexpr int kControlFontPx   = 12;
constexpr int kNavHeaderRowH   = 32;
constexpr int kSubHeaderRowH   = 30;
constexpr int kTitleVisibleGap = 16;
constexpr int kExpandBtnW      = 24;
constexpr int kNavActionIndent = 8;
constexpr int kSubBlockPad     = 14;
constexpr int kSubTitleGap     = 10;
constexpr int kSubActionPad    = 18;
constexpr int kAccentW         = 3;
constexpr int kCheckboxSize    = 18;
constexpr int kCheckboxGap     = 8;
constexpr int kItemPadH        = 6;
constexpr int kTagH            = 28;
constexpr int kTagPadX         = 12;
constexpr int kTagGapX         = 8;
constexpr int kTagGapY         = 8;
constexpr int kBlockGap        = 4;
constexpr int kDividerGap      = 10;

int labelWidth(const QFontMetrics &fm, const QString &text)
{
    return fm.horizontalAdvance(text) + 2;
}

int checkboxItemWidth(const QFontMetrics &fm, const QString &label)
{
    return kItemPadH + kCheckboxSize + kCheckboxGap + labelWidth(fm, label) + kItemPadH;
}

int actionTagWidth(const QFontMetrics &fm, const QString &label)
{
    return labelWidth(fm, label) + 2 * kTagPadX;
}

QFont titleFontFor(const QFont &base, permission::RolePermissionPageDelegate::TitleStyle style)
{
    QFont f = base;
    f.setPixelSize(style == permission::RolePermissionPageDelegate::TitleStyle::Nav ? kNavTitleFontPx
                                                                                    : kSubTitleFontPx);
    f.setBold(true);
    return f;
}

QFont controlFontFor(const QFont &base)
{
    QFont f = base;
    f.setPixelSize(kControlFontPx);
    f.setBold(false);
    return f;
}

QVector<QPair<QRect, qint64>> layoutActionTagRows(int left, int top, int width, const QFont &font,
                                                  const QVector<permission::PermissionRecord> &actions)
{
    QVector<QPair<QRect, qint64>> out;
    if (actions.isEmpty())
        return out;

    const QFontMetrics fm(font);
    int              x = left;
    int              y = top;

    for (const auto &act : actions) {
        const int w = actionTagWidth(fm, act.displayName);
        if (x > left && x + w > left + width) {
            x = left;
            y += kTagH + kTagGapY;
        }
        out.append({QRect(x, y, w, kTagH), act.id});
        x += w + kTagGapX;
    }
    return out;
}

int actionsAreaHeight(const QVector<QPair<QRect, qint64>> &rects, int fallbackTop)
{
    if (rects.isEmpty())
        return 0;
    int bottom = fallbackTop;
    for (const auto &pair : rects)
        bottom = qMax(bottom, pair.first.bottom());
    return bottom - fallbackTop + kBlockGap;
}

} // namespace

namespace permission {

RolePermissionPageDelegate::RolePermissionPageDelegate(RolePermissionListModel *model,
                                                         QObject *parent)
    : QStyledItemDelegate(parent)
    , m_model(model)
{
}

void RolePermissionPageDelegate::resetNavExpandState() { m_collapsedNavIds.clear(); }

void RolePermissionPageDelegate::setInteractionEnabled(bool enabled)
{
    m_interactionEnabled = enabled;
}

bool RolePermissionPageDelegate::isNavExpanded(qint64 navId) const
{
    return !m_collapsedNavIds.contains(navId);
}

void RolePermissionPageDelegate::toggleNavExpanded(qint64 navId)
{
    if (m_collapsedNavIds.contains(navId))
        m_collapsedNavIds.remove(navId);
    else
        m_collapsedNavIds.insert(navId);
}

void RolePermissionPageDelegate::relayoutListView(const QStyleOptionViewItem &option,
                                                  const QModelIndex          &index) const
{
    if (QWidget *widget = const_cast<QWidget *>(option.widget)) {
        if (auto *view = qobject_cast<QAbstractItemView *>(widget)) {
            view->doItemsLayout();
            view->update(index);
        }
    }
}

int RolePermissionPageDelegate::groupAllButtonWidth(const QFont &font) const
{
    return checkboxItemWidth(QFontMetrics(controlFontFor(font)), PTXT("perm.common.select_all"));
}

RolePermissionPageDelegate::LayoutMetrics RolePermissionPageDelegate::computeLayout(
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    LayoutMetrics metrics;
    if (!m_model || !index.isValid())
        return metrics;

    int viewWidth = option.rect.width();
    if (viewWidth <= 0 && option.widget)
        viewWidth = option.widget->width();

    QRect outer = option.rect;
    if (outer.width() <= 0)
        outer.setWidth(viewWidth);
    outer = outer.adjusted(0, kCardMarginV, 0, -kCardMarginV);

    const RolePermListRow &row = m_model->rowAt(index.row());
    metrics.navExpanded        = isNavExpanded(row.nav.id);

    const QFont navTitleFont = titleFontFor(option.font, TitleStyle::Nav);
    const QFont subTitleFont = titleFontFor(option.font, TitleStyle::SubPage);
    const QFont controlFont   = controlFontFor(option.font);
    QFont       tagMeasureFont = option.font;
    tagMeasureFont.setPixelSize(kControlFontPx);

    const int contentLeft = outer.left() + kCardPad;
    const int contentW    = qMax(0, outer.width() - 2 * kCardPad);
    int       y           = outer.top() + kCardPad;

    const QFontMetrics navTitleFm(navTitleFont);
    const QFontMetrics subTitleFm(subTitleFont);
    const QFontMetrics controlFm(controlFont);
    const int          visibleW = checkboxItemWidth(controlFm, PTXT("perm.roles.visible"));
    const int          groupW   = groupAllButtonWidth(controlFont);

    metrics.navExpandRect = QRect(contentLeft, y, kExpandBtnW, kNavHeaderRowH);
    metrics.navTitleLeft  = contentLeft + kExpandBtnW + 8;

    int visibleX =
        metrics.navTitleLeft + labelWidth(navTitleFm, row.nav.displayName) + kTitleVisibleGap;
    metrics.navVisibleRect = QRect(visibleX, y, visibleW, kNavHeaderRowH);
    metrics.navGroupAllRect =
        QRect(metrics.navVisibleRect.right() + kTitleVisibleGap, y, groupW, kNavHeaderRowH);

    y += kNavHeaderRowH + kBlockGap;

    if (!metrics.navExpanded) {
        y += kCardPad;
        metrics.cardRect    = QRect(outer.left(), outer.top(), outer.width(), y - outer.top());
        metrics.totalHeight = metrics.cardRect.height();
        return metrics;
    }

    if (!row.navActions.isEmpty()) {
        const int actionTop = y;
        metrics.navActionRects = layoutActionTagRows(contentLeft + kNavActionIndent, y,
                                                     contentW - kNavActionIndent, tagMeasureFont,
                                                     row.navActions);
        y = actionTop + actionsAreaHeight(metrics.navActionRects, actionTop) + kBlockGap;
    }

    for (const auto &block : row.subPages) {
        y += kDividerGap;
        y += 1 + kDividerGap;

        const int blockTop      = y;
        const int blockLineLeft = contentLeft + kSubBlockPad;
        const int subTitleLeft  = blockLineLeft + kAccentW + kSubTitleGap;
        const int tagLeft       = subTitleLeft + kSubActionPad;
        const int tagAreaWidth  = qMax(0, contentLeft + contentW - tagLeft);

        SubBlockLayout subLayout;
        subLayout.subPageId    = block.subPage.id;
        subLayout.subTitleLeft = subTitleLeft;

        const int subVisX =
            subTitleLeft + labelWidth(subTitleFm, block.subPage.displayName) + kTitleVisibleGap;
        subLayout.visibleRect = QRect(subVisX, y, visibleW, kSubHeaderRowH);
        subLayout.groupAllRect =
            QRect(subLayout.visibleRect.right() + kTitleVisibleGap, y, groupW, kSubHeaderRowH);

        y += kSubHeaderRowH + 6;

        const int actionTop = y;
        subLayout.actionRects =
            layoutActionTagRows(tagLeft, y, tagAreaWidth, tagMeasureFont, block.actions);

        int actionBottom = actionTop;
        if (!subLayout.actionRects.isEmpty()) {
            for (const auto &pair : subLayout.actionRects)
                actionBottom = qMax(actionBottom, pair.first.bottom());
        } else {
            actionBottom = actionTop + kTagH - 1;
        }

        subLayout.accentRect = QRect(blockLineLeft, blockTop, kAccentW,
                                     qMax(kSubHeaderRowH, actionBottom - blockTop + 4));
        y = actionBottom + kBlockGap + kDividerGap;
        metrics.subBlocks.append(subLayout);
    }

    y += kCardPad;
    metrics.cardRect    = QRect(outer.left(), outer.top(), outer.width(), y - outer.top());
    metrics.totalHeight = metrics.cardRect.height();
    return metrics;
}

QSize RolePermissionPageDelegate::sizeHint(const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    const LayoutMetrics layout = computeLayout(option, index);
    int width = option.rect.width();
    if (width <= 0 && option.widget)
        width = option.widget->width();
    if (width <= 0)
        width = 320;
    return QSize(width, layout.totalHeight + 2 * kCardMarginV);
}

void RolePermissionPageDelegate::paintCheckboxItem(QPainter *painter, const QRect &itemRect,
                                                   bool checked, bool enabled,
                                                   const QString &label) const
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    painter->save();
    painter->setOpacity(enabled ? 1.0 : 0.45);

    const int boxLeft = itemRect.left() + kItemPadH;
    QRect     box(boxLeft, itemRect.center().y() - kCheckboxSize / 2, kCheckboxSize, kCheckboxSize);
    if (enabled)
        permission_ui::paintPermissionCheckbox(painter, box, checked, t);
    else {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setBrush(t.inputBackground);
        painter->setPen(QPen(t.inputBorder, 1));
        painter->drawRoundedRect(box, 3, 3);
    }

    QFont f = controlFontFor(painter->font());
    painter->setFont(f);
    painter->setPen(enabled ? t.textSecondary : t.mutedText);
    const int textLeft = box.right() + kCheckboxGap;
    painter->drawText(QRect(textLeft, itemRect.top(), itemRect.right() - textLeft, itemRect.height()),
                      Qt::AlignLeft | Qt::AlignVCenter, label);
    painter->restore();
}

void RolePermissionPageDelegate::paintTitleWithVisible(
    QPainter *painter, const QRect &lineRect, const QString &title, TitleStyle titleStyle,
    const QRect &visibleRect, bool visibleChecked, bool visibleEnabled) const
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    painter->save();
    QFont tf = titleFontFor(painter->font(), titleStyle);
    painter->setFont(tf);
    painter->setPen(titleStyle == TitleStyle::Nav ? t.highlightText : t.textPrimary);
    painter->drawText(
        QRect(lineRect.left(), lineRect.top(), visibleRect.left() - lineRect.left() - 4,
              lineRect.height()),
        Qt::AlignLeft | Qt::AlignVCenter, title);
    paintCheckboxItem(painter, visibleRect, visibleChecked, visibleEnabled, PTXT("perm.roles.visible"));
    painter->restore();
}

void RolePermissionPageDelegate::paintExpandToggle(QPainter *painter, const QRect &rect,
                                                 bool expanded) const
{
    permission_ui::paintPermissionExpandToggle(painter, rect, expanded, ui::ThemeTokens::resolve());
}

void RolePermissionPageDelegate::paintNavHeaderRow(QPainter *painter,
                                                   const QStyleOptionViewItem &option,
                                                   const RolePermListRow &row,
                                                   const LayoutMetrics &layout, bool navGranted,
                                                   bool navAllOn) const
{
    const QRect navLine(layout.navTitleLeft, layout.navExpandRect.top(),
                        layout.navGroupAllRect.right() - layout.navTitleLeft + 1, kNavHeaderRowH);
    paintExpandToggle(painter, layout.navExpandRect, layout.navExpanded);
    paintTitleWithVisible(painter, navLine, row.nav.displayName, TitleStyle::Nav,
                          layout.navVisibleRect, navGranted, m_interactionEnabled);
    paintCheckboxItem(painter, layout.navGroupAllRect, navAllOn, m_interactionEnabled,
                      PTXT("perm.common.select_all"));
}

void RolePermissionPageDelegate::paintSectionAccent(QPainter *painter,
                                                    const QRect &accentRect) const
{
    if (accentRect.height() <= 0)
        return;

    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(Qt::NoPen);
    painter->setBrush(t.accent);
    painter->drawRect(accentRect);
    painter->restore();
}

void RolePermissionPageDelegate::paintActionTags(
    QPainter *painter, const QVector<QPair<QRect, qint64>> &tagRects,
    const QVector<PermissionRecord> &actions, bool parentEnabled) const
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    for (const auto &pair : tagRects) {
        QString label;
        for (const auto &act : actions) {
            if (act.id == pair.second) {
                label = act.displayName;
                break;
            }
        }
        if (label.isEmpty())
            continue;
        const bool selected = m_model->isGranted(pair.second);
        permission_ui::paintPermissionActionTag(painter, pair.first, label, selected,
                                                parentEnabled, t);
    }
}

void RolePermissionPageDelegate::paintNavCard(QPainter *painter,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index,
                                              const LayoutMetrics &layout) const
{
    if (!m_model)
        return;

    const RolePermListRow &row       = m_model->rowAt(index.row());
    const bool             navGranted = m_model->isNavGranted(row.nav.id);
    const bool             navAllOn   = m_model->isNavSubtreeFullyGranted(row.nav.id);

    const ui::ThemeTokens t = ui::ThemeTokens::resolve();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    path.addRoundedRect(layout.cardRect, kCardRadius, kCardRadius);
    painter->fillPath(path, t.surfaceElevated);
    painter->setPen(QPen(t.surfaceBorder, 1));
    painter->drawPath(path);

    paintNavHeaderRow(painter, option, row, layout, navGranted, navAllOn);

    if (!layout.navExpanded)
    {
        painter->restore();
        return;
    }

    if (!row.navActions.isEmpty())
        paintActionTags(painter, layout.navActionRects, row.navActions,
                        navGranted && m_interactionEnabled);

    for (int i = 0; i < layout.subBlocks.size(); ++i) {
        const auto &subLayout  = layout.subBlocks.at(i);
        const auto &block      = row.subPages.at(i);
        const bool  subGranted = m_model->isSubPageGranted(block.subPage.id);
        const bool  subAllOn    = m_model->isSubPageFullyGranted(block.subPage.id);
        const bool  subEnabled  = navGranted && m_interactionEnabled;

        const int y = subLayout.visibleRect.top();

        QPen dividerPen(t.surfaceBorder, 1);
        painter->setPen(dividerPen);
        painter->drawLine(layout.cardRect.left() + kCardPad, y - kDividerGap - 1,
                          layout.cardRect.right() - kCardPad, y - kDividerGap - 1);

        const QRect subLine(subLayout.subTitleLeft, y,
                            subLayout.groupAllRect.right() - subLayout.subTitleLeft + 1,
                            kSubHeaderRowH);
        paintTitleWithVisible(painter, subLine, block.subPage.displayName, TitleStyle::SubPage,
                              subLayout.visibleRect, subGranted, subEnabled);
        paintCheckboxItem(painter, subLayout.groupAllRect, subAllOn, subEnabled, PTXT("perm.common.select_all"));

        paintSectionAccent(painter, subLayout.accentRect);
        paintActionTags(painter, subLayout.actionRects, block.actions, subEnabled && subGranted);
    }

    painter->restore();
}

void RolePermissionPageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    if (!m_model || !index.isValid())
        return;

    const LayoutMetrics layout = computeLayout(option, index);
    paintNavCard(painter, option, index, layout);
}

RolePermissionPageDelegate::HitRegion RolePermissionPageDelegate::hitTest(
    const LayoutMetrics &layout, const QPoint &pos) const
{
    HitRegion hit;
    if (layout.navExpandRect.contains(pos)) {
        hit.target = HitTarget::NavExpand;
        hit.rect   = layout.navExpandRect;
        return hit;
    }
    if (layout.navGroupAllRect.contains(pos)) {
        hit.target = HitTarget::NavGroupAll;
        hit.rect   = layout.navGroupAllRect;
        return hit;
    }
    if (layout.navVisibleRect.contains(pos)) {
        hit.target = HitTarget::NavVisible;
        hit.rect   = layout.navVisibleRect;
        return hit;
    }
    if (!layout.navExpanded)
        return hit;

    for (const auto &pair : layout.navActionRects) {
        if (pair.first.contains(pos)) {
            hit.target     = HitTarget::NavAction;
            hit.resourceId = pair.second;
            hit.rect       = pair.first;
            return hit;
        }
    }
    for (const auto &sub : layout.subBlocks) {
        if (sub.visibleRect.contains(pos)) {
            hit.target     = HitTarget::SubVisible;
            hit.resourceId = sub.subPageId;
            hit.rect       = sub.visibleRect;
            return hit;
        }
        if (sub.groupAllRect.contains(pos)) {
            hit.target     = HitTarget::SubGroupAll;
            hit.resourceId = sub.subPageId;
            hit.rect       = sub.groupAllRect;
            return hit;
        }
        for (const auto &pair : sub.actionRects) {
            if (pair.first.contains(pos)) {
                hit.target     = HitTarget::SubAction;
                hit.resourceId = pair.second;
                hit.rect       = pair.first;
                return hit;
            }
        }
    }
    return hit;
}

bool RolePermissionPageDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index)
{
    Q_UNUSED(model);

    if (!m_model || !index.isValid())
        return false;

    auto *mouseEvent = dynamic_cast<QMouseEvent *>(event);
    if (!mouseEvent || mouseEvent->button() != Qt::LeftButton)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    if (event->type() != QEvent::MouseButtonRelease)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    const LayoutMetrics layout = computeLayout(option, index);
    const HitRegion     hit    = hitTest(layout, mouseEvent->pos());
    if (hit.target == HitTarget::None)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    if (!m_interactionEnabled && hit.target != HitTarget::NavExpand)
        return true;

    const RolePermListRow &row = m_model->rowAt(index.row());

    switch (hit.target) {
    case HitTarget::NavExpand:
        toggleNavExpanded(row.nav.id);
        relayoutListView(option, index);
        return true;
    case HitTarget::NavGroupAll:
        m_model->setNavSelectAll(row.nav.id, !m_model->isNavSubtreeFullyGranted(row.nav.id));
        break;
    case HitTarget::NavVisible:
        m_model->setNavGranted(row.nav.id, !m_model->isNavGranted(row.nav.id));
        break;
    case HitTarget::NavAction:
        if (!m_model->isNavGranted(row.nav.id))
            return true;
        m_model->setActionGranted(hit.resourceId, !m_model->isGranted(hit.resourceId));
        break;
    case HitTarget::SubVisible:
        if (!m_model->isNavGranted(row.nav.id))
            return true;
        m_model->setSubPageGranted(hit.resourceId, !m_model->isSubPageGranted(hit.resourceId));
        break;
    case HitTarget::SubGroupAll:
        if (!m_model->isNavGranted(row.nav.id))
            return true;
        m_model->setSubPageSelectAll(hit.resourceId,
                                     !m_model->isSubPageFullyGranted(hit.resourceId));
        break;
    case HitTarget::SubAction: {
        const RolePermSubPageBlock *block = nullptr;
        for (const auto &b : row.subPages) {
            for (const auto &act : b.actions) {
                if (act.id == hit.resourceId) {
                    block = &b;
                    break;
                }
            }
            if (block)
                break;
        }
        if (!block || !m_model->isNavGranted(row.nav.id)
            || !m_model->isSubPageGranted(block->subPage.id))
            return true;
        m_model->setActionGranted(hit.resourceId, !m_model->isGranted(hit.resourceId));
        break;
    }
    default:
        break;
    }

    if (QWidget *widget = const_cast<QWidget *>(option.widget)) {
        if (auto *view = qobject_cast<QAbstractItemView *>(widget))
            view->viewport()->update();
    }
    return true;
}

} // namespace permission
