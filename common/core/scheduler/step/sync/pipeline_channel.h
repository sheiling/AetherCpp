#pragma once

// ============================================================
// pipeline_channel.h — 工位间流水线消息通道
//
// 【PipelineChannel】有界/无界的线程安全消息队列
//   上游工位 send() 消息，下游工位 receive() 消费
//
// 【PipelineChannelHub】全局命名通道注册中心（单例）
//
// 【ChannelSendStep】流程步骤：将 ctx 变量发送到指定通道
//   JSON 配置：
//   {
//     "type": "sync.channel_send",
//     "name": "发送工件数据",
//     "params": { "channel": "ch-a-to-b", "ctxKey": "workpiece" }
//   }
//
// 【ChannelReceiveStep】流程步骤：从指定通道接收消息到 ctx 变量
//   JSON 配置：
//   {
//     "type": "sync.channel_receive",
//     "name": "接收工件数据",
//     "params": { "channel": "ch-a-to-b", "ctxKey": "workpiece" }
//   }
//
// 流水线示意图：
//   StationA → [channel ch-a-to-b] → StationB → [channel ch-b-to-c] → StationC
// ============================================================

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <stdexcept>

#include "../../scheduler_global.h"
#include "../../context/flow_context.h"
#include "../i_step.h"
#include <Logger.h>

namespace scheduler {

// ============================================================
// PipelineChannel — 有界/无界线程安全消息队列
// ============================================================
class SCHEDULER_EXPORT PipelineChannel {
public:
    using Message = json;

    explicit PipelineChannel(std::string name, int capacity = 0)
        : m_name_(std::move(name)), m_capacity_(capacity)
    {}

    const std::string& name()     const { return m_name_; }
    int                capacity() const { return m_capacity_; }

    // 发送消息（若队列已满则阻塞等待，直到有空位或超时）
    // capacity==0 表示无界队列，发送不阻塞
    bool send(Message msg, int timeoutMs = 5000) {
        std::unique_lock<std::mutex> lk(m_mutex_);
        if (m_capacity_ > 0) {
            bool ok = m_notFull_.wait_for(lk,
                std::chrono::milliseconds(timeoutMs),
                [this]{ return static_cast<int>(m_queue_.size()) < m_capacity_; });
            if (!ok) return false;
        }
        m_queue_.push(std::move(msg));
        m_notEmpty_.notify_one();
        return true;
    }

    // 接收消息（阻塞直到有消息或超时，返回 nullopt 表示超时）
    std::optional<Message> receive(int timeoutMs = 30000) {
        std::unique_lock<std::mutex> lk(m_mutex_);
        bool ok = m_notEmpty_.wait_for(lk,
            std::chrono::milliseconds(timeoutMs),
            [this]{ return !m_queue_.empty(); });
        if (!ok) return std::nullopt;
        Message msg = std::move(m_queue_.front());
        m_queue_.pop();
        m_notFull_.notify_one();
        return msg;
    }

    // 非阻塞尝试接收
    std::optional<Message> tryReceive() {
        std::lock_guard<std::mutex> lk(m_mutex_);
        if (m_queue_.empty()) return std::nullopt;
        Message msg = std::move(m_queue_.front());
        m_queue_.pop();
        m_notFull_.notify_one();
        return msg;
    }

    bool isEmpty() const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        return m_queue_.empty();
    }

    int size() const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        return static_cast<int>(m_queue_.size());
    }

    void clear() {
        std::lock_guard<std::mutex> lk(m_mutex_);
        while (!m_queue_.empty()) m_queue_.pop();
        m_notFull_.notify_all();
    }

private:
    std::string             m_name_;
    int                     m_capacity_{0};   // 0=无界
    mutable std::mutex      m_mutex_;
    std::condition_variable m_notEmpty_;
    std::condition_variable m_notFull_;
    std::queue<Message>     m_queue_;
};

// ============================================================
// PipelineChannelHub — 全局命名通道注册中心（单例）
// ============================================================
class SCHEDULER_EXPORT PipelineChannelHub {
public:
    static PipelineChannelHub& instance() {
        static PipelineChannelHub inst;
        return inst;
    }

    // 创建并注册通道（已存在则覆盖）
    std::shared_ptr<PipelineChannel> createChannel(
        const std::string& name, int capacity = 0)
    {
        std::lock_guard<std::mutex> lk(m_mutex_);
        auto ch = std::make_shared<PipelineChannel>(name, capacity);
        m_channels_[name] = ch;
        return ch;
    }

    // 获取通道，不存在则按 capacity 自动创建
    std::shared_ptr<PipelineChannel> getOrCreate(
        const std::string& name, int capacity = 0)
    {
        std::lock_guard<std::mutex> lk(m_mutex_);
        auto it = m_channels_.find(name);
        if (it != m_channels_.end()) return it->second;
        auto ch = std::make_shared<PipelineChannel>(name, capacity);
        m_channels_[name] = ch;
        return ch;
    }

