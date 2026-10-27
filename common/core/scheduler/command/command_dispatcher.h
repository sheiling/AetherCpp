#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "i_device_command.h"

namespace scheduler {

class SCHEDULER_EXPORT CommandDispatcher {
public:
    CommandDispatcher();
    ~CommandDispatcher();

    CommandDispatcher(const CommandDispatcher&) = delete;
    CommandDispatcher& operator=(const CommandDispatcher&) = delete;

    std::future<CommandResult> submit(std::shared_ptr<IDeviceCommand> command);

    std::vector<std::future<CommandResult>> submitBatch(
        std::vector<std::shared_ptr<IDeviceCommand>> commands,
        bool parallel = true);

    void submitWithCallback(std::shared_ptr<IDeviceCommand> command,
                            std::function<void(const CommandResult&)> callback);

    bool cancel(const std::string& commandId);
    void cancelAll();

    CommandState getState(const std::string& commandId) const;
    int pendingCount(const std::string& deviceKey) const;

    void onDeviceResponse(const std::string& commandId,
                         json responseData,
                         int errorCode = 0);

private:
    struct Pending;
    struct Batch;
    struct SharedState;

    std::future<CommandResult> enqueue(
        std::shared_ptr<IDeviceCommand> command,
        std::function<void(const CommandResult&)> callback,
        const std::string& batchId = {},
        bool activated = true);

    static void dispatchAttempt(const std::shared_ptr<SharedState>& state,
                                const std::string& commandId);
    static void handleCommandResult(const std::shared_ptr<SharedState>& state,
                                    const std::string& commandId,
                                    const CommandResult& result);
    static void completePending(const std::shared_ptr<SharedState>& state,
                                const std::string& commandId,
                                CommandResult result,
                                bool allowBatchAdvance = true);
    static void timeoutLoop(std::shared_ptr<SharedState> state);
    static void rememberState(SharedState& state,
                              const std::string& commandId,
                              CommandState commandState);
    static void deliver(Pending pending, const CommandResult& result);

    std::shared_ptr<SharedState> m_state_;
    std::thread m_timeoutThread_;
};

} // namespace scheduler
