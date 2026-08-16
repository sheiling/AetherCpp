#include "machine_motion_steps.h"

#include "data/dataModel/system_config_model.h"

#include "extensionsystem/pluginmanager.h"
#include "imotion_control_service.h"

#include <scheduler/step/step_factory.h>

#include <common/include/macros.h>
#include <Logger.h>

#include <future>

namespace aoi::step {
namespace {

using motion_control_plugin::IMotionControlService;

bool motionReady(QString *errorOut)
{
    auto *motion = ExtensionSystem::PluginManager::getInterface<IMotionControlService>();
    if (!motion) {
        if (errorOut)
            *errorOut = QStringLiteral("运控服务未加载");
        return false;
    }
    if (!motion->isReady()) {
        if (errorOut)
            *errorOut = QStringLiteral("运控未就绪");
        return false;
    }
    return true;
}

bool axisExists(IMotionControlService *motion, const QString &axisName, QString *errorOut)
{
    if (axisName.isEmpty()) {
        if (errorOut)
            *errorOut = QStringLiteral("轴名为空");
        return false;
    }
    if (!motion->axisNames().contains(axisName)) {
        if (errorOut)
            *errorOut = QStringLiteral("运控轴不存在: %1").arg(axisName);
        return false;
    }
    return true;
}

/** 单轴回零；底层 AxisReset 阻塞直至完成或超时 */
bool homeAxisBlocking(IMotionControlService *motion, const QString &axisName, QString *errorOut)
{
    if (!motion->homeAxis(axisName)) {
        if (errorOut)
            *errorOut = QStringLiteral("轴 %1 回零失败").arg(axisName);
        return false;
    }

    motion->refreshAxisStates();
    const auto st = motion->axisState(axisName);
    if (st.alarm) {
        if (errorOut)
            *errorOut = QStringLiteral("轴 %1 回零后仍存在报警").arg(axisName);
        return false;
    }
    return true;
}

scheduler::StepResult runMachineReset(const CommonParams &common)
{
    QString err;
    if (!motionReady(&err))
        return scheduler::StepResult::fail(err.toStdString());

    auto *motion = ExtensionSystem::PluginManager::getInterface<IMotionControlService>();
    if (!axisExists(motion, common.axisXName, &err)
        || !axisExists(motion, common.axisYName, &err)) {
        return scheduler::StepResult::fail(err.toStdString());
    }

    const QString &zName = common.axisZName;
    if (!zName.isEmpty()) {
        if (!axisExists(motion, zName, &err))
            return scheduler::StepResult::fail(err.toStdString());

        LOG_INFO("[工步] 整机复位：Z 轴 {} 回零", zName.toStdString());
        if (!homeAxisBlocking(motion, zName, &err))
            return scheduler::StepResult::fail(err.toStdString());
    } else {
        LOG_INFO("[工步] 整机复位：未配置 Z 轴，跳过 Z 回零");
    }

    LOG_INFO("[工步] 整机复位：X/Y 轴并行回零 ({}, {})",
             common.axisXName.toStdString(), common.axisYName.toStdString());

    QString errX;
    QString errY;
    auto futureX = std::async(std::launch::async, [&]() {
        return homeAxisBlocking(motion, common.axisXName, &errX);
    });
    auto futureY = std::async(std::launch::async, [&]() {
        return homeAxisBlocking(motion, common.axisYName, &errY);
    });

    const bool okX = futureX.get();
    const bool okY = futureY.get();
    if (!okX)
        return scheduler::StepResult::fail(errX.toStdString());
    if (!okY)
        return scheduler::StepResult::fail(errY.toStdString());

    LOG_INFO_REPORT("[工步] 整机复位：全部轴回零完成");
    return scheduler::StepResult::ok();
}

scheduler::StepResult runMachineEstop(const CommonParams & /*common*/)
{
    QString err;
    if (!motionReady(&err))
        return scheduler::StepResult::fail(err.toStdString());

    auto *motion = ExtensionSystem::PluginManager::getInterface<IMotionControlService>();
    if (!motion->emergencyStopAll()) {
        return scheduler::StepResult::fail("emergency stop all axes failed");
    }

    LOG_ERROR_REPORT("[工步] 整机急停：已停止全部运控轴");
    return scheduler::StepResult::ok();
}

} // namespace

scheduler::StepResult AoiMachineResetStep::execute(scheduler::FlowContext &ctx)
{
    Q_UNUSED(ctx);
    const CommonParams common = SystemConfigRepository::load().common;
    return runMachineReset(common);
}

scheduler::StepResult AoiMachineEstopStep::execute(scheduler::FlowContext &ctx)
{
    Q_UNUSED(ctx);
    const CommonParams common = SystemConfigRepository::load().common;
    return runMachineEstop(common);
}

SCHEDULER_REGISTER_STEP_CLASS(aoi::step::AoiMachineResetStep, "aoi.machine_reset")
SCHEDULER_REGISTER_STEP_CLASS(aoi::step::AoiMachineEstopStep, "aoi.machine_estop")

} // namespace aoi::step
