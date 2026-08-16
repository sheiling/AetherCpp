#include "binding_session.h"

// 页面级绑定会话：登记句柄、批量 flush、嵌套暂停 VM→View 推送

BindingHandle *BindingSession::add(std::unique_ptr<BindingHandle> handle)
{
    if (!handle)
        return nullptr;
    BindingHandle *raw = handle.get();
    m_handles.push_back(std::move(handle));
    return raw;
}

void BindingSession::updateAllSources()
{
    for (const auto &h : m_handles) {
        if (h)
            h->updateSource();
    }
}

void BindingSession::beginSuspendUpdates()
{
    ++m_suspendCount;
}

void BindingSession::endSuspendUpdates()
{
    if (m_suspendCount > 0)
        --m_suspendCount;
}
