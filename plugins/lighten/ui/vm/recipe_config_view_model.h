#pragma once

/**
 * @file recipe_config_view_model.h
 * @brief 产品配方配置 ViewModel（配方 CRUD、ROI、导入导出）
 */

#include "lighten_viewmodel_macros.h"
#include "command.h"
#include "recipe_model.h"

#include <QList>

namespace lighten {

/**
 * @brief 产品配方配置 ViewModel
 *
 * 绑定 RecipeConfigView：配方列表、当前配方文档、顶栏状态、调试区工具条。
 */
class RecipeConfigViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_RECIPES         = "recipes";
    static constexpr const char *PROP_CURRENT_ID      = "currentRecipeId";
    static constexpr const char *PROP_DOCUMENT        = "document";
    static constexpr const char *PROP_STATUS_BAR      = "statusBar";
    static constexpr const char *PROP_SEARCH_KEYWORD  = "searchKeyword";
    static constexpr const char *PROP_ACTIVE_CAMERA   = "activeCamera";
    static constexpr const char *PROP_ACTIVE_COLOR    = "activeColorChannel";
    static constexpr const char *PROP_STATUS          = "status";

    LIGHTEN_VIEWMODEL_PROPERTY(QString, searchKeyword, SearchKeyword)
    LIGHTEN_VIEWMODEL_PROPERTY(QString, currentRecipeId, CurrentRecipeId)
    LIGHTEN_VIEWMODEL_PROPERTY(int,     activeCamera, ActiveCamera)
    LIGHTEN_VIEWMODEL_PROPERTY(int,     activeColorChannel, ActiveColorChannel)
    LIGHTEN_VIEWMODEL_PROPERTY(QString, status, Status)
    LIGHTEN_VIEWMODEL_STRUCT_PROPERTY(RecipeDocument, document, Document)

signals:
    void sig_searchKeywordChanged();
    void sig_currentRecipeIdChanged();
    void sig_activeCameraChanged();
    void sig_activeColorChannelChanged();
    void sig_statusChanged();
    void sig_recipesChanged();
    /** 每次保存配方写入成功时发出（与 status 是否变化无关） */
    void sig_saveSucceeded();
    void sig_documentChanged();
    void sig_statusBarChanged();
    void sig_debugToolbarChanged();
    /** 完成配方加载并切换当前文档后发出（含列表点选、增删复制等） */
    void sig_recipeSelected();

public:
    explicit RecipeConfigViewModel(QObject *parent = nullptr);

    /** 当前配方列表（含搜索过滤后的可见项） */
    const QVector<RecipeListItem> &recipes() const { return m_recipes; }
    /** 顶栏状态行（当前配方、参数完整性；图像质量 Pill 由 kRecipeShowImageQualityUi 控制） */
    RecipeStatusBar statusBar() const { return m_statusBar; }
    /** 调试工具条状态（相机、颜色通道、ROI 模式等） */
    RecipeDebugToolbar debugToolbar() const;

    /** 获取指定相机+颜色通道的 ROI 点集 */
    QList<double> roiPoints(CameraIndex camera, ColorChannel channel) const;
    /** 获取当前活动相机+颜色通道的 ROI 点集 */
    QList<double> currentRoiPoints() const;

    /** 保存当前配方命令 */
    Command *saveCommand() const { return m_saveCommand; }
    /** 设为默认配方命令（更新默认 ID 与 current_recipe.json；不刷新 FlowContext） */
    Command *setDefaultCommand() const { return m_setDefaultCommand; }

    /** 当前选中配方是否允许删除 */
    bool canRemoveRecipe() const;

public slots:
    /** 加载配方列表与当前配方文档 */
    void slot_load();
    /** 仅刷新配方列表 */
    void slot_loadRecipeList();
    /** 切换当前编辑的配方 */
    void slot_selectRecipe(const QString &recipeId);
    /** 新增配方 */
    void slot_addRecipe(const QString &name);
    /** 复制当前配方 */
    void slot_copyRecipe(const QString &name);
    /** 删除当前配方 */
    void slot_removeRecipe();
    /** 导出全部配方；返回弹框文案（全部导出成功 / 部分导出失败 / 全部导出失败） */
    QString slot_exportAllRecipes(const QString &dirPath);
    /** 从 JSON 文件批量导入；返回弹框文案（全部导入成功 / 部分导入失败 / 全部导入失败） */
    QString slot_importRecipesFromFiles(const QStringList &filePaths);
    /** 按关键字过滤配方列表 */
    void slot_applySearch(const QString &keyword);
    /** 更新调试工具条状态 */
    void slot_updateDebugToolbar(const RecipeDebugToolbar &toolbar);
    /** 设置当前 ROI 点集（内存态） */
    void slot_setCurrentRoiPoints(const QList<double> &points);
    /** 保存区域定位（roiEnabled + 四点 ROI 坐标）到配方文档 */
    void slot_saveCurrentRoiPoints(bool roiEnabled, const QList<double> &points);

private:
    void updateStatusBarFromDocument();
    void updateCommandStates();

    QVector<RecipeListItem> m_recipes;
    RecipeStatusBar         m_statusBar;

    FunctionalCommand *m_saveCommand       = nullptr;
    FunctionalCommand *m_setDefaultCommand = nullptr;
};

} // namespace lighten
