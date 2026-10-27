#pragma once

#include <string>
#include <memory>
#include <functional>
#include <future>
#include <thread>
#include <vector>
#include <common/3rdparty/nlohmann/json.hpp>
#include "../core/flow_types.h"
#include "../context/flow_context.h"
#include "../command/command_dispatcher.h"

namespace scheduler {

using json = nlohmann::json;

// ============================================================
// StepConfig — 步骤配置
// ============================================================
struct StepConfig {
    std::string     name;
    std::string     type;
    int             timeoutMs       { 5000 };
    int             retryCount      { 0 };
    int             retryIntervalMs { 500 };
    bool            skipOnFail      { false };
    bool            optional        { false };
    bool            isCheckpoint    { false };
    json            params;
    std::shared_ptr<class IStep> compensationStep;  // 补偿步骤

    // ── 步骤级拦截器（按顺序应用，应用顺序：流程级 → 步骤级）──────
    // JSON 中配置：{ "interceptors": ["timing", "logging", "my_custom"] }
    // 拦截器在 StepInterceptorRegistry 中按名称注册，运行时动态查找
    std::vector<std::string> interceptorNames;
};

// ============================================================
// IStep — 步骤基类接口
// ============================================================
class IStep {
public:
    virtual ~IStep() = default;

    void setConfig(StepConfig cfg)      { m_config_ = std::move(cfg); }
    const StepConfig& config() const    { return m_config_; }
    const std::string& name() const     { return m_config_.name; }

    // ── 子类必须实现 ─────────────────────────────────────────
    virtual StepResult execute(FlowContext& ctx) = 0;
    virtual std::string typeName() const = 0;

    // ── 可选重写 ──────────────────────────────────────────────
    virtual void cancel()                  {}
    virtual void cleanup(FlowContext& /*ctx*/) {}

    // reflectFromJson — 从 JSON 参数初始化步骤私有配置
    //   调用时机：setConfig() 之后，Init() 之前
    //   参数 j：等同于 StepConfig::params（步骤专属参数块）
    //   用途：子类从 params 读取自己的配置字段（如设备名称、阈值等）
    //   示例：
    //     void reflectFromJson(const json& j) override {
    //         m_deviceName_ = j.value("device", "");
    //         m_threshold_  = j.value("threshold", 0.0);
    //     }
    virtual void reflectFromJson(const nlohmann::json& j) { (void)j; }

    // Init — 步骤硬件/资源初始化（在 reflectFromJson 之后调用）
    //   调用时机：reflectFromJson() 之后，流程首次运行之前
    //   返回值：true=初始化成功，false=初始化失败（将抛出异常阻止流程加载）
    //   用途：建立设备连接、检查资源可用性、预加载数据等
    //   注意：如果初始化代价较高或依赖运行时状态，考虑延迟到 execute() 内部进行
    virtual bool Init() { return true; }

    virtual void reflectToJson(nlohmann::json& j) const { (void)j; }

    // ── 异步执行（默认用 std::async 包装同步版本）────────────
    virtual std::future<StepResult> executeAsync(FlowContext& ctx) {
        return std::async(std::launch::async, [this, &ctx]{
            return execute(ctx);
        });
    }

    // ── 子步骤（用于 Sequential / Parallel 等容器步骤）────────
    virtual void addChild(std::shared_ptr<IStep> child) {
        m_children_.push_back(std::move(child));
    }
    const std::vector<std::shared_ptr<IStep>>& children() const { return m_children_; }

    // ── 重试包装（带重试逻辑的执行）──────────────────────────
    StepResult executeWithRetry(FlowContext& ctx) {
        StepResult result = StepResult::fail("Not executed");
        int attempts = m_config_.retryCount + 1;
        for (int i = 0; i < attempts; ++i) {
            result = execute(ctx);
            if (result.isOk() || result.isSkipped()) return result;
            if (i < attempts - 1 && m_config_.retryIntervalMs > 0)
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(m_config_.retryIntervalMs));
        }
        return result;
    }

protected:
    StepConfig                          m_config_;
    std::vector<std::shared_ptr<IStep>> m_children_;
    // std::shared_ptr<CommandDispatcher>  m_dispatcher_;

    // CommandDispatcher& commandDispatcher() {
    //     if (!m_dispatcher_)
    //         m_dispatcher_ = std::make_shared<CommandDispatcher>();
    //     return *m_dispatcher_;
    // }
};

} // namespace scheduler
