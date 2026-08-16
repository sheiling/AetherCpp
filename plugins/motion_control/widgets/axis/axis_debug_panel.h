#pragma once

/**
 * @file axis_debug_panel.h
 * @brief 单轴运行时调试面板
 *
 * 展示 `AxisRuntimeState` 指示灯、命令/反馈位置（单位从 `axisConfig().unitType` 只读展示），
 * 并提供使能、回零、点动、绝对/相对移动等操作。
 * 面板可见时开启 `IMotionControlService::setAutoRefreshEnabled(true)` 以接收 `axisStatesUpdated`。
 *
 * ## 指示灯语义
 * | key | 字段 | alarmTone |
 * |-----|------|-----------|
 * | alarm | state.alarm | 是（danger） |
 * | elp/eln/slp/sln | 硬/软限位 | 是 |
 * | org/inp/en | 原点/到位/使能 | 否（success） |
 *
 * @see MotionAxisSettingsView（Splitter 下半区）
 */

#include <imotion_control_service.h>

#include <QHash>
#include <QString>
#include <QWidget>

class QButtonGroup;
class QFrame;

namespace ui {
class ThemedSectionPanel;
class ThemedFormLabel;
class ThemedLineEdit;
class ThemedDoubleSpinBox;
class ThemedGlassIndicatorDot;
class ThemedPushButton;
class CursorToast;
struct ThemeTokens;
} // namespace ui

namespace motion_control_plugin {

class AxisDebugPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AxisDebugPanel(IMotionControlService *service, QWidget *parent = nullptr);

    void setAxisName(const QString &axisName);
    QString axisName() const { return m_axisName; }

    /** 从 m_service->axisState 拉取并刷新指示灯与位置只读框 */
    void refreshState();

protected:
    void changeEvent(QEvent *event) override;
    /** 显示时开自动刷新并 refreshState；隐藏时关自动刷新（避免后台轮询） */
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void buildUi();
    QWidget *buildIndicatorSection();
    QWidget *buildPosFeedbackSection();
    QWidget *buildTargetSpeedSection();
    QWidget *buildButtonSection();
    void bindService();
    void retranslateUi();
    void refreshContentStyle();
    void showHomeResultToast(const QString &axisName, bool success, const QString &errorMessage = {});
    void updateIndicators(const AxisRuntimeState &state);
    void setIndicator(ui::ThemedGlassIndicatorDot *dot, bool active, bool alarmTone,
                      const ui::ThemeTokens &tokens);
    /** 使能按钮 checked 与 state.enabled 同步，不触发 toggled */
    void syncEnableButton(bool enabled);
    /** 报警时清除报警按钮切换为 Danger 红底，与正常 Secondary 区分 */
    void syncClearAlarmButton(bool alarm);
    /** 点动速度比例：m_stepGroup 选中项 property("step")，默认 0.1 */
    double stepSize() const;

    IMotionControlService *m_service = nullptr;
    QString                m_axisName;

    ui::ThemedSectionPanel  *m_panel       = nullptr;
    QFrame                  *m_contentBg   = nullptr;
    ui::ThemedFormLabel     *m_axisNameLbl = nullptr;
    ui::ThemedLineEdit      *m_cmdPosEdit   = nullptr;
    ui::ThemedLineEdit      *m_fbPosEdit    = nullptr;
    ui::ThemedDoubleSpinBox *m_targetPos    = nullptr;
    QButtonGroup            *m_stepGroup    = nullptr;
    ui::ThemedPushButton    *m_enableBtn     = nullptr;
    ui::ThemedPushButton    *m_clearAlarmBtn = nullptr;
    ui::CursorToast         *m_homeToast     = nullptr;
    QString                  m_pendingHomeAxis;
    AxisRuntimeState         m_lastRuntimeState;
    int                      m_unitType      = 1;
    bool                     m_hasLastState  = false;

    /** 指示灯 dotKey → ThemedGlassIndicatorDot（与 buildUi indicators 列表 key 对应） */
    QHash<QString, ui::ThemedGlassIndicatorDot *> m_indicatorDots;
};

} // namespace motion_control_plugin
