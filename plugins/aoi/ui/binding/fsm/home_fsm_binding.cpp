#include "home_fsm_binding.h"

#include "vm/home/home_view_model.h"

#include <scheduler/machine/machine_status_notifier.h>

namespace aoi::binding::fsm {
namespace {

FunctionalCommand *makeFsmCommand(
    QObject *owner,
    const std::shared_ptr<scheduler::MachineFSM> &fsm,
    const QString &name,
    std::function<void()> execute,
    std::function<bool(const scheduler::MachineStatus &)> canExecute)
{
    auto *cmd = new FunctionalCommand(
        std::move(execute),
        [fsm, canExecute]() { return canExecute(fsm->currentStatus()); },
        owner);
    cmd->setName(name);
    return cmd;
}

} // namespace

void bindHomeMachineFsm(HomeViewModel *vm,
                        const std::shared_ptr<scheduler::MachineFSM> &fsm,
                        const HomeSidebarActions &actions,
                        HomeSidebarCommands *outCommands)
{
    if (!vm || !fsm || !outCommands)
        return;

    using scheduler::canClearAlarm;
    using scheduler::canClearMaterial;
    using scheduler::canReset;
    using scheduler::canSingleInspect;
    using scheduler::canStart;
    using scheduler::canStop;

    *outCommands = {};
    outCommands->start = makeFsmCommand(
        vm, fsm, QStringLiteral("home.start"), actions.start, canStart);
    outCommands->stop = makeFsmCommand(
        vm, fsm, QStringLiteral("home.pause"), actions.stop, canStop);
    outCommands->reset = makeFsmCommand(
        vm, fsm, QStringLiteral("home.reset"), actions.reset, canReset);
    outCommands->clearAlarm = makeFsmCommand(
        vm, fsm, QStringLiteral("home.clear_alarm"), actions.clearAlarm, canClearAlarm);
    outCommands->clearMaterial = makeFsmCommand(
        vm, fsm, QStringLiteral("home.clear_material"), actions.clearMaterial, canClearMaterial);
    outCommands->singleInspect = makeFsmCommand(
        vm, fsm, QStringLiteral("home.single_inspect"), actions.singleInspect, canSingleInspect);

    if (auto *notifier = fsm->statusNotifier()) {
        QObject::connect(
            notifier,
            &scheduler::MachineStatusNotifier::statusChanged,
            vm,
            [vm](const scheduler::MachineStatus & /*from*/,
                 const scheduler::MachineStatus &to) {
                vm->applyMachineStatusToKpi(to);
                vm->refreshDeviceCommands();
            });
        vm->applyMachineStatusToKpi(fsm->currentStatus());
    }
}

} // namespace aoi::binding::fsm
