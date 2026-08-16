#pragma once

#include <scheduler/api/ischeduler_service.h>

#include <QObject>

#include <vector>

class SchedulerRuntime;

/** ISchedulerService 的 Qt 门面，供 PluginManager::getInterface 发现 */
class SCHEDULER_EXPORT SchedulerServiceImpl final : public QObject, public ISchedulerService
{
    Q_OBJECT
    Q_INTERFACES(ISchedulerService)

public:
    explicit SchedulerServiceImpl(std::shared_ptr<SchedulerRuntime> runtime,
                                  QObject *parent = nullptr);

    bool isReady() const override;

    std::shared_ptr<scheduler::Scheduler> scheduler() const override;
    std::shared_ptr<scheduler::MachineFSM> machineFsm() const override;
    std::shared_ptr<scheduler::EventBus> eventBus() const override;
    std::shared_ptr<scheduler::AlarmManager> alarmManager() const override;

    QString flowsName() const override;
    bool areFlowsLoaded() const override;
    void addFlowsLoadedListener(FlowsLoadedListener listener) override;
    std::shared_ptr<scheduler::FlowDefinition> flowDefinition(const QString &name) const override;

    /** Bootstrap 在 loadSchedulerFlowsAfterPlugins 成功后调用 */
    void notifyFlowsLoaded();

private:
    std::shared_ptr<SchedulerRuntime> m_runtime;
    std::vector<FlowsLoadedListener>  m_flowsLoadedListeners_;
};
