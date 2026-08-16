/**
 * @file recipe_list_panel.cpp
 * @brief RecipeListPanel 布局、对话框确认与列表行 widget 构建
 */

#include "list/recipe_list_panel.h"

#include "component/list/themed_list_widget.h"
#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_push_button.h"
#include "component/controls/input/themed_line_edit.h"
#include "component/controls/label/themed_form_label.h"
#include "component/dialog/themed_message_box.h"
#include "component/dialog/themed_panel_dialog.h"

#include "component/theme/theme_font.h"

#include "uibase/languagemanager.h"

#include <QCoreApplication>
#include <QEvent>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

QLabel *makeSectionTitle(const char *key, QWidget *parent)
{
    auto *lbl = new QLabel(TR(key), parent);
    lbl->setObjectName(QStringLiteral("recipeSectionTitle"));
    lbl->setProperty("trKey", QString::fromUtf8(key));
    return lbl;
}

} // namespace

RecipeListPanel::RecipeListPanel(QWidget *parent)
    : QWidget(parent)
{
    buildLayout();
    connectActions();
}

void RecipeListPanel::setRecipeDirectory(const QString &dir)
{
    m_recipeDirectory = dir;
    RecipeRepository::setStorageDirectory(dir);
}

void RecipeListPanel::buildLayout()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_listGroup = new QFrame(this);
    m_listGroup->setObjectName(QStringLiteral("recipeListGroup"));
    m_listGroup->setAttribute(Qt::WA_StyledBackground, true);
    m_listGroup->setFixedWidth(250);

    auto *inner = new QVBoxLayout(m_listGroup);
    inner->setContentsMargins(12, 12, 12, 12);
    inner->setSpacing(8);

    inner->addWidget(makeSectionTitle("aoi.recipe.list_title", m_listGroup));

    auto *actionsShell = new QWidget(m_listGroup);
    actionsShell->setObjectName(QStringLiteral("recipeListActions"));
    auto *actionsGrid  = new QGridLayout(actionsShell);
    actionsGrid->setContentsMargins(0, 0, 0, 0);
    actionsGrid->setHorizontalSpacing(4);
    actionsGrid->setVerticalSpacing(4);

    m_actionButtons.clear();
    for (int i = 0; i < 6; ++i) {
        const QString key = QString::fromUtf8(kListActionKeys[i]);
        const bool    isRemove =
            key == QStringLiteral("aoi.recipe.action.remove");
        const PushButtonVariant variant =
            isRemove ? ui::PushButtonVariant::Danger : ui::PushButtonVariant::Secondary;
        auto *btn = ui::makePushButton(TR(key.toUtf8().constData()), variant, actionsShell);
        btn->setProperty("trKey", key);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_actionButtons.append(btn);
        const int row = i < 3 ? 0 : 1;
        const int col = i < 3 ? i : i - 3;
        actionsGrid->addWidget(btn, row, col);
    }
    inner->addWidget(actionsShell);

    m_searchEdit = ui::makeLineEdit(m_listGroup);
    m_searchEdit->setObjectName(QStringLiteral("recipeListSearch"));
    inner->addWidget(m_searchEdit);

    m_recipeList = ui::makeListWidget(m_listGroup);
    m_recipeList->setItemWidgetRows(true);
    m_recipeList->setContextMenuPolicy(Qt::CustomContextMenu);
    inner->addWidget(m_recipeList, 1);

    layout->addWidget(m_listGroup);
}

