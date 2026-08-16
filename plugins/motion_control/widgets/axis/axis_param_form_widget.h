#pragma once

/**
 * @file axis_param_form_widget.h
 * @brief 单轴静态参数编辑表单
 *
 * 绑定 `IMotionControlService::axisConfig` / `setAxisConfig`，在 Motion 轴设置页上半区展示。
 * 字段以固定尺寸单元（标签 + 控件）组成，由 `ParamFormGridHost` 按滚动区宽度自动换列。
 *
 * ## 数据流
 * 1. `loadAxis(name)` ← `m_service->axisConfig(name)`
 * 2. 用户编辑 → `onSaveClicked()` → `collectConfig()` → `setAxisConfig` + `saveDeviceConfig`
 * 3. 保存成功 emit `configSaved(axisName)`
 *
 * ## 权限
 * 保存按钮受 `MotionActionKeys::AxisSave` 控制（`applySavePermission`）。
 *
 * @see MotionAxisSettingsView
 * @see AxisConfigData
 */

#include <imotion_control_service.h>
#include <motion_control_types.h>

#include <QFrame>
#include <QWidget>

namespace ui {
class ThemedSectionPanel;
class ThemedComboBox;
class ThemedDoubleSpinBox;
class ThemedSpinBox;
class PillSwitch;
class ThemedPushButton;
} // namespace ui

namespace motion_control_plugin {

class AxisParamFormWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AxisParamFormWidget(IMotionControlService *service, QWidget *parent = nullptr);

    /** 切换当前编辑轴并填充表单 */
    void loadAxis(const QString &axisName);
    QString axisName() const { return m_axisName; }

signals:
    void configSaved(const QString &axisName);

private:
    void buildUi();
    void bindFields(const AxisConfigData &cfg);
    /** 填充控制卡下拉（motionCardNames + 当前配置值） */
    void bindCardCombo(const QStringList &cardNames, const QString &current);
    /** 语言切换后重填各 Combo 文案并保留当前 data 选中项 */
    void refillAllCombos();
    /** m_contentBg 使用 t.background，与 SectionPanel 外层 surface 区分 */
    void refreshContentStyle();
    AxisConfigData collectConfig() const;
    void onSaveClicked();
    void applySavePermission();
    void retranslateUi();

protected:
    void changeEvent(QEvent *event) override;

    IMotionControlService *m_service = nullptr;
    QString                m_axisName;

    ui::ThemedSectionPanel *m_panel     = nullptr;
    QFrame                 *m_contentBg = nullptr; ///< 窗体背景色内容底，内嵌滚动网格 + 保存行

    // --- 轴基本属性 ---
    ui::PillSwitch          *m_enabled     = nullptr;
    ui::ThemedSpinBox       *m_axisNo      = nullptr;
    ui::ThemedComboBox      *m_axisType    = nullptr;
    ui::ThemedComboBox      *m_unitType    = nullptr;
    ui::ThemedComboBox      *m_encoderType = nullptr;
    ui::ThemedComboBox      *m_driverType  = nullptr;
    ui::ThemedComboBox      *m_cardName    = nullptr;

    // --- 运动参数（MoveParameters） ---
    ui::ThemedDoubleSpinBox *m_startVel = nullptr;
    ui::ThemedDoubleSpinBox *m_stopVel  = nullptr;
    ui::ThemedDoubleSpinBox *m_maxVel   = nullptr;
    ui::ThemedDoubleSpinBox *m_tacc     = nullptr;
    ui::ThemedDoubleSpinBox *m_tdec     = nullptr;
    ui::ThemedDoubleSpinBox *m_sPara    = nullptr;

    // --- 回零 ---
    ui::ThemedComboBox      *m_homeDir       = nullptr;
    ui::ThemedDoubleSpinBox *m_homeOffset    = nullptr;
    ui::ThemedComboBox      *m_homeVelMode   = nullptr;
    ui::ThemedComboBox      *m_homePinLogic  = nullptr;
    ui::ThemedComboBox      *m_homeSourceSel = nullptr;
    ui::ThemedComboBox      *m_homeMode      = nullptr;
    ui::ThemedDoubleSpinBox *m_homeHighVel   = nullptr;
    ui::ThemedDoubleSpinBox *m_homeTacc      = nullptr;
    ui::ThemedDoubleSpinBox *m_homeLowVel    = nullptr;

    // --- 脉冲 / 软限位 ---
    ui::ThemedDoubleSpinBox *m_pulseEquivalent = nullptr;
    ui::ThemedDoubleSpinBox *m_pulseUnit         = nullptr;
    ui::ThemedDoubleSpinBox *m_negativeLimit     = nullptr;
    ui::ThemedDoubleSpinBox *m_positiveLimit     = nullptr;
    ui::ThemedDoubleSpinBox *m_errorBand         = nullptr;
    ui::ThemedComboBox      *m_sourceSelLimit   = nullptr;
    ui::ThemedComboBox      *m_softLimitAction  = nullptr;

    ui::ThemedPushButton *m_saveBtn = nullptr;
};

} // namespace motion_control_plugin
