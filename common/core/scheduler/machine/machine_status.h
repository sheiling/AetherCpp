#pragma once

#include <cstdint>
#include <string>

#include "../scheduler_global.h"

namespace scheduler {

// ============================================================
// MachineState — 整机主态（互斥，四选一）
//
// 与子标志 MachineSubFlags 组合为 MachineStatus，供 FSM / UI 使用。
// ============================================================
enum class MachineState : uint8_t {
    NotReset,   // 未复位 / 不可生产；软件启动默认
    Paused,     // 暂停；复位成功、Stop、ClearAlarm 后的可启动前置态
    Running,    // 自动运行（含 Draining 排空子标志）
    Alarm,      // 报警停机；**仅**自 Running + Alarm 事件进入
};

// ============================================================
// MachineSubFlags — 整机子标志（位掩码，可组合）
//
// 组合约束（FSM validateSubFlags 强制）：
//   R1 Resetting 与 ResettingSuccess 不同时为 1
//   R2 Draining 仅 main==Running
//   R3 Error 仅 main==Paused
//   R4 Alarm 主态时不应同时 Error
//   R5 EmergencyStop 可与任意主态并存；清除须 Reset 成功
//   R6 不设 ResettingFailed；失败时 main→NotReset 并清 Resetting|ResettingSuccess
// ============================================================
enum class MachineSubFlags : uint32_t {
    None              = 0,
    Resetting         = 1u << 0,  // 复位进行中
    ResettingSuccess  = 1u << 1,  // 复位已成功（Paused 下仍可直接 Start）
    ManualMode        = 1u << 2,  // 手动模式（禁止 Start）
    EmergencyStop     = 1u << 3,  // 急停链路有效；EstopRelease 不清此位
    Draining          = 1u << 4,  // 运行中排空；主态仍为 Running
    Error             = 1u << 5,  // 暂停态报警挂起；须 ClearAlarm 后再 Start
};

/// 主态 + 子标志聚合；EventBus SystemReady payload、UI KPI 映射均使用此结构
struct MachineStatus {
    MachineState    main{MachineState::NotReset};
    MachineSubFlags sub{MachineSubFlags::None};

    bool operator==(const MachineStatus& other) const {
        return main == other.main && sub == other.sub;
    }
    bool operator!=(const MachineStatus& other) const { return !(*this == other); }
};

inline MachineSubFlags operator|(MachineSubFlags a, MachineSubFlags b) {
    return static_cast<MachineSubFlags>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline MachineSubFlags operator&(MachineSubFlags a, MachineSubFlags b) {
    return static_cast<MachineSubFlags>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool hasFlag(MachineSubFlags set, MachineSubFlags bit) {
    return (set & bit) != MachineSubFlags::None;
}

inline MachineSubFlags setFlag(MachineSubFlags set, MachineSubFlags bit) {
    return set | bit;
}

inline MachineSubFlags clearFlag(MachineSubFlags set, MachineSubFlags bit) {
    return static_cast<MachineSubFlags>(
        static_cast<uint32_t>(set) & ~static_cast<uint32_t>(bit));
}

inline MachineSubFlags clearFlags(MachineSubFlags set, MachineSubFlags mask) {
    return static_cast<MachineSubFlags>(
        static_cast<uint32_t>(set) & ~static_cast<uint32_t>(mask));
}

inline bool isResetting(MachineSubFlags flags) {
    return hasFlag(flags, MachineSubFlags::Resetting);
}

inline bool isDraining(const MachineStatus& st) {
    return st.main == MachineState::Running
        && hasFlag(st.sub, MachineSubFlags::Draining);
}

/// 整机 UI / FSM 事件守卫（§3.1.3）；UI canExecute 与 handleEvent 须共用，禁止重复判定
/// Start：main==Paused 且无 ManualMode/EmergencyStop/Error/Resetting/Draining
SCHEDULER_EXPORT bool canStart(const MachineStatus& st);
/// Stop：Running（含 Draining），或复位中（Resetting，用于中止复位工步流）
SCHEDULER_EXPORT bool canStop(const MachineStatus& st);
/// Reset：非 Running 且非 Resetting
SCHEDULER_EXPORT bool canReset(const MachineStatus& st);
/// ClearAlarm：主态 Alarm，或 Paused + Error
SCHEDULER_EXPORT bool canClearAlarm(const MachineStatus& st);
/// 清料：Paused + 复位已成功 + 非复位中 + 无报警/Error/急停
SCHEDULER_EXPORT bool canClearMaterial(const MachineStatus& st);
/// 单次检测：已复位成功、非报警、非 Running
SCHEDULER_EXPORT bool canSingleInspect(const MachineStatus& st);

SCHEDULER_EXPORT const char* machineStateStr(MachineState s);
SCHEDULER_EXPORT const char* machineSubFlagStr(MachineSubFlags bit);
SCHEDULER_EXPORT std::string machineSubFlagsStr(MachineSubFlags flags);
SCHEDULER_EXPORT std::string statusToString(const MachineStatus& st);

} // namespace scheduler
