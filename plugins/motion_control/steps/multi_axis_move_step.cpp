#include "multi_axis_move_step.h"

#include "imotion_control_service.h"

#include <extensionsystem/pluginmanager.h>
#include <common/utility/json_tool/reflector.h>

#include <scheduler/step/step_factory.h>

#include <Logger.h>

#include <exception>
#include <future>

/**
 * @file multi_axis_move_step.cpp
 * @brief motion.multi_axis_move 工步实现（解析/合并/执行均在本文内，辅助函数为文件内 private）
 *
 * 错误码：
 *   -20  axisNames/positions 无效
 *   -21  IMotionControlService 不可用或未就绪
 *   -22  轴名不存在
 *   -23  单轴运动失败
 *   -99  未捕获 std::exception
 */

namespace motion_control_plugin::step {
namespace {

using json = nlohmann::json;

void readStringArray(const json &j, const char *key, std::vector<std::string> *out)
{
    if (!j.contains(key) || !j[key].is_array())
        return;
    out->clear();
    for (const auto &item : j[key]) {
        if (item.is_string())
            out->push_back(item.get<std::string>());
    }
}

void readDoubleArray(const json &j, const char *key, std::vector<double> *out)
{
    if (!j.contains(key) || !j[key].is_array())
        return;
    out->clear();
    for (const auto &item : j[key]) {
        if (item.is_number())
            out->push_back(item.get<double>());
    }
}

MultiAxisMoveParams paramsFromJson(const json &j)
{
    MultiAxisMoveParams params;
    readStringArray(j, "axisNames", &params.axisNames);
    readDoubleArray(j, "positions", &params.positions);
    if (params.positions.empty())
        readDoubleArray(j, "pos", &params.positions);

    fromJson(j, "absolute", params.absolute);
    fromJson(j, "speed", params.speed);
    fromJson(j, "parallel", params.parallel);
    return params;
}

void applyOverride(MultiAxisMoveParams *base, const MultiAxisMoveOverride &overlay)
{
    if (!base)
        return;
    if (!overlay.axisNames.empty())
        base->axisNames = overlay.axisNames;
    if (!overlay.positions.empty())
        base->positions = overlay.positions;
    if (overlay.absolute.has_value())
        base->absolute = *overlay.absolute;
    if (overlay.speed.has_value())
        base->speed = *overlay.speed;
    if (overlay.parallel.has_value())
        base->parallel = *overlay.parallel;
}

void applyOverrideFromJson(MultiAxisMoveParams *base, const json &j)
{
    if (!base || !j.is_object())
        return;

    MultiAxisMoveOverride overlay;
    readStringArray(j, "axisNames", &overlay.axisNames);
    readDoubleArray(j, "positions", &overlay.positions);
    if (overlay.positions.empty())
        readDoubleArray(j, "pos", &overlay.positions);

    if (j.contains("absolute") && j["absolute"].is_boolean())
        overlay.absolute = j["absolute"].get<bool>();
    if (j.contains("speed") && j["speed"].is_number())
        overlay.speed = j["speed"].get<double>();
    if (j.contains("parallel") && j["parallel"].is_boolean())
        overlay.parallel = j["parallel"].get<bool>();

    applyOverride(base, overlay);
}

bool tryApplyContextOverlay(MultiAxisMoveParams *base, const scheduler::FlowContext &ctx,
                            const std::string &key)
{
    if (key.empty())
        return false;

    if (auto full = ctx.tryGet<MultiAxisMoveParams>(key)) {
        if (full->isValid()) {
            *base = *full;
            return true;
        }
    }

    if (auto partial = ctx.tryGet<MultiAxisMoveOverride>(key)) {
        applyOverride(base, *partial);
        return true;
    }

    if (auto jPartial = ctx.tryGet<json>(key)) {
        applyOverrideFromJson(base, *jPartial);
        return true;
    }

    return false;
}

MultiAxisMoveParams resolveParams(const scheduler::FlowContext &ctx,
                                  const MultiAxisMoveParams    &fromConfig,
                                  const std::string            &ctxKey)
{
    MultiAxisMoveParams resolved = fromConfig;

    if (!ctxKey.empty()) {
        tryApplyContextOverlay(&resolved, ctx, ctxKey);
        return resolved;
    }

    tryApplyContextOverlay(&resolved, ctx, kCtxMultiAxisMoveParams);
    return resolved;
}

IMotionControlService *motionServiceOrNull(QString *errorOut)
{
    auto *motion = ExtensionSystem::PluginManager::getInterface<IMotionControlService>();
    if (!motion) {
        if (errorOut)
            *errorOut = QStringLiteral("运控服务未加载");
        return nullptr;
    }
    if (!motion->isReady()) {
        if (errorOut)
            *errorOut = QStringLiteral("运控未就绪");
        return nullptr;
    }
    return motion;
}

bool axisExists(IMotionControlService *motion, const QString &axisName)
{
    return motion && motion->axisNames().contains(axisName);
}

scheduler::StepResult moveOneAxis(IMotionControlService *motion, const QString &axisName,
                                  double position, bool absolute, double speed, QString *errorOut)
{
    if (!axisExists(motion, axisName)) {
        if (errorOut)
            *errorOut = QStringLiteral("运控轴不存在: %1").arg(axisName);
        return scheduler::StepResult::fail(errorOut->toStdString(), -22);
    }

    if (!motion->moveToPosition(axisName, position, absolute, speed)) {
        if (errorOut)
            *errorOut = QStringLiteral("轴 %1 运动失败").arg(axisName);
        return scheduler::StepResult::fail(errorOut->toStdString(), -23);
    }
    return scheduler::StepResult::ok();
}

scheduler::StepResult executeMove(IMotionControlService *motion, const MultiAxisMoveParams &params,
                                const std::string &stepLabel)
{
    const std::string label = stepLabel.empty() ? "multi_axis_move" : stepLabel;

    try {
        if (!params.isValid()) {
            return scheduler::StepResult::fail(
                "MultiAxisMoveStep[" + label
                    + "]: axisNames/positions empty or size mismatch",
                -20);
        }

        QString err;
        if (!motion)
            motion = motionServiceOrNull(&err);
        if (!motion)
            return scheduler::StepResult::fail(err.toStdString(), -21);

        for (const auto &axisName : params.axisNames) {
            if (!axisExists(motion, QString::fromStdString(axisName))) {
                return scheduler::StepResult::fail(
                    "MultiAxisMoveStep[" + label + "]: axis not found: " + axisName, -22);
            }
        }

        LOG_INFO("[工步] 多轴运动 {}: {} 轴, absolute={}, parallel={}",
                 label, params.axisNames.size(), params.absolute, params.parallel);

        if (params.parallel && params.axisNames.size() > 1) {
            std::vector<std::future<scheduler::StepResult>> futures;
            futures.reserve(params.axisNames.size());

            for (size_t i = 0; i < params.axisNames.size(); ++i) {
                const QString axisName = QString::fromStdString(params.axisNames[i]);
                const double  position = params.positions[i];
                futures.push_back(std::async(std::launch::async,
                                             [motion, axisName, position, &params]() {
                                                 QString moveErr;
                                                 return moveOneAxis(motion, axisName, position,
                                                                    params.absolute, params.speed,
                                                                    &moveErr);
                                             }));
            }

            for (auto &future : futures) {
                const scheduler::StepResult result = future.get();
                if (!result.isOk())
                    return result;
            }
        } else {
            for (size_t i = 0; i < params.axisNames.size(); ++i) {
                const QString           axisName = QString::fromStdString(params.axisNames[i]);
                scheduler::StepResult result =
                    moveOneAxis(motion, axisName, params.positions[i], params.absolute,
                                params.speed, &err);
                if (!result.isOk())
                    return result;
            }
        }

        motion->refreshAxisStates();
        return scheduler::StepResult::ok();
    } catch (const std::exception &e) {
        return scheduler::StepResult::fail(e.what(), -99);
    }
}

} // namespace

void MultiAxisMoveStep::reflectFromJson(const json &j)
{
    m_configParams = paramsFromJson(j);
    fromJson(j, "ctxKey", m_ctxKey);
}

scheduler::StepResult MultiAxisMoveStep::execute(scheduler::FlowContext &ctx)
{
    const MultiAxisMoveParams params = resolveParams(ctx, m_configParams, m_ctxKey);
    return executeMove(nullptr, params, name());
}

scheduler::StepResult runMultiAxisMove(IMotionControlService *motion,
                                       const MultiAxisMoveParams &params,
                                       const std::string &stepLabel)
{
    return executeMove(motion, params, stepLabel);
}

scheduler::StepResult runMultiAxisMove(const MultiAxisMoveParams &params,
                                       const std::string &stepLabel)
{
    return executeMove(nullptr, params, stepLabel);
}

SCHEDULER_REGISTER_STEP_CLASS(motion_control_plugin::step::MultiAxisMoveStep,
                              "motion.multi_axis_move")

} // namespace motion_control_plugin::step
