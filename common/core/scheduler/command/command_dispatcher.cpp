#include "command_dispatcher.h"

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <unordered_set>
#include <utility>

#include <Logger.h>

namespace scheduler {

struct CommandDispatcher::Pending {
    std::shared_ptr<IDeviceCommand> command;
    std::shared_ptr<std::promise<CommandResult>> promise;
    std::function<void(const CommandResult&)> callback;
    std::chrono::steady_clock::time_point submittedAt;
    std::chrono::steady_clock::time_point attemptStartedAt;
    std::chrono::steady_clock::time_point deadline;
    std::chrono::steady_clock::time_point retryAt;
    std::string batchId;
    int retriesUsed{0};
    bool activated{true};
    bool inFlight{false};
    bool retryScheduled{false};
    bool finishing{false};
};

struct CommandDispatcher::Batch {
    std::vector<std::string> commandIds;
    size_t currentIndex{0};
};

struct CommandDispatcher::SharedState {
    mutable std::mutex mutex;
    std::condition_variable cv;
    bool stopping{false};
    std::unordered_map<std::string, Pending> pending;
    std::unordered_map<std::string, CommandState> terminalStates;
    std::vector<std::string> terminalStateOrder;
    std::unordered_map<std::string, Batch> batches;
};

CommandDispatcher::CommandDispatcher()
    : m_state_(std::make_shared<SharedState>())
    , m_timeoutThread_([state = m_state_] { timeoutLoop(std::move(state)); })
{
}

CommandDispatcher::~CommandDispatcher()
{
    {
        std::lock_guard<std::mutex> lock(m_state_->mutex);
        m_state_->stopping = true;
    }
    m_state_->cv.notify_all();
    cancelAll();
    if (m_timeoutThread_.joinable()) {
        m_timeoutThread_.join();
    }
}

std::future<CommandResult> CommandDispatcher::submit(
    std::shared_ptr<IDeviceCommand> command)
{
    auto future = enqueue(std::move(command), {});
    return future;
}

std::vector<std::future<CommandResult>> CommandDispatcher::submitBatch(
    std::vector<std::shared_ptr<IDeviceCommand>> commands,
    bool parallel)
{
    if (parallel) {
        std::unordered_set<std::string> ids;
        for (const auto& command : commands) {
            if (!command) {
                throw std::invalid_argument("Command batch cannot contain null commands");
            }
            if (!ids.insert(command->commandId()).second) {
                throw std::invalid_argument("Command batch contains duplicate command IDs");
            }
        }

        std::vector<std::future<CommandResult>> futures;
        futures.reserve(commands.size());
        for (auto& command : commands) {
            futures.push_back(submit(std::move(command)));
        }
        return futures;
    }

    if (commands.empty()) {
        return {};
    }

    std::unordered_set<std::string> ids;
    for (const auto& command : commands) {
        if (!command) {
            throw std::invalid_argument("Command batch cannot contain null commands");
        }
        if (!ids.insert(command->commandId()).second) {
            throw std::invalid_argument("Command batch contains duplicate command IDs");
        }
    }

    const std::string batchId = generateUUID();
    std::vector<std::future<CommandResult>> futures;
    std::vector<std::shared_ptr<std::promise<CommandResult>>> promises;
    futures.reserve(commands.size());
    promises.reserve(commands.size());

    for (size_t i = 0; i < commands.size(); ++i) {
        auto promise = std::make_shared<std::promise<CommandResult>>();
        futures.push_back(promise->get_future());
        promises.push_back(std::move(promise));
    }

    {
        std::lock_guard<std::mutex> lock(m_state_->mutex);
        if (m_state_->stopping) {
            throw std::runtime_error("CommandDispatcher is shutting down");
        }

        for (const auto& command : commands) {
            if (command->state() != CommandState::CREATED) {
                throw std::invalid_argument("Only newly created commands can be submitted");
            }
            if (m_state_->pending.count(command->commandId()) != 0
                || m_state_->terminalStates.count(command->commandId()) != 0) {
                throw std::invalid_argument("Command ID has already been submitted");
            }
        }

        Batch batch;
        batch.commandIds.reserve(commands.size());
        for (size_t i = 0; i < commands.size(); ++i) {
            const auto& command = commands[i];
            const std::string id = command->commandId();
            batch.commandIds.push_back(id);

            Pending pending;
            pending.command = command;
            pending.promise = promises[i];
            pending.submittedAt = std::chrono::steady_clock::now();
            pending.batchId = batchId;
            pending.activated = (i == 0);

            std::weak_ptr<SharedState> weakState = m_state_;
            command->addCompletionListener([weakState, id](const CommandResult& result) {
                if (auto state = weakState.lock()) {
                    handleCommandResult(state, id, result);
                }
            });
            m_state_->pending.emplace(id, std::move(pending));
        }
        m_state_->batches.emplace(batchId, std::move(batch));
    }

    m_state_->cv.notify_all();
    dispatchAttempt(m_state_, commands.front()->commandId());
    return futures;
}

void CommandDispatcher::submitWithCallback(
    std::shared_ptr<IDeviceCommand> command,
    std::function<void(const CommandResult&)> callback)
{
    if (!callback) {
        throw std::invalid_argument("Command completion callback cannot be empty");
    }
    (void)enqueue(std::move(command), std::move(callback));
}

std::future<CommandResult> CommandDispatcher::enqueue(
    std::shared_ptr<IDeviceCommand> command,
    std::function<void(const CommandResult&)> callback,
    const std::string& batchId,
    bool activated)
{
    if (!command) {
        throw std::invalid_argument("Cannot submit a null device command");
    }
    if (command->state() != CommandState::CREATED) {
        throw std::invalid_argument("Only newly created commands can be submitted");
    }

    const std::string id = command->commandId();
    auto promise = std::make_shared<std::promise<CommandResult>>();
    auto future = promise->get_future();

    {
        std::lock_guard<std::mutex> lock(m_state_->mutex);
        if (m_state_->stopping) {
            throw std::runtime_error("CommandDispatcher is shutting down");
        }
        if (m_state_->pending.count(id) != 0
            || m_state_->terminalStates.count(id) != 0) {
            throw std::invalid_argument("Command ID has already been submitted");
        }

        Pending pending;
        pending.command = command;
        pending.promise = std::move(promise);
        pending.callback = std::move(callback);
        pending.submittedAt = std::chrono::steady_clock::now();
        pending.batchId = batchId;
        pending.activated = activated;

        std::weak_ptr<SharedState> weakState = m_state_;
        command->addCompletionListener([weakState, id](const CommandResult& result) {
            if (auto state = weakState.lock()) {
                handleCommandResult(state, id, result);
            }
        });
        m_state_->pending.emplace(id, std::move(pending));
    }

    m_state_->cv.notify_all();
    if (activated) {
        dispatchAttempt(m_state_, id);
    }
    return future;
}

bool CommandDispatcher::cancel(const std::string& commandId)
{
    std::shared_ptr<IDeviceCommand> command;
    bool activated = false;
    bool inFlight = false;
    {
        std::lock_guard<std::mutex> lock(m_state_->mutex);
        const auto it = m_state_->pending.find(commandId);
        if (it == m_state_->pending.end() || it->second.finishing) {
            return false;
        }
        command = it->second.command;
        activated = it->second.activated;
        inFlight = it->second.inFlight;
    }

    if (!activated || !inFlight) {
        completePending(m_state_, commandId,
                        {CommandState::CANCELLED, 0, "Command cancelled", {}, 0},
                        activated);
        return true;
    }

    const int errorCode = command->cancel();
    if (errorCode != 0) {
        LOG_ERROR("[CommandDispatcher] Cancel failed for command {}: {}",
                  commandId, errorCode);
        return false;
    }

    command->notifyCompleted(
        {CommandState::CANCELLED, 0, "Command cancelled", {}, 0});
    return true;
}

void CommandDispatcher::cancelAll()
{
    std::vector<Pending> pendingCommands;
    {
        std::lock_guard<std::mutex> lock(m_state_->mutex);
        pendingCommands.reserve(m_state_->pending.size());
        for (auto& entry : m_state_->pending) {
            pendingCommands.push_back(std::move(entry.second));
            rememberState(*m_state_, entry.first, CommandState::CANCELLED);
        }
        m_state_->pending.clear();
        m_state_->batches.clear();
    }
    m_state_->cv.notify_all();

    for (auto& pending : pendingCommands) {
        int cancelError = 0;
        if (pending.activated && pending.inFlight) {
            cancelError = pending.command->cancel();
            if (cancelError != 0) {
                LOG_ERROR("[CommandDispatcher] Cancel failed for command {}: {}",
                          pending.command->commandId(), cancelError);
            }
        }

        CommandResult result{
            CommandState::CANCELLED,
            cancelError,
            cancelError == 0 ? "Command cancelled" : "Command cancelled by dispatcher; device cancel failed",
            {},
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - pending.submittedAt).count()
        };
        pending.command->completeFromDispatcher(result);
        deliver(std::move(pending), result);
    }
}

