#pragma once

/**
 * @file recipe_config_view.h
 * @brief AOI 配方配置页薄壳：三列布局 + ViewModel 编排
 *
 * 左列 RecipeListPanel、中列 RecipeConfigCenterView、右列 RecipeConfigParamPanel；
 * 配方 CRUD / 保存 / 预览 / 扫描状态由本页统一绑定 VM 与子视图。
 */

#include "pagewidget.h"

namespace ui {

class CursorToast;

} // namespace ui

namespace aoi {

class RecipeConfigCenterView;
class RecipeConfigParamPanel;
class RecipeConfigViewModel;
class RecipeListPanel;

/**
 * @brief 三列：250px 配方列表 | 中间调试区 | 360px 可折叠参数
 */
class RecipeConfigView : public PageWidget
{
    Q_OBJECT

public:
    explicit RecipeConfigView(QWidget *parent = nullptr);

protected:
    void onLoad() override;
    void onHide() override;
    void retranslateUi() override;
    void applyTheme() override;

private:
    void buildLayout();
    /** 配方列表 / 中心 / 参数面板与 VM 的全量信号连接 */
    void setupBindings();
    void refreshRecipeList();
    /** CursorToast 展示 VM status；跳过首次 ready 避免启动闪 Toast */
    void onViewModelStatusChanged();

    RecipeConfigViewModel  *m_viewModel     = nullptr;
    RecipeListPanel        *m_recipeListPanel = nullptr;
    RecipeConfigCenterView *m_centerView    = nullptr;
    RecipeConfigParamPanel *m_paramPanel    = nullptr;
    ui::CursorToast        *m_statusToast   = nullptr;
    bool                    m_skipReadyStatusOnce = true;
};

} // namespace aoi
