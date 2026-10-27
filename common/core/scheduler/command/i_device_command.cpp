#include "i_device_command.h"

#include <stdexcept>

#include <Logger.h>

namespace scheduler {

CommandResult IDeviceCommand::waitForCompletion(int timeoutMs)
{
    if (timeoutMs < 0) {
        throw std::invalid_argument("Command wait timeout cannot be negative");
    }

    std::unique_lock<std::mutex> lock(m_mutex_);
    if (timeoutMs == 0) {
        m_cv_.wait(lock, [this] { return m_lastResult_.has_value(); });
    } else if (!m_cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                               [this] { return m_lastResult_.has_value(); })) {
        return {CommandState::TIMEOUT, -2, "Command wait timed out", {}, 0};
    }
    return *m_lastResult_;
}

void IDeviceCommand::onCompleted(std::function<void(const CommandResult&)> callback)
{
    addCompletionListener(std::move(callback));
}

void IDeviceCommand::setTimeout(int ms)
{
    if (ms <= 0) {
        throw std::invalid_argument("Command timeout must be greater than zero");
    }
    m_timeoutMs_ = ms;
}

void IDeviceCommand::setRetryPolicy(RetryPolicy policy)
{
    if (policy.maxRetries < 0 || policy.retryIntervalMs < 0) {
        throw std::invalid_argument("Command retry values cannot be negative");
    }
    m_retry_ = policy;
}

void IDeviceCommand::notifyCompleted(CommandResult result)
{
    if (m_completionNotified_.exchange(true)) {
        LOG_WARN("[Command] Duplicate completion ignored for command {}", m_commandId_);
        return;
    }

    m_state_.store(result.state);

    std::vector<std::function<void(const CommandResult&)>> callbacks;
    {
        std::lock_guard<std::mutex> lock(m_mutex_);
        m_lastResult_ = result;
        callbacks = m_completionListeners_;
    }
    m_cv_.notify_all();

    for (const auto& callback : callbacks) {
        if (!callback) {
            continue;
        }
        try {
            callback(result);
        } catch (const std::exception& ex) {
            LOG_ERROR("[Command] Completion callback threw for command {}: {}",
                      m_commandId_, ex.what());
        } catch (...) {
            LOG_ERROR("[Command] Completion callback threw for command {}", m_commandId_);
        }
    }
}

void IDeviceCommand::addCompletionListener(
    std::function<void(const CommandResult&)> callback)
{
    if (!callback) {
        throw std::invalid_argument("Command completion callback cannot be empty");
    }
    std::lock_guard<std::mutex> lock(m_mutex_);
    m_completionListeners_.push_back(std::move(callback));
}

void IDeviceCommand::beginAttempt()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex_);
        m_lastResult_.reset();
    }
    m_completionNotified_.store(false);
    m_state_.store(CommandState::DISPATCHED);
}

void IDeviceCommand::completeFromDispatcher(CommandResult result)
{
    m_state_.store(result.state);

    std::vector<std::function<void(const CommandResult&)>> callbacks;
    const bool alreadyNotified = m_completionNotified_.exchange(true);
    {
        std::lock_guard<std::mutex> lock(m_mutex_);
        m_lastResult_ = result;
        if (!alreadyNotified) {
            callbacks = m_completionListeners_;
        }
    }
    m_cv_.notify_all();

    for (const auto& callback : callbacks) {
        if (!callback) {
            continue;
        }
        try {
            callback(result);
        } catch (const std::exception& ex) {
            LOG_ERROR("[Command] Completion callback threw for command {}: {}",
                      m_commandId_, ex.what());
        } catch (...) {
            LOG_ERROR("[Command] Completion callback threw for command {}", m_commandId_);
        }
    }
}

} // namespace scheduler
