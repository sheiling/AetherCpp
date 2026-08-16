#include "containertypeview.h"
#include "thememanager.h"

#include "appservices.h"
#include "container/container.h"
#include "container/application.h"
#include "container/facade.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
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

// ── Facade 定义（用于演示 Facade<T> 静态访问模式）───────────────────────────
// 继承 Container::Facade<IAppLogger>，无需传递容器引用即可访问服务
class AppLog : public Container::Facade<IAppLogger> {};

// =============================================================================
// 构造
// =============================================================================

ContainerTypeView::ContainerTypeView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId("container_type");
    setPageTitle("类型绑定示例");
}

void ContainerTypeView::onLoad()
{
    setupUi();
    setupConnections();
}

// =============================================================================
// setupUi
// =============================================================================
void ContainerTypeView::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 16, 20, 16);
    root->setSpacing(10);

    // ── 标题 ─────────────────────────────────────────────────────────────
    auto *titleLabel = new QLabel(this);
    titleLabel->setText("Container 类型绑定示例");
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
        "每次点击按钮都会创建一个独立的 Container::Container 实例并执行演示，各区块互不干扰。\n"
        "  ①  bind<Abstract, Concrete>()       — 瞬态：每次 make() 返回全新实例\n"
        "  ②  singleton<Abstract, Concrete>()  — 单例：所有 make() 返回同一实例\n"
        "  ③  instance<Abstract>(ptr)          — 预建实例：注册已有对象\n"
        "  ④  extend<Abstract>(decorator)      — 装饰扩展：AOP 包装已有绑定\n"
        "  ⑤  when<A>().needs<B>().give<C>()   — 上下文绑定：按调用者定制依赖\n"
        "  ⑥  tag<A,B>(label) + tagged(label)  — 标签分组：批量解析同类服务");
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
    // ① bind<> 瞬态绑定
    // ─────────────────────────────────────────────────────────────────────
    m_bindGroup = new QGroupBox("① bind<Abstract, Concrete>()  —  瞬态绑定", scrollWidget);
    auto *bindLayout = new QVBoxLayout(m_bindGroup);

    auto *bindHint = new QLabel(
        "代码：\n"
        "  Container::Container c;\n"
        "  c.bind<IAppLogger, ConsoleAppLogger>();\n"
        "  auto a = c.make<IAppLogger>();  // 新实例\n"
        "  auto b = c.make<IAppLogger>();  // 又一个新实例\n"
        "  assert(a.get() != b.get());     // 地址不同！", m_bindGroup);
    bindHint->setStyleSheet("font-family:Consolas; color:#9cdcfe; font-size:11px;");
    bindLayout->addWidget(bindHint);

    m_bindBtn = new QPushButton("▶  运行 bind<> 演示", m_bindGroup);
    m_bindBtn->setStyleSheet(
        "QPushButton{background:#4CAF50;color:white;padding:6px 16px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#45a049;}");
    bindLayout->addWidget(m_bindBtn);
    scrollLayout->addWidget(m_bindGroup);

    // ─────────────────────────────────────────────────────────────────────
    // ② singleton<> 单例绑定
    // ─────────────────────────────────────────────────────────────────────
    m_singletonGroup = new QGroupBox("② singleton<Abstract, Concrete>()  —  单例绑定", scrollWidget);
    auto *singLayout = new QVBoxLayout(m_singletonGroup);

    auto *singHint = new QLabel(
        "代码：\n"
        "  Container::Container c;\n"
        "  c.singleton<IAppStorage, MemoryAppStorage>();\n"
        "  auto a = c.make<IAppStorage>(); a->store(\"x\", \"42\");\n"
        "  auto b = c.make<IAppStorage>();  // 与 a 是同一实例\n"
        "  b->retrieve(\"x\") == \"42\"         // 通过 b 读到 a 写入的数据", m_singletonGroup);
    singHint->setStyleSheet("font-family:Consolas; color:#9cdcfe; font-size:11px;");
    singLayout->addWidget(singHint);

    m_singletonBtn = new QPushButton("▶  运行 singleton<> 演示", m_singletonGroup);
    m_singletonBtn->setStyleSheet(
        "QPushButton{background:#2196F3;color:white;padding:6px 16px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#0b7dda;}");
    singLayout->addWidget(m_singletonBtn);
    scrollLayout->addWidget(m_singletonGroup);

    // ─────────────────────────────────────────────────────────────────────
    // ③ instance<> 预建实例
    // ─────────────────────────────────────────────────────────────────────
    m_instanceGroup = new QGroupBox("③ instance<Abstract>(ptr)  —  注册预建实例", scrollWidget);
    auto *instLayout = new QVBoxLayout(m_instanceGroup);

    auto *instHint = new QLabel(
        "代码：\n"
        "  auto preBuilt = std::make_shared<ConsoleAppLogger>();\n"
        "  preBuilt->info(\"预建实例\");\n"
        "  Container::Container c;\n"
        "  c.instance<IAppLogger>(preBuilt);\n"
        "  auto r = c.make<IAppLogger>();  // r == preBuilt（同一指针）", m_instanceGroup);
    instHint->setStyleSheet("font-family:Consolas; color:#9cdcfe; font-size:11px;");
    instLayout->addWidget(instHint);

    m_instanceBtn = new QPushButton("▶  运行 instance<> 演示", m_instanceGroup);
    m_instanceBtn->setStyleSheet(
        "QPushButton{background:#FF9800;color:white;padding:6px 16px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#e68900;}");
    instLayout->addWidget(m_instanceBtn);
    scrollLayout->addWidget(m_instanceGroup);

    // ─────────────────────────────────────────────────────────────────────
    // ④ extend<> 装饰扩展
    // ─────────────────────────────────────────────────────────────────────
    m_extendGroup = new QGroupBox("④ extend<Abstract>(decorator)  —  AOP 装饰扩展", scrollWidget);
    auto *extLayout = new QVBoxLayout(m_extendGroup);

    auto *extHint = new QLabel(
        "代码：\n"
        "  Container::Container c;\n"
        "  c.singleton<IAppLogger>([]{ return make_shared<ConsoleAppLogger>(); });\n"
        "  // extend 之前：make<IAppLogger>() → ConsoleAppLogger\n"
        "  c.extend<IAppLogger>([](shared_ptr<IAppLogger> base, Container&) {\n"
        "      return make_shared<TimestampLoggerDecorator>(base);\n"
        "  });\n"
        "  // extend 之后：make<IAppLogger>() → TimestampDecorator(ConsoleLogger)", m_extendGroup);
    extHint->setStyleSheet("font-family:Consolas; color:#9cdcfe; font-size:11px;");
    extLayout->addWidget(extHint);

    m_extendBtn = new QPushButton("▶  运行 extend<> 演示", m_extendGroup);
    m_extendBtn->setStyleSheet(
        "QPushButton{background:#9C27B0;color:white;padding:6px 16px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#7B1FA2;}");
    extLayout->addWidget(m_extendBtn);
    scrollLayout->addWidget(m_extendGroup);

    // ─────────────────────────────────────────────────────────────────────
    // ⑤ 上下文绑定（contextual binding）
    // ─────────────────────────────────────────────────────────────────────
    m_contextualGroup = new QGroupBox("⑤ when<A>().needs<B>().give<C>()  —  上下文绑定", scrollWidget);
    auto *ctxLayout = new QVBoxLayout(m_contextualGroup);

    auto *ctxHint = new QLabel(
        "代码：\n"
        "  Container::Container c;\n"
        "  c.bind<IAppLogger>([]{ return make_shared<ConsoleAppLogger>(); });\n"
        "  // 声明：当 ClientB 需要 IAppLogger 时，给它 FileAppLogger\n"
        "  c.when<ClientB>().needs<IAppLogger>().give<FileAppLogger>(\n"
        "      []{ return make_shared<FileAppLogger>(\"client_b.log\"); });\n"
        "  c.bind<ClientA>([](Container& cc){ return make_shared<ClientA>(cc.make<IAppLogger>()); });\n"
        "  c.bind<ClientB>([](Container& cc){ return make_shared<ClientB>(cc.make<IAppLogger>()); });\n"
        "  // ClientA → ConsoleAppLogger（默认）\n"
        "  // ClientB → FileAppLogger（上下文绑定定制）", m_contextualGroup);
    ctxHint->setStyleSheet("font-family:Consolas; color:#9cdcfe; font-size:11px;");
    ctxLayout->addWidget(ctxHint);

    m_contextualBtn = new QPushButton("▶  运行上下文绑定演示", m_contextualGroup);
    m_contextualBtn->setStyleSheet(
        "QPushButton{background:#00BCD4;color:white;padding:6px 16px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#00a5bc;}");
    ctxLayout->addWidget(m_contextualBtn);
    scrollLayout->addWidget(m_contextualGroup);

    // ─────────────────────────────────────────────────────────────────────
    // ⑥ 标签分组（tag/tagged）
    // ─────────────────────────────────────────────────────────────────────
    m_tagGroup = new QGroupBox("⑥ tag<A,B,...>(label) + tagged(label)  —  标签分组", scrollWidget);
    auto *tagLayout = new QVBoxLayout(m_tagGroup);

    auto *tagHint = new QLabel(
        "代码：\n"
        "  Container::Container c;\n"
        "  c.singleton<IAppLogger>([]{ return make_shared<ConsoleAppLogger>(); });\n"
        "  c.singleton<IAppStorage>([]{ return make_shared<MemoryAppStorage>(); });\n"
        "  c.tag<IAppLogger, IAppStorage>(\"core-services\");\n"
        "  auto services = c.tagged(\"core-services\"); // vector<shared_ptr<void>>\n"
        "  // services[0] → IAppLogger, services[1] → IAppStorage", m_tagGroup);
    tagHint->setStyleSheet("font-family:Consolas; color:#9cdcfe; font-size:11px;");
    tagLayout->addWidget(tagHint);

    m_tagBtn = new QPushButton("▶  运行 tag/tagged 演示", m_tagGroup);
    m_tagBtn->setStyleSheet(
        "QPushButton{background:#607D8B;color:white;padding:6px 16px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#546e7a;}");
    tagLayout->addWidget(m_tagBtn);
    scrollLayout->addWidget(m_tagGroup);

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
void ContainerTypeView::setupConnections()
{
    // ── ① bind<> 瞬态绑定 ─────────────────────────────────────────────────
    connect(m_bindBtn, &QPushButton::clicked, this, [this]() {
        appendSection("① bind<IAppLogger, ConsoleAppLogger>() — 瞬态绑定");

        // ★ 每次演示创建独立的局部容器
        Container::Container c;
        c.bind<IAppLogger, ConsoleAppLogger>();

        auto a = c.make<IAppLogger>();
        auto b = c.make<IAppLogger>();
        auto d = c.make<IAppLogger>();

        appendLog(QString("实例 A — name: %1  [ptr: 0x%2]")
            .arg(QString::fromStdString(a->name()))
            .arg(reinterpret_cast<quintptr>(a.get()), 0, 16), "#7ee787");
        appendLog(QString("实例 B — name: %1  [ptr: 0x%2]")
            .arg(QString::fromStdString(b->name()))
            .arg(reinterpret_cast<quintptr>(b.get()), 0, 16), "#7ee787");
        appendLog(QString("实例 D — name: %1  [ptr: 0x%2]")
            .arg(QString::fromStdString(d->name()))
            .arg(reinterpret_cast<quintptr>(d.get()), 0, 16), "#7ee787");
        appendLog(QString("A == B: %1  |  B == D: %2  ← 瞬态绑定每次返回新实例")
            .arg(a.get() == b.get() ? "true" : "false ✓")
            .arg(b.get() == d.get() ? "true" : "false ✓"), "#ffa657");
    });

    // ── ② singleton<> 单例绑定 ───────────────────────────────────────────
    connect(m_singletonBtn, &QPushButton::clicked, this, [this]() {
        appendSection("② singleton<IAppStorage, MemoryAppStorage>() — 单例绑定");

        Container::Container c;
        c.singleton<IAppStorage, MemoryAppStorage>();

        auto a = c.make<IAppStorage>();
        auto b = c.make<IAppStorage>();

        // 通过 a 写入数据
        a->store("demo_key", "hello_singleton");

        // 通过 b 读取，验证是同一实例
        std::string val = b->retrieve("demo_key");

        appendLog(QString("实例 A [ptr: 0x%1]")
            .arg(reinterpret_cast<quintptr>(a.get()), 0, 16), "#79c0ff");
        appendLog(QString("实例 B [ptr: 0x%1]")
            .arg(reinterpret_cast<quintptr>(b.get()), 0, 16), "#79c0ff");
        appendLog(QString("A == B: %1  ← 单例绑定所有 make() 返回同一实例")
            .arg(a.get() == b.get() ? "true ✓" : "false"), "#ffa657");
        appendLog(QString("通过 A 存入 \"demo_key\"=\"hello_singleton\""), "#d4d4d4");
        appendLog(QString("通过 B 读取 \"demo_key\" = \"%1\"  ← 共享同一存储")
            .arg(QString::fromStdString(val)), "#7ee787");
    });

    // ── ③ instance<> 预建实例 ────────────────────────────────────────────
    connect(m_instanceBtn, &QPushButton::clicked, this, [this]() {
        appendSection("③ instance<IAppLogger>(preBuilt) — 注册预建实例");

        // 在容器外预先构建实例
        auto preBuilt = std::make_shared<ConsoleAppLogger>();
        preBuilt->info("我是预建实例，在容器外创建");

        Container::Container c;
        c.instance<IAppLogger>(preBuilt);   // ★ 注册已有对象

        auto r1 = c.make<IAppLogger>();
        auto r2 = c.make<IAppLogger>();

        appendLog(QString("预建实例  [ptr: 0x%1]  lastMsg: \"%2\"")
            .arg(reinterpret_cast<quintptr>(preBuilt.get()), 0, 16)
            .arg(QString::fromStdString(preBuilt->lastMsg())), "#9cdcfe");
        appendLog(QString("make() ①  [ptr: 0x%1]")
            .arg(reinterpret_cast<quintptr>(r1.get()), 0, 16), "#79c0ff");
        appendLog(QString("make() ②  [ptr: 0x%1]")
            .arg(reinterpret_cast<quintptr>(r2.get()), 0, 16), "#79c0ff");
        appendLog(QString("preBuilt == r1 == r2: %1  ← instance<> 始终返回同一预建对象")
            .arg((preBuilt.get() == r1.get() && r1.get() == r2.get()) ? "true ✓" : "false"),
            "#ffa657");
    });

    // ── ④ extend<> 装饰扩展 ─────────────────────────────────────────────
    connect(m_extendBtn, &QPushButton::clicked, this, [this]() {
        appendSection("④ extend<IAppLogger>(decorator) — AOP 装饰扩展");

        Container::Container c;
        // 注册单例
        c.singleton<IAppLogger>([] {
            return std::make_shared<ConsoleAppLogger>();
        });

        // extend 之前：获取原始绑定
        auto original = c.make<IAppLogger>();
        original->info("extend 之前的消息");

        appendLog(QString("extend 前：name = \"%1\"  lastMsg = \"%2\"")
            .arg(QString::fromStdString(original->name()))
            .arg(QString::fromStdString(original->lastMsg())), "#9cdcfe");

        // ★ 装饰：给所有日志调用添加 [TS] 前缀
        c.extend<IAppLogger>(
            [](std::shared_ptr<IAppLogger> base, Container::Container&) {
                return std::make_shared<TimestampLoggerDecorator>(base);
            }
        );

        // extend 之后：工厂被替换，单例缓存被清空，重新创建带装饰的实例
        auto decorated = c.make<IAppLogger>();
        decorated->info("extend 之后的消息");

        appendLog(QString("extend 后：name = \"%1\"  lastMsg = \"%2\"")
            .arg(QString::fromStdString(decorated->name()))
            .arg(QString::fromStdString(decorated->lastMsg())), "#7ee787");
        appendLog("← extend() 不修改原注册代码，以 closure chain 方式包裹工厂", "#ffa657");
    });

    // ── ⑤ 上下文绑定（contextual binding） ──────────────────────────────
    connect(m_contextualBtn, &QPushButton::clicked, this, [this]() {
        appendSection("⑤ when<ClientB>().needs<IAppLogger>().give<FileAppLogger>() — 上下文绑定");

        Container::Container c;

        // 默认绑定：IAppLogger → ConsoleAppLogger（所有未指定上下文的调用者使用此绑定）
        c.bind<IAppLogger>([] {
            return std::make_shared<ConsoleAppLogger>();
        });

        // ★ 上下文绑定：当 ClientB 解析时需要 IAppLogger，给它 FileAppLogger
        c.when<ClientB>()
         .needs<IAppLogger>()
         .give<FileAppLogger>([] {
             return std::make_shared<FileAppLogger>("client_b.log");
         });

        // 注册消费者（工厂中递归 make<IAppLogger>()，触发上下文绑定检测）
        c.bind<ClientA>([](Container::Container& cc) {
            return std::make_shared<ClientA>(cc.make<IAppLogger>());
        });
        c.bind<ClientB>([](Container::Container& cc) {
            return std::make_shared<ClientB>(cc.make<IAppLogger>());
        });

        auto clientA = c.make<ClientA>();
        auto clientB = c.make<ClientB>();

        appendLog(QString("ClientA 获得的 IAppLogger: %1")
            .arg(QString::fromStdString(clientA->logger->name())), "#9cdcfe");
        appendLog(QString("ClientB 获得的 IAppLogger: %1")
            .arg(QString::fromStdString(clientB->logger->name())), "#7ee787");
        appendLog("← ClientB 通过上下文绑定获得 FileAppLogger，ClientA 使用默认 ConsoleAppLogger",
                  "#ffa657");
    });

    // ── ⑥ tag/tagged 标签分组 ────────────────────────────────────────────
    connect(m_tagBtn, &QPushButton::clicked, this, [this]() {
        appendSection("⑥ tag<IAppLogger, IAppStorage>(\"core-services\") + tagged() — 标签分组");

        Container::Container c;
        c.singleton<IAppLogger>([] { return std::make_shared<ConsoleAppLogger>(); });
        c.singleton<IAppStorage>([] { return std::make_shared<MemoryAppStorage>(); });

        // ★ 将多个类型归入同一标签
        c.tag<IAppLogger, IAppStorage>("core-services");

        // ★ 批量解析（返回 vector<shared_ptr<void>>，调用方需手动 cast）
        auto services = c.tagged("core-services");

        appendLog(QString("tagged(\"core-services\") 解析到 %1 个服务：")
            .arg(services.size()), "#9cdcfe");

        auto logger  = std::static_pointer_cast<IAppLogger>(services[0]);
        auto storage = std::static_pointer_cast<IAppStorage>(services[1]);

        appendLog(QString("  services[0] → %1")
            .arg(QString::fromStdString(logger->name())), "#7ee787");
        appendLog(QString("  services[1] → %1")
            .arg(QString::fromStdString(storage->storageName())), "#7ee787");
        appendLog("← tag/tagged 适合\"通知中心\"等需要遍历同类服务的场景", "#ffa657");
    });

    // ── 清空记录 ──────────────────────────────────────────────────────────
    connect(m_clearLogBtn, &QPushButton::clicked, m_logEdit, &QTextEdit::clear);
}

void ContainerTypeView::retranslateUi() {}

// =============================================================================
// 辅助函数
// =============================================================================
void ContainerTypeView::appendLog(const QString &line, const QString &color)
{
    m_logEdit->append(
        QString("<span style='color:%1;'>[%2]</span> %3")
            .arg(color)
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
            .arg(line.toHtmlEscaped()));
}

void ContainerTypeView::appendSection(const QString &title)
{
    m_logEdit->append(
        QString("<span style='color:%1; font-weight:bold;'>━━ %2 ━━</span>")
            .arg(themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name())
            .arg(title.toHtmlEscaped()));
}
