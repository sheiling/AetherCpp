#pragma once

/**
 * @file step_flow_controller.h
 * @brief 步骤名 → 流程 JSON 映射，按名同步执行子流程
 *
 * 配置示例见 docs/ 或 examples/step_flows_config.json
 */

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <common/3rdparty/nlohmann/json.hpp>

#include <scheduler/scheduler_global.h>
#include <scheduler/core/flow_types.h>
#include <scheduler/engine/flow_definition.h>

namespace scheduler {

class FlowLoader;
class EventBus;

using json = nlohmann::json;

class SCHEDULER_EXPORT StepFlowController {
public:
    StepFlowController() = default;

    void loadFromFile(const std::string &configPath, FlowLoader *loader = nullptr);

    void loadFromJson(const json &j, const std::string &configDir, FlowLoader *loader = nullptr);

    void loadFromString(const std::string &jsonStr, const std::string &configDir,
                        FlowLoader *loader = nullptr);

    void clear();

    bool empty() const { return m_steps_.empty(); }
    bool hasStep(const std::string &stepName) const;

    std::vector<std::string> stepNames() const;

    std::shared_ptr<FlowDefinition> definition(const std::string &stepName) const;

    StepResult runStep(const std::string &stepName, const json &initParams = json::object(),
                       std::shared_ptr<EventBus> eventBus = nullptr) const;

private:
    static json extractStepFlowMap(const json &j);
    static std::string resolveFlowPath(const std::string &configDir,
                                       const std::string &pathInConfig);

    std::unordered_map<std::string, std::shared_ptr<FlowDefinition>> m_steps_;
};

} // namespace scheduler
