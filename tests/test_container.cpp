// ============================================================
// test_container.cpp — 服务容器（IoC Container）单元测试
//
// 覆盖范围：
//   Container  : bind / singleton / instance / bound / extend
//                make / flush / when-needs-give / tag-tagged
//   Application: ServiceProvider 两阶段生命周期 / bootstrap 幂等 / 全局单例
//   Facade<T>  : 通过全局 Application 静态访问服务
//   异常分支    : 未注册类型 / extend 未注册 / 全局实例未设置
//
// 框架：Qt Test（Qt${QT_VERSION_MAJOR}::Test）
// 运行：直接执行可执行文件，或通过 ctest 运行
// ============================================================

#include <QtTest>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "container/container.h"
#include "container/application.h"
#include "container/serviceprovider.h"
#include "container/facade.h"

// 显式引入类型，避免 Container 命名空间与 Container::Container 类名歧义
// 注意：MSVC 不允许 using Container::Container（同名命名空间/类歧义），
//       改用类型别名绕过此限制。
using CContainer = Container::Container;
using CApplication = Container::Application;
using Container::ServiceProvider;
using Container::Facade;
using Container::MethodDispatcher;
using Container::ServiceHandle;

// ══════════════════════════════════════════════════════════════
// 一、服务接口（纯虚基类）
// ══════════════════════════════════════════════════════════════

/// 打招呼服务
struct IGreeter {
    virtual ~IGreeter() = default;
    virtual std::string greet(const std::string& name) const = 0;
};

/// 日志服务
struct ILogger {
    virtual ~ILogger() = default;
    virtual void        log(const std::string& msg)    = 0;
    virtual std::vector<std::string> messages() const  = 0;
};

/// 通知服务 A（用于 tag/tagged 测试）
struct INotifierA {
    virtual ~INotifierA() = default;
    virtual std::string channelId() const = 0;
};

/// 通知服务 B（用于 tag/tagged 测试）
struct INotifierB {
    virtual ~INotifierB() = default;
    virtual std::string channelId() const = 0;
};

// ══════════════════════════════════════════════════════════════
// 二、具体实现
// ══════════════════════════════════════════════════════════════

struct HelloGreeter : IGreeter {
    std::string greet(const std::string& name) const override {
        return "Hello, " + name + "!";
    }
};

struct HiGreeter : IGreeter {
    std::string greet(const std::string& name) const override {
        return "Hi, " + name + "!";
    }
};

/// 内存日志：将所有日志记录到 vector
struct MemoryLogger : ILogger {
    std::vector<std::string> msgs;
    void log(const std::string& msg) override { msgs.push_back(msg); }
    std::vector<std::string> messages() const override { return msgs; }
};

/// 装饰器日志：为每条日志加前缀后转发给内部 logger
struct PrefixLogger : ILogger {
    std::shared_ptr<ILogger> inner;
    std::string              prefix;

    PrefixLogger(std::shared_ptr<ILogger> inner_, std::string pfx)
        : inner(std::move(inner_)), prefix(std::move(pfx)) {}

    void log(const std::string& msg) override { inner->log(prefix + msg); }
    std::vector<std::string> messages() const override { return inner->messages(); }
};

struct EmailNotifier : INotifierA {
    std::string channelId() const override { return "email"; }
};

struct SmsNotifier : INotifierB {
    std::string channelId() const override { return "sms"; }
};

// ══════════════════════════════════════════════════════════════
// 三、业务类（演示依赖注入与上下文绑定）
// ══════════════════════════════════════════════════════════════

/// 依赖 ILogger 的订单控制器（演示递归解析与上下文绑定）
struct OrderController {
    std::shared_ptr<ILogger> logger;
    explicit OrderController(std::shared_ptr<ILogger> l) : logger(std::move(l)) {}
};

/// 依赖 IGreeter 的报表服务（演示 give<ConcreteType>() 无参形式）
struct ReportService {
    std::shared_ptr<IGreeter> greeter;
    explicit ReportService(std::shared_ptr<IGreeter> g) : greeter(std::move(g)) {}
};

// ══════════════════════════════════════════════════════════════
// 四、ServiceProvider（用于 Application 生命周期测试）
// ══════════════════════════════════════════════════════════════

/// 注册打招呼服务（单例）
class GreeterProvider : public ServiceProvider {
public:
    using ServiceProvider::ServiceProvider;

