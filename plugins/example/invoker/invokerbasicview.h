#pragma once

#include "pagewidget.h"
#include "calculatorservice.h"

#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

/**
 * @brief InvokerBasicView — 基础 Invoker 用法演示页面
 *
 * 演示要点：
 *  1. ExtensionSystem::invoke<void>  — 调用无返回值方法（logMessage）
 *  2. ExtensionSystem::invoke<QString> — 调用有返回值的方法（greet）
 *  3. ExtensionSystem::invoke<int>   — 调用单参数整数方法（factorial）
 *  4. InvokerBase::wasSuccessful()   — 检测调用是否成功
 *
 * 从 PluginManager 对象池中获取 CalculatorService，再通过
 * ExtensionSystem::invoke<>() 以方法名字符串动态调用其 Q_INVOKABLE 方法。
 */
class InvokerBasicView : public PageWidget
{
    Q_OBJECT

public:
    explicit InvokerBasicView(QWidget *parent = nullptr);

protected:
    void onLoad()        override;
    void retranslateUi() override;

private:
    void setupUi();
    void setupConnections();

    // ── 调用 logMessage (void) ────────────────────────────────────────────
    QGroupBox   *m_voidGroup      = nullptr;
    QLineEdit   *m_logMsgEdit     = nullptr;
    QPushButton *m_logMsgBtn      = nullptr;

    // ── 调用 greet (QString) ──────────────────────────────────────────────
    QGroupBox   *m_greetGroup     = nullptr;
    QLineEdit   *m_greetNameEdit  = nullptr;
    QPushButton *m_greetBtn       = nullptr;
    QLabel      *m_greetResult    = nullptr;

    // ── 调用 factorial (int) ──────────────────────────────────────────────
    QGroupBox   *m_factGroup      = nullptr;
    QLineEdit   *m_factNEdit      = nullptr;
    QPushButton *m_factBtn        = nullptr;
    QLabel      *m_factResult     = nullptr;

    // ── 调用记录 ──────────────────────────────────────────────────────────
    QGroupBox   *m_logGroup       = nullptr;
    QTextEdit   *m_logEdit        = nullptr;
    QPushButton *m_clearLogBtn    = nullptr;

    CalculatorService *m_service  = nullptr;

    void appendLog(const QString &line, const QString &color = "#d4d4d4");
};
