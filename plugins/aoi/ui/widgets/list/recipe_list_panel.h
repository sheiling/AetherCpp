#pragma once

/**
 * @file recipe_list_panel.h
 * @brief 配方列表面板：搜索、操作按钮、列表；持久化由外部 ViewModel 通过 signal 完成
 */

#include "recipe_config_model.h"

#include <QEvent>
#include <QFrame>
#include <QVector>
#include <QWidget>

class QListWidget;
class QListWidgetItem;

namespace ui {

class ThemedLineEdit;
class ThemedListWidget;
class ThemedPushButton;

} // namespace ui

namespace aoi {

/**
 * @brief 配方页左侧列表面板（纯 UI，持久化由 ViewModel 处理）
 *
 * 提供搜索框、6 个操作按钮、可右键重命名的列表。
 * 选中、增删改等通过 signal 向上抛出，不直接写盘。
 */
class RecipeListPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RecipeListPanel(QWidget *parent = nullptr);

    /** 设置配方存储目录并同步 RecipeRepository 根路径 */
    void setRecipeDirectory(const QString &dir);
    QString recipeDirectory() const { return m_recipeDirectory; }

    /** 全量刷新列表项（含默认标签行 widget） */
    void setRecipes(const QVector<RecipeListItem> &recipes);
    /** 按 recipeId 选中当前行，不触发 recipeSelected */
    void setCurrentRecipeId(const QString &recipeId);

signals:
    void recipeSelected(const QString &recipeId);
    void searchKeywordChanged(const QString &keyword);
    void addRecipeRequested(const QString &name);
    void removeRecipeRequested();
    void copyRecipeRequested();
    void setDefaultRecipeRequested();
    void importRecipesRequested(const QStringList &filePaths);
    void exportRecipesRequested(const QString &directoryPath);
    void renameRecipeRequested(const QString &recipeId, const QString &newName);

public slots:
    void retranslatePanel();
    /** 同步行宽并刷新各行选中/悬停背景 */
    void applySelectionTheme();

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildLayout();
    void connectActions();
    QString currentRecipeDisplayName() const;
    bool confirmRemoveRecipe();
    bool confirmSetDefaultRecipe();
    QString suggestNewRecipeName() const;
    void promptRenameRecipe(const QString &recipeId);
    /** 复用 ThemedPanelDialog 的名称输入；取消或空名返回空字符串 */
    QString promptRecipeName(const QString &titleKey, const QString &defaultName);

    QString m_recipeDirectory;

    QFrame                    *m_listGroup      = nullptr;
    ui::ThemedLineEdit            *m_searchEdit     = nullptr;
    ui::ThemedListWidget          *m_recipeList     = nullptr;
    QVector<ui::ThemedPushButton *> m_actionButtons;

    bool m_refreshingList = false;

    static constexpr const char *kListActionKeys[6] = {
        "aoi.recipe.action.add",
        "aoi.recipe.action.remove",
        "aoi.recipe.action.set_default",
        "aoi.recipe.action.copy",
        "aoi.recipe.action.import",
        "aoi.recipe.action.export",
    };
};

} // namespace aoi
