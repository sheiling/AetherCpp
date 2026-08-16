#pragma once

#include "command.h"

#include "uibase/permission/permservice.h"

#include <QAbstractButton>

namespace aoi::binding {

/** 侧栏 QAbstractButton ↔ Command（启用态 + 权限 + clicked→run） */
inline void bindCommandToButton(QAbstractButton *button, Command *command)
{
    if (!button || !command)
        return;

    const auto syncEnabled = [button, command]() { button->setEnabled(command->canExecute()); };
    syncEnabled();

    QObject::connect(command, &Command::canExecuteChanged, button, syncEnabled);

    if (auto *fc = qobject_cast<FunctionalCommand *>(command)) {
        QObject::connect(PermService::instance(), &PermService::permissionsChanged, command,
                         [fc, syncEnabled]() {
                             fc->updateCanExecute();
                             syncEnabled();
                         });
    } else {
        QObject::connect(PermService::instance(), &PermService::permissionsChanged, command,
                         syncEnabled);
    }

    QObject::connect(button, &QAbstractButton::clicked, command, &Command::run);
}

} // namespace aoi::binding