/** 搜索、六按钮、列表选中/右键重命名、文件导入导出对话框 */
void RecipeListPanel::connectActions()
{
    connect(m_searchEdit, &ui::ThemedLineEdit::textChanged, this,
            &RecipeListPanel::searchKeywordChanged);

    if (m_actionButtons.size() >= 6) {
        connect(m_actionButtons[0], &ui::ThemedPushButton::clicked, this, [this]() {
            const QString name = promptRecipeName(QStringLiteral("aoi.recipe.add_dialog.title"),
                                                  suggestNewRecipeName());
            if (!name.isEmpty())
                emit addRecipeRequested(name);
        });
        connect(m_actionButtons[1], &ui::ThemedPushButton::clicked, this, [this]() {
            if (confirmRemoveRecipe())
                emit removeRecipeRequested();
        });
        connect(m_actionButtons[2], &ui::ThemedPushButton::clicked, this, [this]() {
            if (confirmSetDefaultRecipe())
                emit setDefaultRecipeRequested();
        });
        connect(m_actionButtons[3], &ui::ThemedPushButton::clicked, this,
                &RecipeListPanel::copyRecipeRequested);
        connect(m_actionButtons[4], &ui::ThemedPushButton::clicked, this, [this]() {
            const QString startDir = m_recipeDirectory.isEmpty()
                                         ? QCoreApplication::applicationDirPath()
                                         : m_recipeDirectory;
            const QStringList paths = QFileDialog::getOpenFileNames(
                window(), TR("aoi.recipe.action.import"), startDir,
                QStringLiteral("JSON (*.json);;All Files (*)"));
            if (!paths.isEmpty())
                emit importRecipesRequested(paths);
        });
        connect(m_actionButtons[5], &ui::ThemedPushButton::clicked, this, [this]() {
            const QString dir = QFileDialog::getExistingDirectory(
                window(), TR("aoi.recipe.action.export"),
                m_recipeDirectory.isEmpty() ? QString() : m_recipeDirectory);
            if (!dir.isEmpty())
                emit exportRecipesRequested(dir);
        });
    }

    connect(m_recipeList, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        if (!m_recipeList)
            return;
        auto *item = m_recipeList->itemAt(pos);
        if (!item)
            return;
        const QString recipeId = item->data(Qt::UserRole).toString();
        if (recipeId.isEmpty())
            return;

        QMenu menu(this);
        auto *renameAct = menu.addAction(TR("aoi.recipe.context.rename"));
        if (menu.exec(QCursor::pos()) == renameAct)
            promptRenameRecipe(recipeId);
    });

    connect(m_recipeList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *cur, QListWidgetItem *) {
                if (m_recipeList)
                    m_recipeList->refreshAllRows();
                if (!cur || m_refreshingList)
                    return;
                const QString id = cur->data(Qt::UserRole).toString();
                if (!id.isEmpty())
                    emit recipeSelected(id);
            });
}

/** 为每条 RecipeListItem 构建自定义行 widget（名称 + 默认标签 + 分隔线） */
void RecipeListPanel::setRecipes(const QVector<RecipeListItem> &recipes)
{
    if (!m_recipeList)
        return;

    m_refreshingList = true;
    m_recipeList->clear();

    for (const auto &r : recipes) {
        auto *rowW = new QWidget(m_recipeList);
        rowW->setObjectName(QStringLiteral("recipeListRow"));
        rowW->setFont(themeUiFont());
        rowW->setAttribute(Qt::WA_StyledBackground, false);
        rowW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        auto *rowLayout = new QVBoxLayout(rowW);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(0);

        auto *content = new QWidget(rowW);
        content->setObjectName(QStringLiteral("recipeListRowContent"));
        auto *hl = new QHBoxLayout(content);
        hl->setContentsMargins(10, 6, 10, 6);
        hl->setSpacing(4);

        auto *nameLbl = new QLabel(r.name, content);
        nameLbl->setObjectName(QStringLiteral("recipeListName"));
        nameLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        hl->addWidget(nameLbl, 1);

        if (r.isDefault) {
            auto *tagLbl = new QLabel(TR("aoi.recipe.tag.default"), content);
            tagLbl->setObjectName(QStringLiteral("recipeListTag"));
            hl->addWidget(tagLbl, 0, Qt::AlignRight | Qt::AlignVCenter);
        }

        rowLayout->addWidget(content);

        auto *sep = new QFrame(rowW);
        sep->setObjectName(QStringLiteral("recipeListRowSep"));
        sep->setFrameShape(QFrame::NoFrame);
        sep->setFixedHeight(1);
        rowLayout->addWidget(sep);

        auto *item = new QListWidgetItem(m_recipeList);
        item->setData(Qt::UserRole, r.id);
        m_recipeList->addItem(item);
        m_recipeList->setItemWidget(item, rowW);
        rowW->adjustSize();
        item->setSizeHint(rowW->sizeHint().expandedTo(QSize(0, 35)));
    }

    m_refreshingList = false;
    m_recipeList->syncRowWidths();
    applySelectionTheme();
}

void RecipeListPanel::setCurrentRecipeId(const QString &recipeId)
{
    if (!m_recipeList)
        return;

    m_refreshingList = true;
    for (int i = 0; i < m_recipeList->count(); ++i) {
        auto *item = m_recipeList->item(i);
        if (item && item->data(Qt::UserRole).toString() == recipeId) {
            m_recipeList->setCurrentItem(item);
            break;
        }
    }
    m_refreshingList = false;
    applySelectionTheme();
}

void RecipeListPanel::applySelectionTheme()
{
    if (!m_recipeList)
        return;
    m_recipeList->syncRowWidths();
    m_recipeList->refreshAllRows();
}

QString RecipeListPanel::currentRecipeDisplayName() const
{
    if (!m_recipeList)
        return {};
    auto *item = m_recipeList->currentItem();
    if (!item)
        return {};
    auto *rowW = m_recipeList->itemWidget(item);
    if (!rowW)
        return {};
    if (auto *nameLbl = rowW->findChild<QLabel *>(QStringLiteral("recipeListName")))
        return nameLbl->text();
    return {};
}

