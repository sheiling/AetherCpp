#pragma once

/**
 * @file system_config_general_view.h
 * @brief 系统配置 · 通用配置子页（路径设置）
 *
 * 挂载于 PageWithSubNav，绑定 image/result/thirdParty 三类存储路径。
 */

#include "pagewidget.h"
#include "binding_session.h"

#include <QVector>

class QLineEdit;

namespace aoi {

class SystemConfigViewModel;

/**
 * @brief 通用配置：原图 / 结果 / 第三方数据存储路径
 *
 * 数据映射：`paths.imagePath`、`paths.resultPath`、`paths.thirdPartyPath`
 *
 * 绑定策略：`OnLostFocus`；保存前须 `updateAllSources()` flush 路径字段。
 */
class SystemConfigGeneralView : public PageWidget
{
    Q_OBJECT

public:
    explicit SystemConfigGeneralView(SystemConfigViewModel *viewModel, QWidget *parent = nullptr);

    void updateAllSources() { m_bindings.updateAllSources(); }

protected:
    void onLoad() override;
    void retranslateUi() override;

private:
    void buildLayout();
    void setupBindings();

    SystemConfigViewModel *m_viewModel = nullptr;
    BindingSession         m_bindings;

    /** 顺序：原图路径、结果路径、第三方路径 */
    QVector<QLineEdit *> m_pathEdits;
};

} // namespace aoi
