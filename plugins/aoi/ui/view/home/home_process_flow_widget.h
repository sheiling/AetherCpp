#pragma once

/**
 * @file home_process_flow_widget.h
 * @brief 首页工艺流程条：上料 → 检测 → 下料三步骤高亮展示
 */

#include "vm/home/home_runtime_state.h"

#include <QWidget>

class QLabel;

namespace aoi {

/** @brief 横向流程步骤条，active 步骤用 accent 色高亮 */
class HomeProcessFlowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomeProcessFlowWidget(QWidget *parent = nullptr);

    /** 切换当前激活步骤并刷新各 step label 样式 */
    void setActiveStep(HomeProcessStep step);
    void retranslateUi();

private:
    void bindTheme();
    void refreshStyle();
    void updateStepStyles();

    QLabel *m_feedLbl     = nullptr;
    QLabel *m_inspectLbl  = nullptr;
    QLabel *m_dischargeLbl = nullptr;
    HomeProcessStep m_active = HomeProcessStep::Inspect;
};

} // namespace aoi
