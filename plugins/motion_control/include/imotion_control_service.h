#pragma once

/**
 * @file imotion_control_service.h
 * @brief 运控服务对外接口：轴/IO 目录、点动与点位运动、IO 读写、状态缓存与自动刷新
 *
 * 由 MotionControl 插件在 initialize 阶段注册到 ExtensionSystem 对象池；
 * 消费方通过 PluginManager::getInterface<IMotionControlService>() 获取指针。
 *
 * 底层数据与指令均转发至 DeviceManager::MotionHandler()（MotionController）。
 * 运行时轴/IO 状态缓存在服务内，可由后台定时器周期性刷新，或由 UI/工艺主动 refresh。
 *
 * @note 线程约定
 *   - 运动/IO 写操作在调用线程同步执行，失败时 emit motionError。
 *   - refresh* / axisStates / ioStates 等读缓存接口持 m_mutex 拷贝快照，可跨线程调用。
 *   - axisStatesUpdated / ioStatesUpdated 在刷新完成后于服务对象所在线程 emit（通常为 UI 线程）。
 */

#include "motion_control_plugin_global.h"
#include "motion_control_types.h"

#include <QObject>

namespace motion_control_plugin {

/**
 * @brief 运控服务抽象接口（MotionControl 插件）
 *
 * 职责划分：
 *   1. 目录 — 从 MotionController 枚举轴/IO，按 moduleName 分组供 UI 树形展示。
 *   2. 控制 — 点动、绝对/相对定位、使能、回零、清报警、急停、IO 输出。
 *   3. 配置 — 读写 AxisConfigData，持久化经 saveDeviceConfig() 写回 device_config。
 *   4. 状态 — 缓存 AxisRuntimeState / IoRuntimeState，支持自动轮询与即时 refresh。
 *
 * 生命周期：插件 delayedInitialize() 调用 MotionControlServiceImpl::initialize() 后 isReady()==true；
 * aboutToShutdown() 时 shutdown() 停止刷新线程。
 */
class MOTION_CONTROL_PLUGIN_EXPORT IMotionControlService : public QObject
{
    Q_OBJECT

public:
    explicit IMotionControlService(QObject *parent = nullptr) : QObject(parent) {}
    ~IMotionControlService() override = default;

    // -------------------------------------------------------------------------
    // 就绪与目录
    // -------------------------------------------------------------------------

    /**
     * @brief 服务是否已完成 initialize 且 MotionController 可用
     * @return true 表示可安全调用运动/IO/配置接口
     */
    virtual bool isReady() const = 0;

    /** @brief 按 moduleName 分组的轴名目录（模块内名称已排序，空模块名归为 "_default" 后对外清空） */
    virtual AxisModuleMap axesByModule() const = 0;
    /** @brief 按 moduleName 分组的 IO 名目录（排序规则同 axesByModule） */
    virtual IoModuleMap iosByModule() const = 0;
    /** @brief 全部轴名扁平列表（不区分模块，大小写不敏感排序） */
    virtual QStringList axisNames() const = 0;
    /** @brief 全部 IO 名扁平列表 */
    virtual QStringList ioNames() const = 0;

    // -------------------------------------------------------------------------
    // 运动控制
    // -------------------------------------------------------------------------

    /**
     * @brief 启动轴点动
     * @param axisName 轴逻辑名（与 device_config 一致）
     * @param direction 0=负向，非 0=正向（内部映射为 MotionController::AxisVmove 方向码）
     * @param jogVelRatio 点动速度比例（0.001～0.8），传入 AxisVmove，不写入轴配置
     * @return 成功 true；失败 false 并 emit motionError
     */
    virtual bool startJog(const QString &axisName, int direction, double jogVelRatio) = 0;

    /** @brief 停止指定轴点动（AxisStop，减速度模式 1） */
    virtual bool stopJog(const QString &axisName) = 0;

    /**
     * @brief 点位运动（阻塞至到位或失败，params.wait=true）
     * @param position 目标位置或相对位移（由 absolute 决定）
     * @param absolute true=绝对定位，false=相对定位
     * @param speed 速度；≤0 时由 MotionController 使用轴默认速度
     */
    virtual bool moveToPosition(const QString &axisName, double position, bool absolute,
                                double speed = 0.0) = 0;

    // -------------------------------------------------------------------------
    // IO
    // -------------------------------------------------------------------------

    /**
     * @brief 写数字 IO 输出
     * @note 成功后立即更新本地缓存并 emit ioStatesUpdated（不等待下次轮询）
     */
    virtual bool writeIo(const QString &ioName, bool value) = 0;

    /**
     * @brief 从本地缓存读取 IO 状态（非实时读硬件）
     * @param valueOut 非空时写入缓存值
     * @return IO 存在于目录且 valueOut 非空时 true
     */
    virtual bool readIo(const QString &ioName, bool *valueOut) const = 0;

    /** @brief 读取 IO 静态参数，组装为 IoConfigData */
    virtual IoConfigData ioConfig(const QString &ioName) const = 0;

