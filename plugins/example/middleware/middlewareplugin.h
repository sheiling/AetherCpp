#pragma once

#include "extensionsystem/iplugin.h"

/**
 * @brief Middleware 示例插件
 *
 * 注册 ProductView 页面，演示 MVVM Middleware 机制：
 *   - IPropertyMiddleware / PropertyMiddlewarePipeline（属性变更拦截）
 *   - ICommandMiddleware  / CommandMiddlewarePipeline（命令执行拦截）
 *   - LoggingMiddleware       — 属性变更前置/后置日志
 *   - AuditLogMiddleware      — 变更历史审计
 *   - AuthCommandMiddleware   — 命令权限校验
 *   - LoggingCommandMiddleware — 命令执行日志
 */
class MiddlewarePlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID
                      FILE "middlewareplugin.json")

public:
    MiddlewarePlugin()  = default;
    ~MiddlewarePlugin() override = default;

    bool initialize(QString *errorString) override;
    void extensionsInitialized() override {}
};
