/**
 * @file themed_tree_widget.cpp
 */

#include "tree/themed_tree_widget.h"

#include "theme/theme_refresh.h"
#include "tree/tree_widget_style.h"

#include "uibase/languagemanager.h"

#include <QEvent>
#include <QPainter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace ui {

namespace {

constexpr int kModuleRole = Qt::UserRole;
constexpr int kItemRole   = Qt::UserRole + 1;

/** 内嵌树：自绘分支箭头，颜色随主题刷新 */
class ThemedTreeView final : public QTreeWidget
{
public:
    explicit ThemedTreeView(QWidget *parent = nullptr)
        : QTreeWidget(parent)
    {
    }

    void setBranchArrowColor(const QColor &color)
    {
        m_branchColor = color;
        viewport()->update();
    }

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override
    {
        if (!index.isValid() || !model()->hasChildren(index))
            return;

        const bool expanded = isExpanded(index);
        const QColor arrow  = m_branchColor.isValid() ? m_branchColor : palette().text().color();

        const qreal cx = rect.left() + rect.width() * 0.5;
        const qreal cy = rect.top() + rect.height() * 0.5;
        const qreal sz = 4.0;

        QPolygonF tri;
        if (expanded) {
            tri << QPointF(cx - sz, cy - sz * 0.35) << QPointF(cx + sz, cy - sz * 0.35)
                << QPointF(cx, cy + sz * 0.75);
        } else {
            tri << QPointF(cx - sz * 0.45, cy - sz) << QPointF(cx - sz * 0.45, cy + sz)
                << QPointF(cx + sz * 0.75, cy);
        }

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(arrow);
        painter->drawPolygon(tri);
        painter->restore();
    }

private:
    QColor m_branchColor;
};

bool isModuleItem(const QTreeWidgetItem *item)
{
    return item && !item->data(0, kItemRole).isValid();
}

} // namespace

ThemedTreeWidget::ThemedTreeWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("themedTreeWidget"));
    buildUi();
    bindTheme();
    applyTheme();
}

void ThemedTreeWidget::buildUi()
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    m_tree = new ThemedTreeView(this);
    m_tree->setObjectName(QStringLiteral("themedTree"));
    m_tree->setHeaderHidden(true);
    m_tree->setRootIsDecorated(true);
    m_tree->setIndentation(16);
    m_tree->setUniformRowHeights(true);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tree->setFocusPolicy(Qt::StrongFocus);
    m_tree->setAttribute(Qt::WA_StyledBackground, true);
    m_tree->setAnimated(true);

    lay->addWidget(m_tree, 1);

    connect(m_tree, &QTreeWidget::currentItemChanged, this,
            [this](QTreeWidgetItem *, QTreeWidgetItem *) { onCurrentItemChanged(); });

    connect(m_tree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        if (!isModuleItem(item))
            return;
        item->setExpanded(!item->isExpanded());
        m_tree->viewport()->update();
    });

    connect(m_tree, &QTreeWidget::itemExpanded, m_tree, [this](QTreeWidgetItem *) {
        m_tree->viewport()->update();
    });
    connect(m_tree, &QTreeWidget::itemCollapsed, m_tree, [this](QTreeWidgetItem *) {
        m_tree->viewport()->update();
    });
}

void ThemedTreeWidget::bindTheme()
{
    bindThemeRefresh(this, [this]() { applyTheme(); });
}

void ThemedTreeWidget::applyTheme()
{
    if (!m_tree)
        return;

    const ThemeTokens t = ThemeTokens::resolve();
    setStyleSheet(buildTreeWidgetStylesheet());

    if (auto *view = static_cast<ThemedTreeView *>(m_tree))
        view->setBranchArrowColor(t.textSecondary);

    QPalette pal = m_tree->palette();
    pal.setColor(QPalette::Base, t.surface);
    pal.setColor(QPalette::Text, t.textPrimary);
    pal.setColor(QPalette::Highlight, t.accentPressed);
    pal.setColor(QPalette::HighlightedText, t.highlightText);
    m_tree->setPalette(pal);
}

void ThemedTreeWidget::setGroupedItems(const QMap<QString, QStringList> &groups)
{
    if (!m_tree)
        return;

    m_tree->clear();
    m_currentModule.clear();
    m_currentItem.clear();

    QFont moduleFont = m_tree->font();
    moduleFont.setBold(true);

    for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
        const QString moduleName = it.key().isEmpty() ? QStringLiteral("默认模组") : it.key();
        auto *moduleItem         = new QTreeWidgetItem(m_tree);
        moduleItem->setText(0, moduleName);
        moduleItem->setData(0, kModuleRole, it.key());
        moduleItem->setFont(0, moduleFont);
        moduleItem->setFlags((moduleItem->flags() | Qt::ItemIsEnabled) & ~Qt::ItemIsSelectable);

        for (const QString &name : it.value()) {
            auto *leaf = new QTreeWidgetItem(moduleItem);
            leaf->setText(0, name);
            leaf->setData(0, kItemRole, name);
            leaf->setData(0, kModuleRole, it.key());
        }
        moduleItem->setExpanded(true);
    }

    if (m_tree->topLevelItemCount() > 0) {
        QTreeWidgetItem *firstLeaf = nullptr;
        for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
            if (auto *mod = m_tree->topLevelItem(i); mod && mod->childCount() > 0) {
                firstLeaf = mod->child(0);
                break;
            }
        }
        if (firstLeaf)
            m_tree->setCurrentItem(firstLeaf);
    }
}

void ThemedTreeWidget::selectItem(const QString &itemName)
{
    if (!m_tree || itemName.isEmpty())
        return;

    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        auto *mod = m_tree->topLevelItem(i);
        if (!mod)
            continue;
        for (int j = 0; j < mod->childCount(); ++j) {
            if (mod->child(j)->data(0, kItemRole).toString() == itemName) {
                mod->setExpanded(true);
                m_tree->setCurrentItem(mod->child(j));
                return;
            }
        }
    }
}

void ThemedTreeWidget::onCurrentItemChanged()
{
    if (!m_tree)
        return;

    auto *item = m_tree->currentItem();
    if (!item || !item->data(0, kItemRole).isValid())
        return;

    m_currentModule = item->data(0, kModuleRole).toString();
    m_currentItem   = item->data(0, kItemRole).toString();
    emit itemSelected(m_currentModule, m_currentItem);
}

void ThemedTreeWidget::retranslateTree()
{
    if (!m_tree)
        return;

    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        auto *mod = m_tree->topLevelItem(i);
        if (!mod)
            continue;
        const QString key = mod->data(0, kModuleRole).toString();
        if (key.isEmpty())
            mod->setText(0, QStringLiteral("默认模组"));
    }
}

void ThemedTreeWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateTree();
}

} // namespace ui