    /**
     * @brief 写回 IO 参数并重建目录、刷新 IO 状态
     * @note 仅更新内存与 MotionController；持久化需另调 saveDeviceConfig()
     */
    virtual bool setIoConfig(const QString &ioName, const IoConfigData &config) = 0;

    // -------------------------------------------------------------------------
    // 状态刷新
    // -------------------------------------------------------------------------

    /** @brief 开关后台自动轮询；开启后在独立 QThread 上 QTimer 触发 refreshAllStates */
    virtual void setAutoRefreshEnabled(bool enabled) = 0;
    virtual bool isAutoRefreshEnabled() const = 0;

    /** @brief 轮询间隔（毫秒），实现侧下限 50ms */
    virtual void setRefreshIntervalMs(int intervalMs) = 0;
    virtual int  refreshIntervalMs() const = 0;

    /** @brief 在当前调用线程同步读硬件并更新轴缓存，完成后 emit axisStatesUpdated */
    virtual void refreshAxisStates() = 0;
    /** @brief 在当前调用线程同步读硬件并更新 IO 缓存，完成后 emit ioStatesUpdated */
    virtual void refreshIoStates() = 0;
    /** @brief 刷新轴与 IO 缓存，依次 emit axisStatesUpdated、ioStatesUpdated */
    virtual void refreshAllStates() = 0;

    /** @brief 轴运行时状态快照（键为轴名） */
    virtual QHash<QString, AxisRuntimeState> axisStates() const = 0;
    /** @brief IO 运行时状态快照（键为 IO 名） */
    virtual QHash<QString, IoRuntimeState>   ioStates() const = 0;
    /** @brief 单轴状态；不存在时返回默认构造 AxisRuntimeState */
    virtual AxisRuntimeState                 axisState(const QString &axisName) const = 0;
    /** @brief 单路 IO 状态；不存在时返回默认构造 IoRuntimeState */
    virtual IoRuntimeState                   ioState(const QString &ioName) const = 0;

    // -------------------------------------------------------------------------
    // 轴配置与维护
    // -------------------------------------------------------------------------

    /** @brief 读取轴静态参数与运动参数，组装为 AxisConfigData */
    virtual AxisConfigData axisConfig(const QString &axisName) const = 0;

    /**
     * @brief 写回轴参数并重建目录、刷新轴状态
     * @note 仅更新内存与 MotionController；持久化需另调 saveDeviceConfig()
     */
    virtual bool           setAxisConfig(const QString &axisName, const AxisConfigData &config) = 0;

    /** @brief 将 DeviceManager 当前设备配置写入磁盘（device_config.json 等） */
    virtual bool           saveDeviceConfig() = 0;

    virtual bool           setAxisEnabled(const QString &axisName, bool enable) = 0;
    /**
     * @brief 轴回零（MotionController::AxisReset）
     * @param async false（默认）同步阻塞直至完成；true 后台线程执行，完成时发射 homeAxisFinished
     * @return 同步：回零成功 true；异步：任务已提交 true（同一轴重复提交返回 false）
     */
    virtual bool           homeAxis(const QString &axisName, bool async = false) = 0;
    virtual bool           clearAxisAlarm(const QString &axisName) = 0;
    /** @brief 全轴急停（AxisEmgStop） */
    virtual bool           emergencyStopAll() = 0;

    /** @brief 重新从 MotionController 枚举轴/IO 并 refreshAllStates（配置热加载后调用） */
    virtual void           reloadCatalog() = 0;

    /**
     * @brief 新增轴（写入 MotionController 并 saveDeviceConfig）
     * @param moduleName 模块分组名（UI 导航用）
     */
    virtual bool addAxis(const QString &axisName, const QString &moduleName,
                         QString *errorKeyOut = nullptr) = 0;

    /** @brief 删除轴并 saveDeviceConfig */
    virtual bool removeAxis(const QString &axisName, QString *errorKeyOut = nullptr) = 0;

    /**
     * @brief 新增 IO（默认 DI；写入 MotionController 并 saveDeviceConfig）
     * @param moduleName 模块分组名
     */
    virtual bool addIo(const QString &ioName, const QString &moduleName,
                       QString *errorKeyOut = nullptr) = 0;

    /** @brief 删除 IO 并 saveDeviceConfig */
    virtual bool removeIo(const QString &ioName, QString *errorKeyOut = nullptr) = 0;

    /** @brief 运控卡名称列表（DeviceManager MotionController Card 键名） */
    virtual QStringList    motionCardNames() const = 0;

signals:
    /** @brief 轴/IO 目录结构变更（增删轴 IO 后） */
    void catalogChanged();
    /** @brief 轴缓存已更新（自动刷新、refreshAxisStates、setAxisConfig 等路径） */
    void axisStatesUpdated();
    /** @brief IO 缓存已更新 */
    void ioStatesUpdated();
    /** @brief 运动/IO 操作失败时的可读错误信息（英文前缀 + 底层 Message） */
    void motionError(const QString &message);
    /** @brief 轴回零完成（homeAxis 异步任务结束；失败时 errorMessage 为底层原因） */
    void homeAxisFinished(const QString &axisName, bool success, const QString &errorMessage);
};

} // namespace motion_control_plugin
