#pragma once

#include <scheduler/machine/machine_status.h>

#include <QString>

namespace aoi::binding::fsm {

/** MachineStatus → KPI「设备状态」列 i18n key（View 层 TR 展示） */
inline QString deviceStateKey(const scheduler::MachineStatus &status)
{
    using scheduler::MachineState;
    using scheduler::MachineSubFlags;
    using scheduler::hasFlag;

    if (hasFlag(status.sub, MachineSubFlags::EmergencyStop))
        return QStringLiteral("aoi.home.machine_state.alarm");

    switch (status.main) {
    case MachineState::Running:
        return hasFlag(status.sub, MachineSubFlags::Draining)
                   ? QStringLiteral("aoi.home.machine_state.paused")
                   : QStringLiteral("aoi.home.machine_state.running");
    case MachineState::Paused:
        if (hasFlag(status.sub, MachineSubFlags::Error))
            return QStringLiteral("aoi.home.machine_state.alarm");
        return hasFlag(status.sub, MachineSubFlags::ResettingSuccess)
                   ? QStringLiteral("aoi.home.machine_state.idle")
                   : QStringLiteral("aoi.home.machine_state.paused");
    case MachineState::Alarm:
        return QStringLiteral("aoi.home.machine_state.alarm");
    case MachineState::NotReset:
    default:
        return QStringLiteral("aoi.home.machine_state.stopped");
    }
}

/** ManualMode 子标志 → KPI「运行模式」列 i18n key */
inline QString runModeKey(const scheduler::MachineStatus &status)
{
    return scheduler::hasFlag(status.sub, scheduler::MachineSubFlags::ManualMode)
               ? QStringLiteral("aoi.home.run_mode.manual")
               : QStringLiteral("aoi.home.run_mode.auto");
}

} // namespace aoi::binding::fsm
