#include "containernamedview.h"
#include "thememanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QFont>
#include <QDateTime>
#include <QDebug>
#include <QColor>

namespace {
QColor themeColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}
}

// =============================================================================
// 构造
// =============================================================================

ContainerNamedView::ContainerNamedView(Container::Application *app, QWidget *parent)
    : PageWidget(parent)
    , m_app(app)
{
    setPageId("container_named");
    setPageTitle("命名绑定示例");
}

void ContainerNamedView::onLoad()
{
    setupUi();
    setupConnections();
}

// =============================================================================
// setupUi
// =============================================================================
void ContainerNamedView::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 16, 20, 16);
    root->setSpacing(10);

    // ── 标题 ─────────────────────────────────────────────────────────────
    auto *titleLabel = new QLabel(this);
    titleLabel->setText("Container 命名绑定示例");
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
        "命名绑定（Named Binding）核心特性：调用方无需引入服务头文件，仅凭字符串名称完成服务解析与方法调用。\n"
        "  ①  boundNamed(\"name\")                     — 检查命名绑定是否存在\n"
        "  ②  makeNamed(\"logger\") + handle.call()     — 解析句柄并动态调用方法\n"
        "  ③  handle.get(\"path\") / handle.set(\"path\") — 读写属性（readWrite 注册）\n"
        "  ④  makeNamed(\"storage\") + call() 多参数     — 双参数方法与无参方法调用\n"
        "  ⑤  handle.hasMethod(\"name\")                — 检查方法是否存在");
    root->addWidget(descLabel);

    // ── 滚动区域 ──────────────────────────────────────────────────────────
    auto *scrollArea   = new QScrollArea(this);
    auto *scrollWidget = new QWidget(scrollArea);
    auto *scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setContentsMargins(0, 0, 4, 0);
    scrollLayout->setSpacing(10);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    // ─────────────────────────────────────────────────────────────────────
    // ① 注册状态检查
    // ─────────────────────────────────────────────────────────────────────
    m_regGroup = new QGroupBox("① boundNamed()  —  检查命名绑定是否已注册", scrollWidget);
    auto *regLayout = new QVBoxLayout(m_regGroup);
    auto *regHint = new QLabel(
        "代码：\n"
        "  m_app->boundNamed(\"logger\")          // → true\n"
        "  m_app->boundNamed(\"storage\")         // → true\n"
        "  m_app->boundNamed(\"missing-service\") // → false", m_regGroup);
    regHint->setStyleSheet("font-family:Consolas; color:#9cdcfe; font-size:11px;");
    regLayout->addWidget(regHint);
    m_checkRegBtn = new QPushButton("▶  检查命名绑定注册状态", m_regGroup);
    m_checkRegBtn->setStyleSheet(
        "QPushButton{background:#607D8B;color:white;padding:6px 16px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#546e7a;}");
    regLayout->addWidget(m_checkRegBtn);
    scrollLayout->addWidget(m_regGroup);

    // ─────────────────────────────────────────────────────────────────────
    // ② call() 方法调用
    // ─────────────────────────────────────────────────────────────────────
    m_callGroup = new QGroupBox("② makeNamed(\"logger\") + handle.call()  —  动态方法调用", scrollWidget);
    auto *callLayout = new QVBoxLayout(m_callGroup);
    auto *callHint = new QLabel(
        "代码：\n"
        "  auto h = m_app->makeNamed(\"logger\");               // ServiceHandle\n"
        "  h.call(\"info\",  { std::any{std::string(msg)} });   // void info(string)\n"
        "  h.call(\"warn\",  { std::any{std::string(msg)} });   // void warn(string)\n"
        "  auto n = h.call(\"name\");                           // string name() const\n"
        "  auto m = h.call(\"lastMsg\");                        // string lastMsg() const", m_callGroup);
    callHint->setStyleSheet("font-family:Consolas; color:#9cdcfe; font-size:11px;");
    callLayout->addWidget(callHint);

    auto *msgRow = new QHBoxLayout;
    auto *msgLabel = new QLabel("消息内容:", m_callGroup);
    msgLabel->setFixedWidth(60);
    m_msgEdit = new QLineEdit("Hello from named binding!", m_callGroup);
    msgRow->addWidget(msgLabel);
    msgRow->addWidget(m_msgEdit);
    callLayout->addLayout(msgRow);

    auto *btnRow1 = new QHBoxLayout;
    m_infoBtn = new QPushButton("call(\"info\")", m_callGroup);
    m_infoBtn->setStyleSheet(
        "QPushButton{background:#4CAF50;color:white;padding:6px 12px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#45a049;}");
    m_warnBtn = new QPushButton("call(\"warn\")", m_callGroup);
    m_warnBtn->setStyleSheet(
        "QPushButton{background:#FF9800;color:white;padding:6px 12px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#e68900;}");
    m_nameBtn = new QPushButton("call(\"name\")", m_callGroup);
    m_nameBtn->setStyleSheet(
        "QPushButton{background:#2196F3;color:white;padding:6px 12px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#0b7dda;}");
    m_lastMsgBtn = new QPushButton("call(\"lastMsg\")", m_callGroup);
    m_lastMsgBtn->setStyleSheet(
        "QPushButton{background:#9C27B0;color:white;padding:6px 12px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#7B1FA2;}");
    btnRow1->addWidget(m_infoBtn);
    btnRow1->addWidget(m_warnBtn);
    btnRow1->addWidget(m_nameBtn);
    btnRow1->addWidget(m_lastMsgBtn);
    callLayout->addLayout(btnRow1);
    scrollLayout->addWidget(m_callGroup);

    // ─────────────────────────────────────────────────────────────────────
    // ③ 属性 get/set
    // ─────────────────────────────────────────────────────────────────────
    m_propGroup = new QGroupBox("③ handle.get(\"path\") / handle.set(\"path\")  —  属性读写", scrollWidget);
    auto *propLayout = new QVBoxLayout(m_propGroup);
    auto *propHint = new QLabel(
        "代码：\n"
        "  // \"path\" 已通过 d.readWrite(\"path\", &FileAppLogger::path, &FileAppLogger::setPath) 注册\n"
        "  auto pathAny = h.get(\"path\");                         // 返回 std::any{std::string}\n"
        "  auto path    = std::any_cast<std::string>(pathAny);   // 取出路径字符串\n"
        "  h.set(\"path\", std::any{std::string(\"new_path.log\")}); // 写入新路径", m_propGroup);
    propHint->setStyleSheet("font-family:Consolas; color:#9cdcfe; font-size:11px;");
    propLayout->addWidget(propHint);

    auto *curPathRow = new QHBoxLayout;
    auto *curPathTitle = new QLabel("当前路径:", m_propGroup);
    curPathTitle->setFixedWidth(60);
    m_curPathLabel = new QLabel("(未读取)", m_propGroup);
    m_curPathLabel->setStyleSheet("color:#7ee787; font-family:Consolas;");
    curPathRow->addWidget(curPathTitle);
    curPathRow->addWidget(m_curPathLabel);
    curPathRow->addStretch();
    propLayout->addLayout(curPathRow);

    auto *newPathRow = new QHBoxLayout;
    auto *newPathTitle = new QLabel("新路径:", m_propGroup);
    newPathTitle->setFixedWidth(60);
    m_newPathEdit = new QLineEdit("my_custom.log", m_propGroup);
    m_getPathBtn = new QPushButton("get(\"path\")", m_propGroup);
    m_getPathBtn->setStyleSheet(
        "QPushButton{background:#4CAF50;color:white;padding:6px 12px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#45a049;}");
    m_setPathBtn = new QPushButton("set(\"path\")", m_propGroup);
    m_setPathBtn->setStyleSheet(
        "QPushButton{background:#E91E63;color:white;padding:6px 12px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#c2185b;}");
    newPathRow->addWidget(newPathTitle);
    newPathRow->addWidget(m_newPathEdit, 1);
    newPathRow->addWidget(m_getPathBtn);
    newPathRow->addWidget(m_setPathBtn);
    propLayout->addLayout(newPathRow);
    scrollLayout->addWidget(m_propGroup);

    // ─────────────────────────────────────────────────────────────────────
    // ④ storage 多参数调用
    // ─────────────────────────────────────────────────────────────────────
    m_storeGroup = new QGroupBox("④ makeNamed(\"storage\")  —  多参数 & 无参方法调用", scrollWidget);
    auto *storeLayout = new QVBoxLayout(m_storeGroup);
    auto *storeHint = new QLabel(
        "代码：\n"
        "  auto h = m_app->makeNamed(\"storage\");\n"
        "  h.call(\"store\",    { any{key}, any{value} });  // void store(string, string)\n"
        "  h.call(\"retrieve\", { any{key} });              // string retrieve(string) const\n"
        "  h.call(\"has\",      { any{key} });              // bool has(string) const\n"
        "  h.call(\"count\");                               // int count() const\n"
        "  h.call(\"clear\");                               // void clear()", m_storeGroup);
    storeHint->setStyleSheet("font-family:Consolas; color:#9cdcfe; font-size:11px;");
    storeLayout->addWidget(storeHint);

    auto *kvRow = new QHBoxLayout;
    auto *keyLabel   = new QLabel("Key:", m_storeGroup);   keyLabel->setFixedWidth(35);
    m_keyEdit   = new QLineEdit("demo-key",   m_storeGroup);
    auto *valLabel   = new QLabel("Value:", m_storeGroup); valLabel->setFixedWidth(40);
    m_valueEdit = new QLineEdit("demo-value", m_storeGroup);
    kvRow->addWidget(keyLabel);
    kvRow->addWidget(m_keyEdit);
    kvRow->addWidget(valLabel);
    kvRow->addWidget(m_valueEdit);
    storeLayout->addLayout(kvRow);

    auto *storeBtnRow = new QHBoxLayout;
    m_storeBtn    = new QPushButton("store(key,val)", m_storeGroup);
    m_retrieveBtn = new QPushButton("retrieve(key)",  m_storeGroup);
    m_hasBtn      = new QPushButton("has(key)",       m_storeGroup);
    m_countBtn    = new QPushButton("count()",        m_storeGroup);
    m_clearBtn    = new QPushButton("clear()",        m_storeGroup);

    const auto makeStoreBtnStyle = [](const char* bg) {
        return QString("QPushButton{background:%1;color:white;padding:5px 10px;"
                       "border-radius:4px;font-weight:bold;}"
                       "QPushButton:hover{background:%1;}").arg(bg);
    };
    m_storeBtn   ->setStyleSheet(makeStoreBtnStyle("#4CAF50"));
    m_retrieveBtn->setStyleSheet(makeStoreBtnStyle("#2196F3"));
    m_hasBtn     ->setStyleSheet(makeStoreBtnStyle("#FF9800"));
    m_countBtn   ->setStyleSheet(makeStoreBtnStyle("#9C27B0"));
    m_clearBtn   ->setStyleSheet(makeStoreBtnStyle("#f44336"));

    storeBtnRow->addWidget(m_storeBtn);
    storeBtnRow->addWidget(m_retrieveBtn);
    storeBtnRow->addWidget(m_hasBtn);
    storeBtnRow->addWidget(m_countBtn);
    storeBtnRow->addWidget(m_clearBtn);
    storeLayout->addLayout(storeBtnRow);
    scrollLayout->addWidget(m_storeGroup);

    // ─────────────────────────────────────────────────────────────────────
    // ⑤ 存在性检查
    // ─────────────────────────────────────────────────────────────────────
    m_checkGroup = new QGroupBox("⑤ handle.hasMethod()  —  方法存在性检查", scrollWidget);
    auto *checkLayout = new QVBoxLayout(m_checkGroup);
    auto *checkHint = new QLabel(
        "代码：\n"
        "  auto h = m_app->makeNamed(\"logger\");\n"
        "  h.hasMethod(\"info\")      // → true（已通过 d.on(\"info\", ...) 注册）\n"
        "  h.hasMethod(\"notExist\")  // → false（未注册）", m_checkGroup);
    checkHint->setStyleSheet("font-family:Consolas; color:#9cdcfe; font-size:11px;");
    checkLayout->addWidget(checkHint);

    auto *checkBtnRow = new QHBoxLayout;
    m_boundLoggerBtn  = new QPushButton("boundNamed(\"logger\")",      m_checkGroup);
    m_boundMissingBtn = new QPushButton("boundNamed(\"not-exist\")",   m_checkGroup);
    m_hasMethodBtn    = new QPushButton("hasMethod(\"info\")",         m_checkGroup);
    m_noMethodBtn     = new QPushButton("hasMethod(\"notExist\")",     m_checkGroup);

    m_boundLoggerBtn ->setStyleSheet("QPushButton{background:#4CAF50;color:white;padding:5px 10px;border-radius:4px;font-weight:bold;}QPushButton:hover{background:#45a049;}");
    m_boundMissingBtn->setStyleSheet("QPushButton{background:#f44336;color:white;padding:5px 10px;border-radius:4px;font-weight:bold;}QPushButton:hover{background:#d32f2f;}");
    m_hasMethodBtn   ->setStyleSheet("QPushButton{background:#4CAF50;color:white;padding:5px 10px;border-radius:4px;font-weight:bold;}QPushButton:hover{background:#45a049;}");
    m_noMethodBtn    ->setStyleSheet("QPushButton{background:#f44336;color:white;padding:5px 10px;border-radius:4px;font-weight:bold;}QPushButton:hover{background:#d32f2f;}");

    checkBtnRow->addWidget(m_boundLoggerBtn);
    checkBtnRow->addWidget(m_boundMissingBtn);
    checkBtnRow->addWidget(m_hasMethodBtn);
    checkBtnRow->addWidget(m_noMethodBtn);
    checkLayout->addLayout(checkBtnRow);
    scrollLayout->addWidget(m_checkGroup);

    scrollLayout->addStretch();
    root->addWidget(scrollArea, 1);

    // ── 调用记录 ──────────────────────────────────────────────────────────
    m_logGroup = new QGroupBox("📋  演示记录", this);
    auto *logLayout = new QVBoxLayout(m_logGroup);
    m_logEdit = new QTextEdit(m_logGroup);
    m_logEdit->setReadOnly(true);
    m_logEdit->setFont(QFont("Consolas", 9));
    m_logEdit->setStyleSheet(QString("QTextEdit{background:%1;color:%2;border:1px solid %3;}")
                             .arg(themeColorOrDefault("secondaryColor", QColor("#1e2a3a")).name(),
                                  themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
                                  themeColorOrDefault("secondaryLightColor", QColor("#263447")).name()));
    m_logEdit->setMinimumHeight(160);
    logLayout->addWidget(m_logEdit);

    m_clearLogBtn = new QPushButton("清空记录", m_logGroup);
    m_clearLogBtn->setFixedWidth(80);
    logLayout->addWidget(m_clearLogBtn);
    m_logGroup->setMaximumHeight(220);
    root->addWidget(m_logGroup);
}

