/**
 * @file motion_control_service_impl.cpp
 * @brief MotionControlServiceImpl 实现：DeviceManager 适配、目录构建、状态轮询与轴配置映射
 *
 * 依赖：Container::Application → DeviceManager → MotionController。
 * 轴配置读写通过匿名命名空间 toAxisConfig / applyAxisConfig 与底层 AxisData、MoveParameters 互转。
 */

#include "motion_control_service_impl.h"

#include <common/core/container/application.h>
#include <common/core/device/deviceManager/device_manager.h>
#include <common/core/device/motionControl/motion_controller.h>
#include <common/core/device/motionControl/motion_control_impl/motion_types.h>

#include <Logger.h>

#include <QMetaObject>
#include <QPointer>

#include <thread>

namespace motion_control_plugin {

namespace {

/** @brief 安全获取 DeviceManager；Application 未就绪或未绑定时返回 nullptr */
std::shared_ptr<common::device::DeviceManager> tryGetDeviceManager()
{
    try {
        auto *app = Container::Application::instance();
        if (!app || !app->bound<common::device::DeviceManager>())
            return nullptr;
        return app->make<common::device::DeviceManager>();
    } catch (const std::exception &e) {
        LOG_WARN("[MotionControlService] DeviceManager unavailable: {}", e.what());
        return nullptr;
    }
}

QString toQString(const std::string &s)
{
    return QString::fromStdString(s);
}

/** @brief 模块分组键：trim 后空串映射为 "_default"，与 UI 展示时清空 moduleName 配对 */
QString moduleKey(const std::string &moduleName)
{
    const QString key = toQString(moduleName).trimmed();
    return key.isEmpty() ? QStringLiteral("_default") : key;
}

/** @brief 无锁读轴运行时状态；positiveLimit = 硬正 OR 软正限位 */
QHash<QString, AxisRuntimeState> fetchAxisStatesFromHardware(
    const std::shared_ptr<common::device::MotionController> &motionCtrl,
    const QStringList                                        &axisNames)
{
    QHash<QString, AxisRuntimeState> states;
    if (!motionCtrl)
        return states;

    for (const QString &axisName : axisNames) {
        const std::string name = axisName.toStdString();

        AxisRuntimeState state;
        state.name = axisName;

        const auto axisData = motionCtrl->getAxisParams(name);
        state.moduleName = moduleKey(axisData.m_moduleName);
        if (state.moduleName == QStringLiteral("_default"))
            state.moduleName.clear();

        motionCtrl->AxisGetPosition(name, state.cmdPos);
        motionCtrl->AxisGetEncoder(name, state.fbPos);

        bool enable = false;
        if (motionCtrl->AxisIsEnable(name, enable) == nullptr)
            state.enabled = enable;

        common::device::axis::Axis::AxisSta status{};
        if (motionCtrl->AxisReadStatus(name, status) == nullptr) {
            state.alarm             = status.ALM;
            state.inPosition        = status.INP;
            state.origin            = status.ORG;
            state.hardPositiveLimit = status.ELP;
            state.hardNegativeLimit = status.ELN;
            state.softPositiveLimit = status.SLP;
            state.softNegativeLimit = status.SLN;
            state.positiveLimit     = status.ELP || status.SLP;
            state.negativeLimit     = status.ELN || status.SLN;
        }

        states.insert(axisName, state);
    }
    return states;
}

/** @brief 无锁读 IO 运行时状态；m_type==1 视为输出 */
QHash<QString, IoRuntimeState> fetchIoStatesFromHardware(
    const std::shared_ptr<common::device::MotionController> &motionCtrl,
    const QStringList                                        &ioNames)
{
    QHash<QString, IoRuntimeState> states;
    if (!motionCtrl)
        return states;

    for (const QString &ioName : ioNames) {
        const std::string name = ioName.toStdString();

        IoRuntimeState state;
        state.name = ioName;

        const auto ioData = motionCtrl->getDeviceIoData(name);
        state.moduleName = moduleKey(ioData.m_moduleName);
        if (state.moduleName == QStringLiteral("_default"))
            state.moduleName.clear();
        state.isOutput = ioData.m_type == 1;

        bool value = false;
        if (motionCtrl->ReadIO(name, value) == nullptr)
            state.value = value;

        states.insert(ioName, state);
    }
    return states;
}

} // namespace

// =============================================================================
// 构造 / 析构 / 生命周期
// =============================================================================

MotionControlServiceImpl::MotionControlServiceImpl(QObject *parent)
    : IMotionControlService(parent)
{
    m_refreshTimer = new QTimer();
    m_refreshTimer->setTimerType(Qt::PreciseTimer);
    m_refreshTimer->moveToThread(&m_refreshThread);
    // 定时器在刷新线程触发；DirectConnection 在刷新线程 poll，再 Queued 回 UI 线程 emit
    connect(m_refreshTimer, &QTimer::timeout, this, [this]() {
        if (!m_autoRefreshEnabled.load())
            return;
        pollRuntimeStatesFromHardware();
        QMetaObject::invokeMethod(this, &MotionControlServiceImpl::emitStatesUpdated,
                                  Qt::QueuedConnection);
    }, Qt::DirectConnection);
}

MotionControlServiceImpl::~MotionControlServiceImpl()
{
    shutdown();
}

bool MotionControlServiceImpl::initialize(QString *errorOut)
{
    const auto motionCtrl = motion();
    if (!motionCtrl) {
        if (errorOut)
            *errorOut = QStringLiteral("DeviceManager or MotionController not available");
        return false;
    }

    rebuildCatalog();
    refreshAllStates();

    m_refreshThread.start();
    QMetaObject::invokeMethod(m_refreshTimer, [this]() { m_refreshTimer->setInterval(m_refreshIntervalMs.load()); },
                              Qt::QueuedConnection);

    m_initialized = true;
    return true;
}

void MotionControlServiceImpl::shutdown()
{
    setAutoRefreshEnabled(false);

    if (m_refreshThread.isRunning()) {
        m_refreshThread.quit();
        m_refreshThread.wait(3000);
    }

    delete m_refreshTimer;
    m_refreshTimer = nullptr;

    m_initialized = false;
}

bool MotionControlServiceImpl::isReady() const
{
    return m_initialized && motion() != nullptr;
}

std::shared_ptr<common::device::MotionController> MotionControlServiceImpl::motion() const
{
    const auto dm = tryGetDeviceManager();
    return dm ? dm->MotionHandler() : nullptr;
}

// =============================================================================
// 目录构建
// =============================================================================

void MotionControlServiceImpl::rebuildCatalog()
{
    const auto motionCtrl = motion();
    if (!motionCtrl)
        return;

    AxisModuleMap axesByModule;
    IoModuleMap   iosByModule;
    QStringList   axisNames;
    QStringList   ioNames;

    for (const auto &axisName : motionCtrl->getAllAxisName()) {
        const QString qName = toQString(axisName);
        axisNames.append(qName);

        const auto axisData = motionCtrl->getAxisParams(axisName);
        const QString mod   = moduleKey(axisData.m_moduleName);
        axesByModule[mod].append(qName);
    }

    for (const auto &ioName : motionCtrl->getAllIoName()) {
        const QString qName = toQString(ioName);
        ioNames.append(qName);

        const auto ioData = motionCtrl->getDeviceIoData(ioName);
        const QString mod = moduleKey(ioData.m_moduleName);
        iosByModule[mod].append(qName);
    }

    for (auto it = axesByModule.begin(); it != axesByModule.end(); ++it)
        it.value().sort(Qt::CaseInsensitive);
    for (auto it = iosByModule.begin(); it != iosByModule.end(); ++it)
        it.value().sort(Qt::CaseInsensitive);

    axisNames.sort(Qt::CaseInsensitive);
    ioNames.sort(Qt::CaseInsensitive);

    QMutexLocker lock(&m_mutex);
    m_axesByModule = std::move(axesByModule);
    m_iosByModule  = std::move(iosByModule);
    m_axisNames    = std::move(axisNames);
    m_ioNames      = std::move(ioNames);
}

AxisModuleMap MotionControlServiceImpl::axesByModule() const
{
    QMutexLocker lock(&m_mutex);
    return m_axesByModule;
}

IoModuleMap MotionControlServiceImpl::iosByModule() const
{
    QMutexLocker lock(&m_mutex);
    return m_iosByModule;
}

QStringList MotionControlServiceImpl::axisNames() const
{
    QMutexLocker lock(&m_mutex);
    return m_axisNames;
}

QStringList MotionControlServiceImpl::ioNames() const
{
    QMutexLocker lock(&m_mutex);
    return m_ioNames;
}

// =============================================================================
// 运动控制
// =============================================================================

bool MotionControlServiceImpl::startJog(const QString &axisName, int direction, double jogVelRatio)
{
    const auto motionCtrl = motion();
    if (!motionCtrl) {
        emit motionError(QStringLiteral("Motion controller unavailable"));
        return false;
    }

    const std::string name = axisName.toStdString();
    double            ratio = jogVelRatio;
    if (ratio < 0.001)
        ratio = 0.001;
    if (ratio > 0.8)
        ratio = 0.8;

    const auto err =
        motionCtrl->AxisVmove(name, static_cast<uint16_t>(direction > 0 ? 1 : 0), ratio);
    if (err) {
        emit motionError(QStringLiteral("Jog start failed: %1").arg(toQString(err->Message())));
        return false;
    }
    return true;
}

bool MotionControlServiceImpl::stopJog(const QString &axisName)
{
    const auto motionCtrl = motion();
    if (!motionCtrl)
        return false;

    const auto err = motionCtrl->AxisStop(axisName.toStdString(), 1);
    if (err) {
        emit motionError(QStringLiteral("Jog stop failed: %1").arg(toQString(err->Message())));
        return false;
    }
    return true;
}

bool MotionControlServiceImpl::moveToPosition(const QString &axisName, double position, bool absolute,
                                              double speed)
{
    const auto motionCtrl = motion();
    if (!motionCtrl) {
        emit motionError(QStringLiteral("Motion controller unavailable"));
        return false;
    }

    common::device::motion::AxisMoveParams params;
    params.dist     = position;
    params.posiMode = absolute ? 1u : 0u;
    params.speed    = speed;
    params.wait     = true; // 阻塞至到位，供工艺 Step 同步等待

    const auto err = motionCtrl->AxisMove(axisName.toStdString(), params);
    if (err) {
        emit motionError(QStringLiteral("Move failed: %1").arg(toQString(err->Message())));
        return false;
    }
    return true;
}

// =============================================================================
// IO
// =============================================================================

bool MotionControlServiceImpl::writeIo(const QString &ioName, bool value)
{
    const auto motionCtrl = motion();
    if (!motionCtrl) {
        emit motionError(QStringLiteral("Motion controller unavailable"));
        return false;
    }

    const auto err = motionCtrl->WriteIO(ioName.toStdString(), value);
    if (err) {
        emit motionError(QStringLiteral("Write IO failed: %1").arg(toQString(err->Message())));
        return false;
    }

    // 写成功后乐观更新缓存，避免 UI 等待下一轮 poll
    {
        QMutexLocker lock(&m_mutex);
        if (m_ioStates.contains(ioName)) {
            m_ioStates[ioName].value = value;
        }
    }
    emit ioStatesUpdated();
    return true;
}

bool MotionControlServiceImpl::readIo(const QString &ioName, bool *valueOut) const
{
    if (!valueOut)
        return false;

    QMutexLocker lock(&m_mutex);
    if (!m_ioStates.contains(ioName))
        return false;
    *valueOut = m_ioStates.value(ioName).value;
    return true;
}

// =============================================================================
// 自动刷新
// =============================================================================

void MotionControlServiceImpl::setAutoRefreshEnabled(bool enabled)
{
    m_autoRefreshEnabled.store(enabled);
    if (!m_refreshTimer)
        return;

    // 启停须在 timer 所在线程执行，避免跨线程直接 start/stop QTimer
    if (enabled) {
        QMetaObject::invokeMethod(m_refreshTimer, [this]() {
            m_refreshTimer->setInterval(m_refreshIntervalMs.load());
            if (!m_refreshTimer->isActive())
                m_refreshTimer->start();
        }, Qt::QueuedConnection);
    } else {
        QMetaObject::invokeMethod(m_refreshTimer, [this]() { m_refreshTimer->stop(); },
                                  Qt::QueuedConnection);
    }
}

bool MotionControlServiceImpl::isAutoRefreshEnabled() const
{
    return m_autoRefreshEnabled.load();
}

void MotionControlServiceImpl::setRefreshIntervalMs(int intervalMs)
{
    const int clamped = qMax(50, intervalMs);
    m_refreshIntervalMs.store(clamped);
    if (m_refreshTimer) {
        QMetaObject::invokeMethod(m_refreshTimer, [this, clamped]() {
            m_refreshTimer->setInterval(clamped);
        }, Qt::QueuedConnection);
    }
}

int MotionControlServiceImpl::refreshIntervalMs() const
{
    return m_refreshIntervalMs.load();
}

// =============================================================================
// 状态轮询（硬件读无锁；写缓存短临界区）
// =============================================================================

void MotionControlServiceImpl::pollAxisStatesFromHardware()
{
    QStringList axisNames;
    {
        QMutexLocker lock(&m_mutex);
        axisNames = m_axisNames;
    }

    QHash<QString, AxisRuntimeState> states =
        fetchAxisStatesFromHardware(motion(), axisNames);

    QMutexLocker lock(&m_mutex);
    m_axisStates = std::move(states);
}

void MotionControlServiceImpl::pollIoStatesFromHardware()
{
    QStringList ioNames;
    {
        QMutexLocker lock(&m_mutex);
        ioNames = m_ioNames;
    }

    QHash<QString, IoRuntimeState> states = fetchIoStatesFromHardware(motion(), ioNames);

    QMutexLocker lock(&m_mutex);
    m_ioStates = std::move(states);
}

void MotionControlServiceImpl::pollRuntimeStatesFromHardware()
{
    QStringList axisNames;
    QStringList ioNames;
    {
        QMutexLocker lock(&m_mutex);
        axisNames = m_axisNames;
        ioNames   = m_ioNames;
    }

    const auto                       motionCtrl = motion();
    QHash<QString, AxisRuntimeState> axisStates = fetchAxisStatesFromHardware(motionCtrl, axisNames);
    QHash<QString, IoRuntimeState>   ioStates   = fetchIoStatesFromHardware(motionCtrl, ioNames);

    QMutexLocker lock(&m_mutex);
    m_axisStates = std::move(axisStates);
    m_ioStates   = std::move(ioStates);
}

void MotionControlServiceImpl::refreshAxisStates()
{
    pollAxisStatesFromHardware();
    emit axisStatesUpdated();
}

void MotionControlServiceImpl::refreshIoStates()
{
    pollIoStatesFromHardware();
    emit ioStatesUpdated();
}

void MotionControlServiceImpl::refreshAllStates()
{
    pollRuntimeStatesFromHardware();
    emit axisStatesUpdated();
    emit ioStatesUpdated();
}

void MotionControlServiceImpl::emitStatesUpdated()
{
    emit axisStatesUpdated();
    emit ioStatesUpdated();
}

QHash<QString, AxisRuntimeState> MotionControlServiceImpl::axisStates() const
{
    QMutexLocker lock(&m_mutex);
    return m_axisStates;
}

QHash<QString, IoRuntimeState> MotionControlServiceImpl::ioStates() const
{
    QMutexLocker lock(&m_mutex);
    return m_ioStates;
}

AxisRuntimeState MotionControlServiceImpl::axisState(const QString &axisName) const
{
    QMutexLocker lock(&m_mutex);
    return m_axisStates.value(axisName);
}

IoRuntimeState MotionControlServiceImpl::ioState(const QString &ioName) const
{
    QMutexLocker lock(&m_mutex);
    return m_ioStates.value(ioName);
}

// =============================================================================
// 轴配置：底层 HomeMode 与 AxisConfigData 字符串互转
// =============================================================================

namespace {

QString homeModeToQString(common::device::axis::HomeModeType mode)
{
    switch (mode) {
    case common::device::axis::HomeModeType::Origin_FindHome:
        return QStringLiteral("Origin_FindHome");
    case common::device::axis::HomeModeType::Z_FindHome:
        return QStringLiteral("Z_FindHome");
    case common::device::axis::HomeModeType::Limit_FindHome:
        return QStringLiteral("Limit_FindHome");
    case common::device::axis::HomeModeType::Origin_Add_Z_FindHome:
        return QStringLiteral("Origin_Add_Z_FindHome");
    case common::device::axis::HomeModeType::Limit_Add_Z_FindHome:
        return QStringLiteral("Limit_Add_Z_FindHome");
    case common::device::axis::HomeModeType::Current_Position:
        return QStringLiteral("Current_Position");
    case common::device::axis::HomeModeType::Enable:
        return QStringLiteral("Enable");
    }
    return QStringLiteral("Origin_FindHome");
}

common::device::axis::HomeModeType homeModeFromQString(const QString &mode)
{
    if (mode == QStringLiteral("Z_FindHome"))
        return common::device::axis::HomeModeType::Z_FindHome;
    if (mode == QStringLiteral("Limit_FindHome"))
        return common::device::axis::HomeModeType::Limit_FindHome;
    if (mode == QStringLiteral("Origin_Add_Z_FindHome"))
        return common::device::axis::HomeModeType::Origin_Add_Z_FindHome;
    if (mode == QStringLiteral("Limit_Add_Z_FindHome"))
        return common::device::axis::HomeModeType::Limit_Add_Z_FindHome;
    if (mode == QStringLiteral("Current_Position"))
        return common::device::axis::HomeModeType::Current_Position;
    if (mode == QStringLiteral("Enable"))
        return common::device::axis::HomeModeType::Enable;
    return common::device::axis::HomeModeType::Origin_FindHome;
}

/** @brief MotionController 轴/运动参数 → 插件 AxisConfigData（供参数页绑定） */
AxisConfigData toAxisConfig(
    const QString &axisName, const common::device::axis::Axis::AxisData &axisData,
    const common::device::axis::Axis::MoveParameters &moveData)
{
    AxisConfigData cfg;
    cfg.name                = axisName;
    cfg.moduleName          = toQString(axisData.m_moduleName);
    cfg.cardName            = toQString(axisData.m_cardName);
    cfg.enabled             = axisData.m_enable;
    cfg.axisNo              = axisData.m_axisNO;
    cfg.axisType            = axisData.m_axisType;
    cfg.unitType            = axisData.m_unitType;
    cfg.encoderType         = axisData.m_encoderType;
    cfg.driverType          = axisData.m_driverType;
    cfg.homeDir             = static_cast<int>(axisData.m_homeDir);
    cfg.homeEdgeType        = static_cast<int>(axisData.m_homeEdgeType);
    cfg.homeVelMode         = static_cast<int>(axisData.m_homeVelMode);
    cfg.homePinLogic        = axisData.m_homePinLogic;
    cfg.homeSourceSel       = axisData.m_homeSourceSel;
    cfg.homeMode            = homeModeToQString(axisData.m_homeMode);
    cfg.homeOffset          = axisData.m_homeOffset;
    cfg.homeHighVel         = axisData.m_homeHighVel;
    cfg.homeLowVel          = axisData.m_homeLowVel;
    cfg.homeTacc            = axisData.m_homeTacc;
    cfg.pulseUnit           = axisData.m_pulseUnit;
    cfg.pulseEquivalent     = axisData.m_equiv;
    cfg.negativeLimit       = axisData.m_negativeLimit;
    cfg.positiveLimit       = axisData.m_positiveLimit;
    cfg.errorBandPluse      = axisData.m_errorBandPluse;
    cfg.sourceSelLimit      = axisData.m_sourceSelLimit;
    cfg.softwareLimitAction = axisData.m_softwareLimitAction;
    cfg.startVel            = moveData.m_startVel;
    cfg.stopVel             = moveData.m_stopVel;
    cfg.maxVel              = moveData.m_maxVel;
    cfg.tacc                = moveData.m_tacc;
    cfg.tdec                = moveData.m_tdec;
    cfg.sPara               = moveData.m_sPara;
    return cfg;
}

/** @brief AxisConfigData → 写回 MotionController 内存结构（不含 saveDeviceConfig） */
void applyAxisConfig(
    const AxisConfigData &config, common::device::axis::Axis::AxisData &axisData,
    common::device::axis::Axis::MoveParameters &moveData)
{
    axisData.m_enable              = config.enabled;
    axisData.m_axisNO              = config.axisNo;
    axisData.m_axisType            = config.axisType;
    axisData.m_unitType            = config.unitType;
    axisData.m_encoderType         = config.encoderType;
    axisData.m_driverType          = config.driverType;
    axisData.m_cardName            = config.cardName.toStdString();
    axisData.m_moduleName          = config.moduleName.toStdString();
    axisData.m_homeDir             = static_cast<uint32_t>(config.homeDir);
    axisData.m_homeEdgeType        = static_cast<uint32_t>(config.homeEdgeType);
    axisData.m_homeVelMode         = static_cast<uint32_t>(config.homeVelMode);
    axisData.m_homePinLogic        = config.homePinLogic;
    axisData.m_homeSourceSel       = config.homeSourceSel;
    axisData.m_homeMode            = homeModeFromQString(config.homeMode);
    axisData.m_homeOffset          = config.homeOffset;
    axisData.m_homeHighVel         = config.homeHighVel;
    axisData.m_homeLowVel          = config.homeLowVel;
    axisData.m_homeTacc            = config.homeTacc;
    axisData.m_pulseUnit           = config.pulseUnit;
    axisData.m_equiv               = config.pulseEquivalent;
    axisData.m_negativeLimit       = config.negativeLimit;
    axisData.m_positiveLimit       = config.positiveLimit;
    axisData.m_errorBandPluse      = config.errorBandPluse;
    axisData.m_sourceSelLimit      = config.sourceSelLimit;
    axisData.m_softwareLimitAction = config.softwareLimitAction;
    moveData.m_startVel            = config.startVel;
    moveData.m_stopVel             = config.stopVel;
    moveData.m_maxVel              = config.maxVel;
    moveData.m_tacc                = config.tacc;
    moveData.m_tdec                = config.tdec;
    moveData.m_sPara               = config.sPara;
}

} // namespace

// =============================================================================
// 轴配置与维护指令
// =============================================================================

AxisConfigData MotionControlServiceImpl::axisConfig(const QString &axisName) const
{
    const auto motionCtrl = motion();
    if (!motionCtrl)
        return {};

    const auto axisData = motionCtrl->getAxisParams(axisName.toStdString());
    const auto moveData = motionCtrl->getAxisMoveParams(axisName.toStdString());
    return toAxisConfig(axisName, axisData, moveData);
}

bool MotionControlServiceImpl::setAxisConfig(const QString &axisName, const AxisConfigData &config)
{
    const auto motionCtrl = motion();
    if (!motionCtrl)
        return false;

    auto axisData = motionCtrl->getAxisParams(axisName.toStdString());
    auto moveData = motionCtrl->getAxisMoveParams(axisName.toStdString());
    applyAxisConfig(config, axisData, moveData);
    motionCtrl->setAxisParams(axisName.toStdString(), axisData);
    motionCtrl->setAxisMoveParams(axisName.toStdString(), moveData);
    rebuildCatalog();
    refreshAxisStates();
    return true;
}

bool MotionControlServiceImpl::saveDeviceConfig()
{
    const auto dm = tryGetDeviceManager();
    if (!dm)
        return false;
    return dm->saveDeviceConfig();
}

bool MotionControlServiceImpl::setAxisEnabled(const QString &axisName, bool enable)
{
    const auto motionCtrl = motion();
    if (!motionCtrl)
        return false;
    return motionCtrl->AxisEnable(axisName.toStdString(), enable) == nullptr;
}

bool MotionControlServiceImpl::homeAxis(const QString &axisName, bool async)
{
    const auto motionCtrl = motion();
    if (!motionCtrl)
        return false;

    if (!async) {
        const auto err = motionCtrl->AxisReset(axisName.toStdString());
        if (err) {
            emit motionError(QStringLiteral("Home failed: %1").arg(toQString(err->Message())));
            return false;
        }
        refreshAxisStates();
        return true;
    }

    {
        QMutexLocker lock(&m_mutex);
        if (m_homingAxes.contains(axisName))
            return false;
        m_homingAxes.insert(axisName);
    }

    const std::string                                       axisStd = axisName.toStdString();
    const std::shared_ptr<common::device::MotionController> ctrl    = motionCtrl;
    QPointer<MotionControlServiceImpl>                      self(this);

    // AxisReset 阻塞直至完成或超时，须在 UI 线程外执行
    std::thread([self, ctrl, axisStd, axisName]() {
        QString errMsg;
        bool    ok = false;
        if (ctrl) {
            const auto err = ctrl->AxisReset(axisStd);
            ok             = err == nullptr;
            if (!ok && err)
                errMsg = toQString(err->Message());
        }
        if (!self)
            return;
        QMetaObject::invokeMethod(self, "finishHomeAxis", Qt::QueuedConnection,
                                  Q_ARG(QString, axisName), Q_ARG(bool, ok), Q_ARG(QString, errMsg));
    }).detach();

    return true;
}

void MotionControlServiceImpl::finishHomeAxis(const QString &axisName, bool success,
                                              const QString &errorMessage)
{
    {
        QMutexLocker lock(&m_mutex);
        m_homingAxes.remove(axisName);
    }
    if (!success && !errorMessage.isEmpty())
        emit motionError(QStringLiteral("Home failed: %1").arg(errorMessage));
    refreshAxisStates();
    emit homeAxisFinished(axisName, success, errorMessage);
}

bool MotionControlServiceImpl::clearAxisAlarm(const QString &axisName)
{
    const auto motionCtrl = motion();
    if (!motionCtrl)
        return false;
    return motionCtrl->AxisAlarmClear(axisName.toStdString()) == nullptr;
}

bool MotionControlServiceImpl::emergencyStopAll()
{
    const auto motionCtrl = motion();
    if (!motionCtrl)
        return false;
    return motionCtrl->AxisEmgStop() == nullptr;
}

void MotionControlServiceImpl::reloadCatalog()
{
    rebuildCatalog();
    refreshAllStates();
}

QStringList MotionControlServiceImpl::motionCardNames() const
{
    const auto motionCtrl = motion();
    if (!motionCtrl)
        return {};

    QStringList names;
    for (const auto &name : motionCtrl->getAllCardName())
        names.append(toQString(name));
    names.sort(Qt::CaseInsensitive);
    return names;
}

namespace {

IoConfigData toIoConfig(const QString &ioName,
                        const common::device::deviceIO::DeviceIO::DeviceIoData &data)
{
    IoConfigData cfg;
    cfg.name       = ioName;
    cfg.moduleName = toQString(data.m_moduleName);
    cfg.cardName   = toQString(data.m_cardName);
    cfg.isOutput   = data.m_type == 1;
    cfg.port       = data.m_port;
    cfg.logic      = data.m_logic;
    cfg.extend     = data.m_extend;
    cfg.node       = data.m_node;
    return cfg;
}

void applyIoConfig(const IoConfigData &config, common::device::deviceIO::DeviceIO::DeviceIoData &data)
{
    data.m_moduleName = config.moduleName.toStdString();
    data.m_cardName   = config.cardName.toStdString();
    data.m_type       = config.isOutput ? 1 : 0;
    data.m_port       = config.port;
    data.m_logic      = config.logic;
    data.m_extend     = config.extend;
    data.m_node       = static_cast<uint16_t>(config.node);
}

} // namespace

IoConfigData MotionControlServiceImpl::ioConfig(const QString &ioName) const
{
    const auto motionCtrl = motion();
    if (!motionCtrl)
        return {};

    const auto data = motionCtrl->getDeviceIoData(ioName.toStdString());
    return toIoConfig(ioName, data);
}

bool MotionControlServiceImpl::setIoConfig(const QString &ioName, const IoConfigData &config)
{
    const auto motionCtrl = motion();
    if (!motionCtrl)
        return false;

    auto data = motionCtrl->getDeviceIoData(ioName.toStdString());
    applyIoConfig(config, data);
    motionCtrl->setDeviceIoData(ioName.toStdString(), data);
    rebuildCatalog();
    refreshIoStates();
    return true;
}

bool MotionControlServiceImpl::addAxis(const QString &axisName, const QString &moduleName,
                                       QString *errorKeyOut)
{
    const QString name = axisName.trimmed();
    if (name.isEmpty()) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.axis.add.error.empty_name");
        return false;
    }