CommandState CommandDispatcher::getState(const std::string& commandId) const
{
    std::lock_guard<std::mutex> lock(m_state_->mutex);
    const auto pending = m_state_->pending.find(commandId);
    if (pending != m_state_->pending.end()) {
        return pending->second.command->state();
    }
    const auto terminal = m_state_->terminalStates.find(commandId);
    return terminal == m_state_->terminalStates.end()
        ? CommandState::CREATED
        : terminal->second;
}

int CommandDispatcher::pendingCount(const std::string& deviceKey) const
{
    std::lock_guard<std::mutex> lock(m_state_->mutex);
    return static_cast<int>(std::count_if(
        m_state_->pending.begin(), m_state_->pending.end(),
        [&deviceKey](const auto& entry) {
            return entry.second.command->deviceKey() == deviceKey;
        }));
}

void CommandDispatcher::onDeviceResponse(const std::string& commandId,
                                         json responseData,
                                         int errorCode)
{
    std::shared_ptr<IDeviceCommand> command;
    {
        std::lock_guard<std::mutex> lock(m_state_->mutex);
        const auto it = m_state_->pending.find(commandId);
        if (it != m_state_->pending.end() && !it->second.finishing) {
            command = it->second.command;
        }
    }

    if (!command) {
        LOG_WARN("[CommandDispatcher] Response received for unknown command {}", commandId);
        return;
    }

    CommandResult result;
    result.state = errorCode == 0 ? CommandState::COMPLETED : CommandState::FAILED;
    result.errorCode = errorCode;
    result.message = errorCode == 0 ? "Device command completed" : "Device command failed";
    result.responseData = std::move(responseData);
    command->notifyCompleted(std::move(result));
}

