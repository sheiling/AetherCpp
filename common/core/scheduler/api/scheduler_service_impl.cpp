#include "scheduler_service_impl.h"

#include "scheduler_runtime.h"

#include <utility>

SchedulerServiceImpl::SchedulerServiceImpl(std::shared_ptr<SchedulerRuntime> runtime,
                                             QObject *parent)
    : QObject(parent)
    , m_runtime(std::move(runtime))
{
}

bool SchedulerServiceImpl::isReady() const
{
    return m_runtime && m_runtime->isReady();
}

std::shared_ptr<scheduler::Scheduler> SchedulerServiceImpl::scheduler() const
{
    return m_runtime ? m_runtime->scheduler() : nullptr;
}

std::shared_ptr<scheduler::MachineFSM> SchedulerServiceImpl::machineFsm() const
{
    return m_runtime ? m_runtime->machineFsm() : nullptr;
}

std::shared_ptr<scheduler::EventBus> SchedulerServiceImpl::eventBus() const
{
    return m_runtime ? m_runtime->eventBus() : nullptr;
}

std::shared_ptr<scheduler::AlarmManager> SchedulerServiceImpl::alarmManager() const
{
    return m_runtime ? m_runtime->alarmManager() : nullptr;
}

QString SchedulerServiceImpl::flowsName() const
{
    return m_runtime ? QString::fromStdString(m_runtime->flowsName()) : QString();
}

bool SchedulerServiceImpl::areFlowsLoaded() const
{
    return m_runtime && m_runtime->areFlowsLoaded();
}

void SchedulerServiceImpl::addFlowsLoadedListener(FlowsLoadedListener listener)
{
    if (!listener)
        return;
    if (areFlowsLoaded()) {
        listener(this);
        return;
    }
    m_flowsLoadedListeners_.push_back(std::move(listener));
}

void SchedulerServiceImpl::notifyFlowsLoaded()
{
    if (!areFlowsLoaded())
        return;

    auto listeners = std::move(m_flowsLoadedListeners_);
    m_flowsLoadedListeners_.clear();
    for (auto &listener : listeners) {
        if (listener)
            listener(this);
    }
}

std::shared_ptr<scheduler::FlowDefinition> SchedulerServiceImpl::flowDefinition(
    const QString &name) const
{
    if (!m_runtime)
        return nullptr;
    return m_runtime->flowDefinition(name.toStdString());
}
