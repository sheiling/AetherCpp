#ifndef DEVICEVIEW_H
#define DEVICEVIEW_H

#include "pagewidget.h"
#include "deviceviewmodel.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QProgressBar>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QGroupBox>
#include <QFormLayout>

/**
 * @brief 设备管理视图（View层）—— MVVM 增强示例
 *
 * 演示所有新增控件绑定和结构体字段绑定：
 *  - 结构体绑定：DeviceInfo、SensorConfig 字段分别绑到各控件
 *  - QSlider        双向绑定（音量）
 *  - QDoubleSpinBox 双向绑定（传感器阈值）
 *  - QProgressBar   单向绑定（模拟进度）
 *  - QDateEdit      双向绑定（校准日期）
 *  - QTimeEdit      双向绑定（触发时间）
 *  - QRadioButton   双向绑定（启用报警）
 *  - bindEnabled    控件可用状态绑定
 *  - bindVisible    控件可见状态绑定（报警阈值面板）
 */
class DeviceView : public PageWidget
{
    Q_OBJECT

public:
    explicit DeviceView(QWidget *parent = nullptr);
    ~DeviceView() override;

protected:
    void onLoad() override;
    void retranslateUi() override;
    void applyTheme() override;

private:
    void setupUi();
    void setupBindings();

private:
    DeviceViewModel *m_viewModel = nullptr;

    // ── DeviceInfo 结构体字段 ──────────────────────────────────────────────────
    QLineEdit  *m_devNameEdit    = nullptr;
    QLineEdit  *m_devSnEdit      = nullptr;
    QComboBox  *m_devFwCombo     = nullptr;
    QSpinBox   *m_devHealthSpin  = nullptr;
    QProgressBar *m_devHealthBar = nullptr;
    QCheckBox  *m_devOnlineChk   = nullptr;

    // ── SensorConfig 结构体字段 ────────────────────────────────────────────────
    QDoubleSpinBox *m_thresholdSpin = nullptr;
    QSpinBox       *m_sampleRateSpin = nullptr;
    QDateEdit      *m_calibDateEdit  = nullptr;
    QTimeEdit      *m_triggerTimeEdit = nullptr;

    // ── 独立属性绑定控件 ───────────────────────────────────────────────────────
    QSlider      *m_volumeSlider   = nullptr;
    QLabel       *m_volumeLabel    = nullptr;    // 实时显示音量值
    QProgressBar *m_progressBar    = nullptr;    // 单向绑定进度
    QRadioButton *m_alertOnRadio   = nullptr;    // useAlert = true
    QRadioButton *m_alertOffRadio  = nullptr;    // useAlert = false

    // ── 报警配置面板（bindVisible 演示）──────────────────────────────────────
    QGroupBox *m_alertGroup = nullptr;

    // ── 状态 ──────────────────────────────────────────────────────────────────
    QLabel *m_statusLabel = nullptr;

    // ── 按钮 ──────────────────────────────────────────────────────────────────
    QPushButton *m_applyBtn    = nullptr;
    QPushButton *m_resetBtn    = nullptr;
    QPushButton *m_simulateBtn = nullptr;

    // ── 需要动态翻译的控件 ─────────────────────────────────────────────────────
    QLabel    *m_titleLabel  = nullptr;
    QLabel    *m_descLabel   = nullptr;
    QGroupBox *m_deviceGroup = nullptr;
    QGroupBox *m_sensorGroup = nullptr;
    QGroupBox *m_ctrlGroup   = nullptr;
    QGroupBox *m_statusGroup = nullptr;
};

#endif // DEVICEVIEW_H
