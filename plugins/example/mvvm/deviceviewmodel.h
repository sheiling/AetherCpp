#ifndef DEVICEVIEWMODEL_H
#define DEVICEVIEWMODEL_H

#include "viewmodel.h"
#include "command.h"
#include "devicemodel.h"

/**
 * @brief 设备管理 ViewModel —— MVVM 增强示例
 *
 * 演示内容：
 *  1. VIEWMODEL_STRUCT_PROPERTY   — 结构体属性双向绑定
 *  2. VIEWMODEL_PROPERTY(double)  — 浮点属性 <-> QDoubleSpinBox
 *  3. VIEWMODEL_PROPERTY(int)     — 整数属性 <-> QSlider / QProgressBar（单向）
 *  4. VIEWMODEL_PROPERTY(QDate)   — 日期属性 <-> QDateEdit
 *  5. VIEWMODEL_PROPERTY(QTime)   — 时间属性 <-> QTimeEdit
 *  6. VIEWMODEL_PROPERTY(bool)    — 布尔属性 <-> QRadioButton
 *  7. bindEnabled / bindVisible   — 控件状态绑定
 */
class DeviceViewModel : public ViewModel
{
    Q_OBJECT

public:
    // ── 属性名常量（避免硬编码字符串）────────────────────────────────────────
    static constexpr const char *PROP_DEVICE        = "device";        ///< DeviceInfo 结构体
    static constexpr const char *PROP_SENSOR        = "sensor";        ///< SensorConfig 结构体
    static constexpr const char *PROP_VOLUME        = "volume";        ///< 音量 0-100（QSlider）
    static constexpr const char *PROP_PROGRESS      = "progress";      ///< 模拟进度 0-100（QProgressBar）
    static constexpr const char *PROP_USE_ALERT     = "useAlert";      ///< 是否启用报警（QRadioButton）
    static constexpr const char *PROP_ALERT_VISIBLE = "alertVisible";  ///< 报警面板可见（bindVisible）
    static constexpr const char *PROP_STATUS        = "status";        ///< 状态文本

    // ── 结构体属性 ────────────────────────────────────────────────────────────
    VIEWMODEL_STRUCT_PROPERTY(DeviceInfo,   device, Device)
    VIEWMODEL_STRUCT_PROPERTY(SensorConfig, sensor, Sensor)

    // ── 基础属性 ──────────────────────────────────────────────────────────────
    VIEWMODEL_PROPERTY(int,     volume,      Volume)
    VIEWMODEL_PROPERTY(int,     progress,    Progress)
    VIEWMODEL_PROPERTY(bool,    useAlert,    UseAlert)
    VIEWMODEL_PROPERTY(bool,    alertVisible,AlertVisible)
    VIEWMODEL_PROPERTY(QString, status,      Status)

signals:
    void deviceChanged();
    void sensorChanged();
    void volumeChanged();
    void progressChanged();
    void useAlertChanged();
    void alertVisibleChanged();
    void statusChanged();

public:
    explicit DeviceViewModel(QObject *parent = nullptr);
    ~DeviceViewModel() override = default;

    // Commands
    Command *applyCommand()  const { return m_applyCommand;  }
    Command *resetCommand()  const { return m_resetCommand;  }
    Command *simulateCommand() const { return m_simulateCommand; }

public slots:
    void applyDevice();
    void resetDevice();
    void simulateProgress();

private:
    void refreshStatus();

    FunctionalCommand *m_applyCommand    = nullptr;
    FunctionalCommand *m_resetCommand    = nullptr;
    FunctionalCommand *m_simulateCommand = nullptr;

    int m_simulateStep = 0;
};

#endif // DEVICEVIEWMODEL_H