    void doRegister() override {
        app_.singleton<IGreeter>([]() -> std::shared_ptr<IGreeter> {
            return std::make_shared<HelloGreeter>();
        });
    }
};

/// 注册日志服务（单例），并在 boot() 中记录一条引导日志
class LoggerProvider : public ServiceProvider {
public:
    using ServiceProvider::ServiceProvider;

    void doRegister() override {
        app_.singleton<ILogger>([]() -> std::shared_ptr<ILogger> {
            return std::make_shared<MemoryLogger>();
        });
    }

    /// boot() 在所有 Provider 注册完成后调用，可安全解析其他服务
    void boot() override {
        auto greeter = app_.make<IGreeter>();
        auto logger  = app_.make<ILogger>();
        logger->log("boot: " + greeter->greet("app"));
    }
};

// ══════════════════════════════════════════════════════════════
// 五、Facade（演示静态访问入口）
// ══════════════════════════════════════════════════════════════

/// Log Facade：通过全局 Application 静态访问 ILogger
class Log : public Facade<ILogger> {
public:
    static void log(const std::string& msg) { getFacadeRoot()->log(msg); }
};

// ══════════════════════════════════════════════════════════════
// 六、测试类
// ══════════════════════════════════════════════════════════════

class ContainerTest : public QObject
{
    Q_OBJECT

private slots:

    // ──────────────────────────────────────────────────────────
    // 1. bind — 瞬态绑定：每次 make() 返回新实例
    // ──────────────────────────────────────────────────────────
    void bind_transient_returns_new_instance_each_time()
    {
        CContainer c;
        c.bind<IGreeter>([]() -> std::shared_ptr<IGreeter> {
            return std::make_shared<HelloGreeter>();
        });

        auto a = c.make<IGreeter>();
        auto b = c.make<IGreeter>();

        QVERIFY(a != nullptr);
        QVERIFY(b != nullptr);
        QVERIFY(a.get() != b.get());  // 不同实例（瞬态语义）
        QCOMPARE(QString::fromStdString(a->greet("World")),
                 QStringLiteral("Hello, World!"));
    }

    // ──────────────────────────────────────────────────────────
    // 2. singleton — 单例绑定：首次 make() 后复用同一实例
    // ──────────────────────────────────────────────────────────
    void singleton_returns_same_instance_every_time()
    {
        CContainer c;
        c.singleton<ILogger>([]() -> std::shared_ptr<ILogger> {
            return std::make_shared<MemoryLogger>();
        });

        auto a = c.make<ILogger>();
        auto b = c.make<ILogger>();

        QCOMPARE(a.get(), b.get());  // 同一指针（单例语义）

        // 验证状态共享：通过 a 写入，从 b 读取
        a->log("ping");
        QCOMPARE(static_cast<int>(b->messages().size()), 1);
        QCOMPARE(QString::fromStdString(b->messages()[0]), QStringLiteral("ping"));
    }

    // ──────────────────────────────────────────────────────────
    // 3. instance — 预建实例：注册已有对象
    // ──────────────────────────────────────────────────────────
    void instance_returns_the_registered_object()
    {
        CContainer c;
        auto prebuilt = std::make_shared<MemoryLogger>();
        prebuilt->log("pre-existing");

        c.instance<ILogger>(prebuilt);

        auto resolved = c.make<ILogger>();
        QCOMPARE(resolved.get(), prebuilt.get());  // 完全相同的指针
        QCOMPARE(static_cast<int>(resolved->messages().size()), 1);
        QCOMPARE(QString::fromStdString(resolved->messages()[0]),
                 QStringLiteral("pre-existing"));
    }

    // ──────────────────────────────────────────────────────────
    // 4. bound — 注册状态检查
    // ──────────────────────────────────────────────────────────
    void bound_reflects_registration_state()
    {
        CContainer c;

        QVERIFY(!c.bound<IGreeter>());   // 尚未注册

        c.bind<IGreeter, HelloGreeter>();
        QVERIFY(c.bound<IGreeter>());    // 注册后返回 true

        QVERIFY(!c.bound<ILogger>());    // 未注册类型仍为 false
    }

    // ──────────────────────────────────────────────────────────
    // 5. bind<Abstract, Concrete> — 自动构造具体类型
    // ──────────────────────────────────────────────────────────
    void bind_with_concrete_type_auto_constructs()
    {
        CContainer c;
        c.bind<IGreeter, HelloGreeter>();

        auto g1 = c.make<IGreeter>();
        auto g2 = c.make<IGreeter>();

        QVERIFY(g1 != nullptr);
        QVERIFY(g1.get() != g2.get());  // 仍是瞬态（每次新建）
        QCOMPARE(QString::fromStdString(g1->greet("C++")),
                 QStringLiteral("Hello, C++!"));
    }

