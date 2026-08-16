// button_monitor.cpp — 物理按钮 IO 边沿检测，映射为 MachineEvent 送入 MachineFSM

#include "button_monitor.h"
#include <Logger.h>
#include <chrono>

namespace scheduler {

ButtonMonitor::ButtonMonitor(std::shared_ptr<MachineFSM> fsm,
                             IoReader                    reader,
                             Config                      cfg)
    : m_fsm_(std::move(fsm))
    , m_reader_(std::move(reader))
    , m_cfg_(std::move(cfg))
{
    if (!m_fsm_) {
        LOG_ERROR("[ButtonMonitor] 构造失败：MachineFSM 为空！");
    }
    if (!m_reader_) {
        LOG_ERROR("[ButtonMonitor] 构造失败：IoReader 为空！");
    }
}

ButtonMonitor::~ButtonMonitor() {
    stop();
}

void ButtonMonitor::start() {
    if (m_running_.load()) {
        LOG_WARN("[ButtonMonitor] 已在运行，忽略重复 start()");
        return;
    }
    if (!m_fsm_ || !m_reader_) {
        LOG_ERROR("[ButtonMonitor] 无法启动：FSM 或 IoReader 未就绪");
        return;
    }

    m_running_.store(true);
    m_thread_ = std::thread([this]() { pollLoop(); });

    LOG_INFO("[ButtonMonitor] 已启动，轮询间隔 {} ms，监控 IO：",
             m_cfg_.pollIntervalMs);
    if (!m_cfg_.startBtn.empty())
        LOG_INFO("[ButtonMonitor]   启动按钮  → {}", m_cfg_.startBtn);
    if (!m_cfg_.stopBtn.empty())
        LOG_INFO("[ButtonMonitor]   停止按钮  → {}", m_cfg_.stopBtn);
    if (!m_cfg_.resetBtn.empty())
        LOG_INFO("[ButtonMonitor]   复位按钮  → {}", m_cfg_.resetBtn);
    if (!m_cfg_.modeSwitch.empty())
        LOG_INFO("[ButtonMonitor]   手自动切换 → {}", m_cfg_.modeSwitch);
    if (!m_cfg_.estop.empty())
        LOG_INFO("[ButtonMonitor]   急停按钮  → {}", m_cfg_.estop);
}

void ButtonMonitor::stop() {
    if (!m_running_.exchange(false)) return;
    if (m_thread_.joinable())
        m_thread_.join();
    LOG_INFO("[ButtonMonitor] 已停止");
}

bool ButtonMonitor::readIO(const std::string& name, bool& value) const {
    if (name.empty()) return false;
    if (!m_reader_) return false;
    return m_reader_(name, value);
}

// ── 初始化按钮上次状态（防止启动时产生误触发）────────────────
void ButtonMonitor::initLastState(ButtonState& last) {
    if (!m_cfg_.physicalButtonsEnabled) {
        LOG_INFO("[ButtonMonitor] 虚拟运控模式，跳过物理按钮 IO 初始化与边沿监听");
        return;
    }

    bool val = false;

    // 普通按钮初始视为未按下（低电平）
    last.start = false;
    last.stop  = false;
    last.reset = false;

    // 手自动切换：读取当前实际电平作为初始状态
    if (readIO(m_cfg_.modeSwitch, val))
        last.mode = val;
    else
        last.mode = false;  // 默认手动（保守策略）

    // 急停：常闭触点正常状态为高电平，读取实际状态
    if (readIO(m_cfg_.estop, val))
        last.estop = val;
    else
        last.estop = true;  // 默认正常（高电平，未急停）

    LOG_INFO("[ButtonMonitor] 初始按钮状态："
             "手自动={}, 急停={}",
             last.mode ? "自动" : "手动",
             last.estop ? "正常" : "【急停触发！】");

    // 启动时若急停已触发：送 EmergencyStop（非 EstopRelease）
    if (!last.estop && m_fsm_)
        m_fsm_->sendEvent(MachineEvent::EmergencyStop);
    // 启动时若拨钮在手动侧：通知 ManualSwitch
    if (!last.mode && m_fsm_)
        m_fsm_->sendEvent(MachineEvent::ManualSwitch);
}

// ── 后台轮询主循环 ────────────────────────────────────────────
void ButtonMonitor::pollLoop() {
    ButtonState last;
    initLastState(last);

    while (m_running_.load()) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(m_cfg_.pollIntervalMs));

        if (!m_fsm_) continue;

        // 虚拟运控：无真实面板 DI，仅保留线程空转（正常由 Bootstrap 不启动）
        if (!m_cfg_.physicalButtonsEnabled)
            continue;

        bool cur = false;

        // ── 启动按钮：上升沿（0→1）触发 ──────────────────────
        if (readIO(m_cfg_.startBtn, cur)) {
            if (cur && !last.start) {
                LOG_DEBUG("[ButtonMonitor] 启动按钮 ↑ 触发");
                m_fsm_->sendEvent(MachineEvent::Start);
            }
            last.start = cur;
        }

        // ── 停止按钮：上升沿（0→1）触发（语义=暂停）─────────
        if (readIO(m_cfg_.stopBtn, cur)) {
            if (cur && !last.stop) {
                LOG_DEBUG("[ButtonMonitor] 停止按钮 ↑ 触发（暂停语义）");
                m_fsm_->sendEvent(MachineEvent::Stop);
            }
            last.stop = cur;
        }

        // ── 复位按钮：上升沿（0→1）触发 ──────────────────────
        if (readIO(m_cfg_.resetBtn, cur)) {
            if (cur && !last.reset) {
                LOG_DEBUG("[ButtonMonitor] 复位按钮 ↑ 触发");
                m_fsm_->sendEvent(MachineEvent::Reset);
            }
            last.reset = cur;
        }

        // ── 手自动切换：双向电平变化检测 ──────────────────────
        // 高电平 → 自动模式（AutoSwitch）
        // 低电平 → 手动模式（ManualSwitch）
        if (readIO(m_cfg_.modeSwitch, cur)) {
            if (cur && !last.mode) {
                LOG_INFO("[ButtonMonitor] 手自动拨钮 → 自动模式");
                m_fsm_->sendEvent(MachineEvent::AutoSwitch);
            } else if (!cur && last.mode) {
                LOG_INFO("[ButtonMonitor] 手自动拨钮 → 手动模式");
                m_fsm_->sendEvent(MachineEvent::ManualSwitch);
            }
            last.mode = cur;
        }

        // ── 急停：常闭触点，下降沿=触发，上升沿=释放 ─────────
        // 正常：常闭高电平（1）；急停按下：触点断开，低电平（0）
        if (readIO(m_cfg_.estop, cur)) {
            if (!cur && last.estop) {
                // 1→0：常闭触点断开，急停触发
                LOG_ERROR("[ButtonMonitor] 急停触发！常闭触点断开");
                m_fsm_->sendEvent(MachineEvent::EmergencyStop);
            } else if (cur && !last.estop) {
                // 0→1：常闭触点恢复；FSM 仅记 EstopRelease，不清 EmergencyStop 位
                LOG_INFO("[ButtonMonitor] 急停已释放，触点恢复");
                m_fsm_->sendEvent(MachineEvent::EstopRelease);
            }
            last.estop = cur;
        }
    }
}

} // namespace scheduler
