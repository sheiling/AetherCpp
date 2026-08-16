#pragma once

/**
 * @file home_view_model.h
 * @brief 首页 ViewModel（KPI、进度、日志、预览区状态绑定）
 */

#include "lighten_viewmodel_macros.h"
#include "home_model.h"

namespace lighten {

/**
 * @brief 首页 ViewModel
 *
 * 绑定 HomeView：KPI、硬件状态、检测进度、当前结果快照、运行日志、预览区选项。
 * 产线初始化/启停由应用层调度模块负责，本 ViewModel 仅维护界面展示状态。
 */
class HomeViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_RUNTIME        = "runtime";
    static constexpr const char *PROP_STATUS         = "status";
    static constexpr const char *PROP_SELECTED_COLOR = "selectedColor";
    static constexpr const char *PROP_SELECTED_CAMERA = "selectedCamera";
    static constexpr const char *PROP_SHOW_RESULT_IMAGE = "showResultImage";

    LIGHTEN_VIEWMODEL_STRUCT_PROPERTY(HomeRuntimeState, runtime, Runtime)
    LIGHTEN_VIEWMODEL_PROPERTY(int,     selectedColor,      SelectedColor)
    LIGHTEN_VIEWMODEL_PROPERTY(int,     selectedCamera,     SelectedCamera)
    LIGHTEN_VIEWMODEL_PROPERTY(bool,    showResultImage,    ShowResultImage)
    LIGHTEN_VIEWMODEL_PROPERTY(QString, status,             Status)

signals:
    void sig_runtimeChanged();
    void sig_selectedColorChanged();
    void sig_selectedCameraChanged();
    void sig_showResultImageChanged();
    void sig_statusChanged();

public:
    explicit HomeViewModel(QObject *parent = nullptr);

public slots:
    /** 从 Model 刷新运行时状态到 ViewModel */
    void slot_refreshFromModel();
    /** 默认配方变更后同步首页「当前配方」KPI */
    void slot_updateCurrentRecipe(const QString &recipeId);
    /** 仅更新对插次数上限（系统配置变更时） */
    void slot_updateMaxUseCount(int maxUseCount);
    /** 从 lighten_production_kpi 加载并刷新首页 KPI 条（总点亮板数、OK/NG/RW、对插次数、通过率） */
    void slot_refreshProductionKpiFromService();
    /** 更新指定首页检测项进度槽位的状态文本 */
    void slot_updateProgressStatus(int slotIndex, const QString &statusText);
    /** 追加一条运行日志 */
    void slot_appendLog(const QString &message, const QString &level = QStringLiteral("INFO"));
};

} // namespace lighten