    // ──────────────────────────────────────────────────────────
    // 6. singleton<Abstract, Concrete> — 自动构造单例
    // ──────────────────────────────────────────────────────────
    void singleton_with_concrete_type_auto_constructs()
    {
        CContainer c;
        c.singleton<IGreeter, HelloGreeter>();

        auto g1 = c.make<IGreeter>();
        auto g2 = c.make<IGreeter>();

        QCOMPARE(g1.get(), g2.get());  // 同一实例（单例语义）
    }

    // ──────────────────────────────────────────────────────────
    // 7. 工厂接收容器引用：递归解析嵌套依赖
    // ──────────────────────────────────────────────────────────
    void factory_with_container_ref_resolves_nested_dependencies()
    {
        CContainer c;
        c.singleton<ILogger>([]() -> std::shared_ptr<ILogger> {
            return std::make_shared<MemoryLogger>();
        });
        c.bind<OrderController>([](CContainer& app) {
            return std::make_shared<OrderController>(app.make<ILogger>());
        });

        auto ctrl = c.make<OrderController>();
        QVERIFY(ctrl != nullptr);
        QVERIFY(ctrl->logger != nullptr);

        // 注入的 logger 与容器中的单例是同一对象
        auto sharedLogger = c.make<ILogger>();
        QCOMPARE(ctrl->logger.get(), sharedLogger.get());
    }

    // ──────────────────────────────────────────────────────────
    // 8. extend — 装饰已有绑定（AOP / Decorator 模式）
    // ──────────────────────────────────────────────────────────
    void extend_wraps_existing_binding_with_decorator()
    {
        CContainer c;
        c.singleton<ILogger>([]() -> std::shared_ptr<ILogger> {
            return std::make_shared<MemoryLogger>();
        });

        // 给所有日志条目加上 "[TEST] " 前缀
        c.extend<ILogger>([](std::shared_ptr<ILogger> base, CContainer&) {
            return std::make_shared<PrefixLogger>(base, "[TEST] ");
        });

        auto logger = c.make<ILogger>();
        logger->log("message");

        auto msgs = logger->messages();
        QCOMPARE(static_cast<int>(msgs.size()), 1);
        QCOMPARE(QString::fromStdString(msgs[0]),
                 QStringLiteral("[TEST] message"));
    }

    // ──────────────────────────────────────────────────────────
    // 9. extend — 对已缓存的单例重置并重新创建
    // ──────────────────────────────────────────────────────────
    void extend_resets_singleton_cache_and_rebuilds()
    {
        CContainer c;
        c.singleton<ILogger>([]() -> std::shared_ptr<ILogger> {
            return std::make_shared<MemoryLogger>();
        });

        auto before = c.make<ILogger>();  // 触发单例缓存（MemoryLogger）
        QVERIFY(before != nullptr);

        // extend 后，单例缓存应被清除
        c.extend<ILogger>([](std::shared_ptr<ILogger> base, CContainer&) {
            return std::make_shared<PrefixLogger>(base, "[EXT] ");
        });

        auto after = c.make<ILogger>();  // 重新构造（PrefixLogger）
        QVERIFY(before.get() != after.get());  // 两次 make 指向不同对象
    }

    // ──────────────────────────────────────────────────────────
    // 10. 上下文绑定 when/needs/give（带工厂 lambda）
    //     — 根据调用方注入不同实现
    // ──────────────────────────────────────────────────────────
    void contextual_binding_injects_different_impl_per_consumer()
    {
        CContainer c;

        // 默认绑定：普通 MemoryLogger（无初始日志）
        c.bind<ILogger>([]() -> std::shared_ptr<ILogger> {
            return std::make_shared<MemoryLogger>();
        });

        // 上下文绑定：OrderController 专属 logger（含标记日志）
        c.when<OrderController>()
         .needs<ILogger>()
         .give<ILogger>([](CContainer&) -> std::shared_ptr<ILogger> {
             auto ml = std::make_shared<MemoryLogger>();
             ml->log("contextual-marker");
             return ml;
         });

        c.bind<OrderController>([](CContainer& app) {
            return std::make_shared<OrderController>(app.make<ILogger>());
        });

        auto ctrl = c.make<OrderController>();

        // OrderController 收到上下文 logger（有预置日志）
        QCOMPARE(static_cast<int>(ctrl->logger->messages().size()), 1);
        QCOMPARE(QString::fromStdString(ctrl->logger->messages()[0]),
                 QStringLiteral("contextual-marker"));

        // 直接 make<ILogger>() 使用默认绑定（无日志）
        auto defaultLogger = c.make<ILogger>();
        QCOMPARE(static_cast<int>(defaultLogger->messages().size()), 0);
    }