bool RecipeListPanel::confirmRemoveRecipe()
{
    if (!m_recipeList || !m_recipeList->currentItem())
        return false;

    const QString name = currentRecipeDisplayName();
    return ui::ThemedMessageBox::ask(
        window(),
        TR("aoi.recipe.confirm.remove_title"),
        TR("aoi.recipe.confirm.remove_text").arg(name),
        TR("aoi.recipe.action.remove"),
        TR("aoi.recipe.add_dialog.cancel"),
        ui::ThemedMessageBoxIcon::Warning,
        false,
        ui::PushButtonVariant::Danger);
}

bool RecipeListPanel::confirmSetDefaultRecipe()
{
    if (!m_recipeList || !m_recipeList->currentItem())
        return false;

    const QString name = currentRecipeDisplayName();
    return ui::ThemedMessageBox::ask(
        window(),
        TR("aoi.recipe.confirm.set_default_title"),
        TR("aoi.recipe.confirm.set_default_text").arg(name),
        TR("aoi.recipe.add_dialog.confirm"),
        TR("aoi.recipe.add_dialog.cancel"),
        ui::ThemedMessageBoxIcon::Question,
        false,
        ui::PushButtonVariant::Primary);
}

void RecipeListPanel::retranslatePanel()
{
    for (auto *lbl : findChildren<QLabel *>(QStringLiteral("recipeSectionTitle"))) {
        const QVariant key = lbl->property("trKey");
        if (key.isValid())
            lbl->setText(TR(key.toString().toUtf8().constData()));
    }
    for (auto *btn : m_actionButtons) {
        const QVariant key = btn->property("trKey");
        if (key.isValid())
            btn->setText(TR(key.toString().toUtf8().constData()));
    }
    if (m_searchEdit)
        m_searchEdit->setPlaceholderText(TR("aoi.recipe.search_placeholder"));

    if (!m_recipeList)
        return;
    for (int i = 0; i < m_recipeList->count(); ++i) {
        auto *item = m_recipeList->item(i);
        auto *w    = item ? m_recipeList->itemWidget(item) : nullptr;
        if (!w)
            continue;
        if (auto *tag = w->findChild<QLabel *>(QStringLiteral("recipeListTag")))
            tag->setText(TR("aoi.recipe.tag.default"));
    }
}

void RecipeListPanel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslatePanel();
}

QString RecipeListPanel::suggestNewRecipeName() const
{
    return RecipeRepository::generateNewRecipeId();
}

QString RecipeListPanel::promptRecipeName(const QString &titleKey, const QString &defaultName)
{
    ui::ThemedPanelDialog dlg(window());
    dlg.setModal(true);
    dlg.setTitleKey(titleKey);
    dlg.setMinimumPanelWidth(320);

    auto *body = new QWidget;
    auto *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(6);

    auto *nameLabel = ui::makeFormLabel(QStringLiteral("aoi.recipe.add_dialog.label"), body, 11);

    auto *nameEdit = ui::makeLineEdit(body);
    nameEdit->setText(defaultName);
    nameEdit->setMinimumWidth(260);
    nameEdit->setMaximumWidth(340);
    nameEdit->setMinimumHeight(30);
    nameEdit->setPlaceholderText(TR("aoi.recipe.add_dialog.placeholder"));

    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);
    btnRow->addStretch();
    auto *cancelBtn =
        ui::makePushButton(TR("aoi.recipe.add_dialog.cancel"), ui::PushButtonVariant::Secondary, body);
    auto *confirmBtn =
        ui::makePushButton(TR("aoi.recipe.add_dialog.confirm"), ui::PushButtonVariant::Primary, body);
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(confirmBtn);

    bodyLayout->addWidget(nameLabel);
    bodyLayout->addWidget(nameEdit);
    bodyLayout->addLayout(btnRow);

    dlg.setBodyWidget(body);
    dlg.finalizeLayoutSize();

    connect(cancelBtn, &ui::ThemedPushButton::clicked, &dlg, &QDialog::reject);
    connect(confirmBtn, &ui::ThemedPushButton::clicked, &dlg, &QDialog::accept);
    connect(nameEdit, &ui::ThemedLineEdit::returnPressed, &dlg, &QDialog::accept);

    nameEdit->setFocus();
    nameEdit->selectAll();

    if (dlg.exec() != QDialog::Accepted)
        return {};
    return nameEdit->text().trimmed();
}

void RecipeListPanel::promptRenameRecipe(const QString &recipeId)
{
    if (recipeId.isEmpty())
        return;

    RecipeDocument doc;
    if (!RecipeRepository::load(recipeId, doc))
        return;

    const QString newName =
        promptRecipeName(QStringLiteral("aoi.recipe.rename_dialog.title"), doc.basic.name);
    if (newName.isEmpty() || newName == doc.basic.name)
        return;

    emit renameRecipeRequested(recipeId, newName);
}

} // namespace aoi
