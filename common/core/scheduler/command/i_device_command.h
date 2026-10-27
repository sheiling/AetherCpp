#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../core/uuid_gen.h"
#include "../scheduler_global.h"
#include <common/3rdparty/nlohmann/json.hpp>

namespace scheduler {

using json = nlohmann::json;

enum class CommandState {
    CREATED,
    DISPATCHED,
    EXECUTING,
    COMPLETED,
    FAILED,
    TIMEOUT,
    CANCELLED
};

struct CommandResult {
    CommandState state{CommandState::FAILED};
    int errorCode{0};
    std::string message;
    json responseData;
    int64_t durationMs{0};
};

struct RetryPolicy {
    int maxRetries{0};
    int retryIntervalMs{500};
};

class CommandDispatcher;

class SCHEDULER_EXPORT IDeviceCommand {
public:
    virtual ~IDeviceCommand() = default;

    const std::string& commandId() const noexcept { return m_commandId_; }
    const std::string& commandType() const noexcept { return m_type_; }
    const std::string& deviceKey() const noexcept { return m_deviceKey_; }
    CommandState state() const noexcept { return m_state_.load(); }
    const json& params() const noexcept { return m_params_; }
    int timeoutMs() const noexcept { return m_timeoutMs_; }
    RetryPolicy retryPolicy() const noexcept { return m_retry_; }

    virtual int dispatch() = 0;
    virtual int cancel() = 0;

    virtual CommandResult waitForCompletion(int timeoutMs = 0);

    void onCompleted(std::function<void(const CommandResult&)> callback);
    void setType(std::string type) { m_type_ = std::move(type); }
    void setDeviceKey(std::string deviceKey) { m_deviceKey_ = std::move(deviceKey); }
    void setParams(json params) { m_params_ = std::move(params); }
    void setTimeout(int ms);
    void setRetryPolicy(RetryPolicy policy);

protected:
    void notifyCompleted(CommandResult result);
    void setExecuting() noexcept { m_state_.store(CommandState::EXECUTING); }

private:
    friend class CommandDispatcher;

    void addCompletionListener(std::function<void(const CommandResult&)> callback);
    void beginAttempt();
    void completeFromDispatcher(CommandResult result);

    std::string m_commandId_{generateUUID()};
    std::string m_type_;
    std::string m_deviceKey_;
    json m_params_;
    int m_timeoutMs_{5000};
    RetryPolicy m_retry_;
    std::atomic<CommandState> m_state_{CommandState::CREATED};
    std::atomic<bool> m_completionNotified_{false};
    mutable std::mutex m_mutex_;
    std::condition_variable m_cv_;
    std::optional<CommandResult> m_lastResult_;
    std::vector<std::function<void(const CommandResult&)>> m_completionListeners_;
};

} // namespace scheduler
