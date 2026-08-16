#ifndef AUDITCOMMANDMIDDLEWARE_H
#define AUDITCOMMANDMIDDLEWARE_H

#include "imiddleware.h"

#include <memory>

/**
 * @brief 全局命令审计中间件（post 阶段通过 AuditLogRegistry 回调落库）
 */
class MVVM_EXPORT AuditCommandMiddleware : public ICommandMiddleware
{
public:
    void process(CommandContext &ctx, Next next) override;
};

#endif // AUDITCOMMANDMIDDLEWARE_H
