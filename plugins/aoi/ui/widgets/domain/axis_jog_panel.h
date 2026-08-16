#pragma once

/**
 * @file axis_jog_panel.h
 * @brief 多轴点动调试面板：按轴名列表自动生成行（速度比例、点动、指令/反馈位置）
 */

#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/spinbox/spinbox_paint_utils.h"

#include <QEvent>
#include <QHash>
#include <QLabel>
#include <QStringList>
#include <QWidget>

namespace ui {

class ThemedIconButton;
class ThemedComboBox;
class ThemedDoubleSpinBox;
class ThemedPushButton;

} // namespace ui

namespace aoi {

/** 单轴行内可绑定的控件句柄，供外部 Controller 读写速度与位置 */
struct AxisRowWidgets
{
    ui::ThemedComboBox       *speedRatio = nullptr; ///< 点动速度比例（0.5 / 0.1 / 0.001）
    ui::ThemedIconButton     *negBtn     = nullptr; ///< 负向点动
    ui::ThemedIconButton     *posBtn     = nullptr; ///< 正向点动
    ui::ThemedDoubleSpinBox  *targetPos = nullptr; ///< 绝对/相对运动目标值
    ui::ThemedPushButton     *absMoveBtn = nullptr; ///< 绝对运动
    ui::ThemedPushButton     *relMoveBtn = nullptr; ///< 相对运动
    QLabel                   *cmdPos     = nullptr; ///< 指令位置只读显示
    QLabel                   *fbPos      = nullptr; ///< 反馈位置只读显示
};

/**
 * @brief 配方调试页多轴点动面板
 *
 * 按 `axisNames` 动态生成行：速度比例下拉、正负向点动按钮（按下运动/抬起停止）、指令/反馈位置。
 * 轴名通常为运控配置名（如「X轴」）；箭头方向按名称中的 X/Y/Z 推断。
 * 无绑定轴时隐藏本面板及其父级调试区容器（与改前行为一致）。
 */
class AxisJogPanel : public QWidget
{
    Q_OBJECT

public:
    /** @param axisNames 参与点动的运控轴名列表 */
    explicit AxisJogPanel(const QStringList &axisNames, QWidget *parent = nullptr);

    /** 当前轴名列表 */
    QStringList axisNames() const { return m_axisNames; }

    /** 按轴名取行内控件；未知键返回空指针字段 */
    AxisRowWidgets rowWidgets(const QString &axisKey) const;
    /** 读取速度比例下拉当前值，缺行时返回 0.1 */
    double         speedRatio(const QString &axisKey) const;
    /** 读取反馈位置标签数值，缺行时返回 0 */
    double         feedbackPosition(const QString &axisKey) const;
    /** 读取目标位置输入框数值，缺行时返回 0 */
    double         targetPosition(const QString &axisKey) const;

    /** 更新指令/反馈位置显示（三位小数） */
    void setAxisPositions(const QString &axisKey, double cmdPos, double fbPos);

    /** 按采集器配置替换轴列表并重建行 */
    void setAxisNames(const QStringList &axisNames);
    /** 将所有轴恢复默认：速度比例 0.1、位置显示 0.000 */
    void resetForm();
    /** 语言切换时刷新标题与表单标签 */
    void retranslatePanel();
    /** 紧凑布局：统一字号、控件高度与下拉框固定宽度 */
    void refreshCompactLayout();

signals:
    /** @param direction 0 负向，1 正向 */
    void jogPressed(const QString &axisName, int direction, double speedRatio);
    void jogReleased(const QString &axisName);
    void absoluteMoveRequested(const QString &axisName, double targetPos);
    void relativeMoveRequested(const QString &axisName, double delta);

protected:
    void changeEvent(QEvent *event) override;

private:
    void         rebuildAxisRows();
    void         updateVisibility();
    QWidget     *buildAxisRow(const QString &axisKey);
    ui::TriangleDirection inferNegDirection(const QString &axisKey) const;
    ui::TriangleDirection inferPosDirection(const QString &axisKey) const;

    QStringList                    m_axisNames;
    QHash<QString, AxisRowWidgets> m_rows;
    QWidget                       *m_axisWrap = nullptr;
};

} // namespace aoi