void CommandDispatcher::dispatchAttempt(const std::shared_ptr<SharedState>& state,
                                        const std::string& commandId)
{
    std::shared_ptr<IDeviceCommand> command;
    bool stopping = false;
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        const auto it = state->pending.find(commandId);
        if (it == state->pending.end() || it->second.finishing
            || !it->second.activated || it->second.inFlight) {
            return;
        }
        if (state->stopping) {
            command = it->second.command;
            stopping = true;
        } else {
            Pending& pending = it->second;
            command = pending.command;
            pending.inFlight = true;
            pending.retryScheduled = false;
            pending.attemptStartedAt = std::chrono::steady_clock::now();
            pending.deadline = pending.attemptStartedAt
                + std::chrono::milliseconds(command->timeoutMs());
        }
    }

    if (stopping) {
        completePending(state, commandId,
                        {CommandState::CANCELLED, 0, "Command dispatcher is shutting down", {}, 0});
        return;
    }

    command->beginAttempt();
    try {
        const int errorCode = command->dispatch();
        if (errorCode != 0) {
            command->notifyCompleted(
                {CommandState::FAILED, errorCode, "Device command dispatch failed", {}, 0});
        }
    } catch (const std::exception& ex) {
        command->notifyCompleted(
            {CommandState::FAILED, -1, ex.what(), {}, 0});
    } catch (...) {
        command->notifyCompleted(
            {CommandState::FAILED, -1, "Unknown exception while dispatching device command", {}, 0});
    }
}

void CommandDispatcher::handleCommandResult(const std::shared_ptr<SharedState>& state,
                                            const std::string& commandId,
                                            const CommandResult& commandResult)
{
    bool shouldComplete = false;
    CommandResult result = commandResult;
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        const auto it = state->pending.find(commandId);
        if (it == state->pending.end() || it->second.finishing
            || !it->second.inFlight) {
            return;
        }

        Pending& pending = it->second;
        pending.inFlight = false;
        if (result.durationMs <= 0) {
            result.durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - pending.submittedAt).count();
        }

        const RetryPolicy retry = pending.command->retryPolicy();
        if (result.state == CommandState::FAILED
            && pending.retriesUsed < retry.maxRetries
            && !state->stopping) {
            ++pending.retriesUsed;
            pending.retryAt = std::chrono::steady_clock::now()
                + std::chrono::milliseconds(retry.retryIntervalMs);
            pending.retryScheduled = true;
        } else {
            pending.finishing = true;
            shouldComplete = true;
        }
    }

    state->cv.notify_all();
    if (shouldComplete) {
        completePending(state, commandId, std::move(result));
    }
}

