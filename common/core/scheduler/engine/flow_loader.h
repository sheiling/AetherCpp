#pragma once

// ============================================================
// flow_loader.h — JSON 配置文件驱动的流程加载器
//
// 包含 ScriptRegistry、FlowLoader 等：
//
// 【ScriptRegistry】 命名脚本处理器注册表
//   JSON 配置中无法内嵌 C++ Lambda，因此通过名称引用
//   在应用启动时注册：
//     ScriptRegistry::instance().registerScript("init_handler",
//         [](FlowContext& ctx) {
//             // ... 执行业务逻辑
//             return StepResult::ok();
//         });
//
// 【StepFactory】 内置与业务自定义 IStep 均在此注册（见 step_factory.h）：
//     StepFactory::instance().registerType("my.product.inspect",
//         [] { return std::make_shared<MyInspectStep>(); });
//   或使用模板便捷注册（类型名需与 JSON 中 "type" 一致）：
//     registerStepClass<MyInspectStep>("my.product.inspect");
//   FlowLoader::buildStep 在解析 JSON 时通过 StepFactory 按 type 创建步骤。
//
// 【FlowLoader】 从 JSON 文件/字符串加载 FlowDefinition
//
// JSON 流程格式（flow_xxx.json）：
// {
//   "name":               "自动检测流程",
//   "description":        "产线自动检测工序",
//   "version":            1,
//   "timeoutMs":          60000,
//   "enableCompensation": true,
//   "priority":           0,
//   "maxInstances":       1,
//   "requiredDevices":    ["camera", "robot_arm"],
//   "steps": {
//     "type": "control.sequential",
//     "name": "root",
//     "children": [
//       {
//         "type":        "control.script",
//         "name":        "初始化",
//         "script":      "init_handler",
//         "timeoutMs":   5000,
//         "retryCount":  2,
//         "isCheckpoint": true
//       },
//       {
//         "type":   "control.delay",
//         "name":   "预热延时",
//         "params": { "ms": 500 }
//       },
//       {
//         "type": "control.parallel",
//         "name": "并行检测",
//         "children": [
//           { "type": "control.script", "name": "视觉检测", "script": "vision_check" },
//           { "type": "control.script", "name": "尺寸测量", "script": "dimension_check" }
//         ]
//       },
//       {
//         "type":   "control.script",
//         "name":   "上报结果",
//         "script": "report_result",
//         "compensation": {
//           "type":   "control.script",
//           "name":   "撤销上报",
//           "script": "cancel_report"
//         }
//       }
//     ]
//   }
// }
// ============================================================

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <stdexcept>

#include "../scheduler_global.h"
#include "../core/flow_types.h"
#include "flow_definition.h"
#include "../step/i_step.h"

namespace scheduler {

class Scheduler;  // 前向声明，避免循环依赖

// ============================================================
// ScriptRegistry — 全局命名脚本处理器注册表（单例）
// ============================================================
class SCHEDULER_EXPORT ScriptRegistry {
public:
    using ScriptFn = std::function<StepResult(FlowContext&)>;

    static ScriptRegistry& instance() {
        static ScriptRegistry inst;
        return inst;
    }

    // 注册命名脚本处理函数
    void registerScript(const std::string& name, ScriptFn fn) {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_scripts_[name] = std::move(fn);
    }

    // 批量注册（初始化时方便使用）
    void registerScripts(std::initializer_list<std::pair<std::string, ScriptFn>> list) {
        std::lock_guard<std::mutex> lk(m_mutex_);
        for (auto& [name, fn] : list)
            m_scripts_[name] = fn;
    }

    // 获取命名脚本（不存在则返回 nullptr）
    ScriptFn getScript(const std::string& name) const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        auto it = m_scripts_.find(name);
        return (it != m_scripts_.end()) ? it->second : nullptr;
    }

    bool hasScript(const std::string& name) const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        return m_scripts_.count(name) > 0;
    }

    // 列出所有已注册脚本名
    std::vector<std::string> registeredNames() const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        std::vector<std::string> names;
        names.reserve(m_scripts_.size());
        for (auto& [k, _] : m_scripts_) names.push_back(k);
        return names;
    }

    void clear() {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_scripts_.clear();
    }

private:
    ScriptRegistry() = default;
    mutable std::mutex m_mutex_;
    std::unordered_map<std::string, ScriptFn> m_scripts_;
};

// ============================================================
// FlowLoader — 从 JSON 文件 / 字符串 / json 对象加载流程定义
// ============================================================
class SCHEDULER_EXPORT FlowLoader {
public:
    FlowLoader() = default;

    // ── 加载方式 ─────────────────────────────────────────────
    // 从文件路径加载
    std::shared_ptr<FlowDefinition> loadFromFile(const std::string& filePath) const;

    // 从 JSON 字符串加载
    std::shared_ptr<FlowDefinition> loadFromString(const std::string& jsonStr) const;

    // 从 json 对象加载（底层实现）
    std::shared_ptr<FlowDefinition> loadFromJson(const json& j) const;

    // 批量加载目录下所有 .json 文件（非递归）
    std::vector<std::shared_ptr<FlowDefinition>> loadDirectory(
        const std::string& dirPath) const;

    // 将已加载的所有流程批量注册到调度器
    static void registerAll(
        Scheduler& sched,
        const std::vector<std::shared_ptr<FlowDefinition>>& defs);

private:
    std::shared_ptr<IStep> buildStep(const json& stepJson) const;
};

} // namespace scheduler
