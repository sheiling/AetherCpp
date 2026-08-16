#include "invokerbasicview.h"

#include "extensionsystem/pluginmanager.h"
#include "extensionsystem/invoker.h"
#include "thememanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFont>
#include <QIntValidator>
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

QString solidButtonStyle(const QColor &base, const QColor &hover, const QColor &disabled)
{
    return QString(
        "QPushButton{background:%1;color:%2;padding:6px 14px;border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:%3;}"
        "QPushButton:disabled{background:%4;color:%5;}")
        .arg(base.name(), themeColorOrDefault("primaryTextColor", QColor("#ffffff")).name(),
             hover.name(), disabled.name(), themeColorOrDefault("secondaryTextColor", QColor("#9fb0c7")).name());
}
}

// ============================================================================
// 构造
// ============================================================================

InvokerBasicView::InvokerBasicView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId("invoker_basic");
    setPageTitle("Invoker 基础示例");
}

void InvokerBasicView::onLoad()
{
    // 从对象池获取服务（由 InvokerPlugin 提前注册）
    m_service = PluginManager::getObject<CalculatorService>();
    if (!m_service)
        qWarning() << "[InvokerBasicView] CalculatorService not found in object pool!";

    setupUi();
    setupConnections();
}

// ============================================================================
// setupUi
// ============================================================================
void InvokerBasicView::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(14);

    // ── 标题 ─────────────────────────────────────────────────────────────
    auto *titleLabel = new QLabel(this);
    titleLabel->setText("Invoker 基础用法示例");
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
        "演示 ExtensionSystem::invoke<Result>(target, \"slotName\", args...) 的三种基本形态：\n"
        "  1. invoke<void>    — 调用无返回值方法 logMessage(QString)\n"
        "  2. invoke<QString> — 调用有返回值方法 greet(QString)\n"
        "  3. invoke<int>     — 调用单参数整数方法 factorial(int)\n"
        "目标对象 CalculatorService 通过 PluginManager::getObject<>() 从对象池中获取。");
    root->addWidget(descLabel);

    // ─────────────────────────────────────────────────────────────────────
    // 1. 调用 void logMessage(const QString &msg)
    // ─────────────────────────────────────────────────────────────────────
    m_voidGroup = new QGroupBox("① invoke<void>  —  logMessage(msg)", this);
    auto *voidLayout = new QVBoxLayout(m_voidGroup);

    auto *voidHint = new QLabel(
        "代码：  invoke<void>(service, \"logMessage\", msg);", m_voidGroup);
    voidHint->setStyleSheet(QString("font-family:Consolas; color:%1; font-size:11px;")
                            .arg(themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));

    auto *voidRow = new QHBoxLayout();
    m_logMsgEdit = new QLineEdit(m_voidGroup);
    m_logMsgEdit->setPlaceholderText("输入要记录的消息...");
    m_logMsgEdit->setText("Hello from Invoker!");
    m_logMsgBtn  = new QPushButton("调用 logMessage", m_voidGroup);
    m_logMsgBtn->setStyleSheet(solidButtonStyle(
        themeColorOrDefault("primaryColor", QColor("#0078d7")),
        themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")),
        themeColorOrDefault("secondaryColor", QColor("#1e2a3a"))));
    voidRow->addWidget(m_logMsgEdit, 1);
    voidRow->addWidget(m_logMsgBtn);

    voidLayout->addWidget(voidHint);
    voidLayout->addLayout(voidRow);
    root->addWidget(m_voidGroup);

    // ─────────────────────────────────────────────────────────────────────
    // 2. 调用 QString greet(const QString &name)
    // ─────────────────────────────────────────────────────────────────────
    m_greetGroup = new QGroupBox("② invoke<QString>  —  greet(name)", this);
    auto *greetLayout = new QVBoxLayout(m_greetGroup);

    auto *greetHint = new QLabel(
        "代码：  QString r = invoke<QString>(service, \"greet\", name);", m_greetGroup);
    greetHint->setStyleSheet(QString("font-family:Consolas; color:%1; font-size:11px;")
                             .arg(themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));

    auto *greetForm = new QFormLayout();
    m_greetNameEdit = new QLineEdit(m_greetGroup);
    m_greetNameEdit->setPlaceholderText("输入姓名...");
    m_greetNameEdit->setText("World");
    greetForm->addRow("姓名:", m_greetNameEdit);

    auto *greetBtnRow = new QHBoxLayout();
    m_greetBtn    = new QPushButton("调用 greet", m_greetGroup);
    m_greetBtn->setStyleSheet(solidButtonStyle(
        themeColorOrDefault("primaryColor", QColor("#0078d7")),
        themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")),
        themeColorOrDefault("secondaryColor", QColor("#1e2a3a"))));
    m_greetResult = new QLabel("(返回值将显示在此处)", m_greetGroup);
    m_greetResult->setStyleSheet(QString("color:%1; font-weight:bold;")
                                 .arg(themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name()));
    greetBtnRow->addWidget(m_greetBtn);
    greetBtnRow->addWidget(m_greetResult, 1);

    greetLayout->addWidget(greetHint);
    greetLayout->addLayout(greetForm);
    greetLayout->addLayout(greetBtnRow);
    root->addWidget(m_greetGroup);

    // ─────────────────────────────────────────────────────────────────────
    // 3. 调用 int factorial(int n)
    // ─────────────────────────────────────────────────────────────────────
    m_factGroup = new QGroupBox("③ invoke<int>  —  factorial(n)", this);
    auto *factLayout = new QVBoxLayout(m_factGroup);

    auto *factHint = new QLabel(
        "代码：  int r = invoke<int>(service, \"factorial\", n);", m_factGroup);
    factHint->setStyleSheet(QString("font-family:Consolas; color:%1; font-size:11px;")
                            .arg(themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));

    auto *factRow = new QHBoxLayout();
    m_factNEdit = new QLineEdit(m_factGroup);
    m_factNEdit->setPlaceholderText("输入 n（1-12）");
    m_factNEdit->setText("5");
    m_factNEdit->setValidator(new QIntValidator(1, 12, m_factGroup));
    m_factBtn = new QPushButton("计算 n!", m_factGroup);
    m_factBtn->setStyleSheet(solidButtonStyle(
        themeColorOrDefault("primaryColor", QColor("#0078d7")),
        themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")),
        themeColorOrDefault("secondaryColor", QColor("#1e2a3a"))));
    m_factResult = new QLabel("(返回值将显示在此处)", m_factGroup);
    m_factResult->setStyleSheet(QString("color:%1; font-weight:bold;")
                                .arg(themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name()));
    factRow->addWidget(new QLabel("n =", m_factGroup));
    factRow->addWidget(m_factNEdit);
    factRow->addWidget(m_factBtn);
    factRow->addWidget(m_factResult, 1);

    factLayout->addWidget(factHint);
    factLayout->addLayout(factRow);
    root->addWidget(m_factGroup);

    // ─────────────────────────────────────────────────────────────────────
    // 调用记录日志
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
void InvokerBasicView::setupConnections()
{
    if (!m_service) return;

    // 1. 服务自身的 logAppended 信号 → 日志面板
    connect(m_service, &CalculatorService::logAppended, this,
            [this](const QString &msg) {
                appendLog(QString("[logMessage] %1").arg(msg), "#9cdcfe");
            });

    // 2. 调用 logMessage（void，无返回值）
    connect(m_logMsgBtn, &QPushButton::clicked, this, [this]() {
        QString msg = m_logMsgEdit->text().trimmed();
        if (msg.isEmpty()) return;

        // ★ 核心用法：invoke<void>(target, "slot", arg0)
        invoke<void>(m_service, "logMessage", msg);

        appendLog(QString("[invoke<void>] logMessage(\"%1\") 调用完成").arg(msg), "#4ec9b0");
    });

    // 3. 调用 greet（返回 QString）
    connect(m_greetBtn, &QPushButton::clicked, this, [this]() {
        QString name = m_greetNameEdit->text().trimmed();
        if (name.isEmpty()) name = "World";

        // ★ 核心用法：invoke<QString>(target, "slot", arg0)
        QString result = invoke<QString>(m_service, "greet", name);

        m_greetResult->setText(QString("返回值：\"%1\"").arg(result));
        appendLog(QString("[invoke<QString>] greet(\"%1\") → \"%2\"").arg(name, result),
                  "#7ee787");
    });

    // 4. 调用 factorial（返回 int）
    connect(m_factBtn, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        int n = m_factNEdit->text().toInt(&ok);
        if (!ok || n < 1) n = 5;

        // ★ 核心用法：invoke<int>(target, "slot", arg0)
        int result = invoke<int>(m_service, "factorial", n);

        m_factResult->setText(QString("返回值：%1! = %2").arg(n).arg(result));
        appendLog(QString("[invoke<int>] factorial(%1) → %2").arg(n).arg(result),
                  "#ffa657");
    });

    // 5. 清空日志
    connect(m_clearLogBtn, &QPushButton::clicked, m_logEdit, &QTextEdit::clear);
}

void InvokerBasicView::retranslateUi() {}

// ============================================================================
// 辅助：追加带颜色的日志行
// ============================================================================
void InvokerBasicView::appendLog(const QString &line, const QString &color)
{
    m_logEdit->append(
        QString("<span style='color:%1;'>[%2]</span> %3")
            .arg(color)
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
            .arg(line.toHtmlEscaped()));
}
