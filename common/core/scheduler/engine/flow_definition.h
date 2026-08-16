#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "../step/i_step.h"
#include "../context/flow_context.h"

namespace scheduler {

// ============================================================
// FlowDefinition — 流程定义（模板，不含运行时状态）
// ============================================================
struct FlowDefinition {
    std::string                       name;
    std::string                       description;
    int                               version     {1};
    std::shared_ptr<IStep>            rootStep;
    std::vector<std::string>          requiredDevices;  // 需要加锁的设备列表
    int                               maxInstances{1};
    int                               priority    {0};
    int                               timeoutMs   {0};  // 0=不限时
    bool                              enableCompensation{true};

    // ── 流程级拦截器（对本流程所有步骤生效，执行顺序：流程级 → 步骤级）
    // JSON 中配置：{ "interceptors": ["tracing", "error_detail"] }
    // 在 StepInterceptorRegistry 中按名称注册，FlowLoader 解析时填充
    std::vector<std::string>          interceptorNames;

    std::function<void(FlowContext&)> onStart;
    std::function<void(FlowContext&)> onComplete;
    std::function<void(FlowContext&)> onFailed;
    std::function<void(FlowContext&)> onAbort;
};

} // namespace scheduler
