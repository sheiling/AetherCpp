#include "step_interceptor.h"
#include "i_step.h"
#include "../context/flow_context.h"
#include <Logger.h>
#include <chrono>
#include <sstream>

namespace scheduler {

// ============================================================
// 内置拦截器实现
// ============================================================

// ── "logging"：详细步骤日志 ──────────────────────────────────
// 在步骤开始前打印 INFO 日志，结束后根据结果打印 INFO/WARN/ERROR
static StepInterceptorFn makeLoggingInterceptor() {
    return [](IStep* step, FlowContext& ctx, std::function<StepResult()> proceed) -> StepResult {
        LOG_INFO("[Interceptor/logging] ▶ 步骤 [{}] 开始  (flow={})",
                 step->name(), ctx.flowName);

        auto result = proceed();

        if (result.isOk()) {
            LOG_INFO("[Interceptor/logging] ✓ 步骤 [{}] 成功",
                     step->name());
        } else if (result.isSkipped()) {
            LOG_INFO("[Interceptor/logging] ⇒ 步骤 [{}] 已跳过",
                     step->name());
        } else if (result.status == StepResult::Status::Timeout) {
            LOG_WARN("[Interceptor/logging] ⏱ 步骤 [{}] 超时：{}",
                     step->name(), result.message);
        } else {
            LOG_WARN("[Interceptor/logging] ✗ 步骤 [{}] 失败：{} (code={})",
                     step->name(), result.message, result.errorCode);
        }
        return result;
    };
}

// ── "timing"：耗时统计（结果写入 ctx 变量）─────────────────
// 写入 key：_step_elapsed_<stepName>（单位 ms，类型 int64_t）
static StepInterceptorFn makeTimingInterceptor() {
    return [](IStep* step, FlowContext& ctx, std::function<StepResult()> proceed) -> StepResult {
        auto t0 = std::chrono::steady_clock::now();
        auto result = proceed();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();

        std::string key = "_step_elapsed_" + step->name();
        ctx.set(key, static_cast<int64_t>(elapsedMs));

        LOG_DEBUG("[Interceptor/timing] 步骤 [{}] 耗时 {} ms",
                  step->name(), elapsedMs);
        return result;
    };
}

// ── "tracing"：执行轨迹追踪 ──────────────────────────────────
// 将步骤执行记录追加到 ctx["_trace"]（json array）
// 每条记录: { "step": "<name>", "status": "ok/fail/skip/timeout", "elapsedMs": N }
static StepInterceptorFn makeTracingInterceptor() {
    return [](IStep* step, FlowContext& ctx, std::function<StepResult()> proceed) -> StepResult {
        auto t0 = std::chrono::steady_clock::now();
        auto result = proceed();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();

        // 读取现有 trace（若不存在则初始化）
        json traceArray = json::array();
        auto existing = ctx.tryGet<json>("_trace");
        if (existing.has_value()) {
            traceArray = existing.value();
        }

        // 步骤状态字符串
        std::string statusStr;
        switch (result.status) {
        case StepResult::Status::Success: statusStr = "ok";      break;
        case StepResult::Status::Failed:  statusStr = "fail";    break;
        case StepResult::Status::Skipped: statusStr = "skip";    break;
        case StepResult::Status::Timeout: statusStr = "timeout"; break;
        default:                          statusStr = "unknown";
        }

        traceArray.push_back({
            {"step",      step->name()},
            {"type",      step->typeName()},
            {"status",    statusStr},
            {"elapsedMs", elapsedMs},
            {"message",   result.message}
        });

        ctx.set("_trace", traceArray);
        return result;
    };
}

// ── "error_detail"：失败时输出 ERROR 日志与上下文摘要 ────────
static StepInterceptorFn makeErrorDetailInterceptor() {
    return [](IStep* step, FlowContext& ctx, std::function<StepResult()> proceed) -> StepResult {
        auto result = proceed();

        if (result.isFailed() || result.status == StepResult::Status::Timeout) {
            LOG_ERROR("[Interceptor/error_detail] 步骤 [{}] 异常："
                      "\n  flow     = {}"
                      "\n  instance = {}"
                      "\n  message  = {}"
                      "\n  code     = {}",
                      step->name(),
                      ctx.flowName,
                      ctx.instanceId,
                      result.message,
                      result.errorCode);
        }
        return result;
    };
}

// ============================================================
// 注册所有内置拦截器
// ============================================================
void registerBuiltinInterceptors() {
    auto& reg = StepInterceptorRegistry::instance();
    reg.registerInterceptor("logging",      makeLoggingInterceptor());
    reg.registerInterceptor("timing",       makeTimingInterceptor());
    reg.registerInterceptor("tracing",      makeTracingInterceptor());
    reg.registerInterceptor("error_detail", makeErrorDetailInterceptor());

    LOG_INFO("[StepInterceptorRegistry] 内置拦截器已注册："
             " logging / timing / tracing / error_detail");
}

} // namespace scheduler