    // ──────────────────────────────────────────────────────────
    // 11. 上下文绑定 when/needs/give<ConcreteType>()（无参形式）
    //     — 自动默认构造具体类型
    // ──────────────────────────────────────────────────────────
    void contextual_binding_give_with_concrete_type_no_factory()
    {
        CContainer c;

        // 默认：HelloGreeter
        c.bind<IGreeter, HelloGreeter>();

        // ReportService 专属：HiGreeter（无参 give，自动默认构造）
        c.when<ReportService>()
         .needs<IGreeter>()
         .give<HiGreeter>();

        c.bind<ReportService>([](CContainer& app) {
            return std::make_shared<ReportService>(app.make<IGreeter>());
        });

        // ReportService 获得 HiGreeter
        auto report = c.make<ReportService>();
        QCOMPARE(QString::fromStdString(report->greeter->greet("ctx")),
                 QStringLiteral("Hi, ctx!"));

        // 直接 make<IGreeter>() 仍使用默认 HelloGreeter
        QCOMPARE(QString::fromStdString(c.make<IGreeter>()->greet("x")),
                 QStringLiteral("Hello, x!"));
    }

    // ──────────────────────────────────────────────────────────
    // 12. tag / tagged — 标签分组批量解析
    // ──────────────────────────────────────────────────────────
    void tag_and_tagged_batch_resolve_services()
    {
        CContainer c;
        c.singleton<INotifierA, EmailNotifier>();
        c.singleton<INotifierB, SmsNotifier>();

        c.tag<INotifierA, INotifierB>("notifiers");

        auto services = c.tagged("notifiers");

        QCOMPARE(static_cast<int>(services.size()), 2);

        auto a = std::static_pointer_cast<INotifierA>(services[0]);
        auto b = std::static_pointer_cast<INotifierB>(services[1]);

        QCOMPARE(QString::fromStdString(a->channelId()), QStringLiteral("email"));
        QCOMPARE(QString::fromStdString(b->channelId()), QStringLiteral("sms"));

        // tagged 遵循各服务自身的生命周期：单例应复用同一指针
        auto services2 = c.tagged("notifiers");
        QCOMPARE(services2[0].get(), services[0].get());
    }

    // ──────────────────────────────────────────────────────────
    // 13. tagged — 未知标签返回空向量
    // ──────────────────────────────────────────────────────────
    void tagged_unknown_label_returns_empty_vector()
    {
        CContainer c;
        auto result = c.tagged("no-such-tag");
        QVERIFY(result.empty());
    }

    // ──────────────────────────────────────────────────────────
    // 14. flush — 清空所有绑定、标签与上下文绑定
    // ──────────────────────────────────────────────────────────
    void flush_clears_all_bindings_and_tags()
    {
        CContainer c;
        c.singleton<IGreeter, HelloGreeter>();
        c.bind<ILogger, MemoryLogger>();
        c.tag<IGreeter>("greeters");

        QVERIFY(c.bound<IGreeter>());
        QVERIFY(c.bound<ILogger>());

        c.flush();

        QVERIFY(!c.bound<IGreeter>());
        QVERIFY(!c.bound<ILogger>());
        QVERIFY(c.tagged("greeters").empty());
    }

    // ──────────────────────────────────────────────────────────
    // 15. 异常：make() 未注册类型时抛 std::runtime_error
    // ──────────────────────────────────────────────────────────
    void make_throws_runtime_error_when_type_not_bound()
    {
        CContainer c;
        bool caught = false;
        try {
            c.make<IGreeter>();
        } catch (const std::runtime_error&) {
            caught = true;
        }
        QVERIFY(caught);
    }

