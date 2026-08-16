#include "machine_status_notifier.h"

#include "machine_fsm.h"

#include <QMetaObject>
#include <QThread>

namespace scheduler {

MachineStatusNotifier::MachineStatusNotifier(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<scheduler::MachineStatus>("scheduler::MachineStatus");
}

void MachineStatusNotifier::bind(MachineFSM *fsm)
{
    m_fsm = fsm;
}

MachineStatus MachineStatusNotifier::currentStatus() const
{
    return m_fsm ? m_fsm->currentStatus() : MachineStatus{};
}

void MachineStatusNotifier::postStatusChange(const MachineStatus &from,
                                             const MachineStatus &to)
{
    QMetaObject::invokeMethod(
        this,
        [this, from, to]() { emit statusChanged(from, to); },
        Qt::QueuedConnection);
}

} // namespace scheduler
