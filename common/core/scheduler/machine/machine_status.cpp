#include "machine_status.h"

namespace scheduler {

namespace {

/// 已完成至少一次复位（非 NotReset / 非复位中）
bool hasResetSucceeded(const MachineStatus& st)
{
    return st.main != MachineState::NotReset && !isResetting(st.sub);
}

/// 主态 Alarm 或 Paused 挂 Error
bool isAlarmPending(const MachineStatus& st)
{
    return st.main == MachineState::Alarm
        || hasFlag(st.sub, MachineSubFlags::Error);
}

} // namespace

// 守卫条件与 §3.1.3 一致；UI canExecute 与 MachineFSM::handleEvent 共用

bool canStart(const MachineStatus& st)
{
    if (st.main != MachineState::Paused)
        return false;
    if (hasFlag(st.sub, MachineSubFlags::ManualMode))
        return false;
    if (hasFlag(st.sub, MachineSubFlags::EmergencyStop))
        return false;
    if (hasFlag(st.sub, MachineSubFlags::Error))
        return false;
    if (hasFlag(st.sub, MachineSubFlags::Resetting))
        return false;
    if (hasFlag(st.sub, MachineSubFlags::Draining))
        return false;
    return true;
}

bool canStop(const MachineStatus& st)
{
    if (isResetting(st.sub))
        return true;
    return st.main == MachineState::Running;
}

bool canReset(const MachineStatus& st)
{
    if (isResetting(st.sub))
        return false;
    return st.main != MachineState::Running;
}

bool canClearAlarm(const MachineStatus& st)
{
    if (st.main == MachineState::Alarm)
        return true;
    return st.main == MachineState::Paused
        && hasFlag(st.sub, MachineSubFlags::Error);
}

bool canClearMaterial(const MachineStatus& st)
{
    if (st.main != MachineState::Paused)
        return false;
    if (isResetting(st.sub))
        return false;
    if (!hasFlag(st.sub, MachineSubFlags::ResettingSuccess))
        return false;
    if (hasFlag(st.sub, MachineSubFlags::EmergencyStop))
        return false;
    if (isAlarmPending(st))
        return false;
    return true;
}

bool canSingleInspect(const MachineStatus& st)
{
    if (!hasResetSucceeded(st))
        return false;
    if (isAlarmPending(st))
        return false;
    if (st.main == MachineState::Running)
        return false;
    return true;
}

const char* machineStateStr(MachineState s)
{
    switch (s) {
    case MachineState::NotReset: return "NotReset";
    case MachineState::Paused:   return "Paused";
    case MachineState::Running:  return "Running";
    case MachineState::Alarm:    return "Alarm";
    default:                     return "Unknown";
    }
}

const char* machineSubFlagStr(MachineSubFlags bit)
{
    switch (bit) {
    case MachineSubFlags::Resetting:        return "Resetting";
    case MachineSubFlags::ResettingSuccess: return "ResettingSuccess";
    case MachineSubFlags::ManualMode:       return "ManualMode";
    case MachineSubFlags::EmergencyStop:    return "EmergencyStop";
    case MachineSubFlags::Draining:         return "Draining";
    case MachineSubFlags::Error:            return "Error";
    default:                                return nullptr;
    }
}

std::string machineSubFlagsStr(MachineSubFlags flags)
{
    if (flags == MachineSubFlags::None)
        return "None";

    std::string out;
    const MachineSubFlags bits[] = {
        MachineSubFlags::Resetting,
        MachineSubFlags::ResettingSuccess,
        MachineSubFlags::ManualMode,
        MachineSubFlags::EmergencyStop,
        MachineSubFlags::Draining,
        MachineSubFlags::Error,
    };
    for (MachineSubFlags bit : bits) {
        if (!hasFlag(flags, bit))
            continue;
        if (!out.empty())
            out += '|';
        if (const char* name = machineSubFlagStr(bit))
            out += name;
    }
    return out.empty() ? "None" : out;
}

std::string statusToString(const MachineStatus& st)
{
    return std::string(machineStateStr(st.main)) + '[' + machineSubFlagsStr(st.sub) + ']';
}

} // namespace scheduler
