#pragma once

#include <QString>

class BootstrapContext;

/// 可插拔启动阶段；数字越小越先执行，关闭时按相反顺序调用 onShutdown。
class IBootstrapModule {
public:
    virtual ~IBootstrapModule() = default;

    virtual QString name() const = 0;
    virtual int priority() const { return 100; }

    /// 第一阶段
    virtual bool onRegister(BootstrapContext &) { return true; }

    /// 第二阶段
    virtual bool onBoot(BootstrapContext &) { return true; }

    /// exec() 返回之后，按与引导相反的顺序调用。
    virtual void onShutdown(BootstrapContext &) {}
};
