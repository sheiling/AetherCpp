#pragma once

/**
 * @file system_config_threshold_view.h
 * @brief 系统配置 · 阈值范围子页（各缺陷数量上下限 + 总缺陷上限）
 *
 * 挂载于 PageWithSubNav；Spin 使用 ThemedSpinBox，绑定 defects[kind] 与 totalDefects 阈值。
 */

#include "pagewidget.h"
#include "binding_session.h"
#include "param_define.h"
#include "inspection_types.h"

#include <QVector>

class QLabel;

namespace ui {

class ThemedSpinBox;

} // namespace ui

namespace aoi {

class SystemConfigViewModel;

/**
 * @brief 阈值范围：各缺陷类型数量上下限 + 总缺陷数上下限
 *
 * 数据映射：
 * - `defects[kind].lower` / `upper`
 * - `defects.total.lower` / `upper`
 *
 * 绑定策略：`OnChanged` 即时写回。Spin 使用 `ThemedSpinBox`（自绘箭头，颜色走 ThemeTokens）。
 */
class SystemConfigThresholdView : public PageWidget
{
    Q_OBJECT

public:
    explicit SystemConfigThresholdView(SystemConfigViewModel *viewModel, QWidget *parent = nullptr);

    void updateAllSources() { m_bindings.updateAllSources(); }

protected:
    void onLoad() override;
    void retranslateUi() override;

private:
    void buildLayout();
    void setupBindings();
    /** 缺陷项名称 + 「数量」后缀；优先 config.displayName，否则用默认翻译键 */
    QString defectThresholdLabel(inspection::InspectionDefectType kind) const;

    SystemConfigViewModel *m_viewModel = nullptr;
    BindingSession         m_bindings;

    QVector<ui::ThemedSpinBox *> m_threshLo;
    QVector<ui::ThemedSpinBox *> m_threshHi;
    QVector<QLabel *>           m_threshNameLabels;
    ui::ThemedSpinBox           *m_totalLo = nullptr;
    ui::ThemedSpinBox           *m_totalHi = nullptr;
    QLabel                  *m_totalNameLabel = nullptr;
};

} // namespace aoi