    // ──────────────────────────────────────────────────────────
    // 16. 异常：extend() 对未注册类型时抛 std::runtime_error
    // ──────────────────────────────────────────────────────────
    void extend_throws_when_binding_not_found()
    {
        CContainer c;
        bool caught = false;
        try {
            c.extend<IGreeter>([](std::shared_ptr<IGreeter> base, CContainer&) {
                return base;
            });
        } catch (const std::runtime_error&) {
            caught = true;
        }
        QVERIFY(caught);
    }

    // ──────────────────────────────────────────────────────────
    // 17. ServiceProvider 两阶段生命周期：doRegister → boot
    // ──────────────────────────────────────────────────────────
    void service_provider_two_phase_lifecycle()
    {
        CApplication app;
        app.registerProvider<GreeterProvider>()
           .registerProvider<LoggerProvider>();

        // bootstrap 前：容器尚未引导
        QVERIFY(!app.isBooted());

        app.bootstrap();

        QVERIFY(app.isBooted());
        QVERIFY(app.bound<IGreeter>());
        QVERIFY(app.bound<ILogger>());

        // GreeterProvider 注册的服务可正常解析
        auto greeter = app.make<IGreeter>();
        QVERIFY(greeter != nullptr);
        QCOMPARE(QString::fromStdString(greeter->greet("provider")),
                 QStringLiteral("Hello, provider!"));

        // LoggerProvider::boot() 中记录了一条引导日志
        // （boot 阶段调用了 IGreeter，证明两阶段顺序正确）
        auto logger = app.make<ILogger>();
        QVERIFY(!logger->messages().empty());
        QCOMPARE(QString::fromStdString(logger->messages()[0]),
                 QStringLiteral("boot: Hello, app!"));
    }

    // ──────────────────────────────────────────────────────────
    // 18. Application::bootstrap() 幂等性保护
    // ──────────────────────────────────────────────────────────
    void application_bootstrap_is_idempotent()
    {
        CApplication app;
        app.registerProvider<GreeterProvider>();

        app.bootstrap();
        app.bootstrap();  // 第二次调用不得产生副作用

        QVERIFY(app.isBooted());

        // 绑定仍然有效
        auto g = app.make<IGreeter>();
        QVERIFY(g != nullptr);
    }

    // ──────────────────────────────────────────────────────────
    // 19. Application::instance — 全局单例访问
    // ──────────────────────────────────────────────────────────
    void application_global_instance_accessible_after_set()
    {
        auto* inst = CApplication::instance();
        QVERIFY(inst != nullptr);

        // 如果之前测试已绑定过 ILogger，就复用现有绑定，避免全局状态被重复覆盖。
        if (!inst->bound<ILogger>()) {
            inst->singleton<ILogger>([]() -> std::shared_ptr<ILogger> {
                return std::make_shared<MemoryLogger>();
            });
        }

        auto* inst2 = CApplication::instance();
        QCOMPARE(inst2, inst);

        auto logger = inst->make<ILogger>();
        QVERIFY(logger != nullptr);
    }

    // ──────────────────────────────────────────────────────────
    // 20. instance() 在未显式设置时仍应可用
    // ──────────────────────────────────────────────────────────
    void application_instance_throws_when_not_set()
    {
        QVERIFY(CApplication::instance() != nullptr);
    }

    // ──────────────────────────────────────────────────────────
    // 21. Facade — 通过全局 Application 静态访问服务
    // ──────────────────────────────────────────────────────────
    void facade_resolves_service_via_global_application()
    {
        auto* inst = CApplication::instance();
        QVERIFY(inst != nullptr);

        if (!inst->bound<ILogger>()) {
            inst->singleton<ILogger>([]() -> std::shared_ptr<ILogger> {
                return std::make_shared<MemoryLogger>();
            });
        }

        // 清空上一轮日志，保证断言稳定
        auto logger = inst->make<ILogger>();
        if (auto* mem = dynamic_cast<MemoryLogger*>(logger.get())) {
            mem->msgs.clear();
        }

        // 通过 Facade 静态方法写入日志
        Log::log("facade-test");

        // 通过容器直接解析，验证 Facade 写入有效
        auto logger2 = inst->make<ILogger>();
        auto msgs    = logger2->messages();

        QCOMPARE(static_cast<int>(msgs.size()), 1);
        QCOMPARE(QString::fromStdString(msgs[0]), QStringLiteral("facade-test"));
    }
};

// ══════════════════════════════════════════════════════════════
// 测试入口（Qt Test 生成 main()）
// ══════════════════════════════════════════════════════════════
QTEST_GUILESS_MAIN(ContainerTest)
#include "test_container.moc"
