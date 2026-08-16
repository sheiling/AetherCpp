#pragma once

#include "machine_status.h"

#include <QObject>
#include <memory>

namespace scheduler {

class MachineFSM;

/**
 * @brief MachineFSM 状态变更的 Qt 信号桥（主/子态变化时 statusChanged）
 *
 * 由 MachineFSM 内部持有；UI 侧 connect 即可，勿再散落 onStatusChanged 回调。
 */
class SCHEDULER_EXPORT MachineStatusNotifier final : public QObject
{
    Q_OBJECT

public:
    explicit MachineStatusNotifier(QObject *parent = nullptr);

    void bind(MachineFSM *fsm);

    MachineStatus currentStatus() const;

signals:
    void statusChanged(scheduler::MachineStatus from, scheduler::MachineStatus to);

private:
    friend class MachineFSM;
    void postStatusChange(const MachineStatus &from, const MachineStatus &to);

    MachineFSM *m_fsm = nullptr;
};

} // namespace scheduler

Q_DECLARE_METATYPE(scheduler::MachineStatus)
