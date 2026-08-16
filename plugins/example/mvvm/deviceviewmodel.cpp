#include "deviceviewmodel.h"
#include <QDebug>
#include <QMetaType>

DeviceViewModel::DeviceViewModel(QObject *parent)
    : ViewModel(parent)
{
    // ── 注册元类型（使 QVariant 能存储结构体并支持比较器）────────────────────
    qRegisterMetaType<DeviceInfo>("DeviceInfo");
    qRegisterMetaType<SensorConfig>("SensorConfig");
    QMetaType::registerComparators<DeviceInfo>();
    QMetaType::registerComparators<SensorConfig>();

    // ── 注册属性（必须与 PROP_* 常量一致）────────────────────────────────────
    registerProperty(PROP_DEVICE,        QVariant::fromValue(DeviceInfo{}));
    registerProperty(PROP_SENSOR,        QVariant::fromValue(SensorConfig{}));
    registerProperty(PROP_VOLUME,        60);
    registerProperty(PROP_PROGRESS,      0);
    registerProperty(PROP_USE_ALERT,     true);
    registerProperty(PROP_ALERT_VISIBLE, true);
    registerProperty(PROP_STATUS,        QString::fromUtf8("\u5c31\u7eea"));

    // ── 监听 useAlert 变化，同步控制 alertVisible ──────────────────────────
    connect(this, &ViewModel::propertyChanged, this,
        [this](const QString &name, const QVariant &value) {
            if (name == PROP_USE_ALERT) {
                setAlertVisible(value.toBool());
            }
            refreshStatus();
        });

    // ── Commands ──────────────────────────────────────────────────────────────
    m_applyCommand = new FunctionalCommand(
        [this]() { applyDevice(); },
        nullptr,
        this
    );
    m_applyCommand->setName("ApplyDevice");

    m_resetCommand = new FunctionalCommand(
        [this]() { resetDevice(); },
        nullptr,
        this
    );
    m_resetCommand->setName("ResetDevice");

    m_simulateCommand = new FunctionalCommand(
        [this]() { simulateProgress(); },
        nullptr,
        this
    );
    m_simulateCommand->setName("SimulateProgress");

    refreshStatus();
}

void DeviceViewModel::applyDevice()
{
    DeviceInfo d = device();
    SensorConfig s = sensor();
    QString msg = QString::fromUtf8("\u8bbe\u5907 [%1] \u914d\u7f6e\u5df2\u5e94\u7528\u2014\u2014\u5b57\u8282: SN=%2, \u56fa\u4ef6=%3, \u5065\u5eb7\u5ea6=%4%")
        .arg(d.name, d.serialNumber, d.firmware)
        .arg(d.health);
    setStatus(msg);
    qDebug() << "[DeviceViewModel] applyDevice:" << msg;
    qDebug() << "[DeviceViewModel] SensorConfig: threshold=" << s.threshold
             << "sampleRate=" << s.sampleRate;
}

void DeviceViewModel::resetDevice()
{
    beginBatchUpdate();
    setDevice(DeviceInfo{});
    setSensor(SensorConfig{});
    setVolume(60);
    setProgress(0);
    setUseAlert(true);
    setStatus(QString::fromUtf8("\u5df2\u91cd\u7f6e\u5230\u9ed8\u8ba4\u503c"));
    endBatchUpdate();

    m_simulateStep = 0;
    qDebug() << "[DeviceViewModel] resetDevice";
}

void DeviceViewModel::simulateProgress()
{
    m_simulateStep = (m_simulateStep + 15) % 101;
    setProgress(m_simulateStep);
    setStatus(QString::fromUtf8("\u6a21\u62df\u8fdb\u5ea6: %1%").arg(m_simulateStep));
}

void DeviceViewModel::refreshStatus()
{
    // 仅在状态未被其他操作覆盖时刷新（这里简单地追加信息）
    DeviceInfo d = device();
    if (d.online) {
        // 状态不做干扰，已由 apply/reset 管理
    }
}
