#pragma once

#include "i_step.h"
#include "../command/command_dispatcher.h"

namespace scheduler {

// ============================================================
// DeviceStep — 设备步骤基类
//   - 封装 CommandDispatcher 访问
//   - 子类通过 dispatcher() 异步提交指令
// ============================================================
class DeviceStep : public IStep {
public:
    // 允许外部注入共享的 CommandDispatcher（多步骤复用同一调度器）
    void setDispatcher(std::shared_ptr<CommandDispatcher> d) {
        m_dispatcher_ = std::move(d);
    }

protected:
    CommandDispatcher& dispatcher() {
        if (!m_dispatcher_)
            m_dispatcher_ = std::make_shared<CommandDispatcher>();
        return *m_dispatcher_;
    }

    std::shared_ptr<CommandDispatcher> m_dispatcher_;
};

} // namespace scheduler