void CommandDispatcher::completePending(const std::shared_ptr<SharedState>& state,
                                        const std::string& commandId,
                                        CommandResult result,
                                        bool allowBatchAdvance)
{
    Pending pending;
    std::string nextCommandId;
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        const auto it = state->pending.find(commandId);
        if (it == state->pending.end()) {
            return;
        }
        pending = std::move(it->second);
        state->pending.erase(it);
        rememberState(*state, commandId, result.state);

        if (allowBatchAdvance && pending.activated && !pending.batchId.empty()) {
            const auto batchIt = state->batches.find(pending.batchId);
            if (batchIt != state->batches.end()) {
                Batch& batch = batchIt->second;
                while (batch.currentIndex < batch.commandIds.size()) {
                    const std::string& candidate = batch.commandIds[batch.currentIndex];
                    if (candidate == commandId) {
                        ++batch.currentIndex;
                        continue;
                    }
                    const auto nextIt = state->pending.find(candidate);
                    if (nextIt == state->pending.end()) {
                        ++batch.currentIndex;
                        continue;
                    }
                    nextIt->second.activated = true;
                    nextCommandId = candidate;
                    break;
                }
                if (batch.currentIndex >= batch.commandIds.size()) {
                    state->batches.erase(batchIt);
                }
            }
        }
    }

    pending.command->completeFromDispatcher(result);
    deliver(std::move(pending), result);
    state->cv.notify_all();
    if (!nextCommandId.empty()) {
        dispatchAttempt(state, nextCommandId);
    }
}

void CommandDispatcher::timeoutLoop(std::shared_ptr<SharedState> state)
{
    for (;;) {
        std::vector<std::string> timedOut;
        std::vector<std::string> retry;
        {
            std::unique_lock<std::mutex> lock(state->mutex);
            state->cv.wait_for(lock, std::chrono::milliseconds(25), [&state] {
                return state->stopping;
            });
            if (state->stopping) {
                return;
            }

            const auto now = std::chrono::steady_clock::now();
            for (auto& entry : state->pending) {
                Pending& pending = entry.second;
                if (pending.finishing) {
                    continue;
                }
                if (pending.inFlight && now >= pending.deadline) {
                    pending.finishing = true;
                    timedOut.push_back(entry.first);
                } else if (pending.retryScheduled && now >= pending.retryAt) {
                    pending.retryScheduled = false;
                    retry.push_back(entry.first);
                }
            }
        }

        for (const std::string& id : timedOut) {
            std::shared_ptr<IDeviceCommand> command;
            std::chrono::steady_clock::time_point submittedAt;
            {
                std::lock_guard<std::mutex> lock(state->mutex);
                const auto it = state->pending.find(id);
                if (it == state->pending.end()) {
                    continue;
                }
                command = it->second.command;
                submittedAt = it->second.submittedAt;
            }

            const int cancelError = command->cancel();
            if (cancelError != 0) {
                LOG_ERROR("[CommandDispatcher] Cancel after timeout failed for command {}: {}",
                          id, cancelError);
            }
            CommandResult result{
                CommandState::TIMEOUT,
                cancelError == 0 ? -2 : cancelError,
                cancelError == 0 ? "Device command timed out"
                                 : "Device command timed out; cancellation failed",
                {},
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - submittedAt).count()
            };
            completePending(state, id, std::move(result));
        }

        for (const std::string& id : retry) {
            dispatchAttempt(state, id);
        }
    }
}

void CommandDispatcher::rememberState(SharedState& state,
                                      const std::string& commandId,
                                      CommandState commandState)
{
    constexpr size_t kMaxRememberedStates = 1024;
    const auto inserted = state.terminalStates.emplace(commandId, commandState).second;
    if (!inserted) {
        state.terminalStates[commandId] = commandState;
        return;
    }
    state.terminalStateOrder.push_back(commandId);
    if (state.terminalStateOrder.size() > kMaxRememberedStates) {
        state.terminalStates.erase(state.terminalStateOrder.front());
        state.terminalStateOrder.erase(state.terminalStateOrder.begin());
    }
}

void CommandDispatcher::deliver(Pending pending, const CommandResult& result)
{
    pending.promise->set_value(result);
    if (!pending.callback) {
        return;
    }
    try {
        pending.callback(result);
    } catch (const std::exception& ex) {
        LOG_ERROR("[CommandDispatcher] Completion callback threw for command {}: {}",
                  pending.command->commandId(), ex.what());
    } catch (...) {
        LOG_ERROR("[CommandDispatcher] Completion callback threw for command {}",
                  pending.command->commandId());
    }
}

} // namespace scheduler
