#pragma once

#include <QObject>
#include <QString>

/**
 * @brief CalculatorService — Invoker 调用目标服务
 *
 * 提供若干 Q_INVOKABLE 方法，用于演示 ExtensionSystem::invoke<> 的调用方式：
 *   - greet(name)          → 返回问候字符串（单参数、有返回值）
 *   - add(a, b)            → 整数加法（两个 int 参数）
 *   - subtract(a, b)       → 整数减法
 *   - multiply(a, b)       → 浮点乘法（两个 double 参数）
 *   - logMessage(msg)      → void 方法（无返回值）
 *   - factorial(n)         → 阶乘（递归，演示单 int 参数返回 int）
 *
 * 对象在 InvokerPlugin::initialize() 中通过 PluginManager::addObject() 注册到
 * 对象池，供其他插件 / 页面通过 PluginManager::getObject<CalculatorService>()
 * 获取后，再用 ExtensionSystem::invoke<>() 动态调用其方法。
 */
class CalculatorService : public QObject
{
    Q_OBJECT

public:
    explicit CalculatorService(QObject *parent = nullptr);
    ~CalculatorService() override = default;

    // ── Q_INVOKABLE 方法（均可被 ExtensionSystem::invoke<> 调用）──────────

    /** 返回 "Hello, <name>!" 字符串 */
    Q_INVOKABLE QString greet(const QString &name);

    /** 整数加法：返回 a + b */
    Q_INVOKABLE int add(int a, int b);

    /** 整数减法：返回 a - b */
    Q_INVOKABLE int subtract(int a, int b);

    /** 浮点乘法：返回 a * b */
    Q_INVOKABLE double multiply(double a, double b);

    /** 无返回值：仅向日志追加一行消息（通过 logAppended 信号通知界面） */
    Q_INVOKABLE void logMessage(const QString &msg);

    /** 阶乘：返回 n! */
    Q_INVOKABLE int factorial(int n);

signals:
    /** logMessage() 调用后发射，用于 UI 追加显示 */
    void logAppended(const QString &msg);
};