    // 获取通道，不存在返回 nullptr
    std::shared_ptr<PipelineChannel> getChannel(const std::string& name) const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        auto it = m_channels_.find(name);
        return it != m_channels_.end() ? it->second : nullptr;
    }

    bool hasChannel(const std::string& name) const {
        std::lock_guard<std::mutex> lk(m_mutex_);
        return m_channels_.count(name) > 0;
    }

    void removeChannel(const std::string& name) {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_channels_.erase(name);
    }

    void clearAll() {
        std::lock_guard<std::mutex> lk(m_mutex_);
        m_channels_.clear();
    }

private:
    PipelineChannelHub() = default;
    mutable std::mutex m_mutex_;
    std::unordered_map<std::string, std::shared_ptr<PipelineChannel>> m_channels_;
};

// ============================================================
// ChannelSendStep — 将 FlowContext 中的变量发送到管道通道
//
//  ctx 中 ctxKey 对应的变量必须是 json 类型（或可被 tryGet<json> 获取）
//  若 ctxKey 为空，则发送一个空对象并附加 flowName / timestamp 元信息
// ============================================================
class ChannelSendStep : public IStep {
public:
    ChannelSendStep(std::string channelName, std::string ctxKey)
        : m_channelName_(std::move(channelName))
        , m_ctxKey_(std::move(ctxKey))
    {
        m_config_.type = typeName();
    }

    StepResult execute(FlowContext& ctx) override {
        auto ch = PipelineChannelHub::instance().getChannel(m_channelName_);
        if (!ch) {
            return StepResult::fail("[ChannelSend] 通道不存在: " + m_channelName_);
        }

        // 构建消息：尝试从 ctx 中取 json，否则包装为 {"value": ...}
        json msg = json::object();
        if (!m_ctxKey_.empty()) {
            auto jval = ctx.tryGet<json>(m_ctxKey_);
            if (jval) {
                msg = *jval;
            } else {
                auto sval = ctx.tryGet<std::string>(m_ctxKey_);
                if (sval) msg["value"] = *sval;
                auto ival = ctx.tryGet<int>(m_ctxKey_);
                if (ival) msg["value"] = *ival;
            }
        }
        // 追加元信息（供下游追踪来源）
        msg["_from"]      = ctx.flowName;
        msg["_instanceId"]= ctx.instanceId;
        msg["_timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        int timeout = m_config_.timeoutMs > 0 ? m_config_.timeoutMs : 5000;
        LOG_DEBUG("[ChannelSend] {} → 通道 '{}' (key='{}')",
                  ctx.flowName, m_channelName_, m_ctxKey_);

        if (!ch->send(std::move(msg), timeout)) {
            return StepResult::timeout("[ChannelSend] 通道已满，发送超时: " + m_channelName_);
        }
        return StepResult::ok();
    }

    std::string typeName() const override { return "sync.channel_send"; }

private:
    std::string m_channelName_;
    std::string m_ctxKey_;
};

// ============================================================
// ChannelReceiveStep — 从管道通道接收消息写入 FlowContext
//
//  成功接收后将 json 消息存入 ctx[ctxKey]
// ============================================================
class ChannelReceiveStep : public IStep {
public:
    ChannelReceiveStep(std::string channelName, std::string ctxKey)
        : m_channelName_(std::move(channelName))
        , m_ctxKey_(std::move(ctxKey))
    {
        m_config_.type = typeName();
    }

    StepResult execute(FlowContext& ctx) override {
        auto ch = PipelineChannelHub::instance().getChannel(m_channelName_);
        if (!ch) {
            return StepResult::fail("[ChannelReceive] 通道不存在: " + m_channelName_);
        }

        int timeout = m_config_.timeoutMs > 0 ? m_config_.timeoutMs : 30000;
        LOG_DEBUG("[ChannelReceive] {} ← 通道 '{}' (key='{}'，等待 {}ms)",
                  ctx.flowName, m_channelName_, m_ctxKey_, timeout);

        auto msg = ch->receive(timeout);
        if (!msg) {
            return StepResult::timeout("[ChannelReceive] 等待消息超时: " + m_channelName_);
        }

        if (!m_ctxKey_.empty()) {
            ctx.set(m_ctxKey_, *msg);
        }
        LOG_DEBUG("[ChannelReceive] 已接收来自 '{}' 的消息",
                  msg->value("_from", "?"));
        return StepResult::ok();
    }

    std::string typeName() const override { return "sync.channel_receive"; }

private:
    std::string m_channelName_;
    std::string m_ctxKey_;
};

} // namespace scheduler
