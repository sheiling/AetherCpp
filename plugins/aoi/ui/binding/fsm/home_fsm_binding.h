#pragma once

#include "machine_status_display.h"

#include "command.h"

#include <scheduler/machine/machine_fsm.h>
#include <scheduler/machine/machine_status.h>

#include <functional>
#include <memory>

class HomeViewModel;

namespace aoi::binding::fsm {

struct HomeSidebarActions {
    std::function<void()> start;
    std::function<void()> stop;
    std::function<void()> reset;
    std::function<void()> clearAlarm;
    std::function<void()> clearMaterial;
    std::function<void()> singleInspect;
};

struct HomeSidebarCommands {
    FunctionalCommand *start         = nullptr;
    FunctionalCommand *stop            = nullptr;
    FunctionalCommand *reset           = nullptr;
    FunctionalCommand *clearAlarm      = nullptr;
    FunctionalCommand *clearMaterial   = nullptr;
    FunctionalCommand *singleInspect   = nullptr;
};

/** FSM 状态 → 首页 KPI 设备状态/运行模式（i18n key）；命令 canExecute 随 statusChanged 刷新 */
void bindHomeMachineFsm(HomeViewModel *vm,
                        const std::shared_ptr<scheduler::MachineFSM> &fsm,
                        const HomeSidebarActions &actions,
                        HomeSidebarCommands *outCommands);

} // namespace aoi::binding::fsm
