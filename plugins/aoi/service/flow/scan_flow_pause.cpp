/**
 * @file scan_flow_pause.cpp
 * @brief waitWhileMachinePaused：轮询 MachineFSM 主态直至 Running 或终止
 */

#include "scan_flow_pause.h"

#include "extensionsystem/pluginmanager.h"
#include <scheduler/api/ischeduler_service.h>
#include <scheduler/machine/machine_fsm.h>
#include <scheduler/machine/machine_status.h>

#include <QThread>

namespace aoi::flow {

bool waitWhileMachinePaused(const std::shared_ptr<inspection::InspectionSession> &session)
{
    if (!session)
        return false;

    auto *svc = ExtensionSystem::PluginManager::getInterface<ISchedulerService>();
    if (!svc || !svc->isReady())
        return true;

    const auto fsm = svc->machineFsm();
    if (!fsm)
        return true;

    while (true) {
        if (session->isAbortRequested())
            return false;

        const scheduler::MachineStatus st = fsm->currentStatus();
        if (st.main == scheduler::MachineState::Running)
            return true;
        if (st.main == scheduler::MachineState::Paused) {
            QThread::msleep(20);
            continue;
        }
        return false;
    }
}

} // namespace aoi::flow
