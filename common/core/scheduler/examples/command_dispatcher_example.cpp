#include <scheduler/command/command_dispatcher.h>

#include <iostream>
#include <memory>
#include <vector>

using namespace scheduler;

class ImmediateCommand final : public IDeviceCommand {
public:
    explicit ImmediateCommand(int& executionCount)
        : executionCount_(executionCount)
    {
        setDeviceKey("demo-device");
        setTimeout(1000);
    }

    int dispatch() override
    {
        ++executionCount_;
        notifyCompleted({CommandState::COMPLETED, 0, "completed", json::object(), 0});
        return 0;
    }

    int cancel() override { return 0; }

private:
    int& executionCount_;
};

class RetryCommand final : public IDeviceCommand {
public:
    explicit RetryCommand(int& executionCount)
        : executionCount_(executionCount)
    {
        setRetryPolicy({1, 10});
    }

    int dispatch() override
    {
        if (++executionCount_ == 1) {
            return -1;
        }
        notifyCompleted({CommandState::COMPLETED, 0, "retried", json::object(), 0});
        return 0;
    }

    int cancel() override { return 0; }

private:
    int& executionCount_;
};

class OrderedCommand final : public IDeviceCommand {
public:
    OrderedCommand(std::vector<int>& order, int value)
        : order_(order)
        , value_(value)
    {
    }

    int dispatch() override
    {
        order_.push_back(value_);
        notifyCompleted({CommandState::COMPLETED, 0, "completed", json::object(), 0});
        return 0;
    }

    int cancel() override { return 0; }

private:
    std::vector<int>& order_;
    int value_;
};

class TimeoutCommand final : public IDeviceCommand {
public:
    explicit TimeoutCommand(int& cancelCount)
        : cancelCount_(cancelCount)
    {
        setTimeout(80);
    }

    int dispatch() override { return 0; }
    int cancel() override
    {
        ++cancelCount_;
        return 0;
    }

private:
    int& cancelCount_;
};

int main()
{
    CommandDispatcher dispatcher;

    int completedCount = 0;
    auto successful = dispatcher.submit(
        std::make_shared<ImmediateCommand>(completedCount)).get();
    if (successful.state != CommandState::COMPLETED || completedCount != 1) {
        std::cerr << "Immediate command did not complete successfully\n";
        return 1;
    }

    int retryCount = 0;
    auto retried = dispatcher.submit(
        std::make_shared<RetryCommand>(retryCount)).get();
    if (retried.state != CommandState::COMPLETED || retryCount != 2) {
        std::cerr << "Retry command did not complete after one retry\n";
        return 1;
    }

    std::vector<int> order;
    auto batchResults = dispatcher.submitBatch({
        std::make_shared<OrderedCommand>(order, 1),
        std::make_shared<OrderedCommand>(order, 2)
    }, false);
    for (auto& result : batchResults) {
        if (result.get().state != CommandState::COMPLETED) {
            std::cerr << "Serial batch command failed\n";
            return 1;
        }
    }
    if (order != std::vector<int>{1, 2}) {
        std::cerr << "Serial batch executed out of order\n";
        return 1;
    }

    int cancelCount = 0;
    auto timedOut = dispatcher.submit(
        std::make_shared<TimeoutCommand>(cancelCount)).get();
    if (timedOut.state != CommandState::TIMEOUT || cancelCount != 1) {
        std::cerr << "Command timeout was not handled correctly\n";
        return 1;
    }

    std::cout << "CommandDispatcher example passed\n";
    return 0;
}
