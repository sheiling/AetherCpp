#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "../scheduler_global.h"
#include "../core/flow_types.h"

namespace scheduler {

class IStep;
class FlowContext;

// ============================================================
// StepInterceptorFn — 步骤拦截器函数签名
//
// 采用"职责链"模式：每个拦截器可选择：
//   ① 调用 proceed() — 将执行传递给下一个拦截器（或最终步骤）
//   ② 不调用 proceed() — 短路后续执行，直接返回自定义结果
//
// 参数说明：
//   step    : 当前待执行步骤（含 config()、name() 等信息）
//   ctx     : 当前流程上下文（可读写）
//   proceed : 调用下一个拦截器，最后一个拦截器的 proceed() 会触发实际执行
//
// 示例（在所有步骤前后打印日志）：
//   StepInterceptorFn logger = [](IStep* step, FlowContext& ctx,
//                                  std::function<StepResult()> proceed)
//   {
//       LOG_INFO(">>> 步骤 [{}] 开始", step->name());
//       auto result = proceed();
//       LOG_INFO("<<< 步骤 [{}] 完成，状态={}", step->name(), result.isOk() ? "OK":"FAIL");
//       return result;
//   };
// ============================================================
using StepInterceptorFn = std::function<StepResult(
    IStep*                      step,
    FlowContext&                ctx,
    std::function<StepResult()> proceed     // 继续执行下一个拦截器/步骤
)>;

// ============================================================
// StepInterceptorRegistry — 命名拦截器全局注册表（单例）
//
// 使用方式：
//   // 1. 注册自定义拦截器
//   StepInterceptorRegistry::instance().registerInterceptor(
//       "my_interceptor",
//       [](IStep* s, FlowContext& ctx, auto proceed) {
//           // 前置逻辑 ...
//           auto r = proceed();
//           // 后置逻辑 ...
//           return r;
//       });
//
//   // 2. JSON 中按名称引用
//   { "type": "control.script", "interceptors": ["timing", "my_interceptor"] }
//
//   // 3. 流程级（对所有步骤生效）
//   { "name": "自动检测流程", "interceptors": ["logging"] }
// ============================================================
class SCHEDULER_EXPORT StepInterceptorRegistry {
public:
    static StepInterceptorRegistry& instance() {
        static StepInterceptorRegistry inst;
        return inst;
    }

    // 注册/覆盖拦截器
    void registerInterceptor(const std::string& name, StepInterceptorFn fn) {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_interceptors_[name] = std::move(fn);
    }

    // 获取拦截器（未注册返回 nullptr）
    StepInterceptorFn getInterceptor(const std::string& name) const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        auto it = m_interceptors_.find(name);
        return (it != m_interceptors_.end()) ? it->second : nullptr;
    }

    bool hasInterceptor(const std::string& name) const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        return m_interceptors_.count(name) > 0;
    }

    // 返回所有已注册名称
    std::vector<std::string> registeredNames() const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        std::vector<std::string> names;
        names.reserve(m_interceptors_.size());
        for (auto& [k, _] : m_interceptors_) names.push_back(k);
        return names;
    }

    void clear() {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_interceptors_.clear();
    }

private:
    StepInterceptorRegistry() = default;
    mutable std::mutex                                       m_mutex_;
    std::unordered_map<std::string, StepInterceptorFn>      m_interceptors_;
};

// ============================================================
// 注册内置拦截器（在 scheduler 初始化时调用一次）
//
// 内置拦截器名称及用途：
//   "logging"      — 详细日志：步骤开始/结束/结果
//   "timing"       — 耗时统计：将执行时长写入 ctx（key: "_step_elapsed_<name>"）
//   "tracing"      — 执行轨迹：将步骤执行序列写入 ctx（key: "_trace"）
//   "error_detail" — 失败详情：步骤失败时打印 ERROR 级日志 + ctx 快照摘要
// ============================================================
SCHEDULER_EXPORT void registerBuiltinInterceptors();

} // namespace scheduler