    {
        QMutexLocker lock(&m_mutex);
        if (m_axisNames.contains(name, Qt::CaseInsensitive)) {
            if (errorKeyOut)
                *errorKeyOut = QStringLiteral("motion.axis.add.error.duplicate");
            return false;
        }
    }

    const QStringList cards = motionCardNames();
    if (cards.isEmpty()) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.axis.add.error.no_card");
        return false;
    }
    const QString cardName = cards.first();

    const auto dm = tryGetDeviceManager();
    if (!dm) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.axis.add.error.unavailable");
        return false;
    }

    dm->addAxis(name.toStdString(), cardName.toStdString());

    const auto motionCtrl = motion();
    if (!motionCtrl) {
        dm->removeAxis(name.toStdString());
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.axis.add.error.unavailable");
        return false;
    }

    bool axisExists = false;
    for (const auto &n : motionCtrl->getAllAxisName()) {
        if (toQString(n).compare(name, Qt::CaseInsensitive) == 0) {
            axisExists = true;
            break;
        }
    }
    if (!axisExists) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.axis.add.error.failed");
        return false;
    }

    AxisConfigData cfg = axisConfig(name);
    cfg.name       = name;
    cfg.moduleName = moduleName.trimmed();
    cfg.cardName   = cardName;
    if (!setAxisConfig(name, cfg)) {
        dm->removeAxis(name.toStdString());
        rebuildCatalog();
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.axis.add.error.failed");
        return false;
    }

    if (!saveDeviceConfig()) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.axis.add.error.save_failed");
        return false;
    }

    emit catalogChanged();
    return true;
}

