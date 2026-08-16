#include "step_flow_controller.h"

#include "step_factory.h"

#include <scheduler/engine/flow_loader.h>
#include <scheduler/engine/flow_instance.h>
#include <scheduler/event/event_bus.h>

#include <Logger.h>

#include <fstream>
#include <mutex>
#include <stdexcept>
#include <filesystem>

namespace scheduler {

namespace fs = std::filesystem;

json StepFlowController::extractStepFlowMap(const json &j)
{
    if (j.contains("stepFlows") && j["stepFlows"].is_object())
        return j["stepFlows"];

    if (!j.is_object())
        throw std::runtime_error("[StepFlowController] 配置根须为 JSON 对象");

    json implicit;
    bool ok = true;
    for (auto & [k, v] : j.items()) {
        if (!k.empty() && k[0] == '_')
            continue;
        if (!v.is_string()) {
            ok = false;
            break;
        }
        implicit[k] = v.get<std::string>();
    }
    if (ok && !implicit.empty())
        return implicit;

    throw std::runtime_error(
        "[StepFlowController] 请使用 \"stepFlows\": { \"步骤名\": \"xxx.json\" } "
        "或根对象为「步骤名 → 路径字符串」并可用 \"_xxx\" 键存放说明");
}

std::string StepFlowController::resolveFlowPath(const std::string &configDir,
                                                const std::string &pathInConfig)
{
    fs::path p(pathInConfig);
    if (p.is_absolute())
        return p.lexically_normal().string();
    fs::path base(configDir);
    if (configDir.empty())
        base = fs::current_path();
    return (base / p).lexically_normal().string();
}

void StepFlowController::loadFromFile(const std::string &configPath, FlowLoader *loader)
{
    std::ifstream f(configPath);
    if (!f)
        throw std::runtime_error("[StepFlowController] 无法打开配置: " + configPath);

    json j;
    try {
        f >> j;
    } catch (const std::exception &e) {
        throw std::runtime_error(
            std::string("[StepFlowController] JSON 解析失败: ") + e.what());
    }

    fs::path cfgPath(configPath);
    std::string configDir =
        cfgPath.has_parent_path() ? cfgPath.parent_path().string() : std::string();

    loadFromJson(j, configDir, loader);
    LOG_INFO("[StepFlowController] 已从 '{}' 加载 {} 个步骤流程映射",
             configPath, m_steps_.size());
}

void StepFlowController::loadFromString(const std::string &jsonStr,
                                        const std::string &configDir,
                                        FlowLoader *loader)
{
    json j = json::parse(jsonStr);
    loadFromJson(j, configDir, loader);
}

void StepFlowController::loadFromJson(const json &j, const std::string &configDir,
                                      FlowLoader *loader)
{
    static std::once_flag s_builtinStepsOnce;
    std::call_once(s_builtinStepsOnce, [] { registerBuiltinSteps(); });

    clear();
    json map = extractStepFlowMap(j);

    FlowLoader defaultLoader;
    FlowLoader &useLoader = loader ? *loader : defaultLoader;

    for (auto it = map.begin(); it != map.end(); ++it) {
        const std::string stepName = it.key();
        if (!it.value().is_string()) {
            throw std::runtime_error(
                "[StepFlowController] 步骤 '" + stepName + "' 的流程路径须为字符串");
        }
        std::string flowPath = resolveFlowPath(configDir, it.value().get<std::string>());
        auto def             = useLoader.loadFromFile(flowPath);
        if (!def)
            throw std::runtime_error("[StepFlowController] 加载流程失败: " + flowPath);

        const std::string loadedFlowName = def->name;
        m_steps_[stepName]               = std::move(def);
        LOG_DEBUG("[StepFlowController] 步骤 '{}' -> 流程定义名 '{}' (文件 {})",
                  stepName, loadedFlowName, flowPath);
    }
}

void StepFlowController::clear() { m_steps_.clear(); }

bool StepFlowController::hasStep(const std::string &stepName) const
{
    return m_steps_.count(stepName) > 0;
}

std::vector<std::string> StepFlowController::stepNames() const
{
    std::vector<std::string> out;
    out.reserve(m_steps_.size());
    for (const auto & [k, _] : m_steps_)
        out.push_back(k);
    return out;
}

std::shared_ptr<FlowDefinition> StepFlowController::definition(const std::string &stepName) const
{
    auto it = m_steps_.find(stepName);
    if (it == m_steps_.end())
        return nullptr;
    return it->second;
}

StepResult StepFlowController::runStep(const std::string &stepName, const json &initParams,
                                       std::shared_ptr<EventBus> eventBus) const
{
    auto def = definition(stepName);
    if (!def)
        return StepResult::fail("StepFlowController: 未注册的步骤名: " + stepName, -404);

    auto inst = std::make_shared<FlowInstance>(def, std::move(eventBus));
    inst->context().initParams = initParams;
    inst->start();
    inst->waitForCompletion();

    const FlowState st = inst->state();
    if (st == FlowState::Completed)
        return StepResult::ok();

    if (st == FlowState::Aborted)
        return StepResult::fail("流程已中止", -3);

    std::string msg = "流程失败";
    if (!inst->context().history.empty()) {
        const auto &last = inst->context().history.back();
        if (!last.message.empty())
            msg = last.message;
    }
    return StepResult::fail(msg, -1);
}

} // namespace scheduler
