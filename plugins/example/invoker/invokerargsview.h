#pragma once

#include "pagewidget.h"
#include "calculatorservice.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

/**
 * @brief InvokerArgsView — 多参数 Invoker 用法演示页面
 *
 * 演示要点：
 *  1. invoke<int>(target, "add", a, b)        — 两个 int 参数
 *  2. invoke<int>(target, "subtract", a, b)   — 两个 int 参数
 *  3. invoke<double>(target, "multiply", a, b)— 两个 double 参数
 *  4. Invoker<int> 类模板用法（与 invoke<> 等价，但更显式）
 *  5. InvokerBase::wasSuccessful()            — 验证调用是否成功
 *
 * 使用 ExtensionSystem::Invoker<Result> 类模板展示更显式的调用方式，
 * 与上一页的 invoke<> 函数模板互为补充。
 */
class InvokerArgsView : public PageWidget
{
    Q_OBJECT

public:
    explicit InvokerArgsView(QWidget *parent = nullptr);

protected:
    void onLoad()        override;
    void retranslateUi() override;

private:
    void setupUi();
    void setupConnections();

    // ── 加法 ──────────────────────────────────────────────────────────────
    QGroupBox   *m_addGroup     = nullptr;
    QSpinBox    *m_addA         = nullptr;
    QSpinBox    *m_addB         = nullptr;
    QPushButton *m_addBtn       = nullptr;
    QLabel      *m_addResult    = nullptr;

    // ── 减法 ──────────────────────────────────────────────────────────────
    QGroupBox   *m_subGroup     = nullptr;
    QSpinBox    *m_subA         = nullptr;
    QSpinBox    *m_subB         = nullptr;
    QPushButton *m_subBtn       = nullptr;
    QLabel      *m_subResult    = nullptr;

    // ── 乘法 ──────────────────────────────────────────────────────────────
    QGroupBox   *m_mulGroup     = nullptr;
    QDoubleSpinBox *m_mulA      = nullptr;
    QDoubleSpinBox *m_mulB      = nullptr;
    QPushButton *m_mulBtn       = nullptr;
    QLabel      *m_mulResult    = nullptr;

    // ── Invoker 类模板 ────────────────────────────────────────────────────
    QGroupBox   *m_classGroup   = nullptr;
    QSpinBox    *m_classA       = nullptr;
    QSpinBox    *m_classB       = nullptr;
    QPushButton *m_classBtn     = nullptr;
    QLabel      *m_classResult  = nullptr;

    // ── 调用记录 ──────────────────────────────────────────────────────────
    QGroupBox   *m_logGroup     = nullptr;
    QTextEdit   *m_logEdit      = nullptr;
    QPushButton *m_clearLogBtn  = nullptr;

    CalculatorService *m_service = nullptr;

    void appendLog(const QString &line, const QString &color = "#d4d4d4");
};