bool MotionControlServiceImpl::removeAxis(const QString &axisName, QString *errorKeyOut)
{
    const QString name = axisName.trimmed();
    if (name.isEmpty()) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.axis.delete.error.empty_name");
        return false;
    }

    {
        QMutexLocker lock(&m_mutex);
        if (!m_axisNames.contains(name, Qt::CaseInsensitive)) {
            if (errorKeyOut)
                *errorKeyOut = QStringLiteral("motion.axis.delete.error.not_found");
            return false;
        }
    }

    const auto dm = tryGetDeviceManager();
    if (!dm) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.axis.delete.error.unavailable");
        return false;
    }

    dm->removeAxis(name.toStdString());
    rebuildCatalog();
    refreshAxisStates();

    if (!saveDeviceConfig()) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.axis.delete.error.save_failed");
        return false;
    }

    emit catalogChanged();
    return true;
}

bool MotionControlServiceImpl::addIo(const QString &ioName, const QString &moduleName,
                                     QString *errorKeyOut)
{
    const QString name = ioName.trimmed();
    if (name.isEmpty()) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.io.add.error.empty_name");
        return false;
    }

    {
        QMutexLocker lock(&m_mutex);
        if (m_ioNames.contains(name, Qt::CaseInsensitive)) {
            if (errorKeyOut)
                *errorKeyOut = QStringLiteral("motion.io.add.error.duplicate");
            return false;
        }
    }

    const QStringList cards = motionCardNames();
    if (cards.isEmpty()) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.io.add.error.no_card");
        return false;
    }
    const QString cardName = cards.first();

    const auto dm = tryGetDeviceManager();
    if (!dm) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.io.add.error.unavailable");
        return false;
    }

    dm->addDeviceIO(name.toStdString(), cardName.toStdString());

    const auto motionCtrl = motion();
    if (!motionCtrl) {
        dm->removeDeviceIO(name.toStdString());
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.io.add.error.unavailable");
        return false;
    }

    bool ioExists = false;
    for (const auto &n : motionCtrl->getAllIoName()) {
        if (toQString(n).compare(name, Qt::CaseInsensitive) == 0) {
            ioExists = true;
            break;
        }
    }
    if (!ioExists) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.io.add.error.failed");
        return false;
    }

    IoConfigData cfg;
    cfg.name       = name;
    cfg.moduleName = moduleName.trimmed();
    cfg.cardName   = cardName;
    cfg.isOutput   = false;
    if (!setIoConfig(name, cfg)) {
        dm->removeDeviceIO(name.toStdString());
        rebuildCatalog();
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.io.add.error.failed");
        return false;
    }

    if (!saveDeviceConfig()) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.io.add.error.save_failed");
        return false;
    }

    emit catalogChanged();
    return true;
}

bool MotionControlServiceImpl::removeIo(const QString &ioName, QString *errorKeyOut)
{
    const QString name = ioName.trimmed();
    if (name.isEmpty()) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.io.delete.error.empty_name");
        return false;
    }

    {
        QMutexLocker lock(&m_mutex);
        if (!m_ioNames.contains(name, Qt::CaseInsensitive)) {
            if (errorKeyOut)
                *errorKeyOut = QStringLiteral("motion.io.delete.error.not_found");
            return false;
        }
    }

    const auto dm = tryGetDeviceManager();
    if (!dm) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.io.delete.error.unavailable");
        return false;
    }

    dm->removeDeviceIO(name.toStdString());
    rebuildCatalog();
    refreshIoStates();

    if (!saveDeviceConfig()) {
        if (errorKeyOut)
            *errorKeyOut = QStringLiteral("motion.io.delete.error.save_failed");
        return false;
    }

    emit catalogChanged();
    return true;
}

} // namespace motion_control_plugin
