#include "event_bus.h"
#include <algorithm>

namespace scheduler {

EventBus::EventBus() {
    m_dispatchThread_ = std::thread(&EventBus::dispatchLoop, this);
}

EventBus::~EventBus() {
    m_running_.store(false);
    m_cv_.notify_all();
    if (m_dispatchThread_.joinable())
        m_dispatchThread_.join();
}

EventBus::SubscriberId EventBus::subscribe(SchedulerEventType type, Handler handler) {
    std::unique_lock<std::shared_mutex> lock(m_subMutex_);
    int id = generateSubscriberId();
    m_subscribers_.push_back({id, std::move(handler), false, type});
    return id;
}

EventBus::SubscriberId EventBus::subscribeAll(Handler handler) {
    std::unique_lock<std::shared_mutex> lock(m_subMutex_);
    int id = generateSubscriberId();
    m_subscribers_.push_back({id, std::move(handler), true, {}});
    return id;
}

void EventBus::unsubscribe(SubscriberId id) {
    std::unique_lock<std::shared_mutex> lock(m_subMutex_);
    m_subscribers_.erase(
        std::remove_if(m_subscribers_.begin(), m_subscribers_.end(),
            [id](const Subscriber& s){ return s.id == id; }),
        m_subscribers_.end());
}

void EventBus::publish(SchedulerEvent evt) {
    std::shared_lock<std::shared_mutex> lock(m_subMutex_);
    for (auto& sub : m_subscribers_) {
        if (sub.all || sub.type == evt.type) {
            try { sub.handler(evt); } catch (...) {}
        }
    }
}

void EventBus::publishAsync(SchedulerEvent evt) {
    {
        std::lock_guard<std::mutex> lk(m_queueMutex_);
        m_asyncQueue_.push(std::move(evt));
    }
    m_cv_.notify_one();
}

void EventBus::dispatchLoop() {
    while (m_running_.load()) {
        std::unique_lock<std::mutex> lk(m_queueMutex_);
        m_cv_.wait(lk, [this]{
            return !m_asyncQueue_.empty() || !m_running_.load();
        });
        while (!m_asyncQueue_.empty()) {
            auto evt = std::move(m_asyncQueue_.front());
            m_asyncQueue_.pop();
            lk.unlock();
            publish(evt);
            lk.lock();
        }
    }
}

} // namespace scheduler
