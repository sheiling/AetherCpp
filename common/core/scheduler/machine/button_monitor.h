#pragma once

#include <memory>
#include <functional>
#include <atomic>
#include <thread>
#include <string>

#include "../scheduler_global.h"
#include "machine_fsm.h"

namespace scheduler {

// ============================================================
// ButtonMonitor — 物理按钮 IO 轮询监控器
//
// 通过运动控制卡 IO 接口周期性轮询设备面板按钮信号，
// 检测到边沿跳变后自动向 MachineFSM 发送对应状态机事件。
//
// 【与运动控制卡的解耦方式】
//   不直接依赖 MotionController，而是通过 IoReader 回调函数读取 IO。
//   用户在构造时传入桥接 lambda，例如：
//
//     ButtonMonitor::IoReader reader =
//         [&motionCtrl](const std::string& name, bool& val) -> bool {
//             return motionCtrl.ReadIO(name, val) == nullptr; // nullptr=成功
//         };
//     auto monitor = std::make_shared<ButtonMonitor>(fsm, reader, cfg);
//     monitor->start();
//
// 【按钮 → 事件 映射规则】
//   ┌──────────────┬──────────────────┬────────────────────────────────┐
//   │  物理按钮     │  触发条件         │  状态机事件                    │
//   ├──────────────┼──────────────────┼────────────────────────────────┤
//   │  启动按钮     │  上升沿（0→1）    │  Start                          │
//   │  停止按钮     │  上升沿（0→1）    │  Stop（语义=暂停）               │
//   │  复位按钮     │  上升沿（0→1）    │  Reset                          │
//   │  手自动切换   │  低→高            │  AutoSwitch（切自动）           │
//   │              │  高→低            │  ManualSwitch（切手动）         │
//   │  急停按钮     │  下降沿（1→0）    │  EmergencyStop（常闭触点断开）   │
//   │  （常闭）     │  上升沿（0→1）    │  EstopRelease（仅记硬件释放，不清 EmergencyStop）│
//   └──────────────┴──────────────────┴────────────────────────────────┘
//
// 【急停定稿（§3.1.3）】
//   EstopRelease 不改变主态/子标志；清除 EmergencyStop 须 Reset 成功。
//
// 【IO 名称约定】
//   配置项与 MotionController IO 配置中的名称保持一致。
//   任意配置项留空（""）则跳过该按钮的监控。
//
// 【虚拟运控】app.toml virtual_debug.virtual_motion_enable=true 时
//   Bootstrap 置 physicalButtonsEnabled=false，不启动轮询；pollLoop 亦跳过 DI 边沿检测。
// ============================================================
class SCHEDULER_EXPORT ButtonMonitor {
public:
    // ── IO 读取函数对象 ───────────────────────────────────────
    // 参数：IO名称，输出：电平值（true=高电平）
    // 返回：true=读取成功，false=读取失败（跳过本次边沿检测）
    using IoReader = std::function<bool(const std::string& ioName, bool& value)>;

    // ── 按钮 IO 名称配置 ──────────────────────────────────────
    struct Config {
        std::string startBtn;       // 启动按钮 IO 名称（上升沿触发）
        std::string stopBtn;        // 停止按钮 IO 名称（上升沿触发，语义=暂停）
        std::string resetBtn;       // 复位按钮 IO 名称（上升沿触发）
        std::string modeSwitch;     // 手自动切换 IO 名称（电平变化触发）
                                    //   高电平=自动模式，低电平=手动模式
        std::string estop;          // 急停 IO 名称（常闭触点：高=正常，低=急停触发）
        int         pollIntervalMs{20};  // 轮询间隔（ms），默认 20ms（50Hz）
        bool        physicalButtonsEnabled{true}; ///< false=不监听物理 DI（虚拟运控调试）
    };

    // ── 构造与析构 ────────────────────────────────────────────
    ButtonMonitor(std::shared_ptr<MachineFSM> fsm,
                  IoReader                    reader,
                  Config                      cfg);
    ~ButtonMonitor();

    // 禁止拷贝
    ButtonMonitor(const ButtonMonitor&)            = delete;
    ButtonMonitor& operator=(const ButtonMonitor&) = delete;

    // ── 控制接口 ──────────────────────────────────────────────
    void start();   // 启动后台轮询线程
    void stop();    // 停止轮询线程（析构时自动调用）

    bool isRunning() const { return m_running_.load(); }

    // ── 配置访问 ──────────────────────────────────────────────
    const Config& config() const { return m_cfg_; }

private:
    // 各按钮的上一次采样状态（用于边沿检测）
    struct ButtonState {
        bool start{false};
        bool stop{false};
        bool reset{false};
        bool mode{false};   // 高=自动，低=手动
        bool estop{true};   // 急停常闭：初始高电平（正常闭合）
    };

    void pollLoop();    // 后台轮询主循环
    void initLastState(ButtonState& last);

    // 辅助：读取 IO，失败时返回 false，跳过本次处理
    bool readIO(const std::string& name, bool& value) const;

    std::shared_ptr<MachineFSM> m_fsm_;
    IoReader                    m_reader_;
    Config                      m_cfg_;

    std::thread       m_thread_;
    std::atomic<bool> m_running_{false};
};

} // namespace scheduler
