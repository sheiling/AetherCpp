#include "invokerplugin.h"

#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"

#include "calculatorservice.h"
#include "invokerbasicview.h"
#include "invokerargsview.h"

#include <QDebug>

using namespace ExtensionSystem;

bool InvokerPlugin::initialize(QString *errorString)
{
    Q_UNUSED(errorString)

    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString)
            *errorString = "InvokerPlugin: IMainWindowService not found.";
        qWarning() << "[InvokerPlugin] IMainWindowService not found!";
        return false;
    }

    // ── 1. 创建 CalculatorService 并注册到对象池 ──────────────────────────
    //  子页面通过 PluginManager::getObject<CalculatorService>() 获取此对象，
    //  再用 ExtensionSystem::invoke<>() 动态调用其 Q_INVOKABLE 方法。
    m_calcService = new CalculatorService(this);
    PluginManager::addObject(m_calcService);
    qDebug() << "[InvokerPlugin] CalculatorService registered to object pool.";

    // ── 2. 注册带二级导航的容器页（order=70，排在 Middleware 页面之后）──────
    PageWithSubNav *container = svc->addContainerPage("invoker", "nav.invoker", 70);

    // ── 3. 添加两个子页面 ──────────────────────────────────────────────────
    // 子页面①：基础 invoke<void/QString/int> 用法
    container->addSubPage("invoker_basic", "sub.invoker_basic",
                          new InvokerBasicView());

    // 子页面②：双参数 & Invoker<> 类模板用法
    container->addSubPage("invoker_args", "sub.invoker_args",
                          new InvokerArgsView());

    qDebug() << "[InvokerPlugin] initialize: Invoker demo pages registered.";
    return true;
}

IPlugin::ShutdownFlag InvokerPlugin::aboutToShutdown()
{
    if (m_calcService) {
        PluginManager::removeObject(m_calcService);
        m_calcService = nullptr;
    }
    return SynchronousShutdown;
}
