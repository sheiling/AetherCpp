#pragma once

#include <scheduler/step/i_step.h>

namespace aoi::step {

class AoiLoadRecipeStep final : public scheduler::IStep
{
public:
    scheduler::StepResult execute(scheduler::FlowContext &ctx) override;
    std::string           typeName() const override { return "aoi.load_recipe"; }
};

class AoiMarkLocateStep final : public scheduler::IStep
{
public:
    scheduler::StepResult execute(scheduler::FlowContext &ctx) override;
    std::string           typeName() const override { return "aoi.mark_locate"; }
};

class AoiPlanGeometryStep final : public scheduler::IStep
{
public:
    scheduler::StepResult execute(scheduler::FlowContext &ctx) override;
    std::string           typeName() const override { return "aoi.plan_geometry"; }
};

class AoiFovCaptureStep final : public scheduler::IStep
{
public:
    scheduler::StepResult execute(scheduler::FlowContext &ctx) override;
    std::string           typeName() const override { return "aoi.fov_capture"; }
};

/** 单/双轨扫描流水线（读 common.trackMode，双轨时轨1 规划与轨2 Mark 并行） */
class AoiTrackPipelineStep final : public scheduler::IStep
{
public:
    scheduler::StepResult execute(scheduler::FlowContext &ctx) override;
    std::string           typeName() const override { return "aoi.track_pipeline"; }
};

/** 对已采 FOV 执行缺陷检测（采图阶段已异步检测时可跳过） */
class AoiFovDetectStep final : public scheduler::IStep
{
public:
    scheduler::StepResult execute(scheduler::FlowContext &ctx) override;
    std::string           typeName() const override { return "aoi.fov_detect"; }
};

/** 汇总板级检测结果并上报 UI */
class AoiAggregateResultsStep final : public scheduler::IStep
{
public:
    scheduler::StepResult execute(scheduler::FlowContext &ctx) override;
    std::string           typeName() const override { return "aoi.aggregate_results"; }
};

} // namespace aoi::step
