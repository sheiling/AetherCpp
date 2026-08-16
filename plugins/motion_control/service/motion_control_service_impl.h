#pragma once

/**
 * @file motion_control_service_impl.h
 * @brief IMotionControlService 默认实现：MotionController 适配、状态缓存与后台轮询
 *
 * 架构要点：
 *   - motion() 每次经 Container::Application 解析 DeviceManager::MotionHandler()，不长期持有。
 *   - m_mutex 保护目录与运行时缓存；硬件 poll 无锁读卡，写缓存时短暂持锁。
 *   - m_refreshThread + m_refreshTimer：定时 onRefreshTick，避免在 UI 线程阻塞读硬件。
 *   - m_autoRefreshEnabled / m_refreshIntervalMs 为 atomic，供跨线程开关与改间隔。
 *
 * 插件生命周期：MotionControlPlugin::delayedInitialize → initialize()；
 * aboutToShutdown → shutdown()。
 */

#include <imotion_control_service.h>

#include <QHash>
#include <QMutex>
#include <QSet>
#include <QThread>
#include <QTimer>
#include <atomic>
#include <memory>

namespace common {
namespace device {
class DeviceManager;
class MotionController;
}
} // namespace common

namespace motion_control_plugin {

/**
 * @brief IMotionControlService 的具体实现（final，插件内单例）
 *
 * initialize 成功后 isReady()==true；此前目录与 refresh 接口可能为空操作。
 * 硬件 poll 在无锁区读卡，仅写 m_axisStates / m_ioStates 时短暂持 m_mutex。
 */
class MotionControlServiceImpl final : public IMotionControlService
{
    Q_OBJECT

public:
    explicit MotionControlServiceImpl(QObject *parent = nullptr);
    ~MotionControlServiceImpl() override;

    /**
     * @brief 启动刷新线程、重建轴/IO 目录并全量 refresh
     * @param errorOut 失败时写入原因（MotionController 不可用）
     * @return 成功 true；不自动开启 autoRefresh，由 UI 按需 setAutoRefreshEnabled(true)
     */
    bool initialize(QString *errorOut = nullptr);

    /** @brief 停止自动刷新、退出刷新线程并释放 QTimer（析构与插件 shutdown 调用） */
    void shutdown();

    bool isReady() const override;

    AxisModuleMap axesByModule() const override;
    IoModuleMap   iosByModule() const override;
    QStringList   axisNames() const override;
    QStringList   ioNames() const override;

    bool startJog(const QString &axisName, int direction, double jogVelRatio) override;
    bool stopJog(const QString &axisName) override;
    bool moveToPosition(const QString &axisName, double position, bool absolute,
                        double speed = 0.0) override;

    bool writeIo(const QString &ioName, bool value) override;
    bool readIo(const QString &ioName, bool *valueOut) const override;
    IoConfigData ioConfig(const QString &ioName) const override;
    bool         setIoConfig(const QString &ioName, const IoConfigData &config) override;

    void setAutoRefreshEnabled(bool enabled) override;
    bool isAutoRefreshEnabled() const override;
    void setRefreshIntervalMs(int intervalMs) override;
    int  refreshIntervalMs() const override;

    void refreshAxisStates() override;
    void refreshIoStates() override;
    void refreshAllStates() override;

    QHash<QString, AxisRuntimeState> axisStates() const override;
    QHash<QString, IoRuntimeState>   ioStates() const override;
    AxisRuntimeState                 axisState(const QString &axisName) const override;
    IoRuntimeState                   ioState(const QString &ioName) const override;

    AxisConfigData axisConfig(const QString &axisName) const override;
    bool           setAxisConfig(const QString &axisName, const AxisConfigData &config) override;
    bool           saveDeviceConfig() override;
    bool           setAxisEnabled(const QString &axisName, bool enable) override;
    bool           homeAxis(const QString &axisName, bool async) override;
    bool           clearAxisAlarm(const QString &axisName) override;
    bool           emergencyStopAll() override;
    void           reloadCatalog() override;
    bool           addAxis(const QString &axisName, const QString &moduleName,
                             QString *errorKeyOut = nullptr) override;
    bool           removeAxis(const QString &axisName, QString *errorKeyOut = nullptr) override;
    bool           addIo(const QString &ioName, const QString &moduleName,
                         QString *errorKeyOut = nullptr) override;
    bool           removeIo(const QString &ioName, QString *errorKeyOut = nullptr) override;
    QStringList    motionCardNames() const override;

private slots:
    /** @brief 自动刷新 poll 完成后在 UI 线程 emit 状态信号 */
    void emitStatesUpdated();
    /** @brief 回零后台线程完成后回到服务对象线程收尾（刷新状态、发射 homeAxisFinished） */
    void finishHomeAxis(const QString &axisName, bool success, const QString &errorMessage);

private:
    /** @brief 解析 MotionController；DeviceManager 未绑定或异常时返回 nullptr */
    std::shared_ptr<common::device::MotionController> motion() const;

    /** @brief 从 MotionController 重建 m_axesByModule / m_iosByModule 及扁平名列表（持锁写入） */
    void rebuildCatalog();

    /**
     * @brief 读硬件刷新运行时缓存；持锁仅用于拷贝名列表与写回结果，不在持锁期间访问运控卡
     */
    void pollRuntimeStatesFromHardware();
    void pollAxisStatesFromHardware();
    void pollIoStatesFromHardware();

    /** 保护目录与运行时缓存；poll 写缓存与目录读为短临界区，不持锁调 MotionController */
    mutable QMutex m_mutex;

    AxisModuleMap m_axesByModule;
    IoModuleMap   m_iosByModule;
    QStringList   m_axisNames;
    QStringList   m_ioNames;

    QHash<QString, AxisRuntimeState> m_axisStates;
    QHash<QString, IoRuntimeState>   m_ioStates;
    QSet<QString>                    m_homingAxes; ///< 正在回零的轴名，防重复提交

    QThread              m_refreshThread;   ///< 专用线程，仅运行 m_refreshTimer
    QTimer              *m_refreshTimer = nullptr; ///< 构造时 moveToThread(&m_refreshThread)
    std::atomic<bool>    m_autoRefreshEnabled{false};
    std::atomic<int>     m_refreshIntervalMs{200}; ///< 默认 200ms，setRefreshIntervalMs 下限 50
    bool                 m_initialized = false;    ///< initialize 成功置 true
};

} // namespace motion_control_plugin
