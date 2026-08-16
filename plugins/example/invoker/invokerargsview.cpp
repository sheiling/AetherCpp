#include "invokerargsview.h"

#include "extensionsystem/pluginmanager.h"
#include "extensionsystem/invoker.h"
#include "thememanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QDateTime>
#include <QDebug>
#include <QColor>

using namespace ExtensionSystem;

namespace {
QColor themeColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}
}

// ============================================================================
// 构造
// ============================================================================

InvokerArgsView::InvokerArgsView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId("invoker_args");
    setPageTitle("Invoker 多参数示例");
}

void InvokerArgsView::onLoad()
{
    m_service = PluginManager::getObject<CalculatorService>();
    if (!m_service)
        qWarning() << "[InvokerArgsView] CalculatorService not found in object pool!";

    setupUi();
    setupConnections();
}

// ============================================================================
// setupUi
// ============================================================================
void InvokerArgsView::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(14);

    // ── 标题 ─────────────────────────────────────────────────────────────
    auto *titleLabel = new QLabel(this);
    titleLabel->setText("Invoker 多参数 & 类模板用法");
    QFont tf;
    tf.setPointSize(15);
    tf.setBold(true);
    titleLabel->setFont(tf);
    root->addWidget(titleLabel);

    auto *descLabel = new QLabel(this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(QString("color:%1; font-size:12px;")
                             .arg(themeColorOrDefault("secondaryTextColor", QColor("#9fb0c7")).name()));
    descLabel->setText(
        "演示双参数调用与 Invoker<Result> 类模板的显式用法：\n"
        "  1. invoke<int>(target, \"add\", a, b)         — 函数模板，双 int 参数\n"
        "  2. invoke<int>(target, \"subtract\", a, b)    — 函数模板，双 int 参数\n"
        "  3. invoke<double>(target, \"multiply\", a, b) — 函数模板，双 double 参数\n"
        "  4. Invoker<int>(target, \"add\", a, b)        — 类模板，等价但更显式\n"
        "  5. InvokerBase::wasSuccessful()              — 调用成功性检测");
    root->addWidget(descLabel);

    // ─────────────────────────────────────────────────────────────────────
    // 1. 加法
    // ─────────────────────────────────────────────────────────────────────
    m_addGroup = new QGroupBox("① invoke<int>  —  add(a, b)", this);
    auto *addLayout = new QVBoxLayout(m_addGroup);
    auto *addHint = new QLabel(
        "代码：  int r = invoke<int>(service, \"add\", a, b);", m_addGroup);
    addHint->setStyleSheet(QString("font-family:Consolas; color:%1; font-size:11px;")
                           .arg(themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    addLayout->addWidget(addHint);

    auto *addRow = new QHBoxLayout();
    m_addA = new QSpinBox(m_addGroup);
    m_addA->setRange(-9999, 9999);
    m_addA->setValue(12);
    m_addB = new QSpinBox(m_addGroup);
    m_addB->setRange(-9999, 9999);
    m_addB->setValue(34);
    m_addBtn = new QPushButton("计算 a + b", m_addGroup);
    m_addBtn->setStyleSheet(QString(
        "QPushButton{background:%1;color:%2;padding:6px 14px;border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:%3;}")
        .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    m_addResult = new QLabel("(结果)", m_addGroup);
    m_addResult->setStyleSheet(QString("color:%1; font-weight:bold; font-size:13px;")
                               .arg(themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name()));
    addRow->addWidget(new QLabel("a ="));
    addRow->addWidget(m_addA);
    addRow->addWidget(new QLabel("  b ="));
    addRow->addWidget(m_addB);
    addRow->addWidget(m_addBtn);
    addRow->addWidget(m_addResult, 1);
    addLayout->addLayout(addRow);
    root->addWidget(m_addGroup);

    // ─────────────────────────────────────────────────────────────────────
    // 2. 减法
    // ─────────────────────────────────────────────────────────────────────
    m_subGroup = new QGroupBox("② invoke<int>  —  subtract(a, b)", this);
    auto *subLayout = new QVBoxLayout(m_subGroup);
    auto *subHint = new QLabel(
        "代码：  int r = invoke<int>(service, \"subtract\", a, b);", m_subGroup);
    subHint->setStyleSheet(QString("font-family:Consolas; color:%1; font-size:11px;")
                           .arg(themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    subLayout->addWidget(subHint);

    auto *subRow = new QHBoxLayout();
    m_subA = new QSpinBox(m_subGroup);
    m_subA->setRange(-9999, 9999);
    m_subA->setValue(100);
    m_subB = new QSpinBox(m_subGroup);
    m_subB->setRange(-9999, 9999);
    m_subB->setValue(37);
    m_subBtn = new QPushButton("计算 a - b", m_subGroup);
    m_subBtn->setStyleSheet(QString(
        "QPushButton{background:%1;color:%2;padding:6px 14px;border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:%3;}")
        .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    m_subResult = new QLabel("(结果)", m_subGroup);
    m_subResult->setStyleSheet(QString("color:%1; font-weight:bold; font-size:13px;")
                               .arg(themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name()));
    subRow->addWidget(new QLabel("a ="));
    subRow->addWidget(m_subA);
    subRow->addWidget(new QLabel("  b ="));
    subRow->addWidget(m_subB);
    subRow->addWidget(m_subBtn);
    subRow->addWidget(m_subResult, 1);
    subLayout->addLayout(subRow);
    root->addWidget(m_subGroup);

    // ─────────────────────────────────────────────────────────────────────
    // 3. 乘法（double）
    // ─────────────────────────────────────────────────────────────────────
    m_mulGroup = new QGroupBox("③ invoke<double>  —  multiply(a, b)", this);
    auto *mulLayout = new QVBoxLayout(m_mulGroup);
    auto *mulHint = new QLabel(
        "代码：  double r = invoke<double>(service, \"multiply\", a, b);", m_mulGroup);
    mulHint->setStyleSheet(QString("font-family:Consolas; color:%1; font-size:11px;")
                           .arg(themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    mulLayout->addWidget(mulHint);

    auto *mulRow = new QHBoxLayout();
    m_mulA = new QDoubleSpinBox(m_mulGroup);
    m_mulA->setRange(-9999.0, 9999.0);
    m_mulA->setDecimals(2);
    m_mulA->setValue(3.14);
    m_mulB = new QDoubleSpinBox(m_mulGroup);
    m_mulB->setRange(-9999.0, 9999.0);
    m_mulB->setDecimals(2);
    m_mulB->setValue(2.0);
    m_mulBtn = new QPushButton("计算 a × b", m_mulGroup);
    m_mulBtn->setStyleSheet(QString(
        "QPushButton{background:%1;color:%2;padding:6px 14px;border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:%3;}")
        .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    m_mulResult = new QLabel("(结果)", m_mulGroup);
    m_mulResult->setStyleSheet(QString("color:%1; font-weight:bold; font-size:13px;")
                               .arg(themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name()));
    mulRow->addWidget(new QLabel("a ="));
    mulRow->addWidget(m_mulA);
    mulRow->addWidget(new QLabel("  b ="));
    mulRow->addWidget(m_mulB);
    mulRow->addWidget(m_mulBtn);
    mulRow->addWidget(m_mulResult, 1);
    mulLayout->addLayout(mulRow);
    root->addWidget(m_mulGroup);

    // ─────────────────────────────────────────────────────────────────────
    // 4. Invoker<> 类模板 + wasSuccessful()
    // ─────────────────────────────────────────────────────────────────────
    m_classGroup = new QGroupBox(
        "④ Invoker<int> 类模板  +  wasSuccessful() 检测", this);
    auto *classLayout = new QVBoxLayout(m_classGroup);
    auto *classHint = new QLabel(
        "代码：\n"
        "  Invoker<int> inv(service, \"add\", a, b);\n"
        "  if (inv.wasSuccessful())  int r = (int)inv;",
        m_classGroup);
    classHint->setStyleSheet(QString("font-family:Consolas; color:%1; font-size:11px;")
                             .arg(themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    classLayout->addWidget(classHint);

    auto *classRow = new QHBoxLayout();
    m_classA = new QSpinBox(m_classGroup);
    m_classA->setRange(-9999, 9999);
    m_classA->setValue(7);
    m_classB = new QSpinBox(m_classGroup);
    m_classB->setRange(-9999, 9999);
    m_classB->setValue(8);
    m_classBtn = new QPushButton("用 Invoker<int> 调用 add", m_classGroup);
    m_classBtn->setStyleSheet(QString(
        "QPushButton{background:%1;color:%2;padding:6px 14px;border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:%3;}")
        .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    m_classResult = new QLabel("(结果 + 成功性)", m_classGroup);
    m_classResult->setStyleSheet(QString("color:%1; font-weight:bold; font-size:13px;")
                                 .arg(themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name()));
    classRow->addWidget(new QLabel("a ="));
    classRow->addWidget(m_classA);
    classRow->addWidget(new QLabel("  b ="));
    classRow->addWidget(m_classB);
    classRow->addWidget(m_classBtn);
    classRow->addWidget(m_classResult, 1);
    classLayout->addLayout(classRow);
    root->addWidget(m_classGroup);

    // ─────────────────────────────────────────────────────────────────────
    // 调用记录
    // ─────────────────────────────────────────────────────────────────────
    m_logGroup = new QGroupBox("📋  调用记录", this);
    auto *logLayout = new QVBoxLayout(m_logGroup);
    m_logEdit = new QTextEdit(m_logGroup);
    m_logEdit->setReadOnly(true);
    m_logEdit->setFont(QFont("Consolas", 9));
    m_logEdit->setStyleSheet(QString("QTextEdit{background:%1;color:%2;border:1px solid %3;}")
                             .arg(themeColorOrDefault("secondaryColor", QColor("#1e2a3a")).name(),
                                  themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
                                  themeColorOrDefault("secondaryLightColor", QColor("#263447")).name()));
    m_logEdit->setMinimumHeight(120);
    logLayout->addWidget(m_logEdit);

    m_clearLogBtn = new QPushButton("清空记录", m_logGroup);
    m_clearLogBtn->setFixedWidth(80);
    logLayout->addWidget(m_clearLogBtn);
    root->addWidget(m_logGroup, 1);
}

// ============================================================================
// setupConnections
// ============================================================================
void InvokerArgsView::setupConnections()
{
    if (!m_service) return;

    // 1. 加法：invoke<int>(target, "add", a, b)
    connect(m_addBtn, &QPushButton::clicked, this, [this]() {
        int a = m_addA->value();
        int b = m_addB->value();

        // ★ 函数模板：两个 int 参数
        int result = invoke<int>(m_service, "add", a, b);

        m_addResult->setText(QString("= %1").arg(result));
        appendLog(
            QString("[invoke<int>] add(%1, %2) → %3").arg(a).arg(b).arg(result),
            "#7ee787");
    });

    // 2. 减法：invoke<int>(target, "subtract", a, b)
    connect(m_subBtn, &QPushButton::clicked, this, [this]() {
        int a = m_subA->value();
        int b = m_subB->value();

        // ★ 函数模板：两个 int 参数
        int result = invoke<int>(m_service, "subtract", a, b);

        m_subResult->setText(QString("= %1").arg(result));
        appendLog(
            QString("[invoke<int>] subtract(%1, %2) → %3").arg(a).arg(b).arg(result),
            "#ffa657");
    });

    // 3. 乘法：invoke<double>(target, "multiply", a, b)
    connect(m_mulBtn, &QPushButton::clicked, this, [this]() {
        double a = m_mulA->value();
        double b = m_mulB->value();

        // ★ 函数模板：两个 double 参数
        double result = invoke<double>(m_service, "multiply", a, b);

        m_mulResult->setText(QString("= %1").arg(result, 0, 'f', 4));
        appendLog(
            QString("[invoke<double>] multiply(%1, %2) → %3")
                .arg(a).arg(b).arg(result, 0, 'f', 4),
            "#79c0ff");
    });

    // 4. Invoker<int> 类模板 + wasSuccessful()
    connect(m_classBtn, &QPushButton::clicked, this, [this]() {
        int a = m_classA->value();
        int b = m_classB->value();

        // ★ 类模板：显式构造 Invoker<int>，转型获取结果
        Invoker<int> inv(m_service, "add", a, b);
        bool ok = inv.wasSuccessful();
        int result = (int)inv;

        m_classResult->setText(
            ok ? QString("= %1  ✔ wasSuccessful").arg(result)
               : QString("调用失败  ✘ wasSuccessful"));
        m_classResult->setStyleSheet(
            ok ? "color:#d2a8ff; font-weight:bold; font-size:13px;"
               : "color:#f85149; font-weight:bold; font-size:13px;");

        appendLog(
            QString("[Invoker<int>] add(%1, %2) → %3  [%4]")
                .arg(a).arg(b).arg(result)
                .arg(ok ? "SUCCESS" : "FAILED"),
            ok ? "#d2a8ff" : "#f85149");
    });

    // 5. 清空日志
    connect(m_clearLogBtn, &QPushButton::clicked, m_logEdit, &QTextEdit::clear);
}

void InvokerArgsView::retranslateUi() {}

// ============================================================================
// 辅助：追加带颜色的日志行
// ============================================================================
void InvokerArgsView::appendLog(const QString &line, const QString &color)
{
    m_logEdit->append(
        QString("<span style='color:%1;'>[%2]</span> %3")
            .arg(color)
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
            .arg(line.toHtmlEscaped()));
}
