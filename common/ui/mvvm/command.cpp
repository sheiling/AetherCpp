#include "command.h"

#include "auditcommandmiddleware.h"
#include "command_audit_registry.h"

#include "uibase/permission/permservice.h"

#include <memory>

// ============================================================================
// Command
// ============================================================================

Command::Command(QObject *parent)
    : QObject(parent)
    , m_canExecute(true)
{
}

void Command::setCanExecute(bool can)
{
    if (m_canExecute != can) {
        m_canExecute = can;
        emit canExecuteChanged(can);
    }
}

void Command::setName(const QString &name)
{
    m_name = name;
}

QString Command::name() const
{
    return m_name;
}

void Command::setViewModel(ViewModel *vm)
{
    m_viewModel = vm;
}

ViewModel *Command::viewModel() const
{
    return m_viewModel;
}

void Command::addMiddleware(CommandMiddlewarePtr middleware)
{
    m_cmdPipeline.add(std::move(middleware));
}

void Command::clearMiddleware()
{
    m_cmdPipeline.clear();
}

void Command::run()
{
    if (!canExecute())
        return;

    CommandContext ctx(m_name, m_viewModel);
    const auto terminal = [this](CommandContext &c) {
        if (!c.cancelled)
            execute();
    };

    if (!AuditLogRegistry::hasCommandCallback()) {
        if (m_cmdPipeline.isEmpty())
            terminal(ctx);
        else
            m_cmdPipeline.run(ctx, terminal);
        return;
    }

    // 全局审计（最外层）→ 实例管道（可选）→ execute()
    static const auto globalAudit = std::make_shared<AuditCommandMiddleware>();
    globalAudit->process(ctx, [this, terminal](CommandContext &c) {
        if (m_cmdPipeline.isEmpty())
            terminal(c);
        else
            m_cmdPipeline.run(c, terminal);
    });
}

// ============================================================================
// FunctionalCommand
// ============================================================================

FunctionalCommand::FunctionalCommand(ExecuteFunction executeFunc,
                                     CanExecuteFunction canExecuteFunc,
                                     QObject *parent)
    : Command(parent)
    , m_executeFunc(executeFunc)
    , m_canExecuteFunc(canExecuteFunc)
{
    if (m_canExecuteFunc) {
        m_canExecute = m_canExecuteFunc();
    }
}

void FunctionalCommand::execute()
{
    if (canExecute() && m_executeFunc) {
        m_executeFunc();
    }
}

bool FunctionalCommand::canExecute() const
{
    if (!name().isEmpty()) {
        if (!PermService::instance()->can(name()))
            return false;
        if (m_canExecuteFunc)
            return m_canExecuteFunc();
        // 仅有权限 key、无额外条件时以 PermService 为准，勿沿用登录前缓存的 m_canExecute
        return true;
    }
    if (m_canExecuteFunc)
        return m_canExecuteFunc();
    return Command::canExecute();
}

void FunctionalCommand::updateCanExecute()
{
    setCanExecute(canExecute());
}
