#pragma once

#include <scheduler/scheduler_global.h>

#include <QtPlugin>

#include <functional>
#include <memory>
#include <QString>

namespace scheduler {
class AlarmManager;
class EventBus;
struct FlowDefinition;
class MachineFSM;
class Scheduler;
} // namespace scheduler

/**
 * @brief 调度引擎服务接口（Bootstrap 装配，经 PluginManager / Container 暴露）
 *
 * 插件侧获取：PluginManager::getInterface<ISchedulerService>()
 */
class SCHEDULER_EXPORT ISchedulerService
{
public:
    virtual ~ISchedulerService() = default;

    virtual bool isReady() const = 0;

    virtual std::shared_ptr<scheduler::Scheduler> scheduler() const       = 0;
    virtual std::shared_ptr<scheduler::MachineFSM> machineFsm() const     = 0;
    virtual std::shared_ptr<scheduler::EventBus> eventBus() const         = 0;
    virtual std::shared_ptr<scheduler::AlarmManager> alarmManager() const = 0;

    /** 默认产线流程名（app.toml scheduler.flowsName） */
    virtual QString flowsName() const = 0;

    /** 流程 JSON 是否已在插件加载后完成注册（见 loadFlowDefinitions） */
    virtual bool areFlowsLoaded() const = 0;

    /**
     * 流程定义加载完成时回调（若已加载则立即调用）。
     * 插件在 extensionsInitialized 注册，用于绑定 FlowDefinition 生命周期钩子。
     */
    using FlowsLoadedListener = std::function<void(ISchedulerService *)>;
    virtual void addFlowsLoadedListener(FlowsLoadedListener listener) = 0;

    virtual std::shared_ptr<scheduler::FlowDefinition> flowDefinition(
        const QString &name) const = 0;
};

Q_DECLARE_INTERFACE(ISchedulerService, "com.ess.scheduler.ISchedulerService/1.0")
