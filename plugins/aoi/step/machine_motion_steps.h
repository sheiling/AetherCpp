#pragma once

#include <scheduler/step/i_step.h>

namespace aoi::step {

/** 整机复位：Z 轴回零 → X/Y 轴并行回零 */
class AoiMachineResetStep final : public scheduler::IStep
{
public:
    scheduler::StepResult execute(scheduler::FlowContext &ctx) override;
    std::string           typeName() const override { return "aoi.machine_reset"; }
};

/** 整机急停：停止全部运控轴 */
class AoiMachineEstopStep final : public scheduler::IStep
{
public:
    scheduler::StepResult execute(scheduler::FlowContext &ctx) override;
    std::string           typeName() const override { return "aoi.machine_estop"; }
};

} // namespace aoi::step