// =============================================================================
// setupConnections
// =============================================================================
void ContainerNamedView::setupConnections()
{
    if (!m_app) {
        appendLog("❌ Application 实例为空，无法执行命名绑定演示", "#f85149");
        return;
    }

    // ── ① 注册状态检查 ────────────────────────────────────────────────────
    connect(m_checkRegBtn, &QPushButton::clicked, this, [this]() {
        appendSection("① boundNamed() — 命名绑定注册状态检查");

        bool loggerOk  = m_app->boundNamed("logger");
        bool storageOk = m_app->boundNamed("storage");
        bool missingOk = m_app->boundNamed("missing-service");

        appendLog(QString("boundNamed(\"logger\")          = %1")
            .arg(loggerOk  ? "true  ✓" : "false"), loggerOk  ? "#7ee787" : "#f85149");
        appendLog(QString("boundNamed(\"storage\")         = %1")
            .arg(storageOk ? "true  ✓" : "false"), storageOk ? "#7ee787" : "#f85149");
        appendLog(QString("boundNamed(\"missing-service\") = %1")
            .arg(missingOk ? "true" : "false ✓"), missingOk ? "#f85149" : "#7ee787");
        appendLog("← 命名绑定在 AppLoggerProvider/AppStorageProvider::doRegister() 中注册",
                  "#ffa657");
    });

    // ── ② call() 方法调用 ─────────────────────────────────────────────────
    connect(m_infoBtn, &QPushButton::clicked, this, [this]() {
        appendSection("② handle.call(\"info\") — 调用 void info(string)");
        auto h = m_app->makeNamed("logger");
        std::string msg = m_msgEdit->text().toStdString();
        h.call("info", { std::any{msg} });
        appendLog(QString("call(\"info\", { \"%1\" }) → void (已执行，无返回值)")
            .arg(QString::fromStdString(msg)), "#7ee787");
        appendLog("← info 已通过 d.on(\"info\", &FileAppLogger::info) 注册", "#ffa657");
    });

    connect(m_warnBtn, &QPushButton::clicked, this, [this]() {
        appendSection("② handle.call(\"warn\") — 调用 void warn(string)");
        auto h = m_app->makeNamed("logger");
        std::string msg = m_msgEdit->text().toStdString();
        h.call("warn", { std::any{msg} });
        appendLog(QString("call(\"warn\", { \"%1\" }) → void (已执行)")
            .arg(QString::fromStdString(msg)), "#ffa657");
    });

    connect(m_nameBtn, &QPushButton::clicked, this, [this]() {
        appendSection("② handle.call(\"name\") — 调用 string name() const");
        auto h = m_app->makeNamed("logger");
        auto result = h.call("name");
        std::string name = std::any_cast<std::string>(result);
        appendLog(QString("call(\"name\") → \"%1\"")
            .arg(QString::fromStdString(name)), "#79c0ff");
        appendLog("← 调用方无需引入 FileAppLogger 头文件，仅通过字符串名称调用", "#ffa657");
    });

    connect(m_lastMsgBtn, &QPushButton::clicked, this, [this]() {
        appendSection("② handle.call(\"lastMsg\") — 调用 string lastMsg() const");
        auto h = m_app->makeNamed("logger");
        auto result = h.call("lastMsg");
        std::string lastMsg = std::any_cast<std::string>(result);
        if (lastMsg.empty()) {
            appendLog("call(\"lastMsg\") → \"\" (还未调用过 info/warn)", "#888");
        } else {
            appendLog(QString("call(\"lastMsg\") → \"%1\"")
                .arg(QString::fromStdString(lastMsg)), "#9cdcfe");
        }
    });

    // ── ③ 属性 get/set ────────────────────────────────────────────────────
    connect(m_getPathBtn, &QPushButton::clicked, this, [this]() {
        appendSection("③ handle.get(\"path\") — 读取属性");
        auto h = m_app->makeNamed("logger");
        auto pathAny = h.get("path");
        std::string path = std::any_cast<std::string>(pathAny);
        m_curPathLabel->setText(QString::fromStdString(path));
        appendLog(QString("get(\"path\") → \"%1\"")
            .arg(QString::fromStdString(path)), "#7ee787");
        appendLog("← 属性通过 d.readWrite(\"path\", getter, setter) 注册", "#ffa657");
    });

    connect(m_setPathBtn, &QPushButton::clicked, this, [this]() {
        appendSection("③ handle.set(\"path\") — 写入属性");
        auto h = m_app->makeNamed("logger");
        std::string newPath = m_newPathEdit->text().toStdString();
        h.set("path", std::any{newPath});
        appendLog(QString("set(\"path\", \"%1\") → void")
            .arg(QString::fromStdString(newPath)), "#E91E63");

        // 读回验证
        auto verifyAny  = h.get("path");
        std::string verified = std::any_cast<std::string>(verifyAny);
        m_curPathLabel->setText(QString::fromStdString(verified));
        appendLog(QString("验证：get(\"path\") → \"%1\" ✓")
            .arg(QString::fromStdString(verified)), "#7ee787");
    });

    // ── ④ storage 多参数调用 ──────────────────────────────────────────────
    connect(m_storeBtn, &QPushButton::clicked, this, [this]() {
        appendSection("④ handle.call(\"store\", {key, value}) — 双参数方法");
        auto h = m_app->makeNamed("storage");
        std::string key = m_keyEdit->text().toStdString();
        std::string val = m_valueEdit->text().toStdString();
        h.call("store", { std::any{key}, std::any{val} });
        appendLog(QString("call(\"store\", { \"%1\", \"%2\" }) → void")
            .arg(QString::fromStdString(key))
            .arg(QString::fromStdString(val)), "#4CAF50");
    });

    connect(m_retrieveBtn, &QPushButton::clicked, this, [this]() {
        appendSection("④ handle.call(\"retrieve\", {key}) — 单参数 const 方法");
        auto h = m_app->makeNamed("storage");
        std::string key = m_keyEdit->text().toStdString();
        auto result = h.call("retrieve", { std::any{key} });
        std::string val = std::any_cast<std::string>(result);
        appendLog(QString("call(\"retrieve\", { \"%1\" }) → \"%2\"")
            .arg(QString::fromStdString(key))
            .arg(QString::fromStdString(val)),
            val == "(not found)" ? "#f85149" : "#7ee787");
    });

    connect(m_hasBtn, &QPushButton::clicked, this, [this]() {
        appendSection("④ handle.call(\"has\", {key}) — 布尔返回值");
        auto h = m_app->makeNamed("storage");
        std::string key = m_keyEdit->text().toStdString();
        auto result = h.call("has", { std::any{key} });
        bool exists = std::any_cast<bool>(result);
        appendLog(QString("call(\"has\", { \"%1\" }) → %2")
            .arg(QString::fromStdString(key))
            .arg(exists ? "true ✓" : "false"),
            exists ? "#7ee787" : "#ffa657");
    });

    connect(m_countBtn, &QPushButton::clicked, this, [this]() {
        appendSection("④ handle.call(\"count\") — 无参数，int 返回值");
        auto h = m_app->makeNamed("storage");
        auto result = h.call("count");
        int cnt = std::any_cast<int>(result);
        appendLog(QString("call(\"count\") → %1  (当前存储条目数)").arg(cnt), "#79c0ff");
    });

    connect(m_clearBtn, &QPushButton::clicked, this, [this]() {
        appendSection("④ handle.call(\"clear\") — 无参数，void");
        auto h = m_app->makeNamed("storage");
        h.call("clear");
        appendLog("call(\"clear\") → void (已清空所有条目)", "#f44336");

        // 验证 count() == 0
        auto result = h.call("count");
        int cnt = std::any_cast<int>(result);
        appendLog(QString("验证：call(\"count\") → %1 ✓").arg(cnt), "#7ee787");
    });

    // ── ⑤ 存在性检查 ─────────────────────────────────────────────────────
    connect(m_boundLoggerBtn, &QPushButton::clicked, this, [this]() {
        appendSection("⑤ boundNamed(\"logger\") — 已注册的绑定");
        bool ok = m_app->boundNamed("logger");
        appendLog(QString("boundNamed(\"logger\") → %1").arg(ok ? "true ✓" : "false"),
                  ok ? "#7ee787" : "#f85149");
    });

    connect(m_boundMissingBtn, &QPushButton::clicked, this, [this]() {
        appendSection("⑤ boundNamed(\"not-exist\") — 未注册的绑定");
        bool ok = m_app->boundNamed("not-exist");
        appendLog(QString("boundNamed(\"not-exist\") → %1").arg(ok ? "true" : "false ✓"),
                  ok ? "#f85149" : "#7ee787");
        appendLog("← 未注册时返回 false，可用于防御性检查", "#ffa657");
    });

    connect(m_hasMethodBtn, &QPushButton::clicked, this, [this]() {
        appendSection("⑤ handle.hasMethod(\"info\") — 已注册的方法");
        auto h = m_app->makeNamed("logger");
        bool ok = h.hasMethod("info");
        appendLog(QString("handle.hasMethod(\"info\") → %1").arg(ok ? "true ✓" : "false"),
                  ok ? "#7ee787" : "#f85149");
    });

    connect(m_noMethodBtn, &QPushButton::clicked, this, [this]() {
        appendSection("⑤ handle.hasMethod(\"notExist\") — 未注册的方法");
        auto h = m_app->makeNamed("logger");
        bool ok = h.hasMethod("notExist");
        appendLog(QString("handle.hasMethod(\"notExist\") → %1").arg(ok ? "true" : "false ✓"),
                  ok ? "#f85149" : "#7ee787");
        appendLog("← 未注册时返回 false，call() 会抛出 runtime_error", "#ffa657");
    });

    // ── 清空记录 ──────────────────────────────────────────────────────────
    connect(m_clearLogBtn, &QPushButton::clicked, m_logEdit, &QTextEdit::clear);
}

void ContainerNamedView::retranslateUi() {}

// =============================================================================
// 辅助函数
// =============================================================================
void ContainerNamedView::appendLog(const QString &line, const QString &color)
{
    m_logEdit->append(
        QString("<span style='color:%1;'>[%2]</span> %3")
            .arg(color)
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
            .arg(line.toHtmlEscaped()));
}

void ContainerNamedView::appendSection(const QString &title)
{
    m_logEdit->append(
        QString("<span style='color:%1; font-weight:bold;'>━━ %2 ━━</span>")
            .arg(themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name())
            .arg(title.toHtmlEscaped()));
}
