# fz2_project_template 用户指南

> **基于 Qt + CMake 的插件化桌面应用开发框架**  
> 版本：1.0.0 · C++17 · Qt 5.15 / Qt6

---

## 目录

1. [简介](#1-简介)
2. [快速开始](#2-快速开始)
   - 2.1 [环境要求](#21-环境要求)
   - 2.2 [克隆与构建](#22-克隆与构建)
   - 2.3 [目录结构](#23-目录结构)
3. [核心概念](#3-核心概念)
4. [插件系统 — ExtensionSystem](#4-插件系统--extensionsystem)
   - 4.1 [插件生命周期](#41-插件生命周期)
   - 4.2 [创建一个插件](#42-创建一个插件)
   - 4.3 [插件元数据（JSON）](#43-插件元数据json)
   - 4.4 [对象池](#44-对象池)
   - 4.5 [动态方法调用 — Invoker](#45-动态方法调用--invoker)
5. [IoC 服务容器 — Container](#5-ioc-服务容器--container)
   - 5.1 [绑定与生命周期](#51-绑定与生命周期)
   - 5.2 [服务提供者 — ServiceProvider](#52-服务提供者--serviceprovider)
   - 5.3 [上下文绑定](#53-上下文绑定)
   - 5.4 [标签分组](#54-标签分组)
   - 5.5 [外观模式 — Facade](#55-外观模式--facade)
   - 5.6 [动态命名绑定（跨插件反射调用）](#56-动态命名绑定跨插件反射调用)
6. [MVVM 架构 — mvvm](#6-mvvm-架构--mvvm)
   - 6.1 [ViewModel](#61-viewmodel)
   - 6.2 [属性中间件管道](#62-属性中间件管道)
   - 6.3 [Command（命令模式）](#63-command命令模式)
7. [UI 基础设施 — uibase](#7-ui-基础设施--uibase)
   - 7.1 [无边框窗口 — CFramelessWindow](#71-无边框窗口--cframelesswindow)
   - 7.2 [导航管理 — NavigationManager](#72-导航管理--navigationmanager)
   - 7.3 [权限运行时 — PermService](#73-权限运行时--permservice)
   - 7.4 [主题管理 — ThemeManager](#74-主题管理--thememanager)
   - 7.5 [语言国际化 — LanguageManager](#75-语言国际化--languagemanager)
8. [CMake 构建辅助 — FZ2Helpers](#8-cmake-构建辅助--fz2helpers)
9. [内置示例插件](#9-内置示例插件)
10. [最佳实践](#10-最佳实践)
11. [常见问题](#11-常见问题)

---

## 1. 简介

**fz2_project_template** 是一套面向桌面应用的 **插件化开发框架模板**，以 Qt Widgets + CMake 为基础，提供了一整套开箱即用的基础设施：

| 模块 | 功能 |
|---|---|
| `extensionsystem` | 插件加载、卸载、依赖管理、对象池、动态方法调用 |
| `container` | Laravel 风格 IoC 服务容器（绑定/单例/上下文/Facade） |
| `mvvm` | ViewModel 属性绑定、中间件管道、Command 模式 |
| `uibase` | 无边框窗口、导航管理、主题/语言切换、权限控制 |

**设计目标：**

- 业务代码以**插件（DLL）**为单元独立开发，主程序极简
- 插件之间通过**对象池**或**服务容器**通信，零硬依赖
- 提供完整的 MVVM + 中间件管道，使 UI 与业务逻辑彻底解耦
- 支持运行时**主题切换**、**语言切换**、**角色权限**管控

---

## 2. 快速开始

### 2.1 环境要求

| 工具 | 最低版本 | 说明 |
|---|---|---|
| CMake | 3.16 | 构建系统 |
| Qt | 5.15 / 6.x | 优先检测 Qt6，回退 Qt5 |
| MSVC | VS 2019 / 2022 | Windows 推荐；也可使用 GCC / Clang |
| C++ 标准 | C++17 | `if constexpr`、折叠表达式等特性 |

### 2.2 克隆与构建

**方式一：使用一键构建脚本（Windows）**

```bat
# Release 构建
scripts\build.bat

# Debug 构建
scripts\build.bat Debug

# 先清理再构建
scripts\build.bat clean Release
```

构建产物位于 `build\bin\<Config>\`，插件 DLL 在 `build\bin\<Config>\plugins\`。

**方式二：手动 CMake**

```bash
cmake -S . -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config Release --parallel
```

**方式三：Visual Studio**

用 CMake 生成后，直接打开 `build\PluginApp.sln`，默认启动项目为 `myapp`。

### 2.3 目录结构

```
fz2_project_template/
├── app/                        # 可执行程序入口（main.cpp）
├── common/                     # 平台通用代码（子模块引入）
│   ├── cmake/
│   │   └── FZ2Helpers.cmake    # CMake 辅助函数
│   ├── core/
│   │   ├── container/          # IoC 服务容器
│   │   ├── extensionsystem/    # 插件管理引擎
│   │   ├── communication/      # 通信协议（串口/TCP/Modbus 等）
│   │   └── device/             # 设备管理
│   └── ui/
│       ├── mvvm/               # MVVM 基础库
│       ├── uibase/             # UI 基础组件库
│       ├── component/          # 通用控件（预留扩展）
│       └── resources/          # 图标、样式表、翻译文件
├── plugins/                    # 所有业务插件
│   ├── core/                   # 核心插件（主窗口）
│   ├── home/                   # 首页插件
│   ├── features/               # 功能演示插件
│   ├── settings/               # 设置插件
│   ├── help/                   # 帮助插件
│   ├── mvvm/                   # MVVM 示例插件
│   ├── middleware/             # 中间件示例插件
│   ├── invoker/                # Invoker 示例插件
│   └── container/              # Container 示例插件
├── config/
│   ├── app.toml                # 应用配置
│   └── sql/                    # 权限等资源种子 SQL
├── docs/                       # 文档目录
├── scripts/
│   └── build.bat               # Windows 一键构建
├── tests/                      # 单元测试
└── CMakeLists.txt              # 根构建脚本
```

---

## 3. 核心概念

本框架有三个核心概念，理解它们是使用框架的基础：

```
┌────────────────────────────────────────────────────────────────────┐
│                         应用程序启动                               │
│                                                                    │
│  main()                                                            │
│    │                                                               │
│    ├─ 1. 初始化 ThemeManager / LanguageManager                    │
│    │                                                               │
│    ├─ 2. PluginManager::loadPlugins()                             │
│    │       扫描 plugins/ 目录 → 按依赖顺序加载 DLL                 │
│    │       → IPlugin::initialize()  → extensionsInitialized()     │
│    │                                                               │
│    └─ 3. QApplication::exec()  （主事件循环）                      │
│                                                                    │
│  核心插件（Core）负责创建主窗口、导航栏、菜单配置                  │
│  业务插件各自注册页面到 NavigationManager                          │
└────────────────────────────────────────────────────────────────────┘
```

**三大支柱：**

1. **插件（Plugin）** — 所有功能模块均以 DLL 形式存在，主程序只负责启动和插件加载
2. **服务容器（Container）** — 插件间的服务注册与发现，彻底解耦依赖
3. **MVVM + 中间件** — UI 层与业务层分离，属性变更可拦截、校验、日志记录

---

## 4. 插件系统 — ExtensionSystem

### 4.1 插件生命周期

每个插件在加载过程中会依次经历以下生命周期钩子：

```
加载 DLL
    │
    ▼
IPlugin::initialize(errorString*)
    │  在此阶段：注册服务到对象池、初始化内部状态
    │  返回 false 则插件加载失败
    ▼
IPlugin::extensionsInitialized()
    │  所有插件的 initialize() 均已完成后调用
    │  可在此访问其他插件提供的服务
    ▼
IPlugin::delayedInitialize()    [可选，返回 true 表示需要延迟初始化]
    │  在主事件循环空闲时调用，适合耗时初始化
    ▼
运行中...
    │
    ▼
IPlugin::aboutToShutdown()      [可选]
    │  返回 SynchronousShutdown 或 AsynchronousShutdown
    ▼
插件卸载
```

### 4.2 创建一个插件

**步骤 1：继承 `IPlugin`**

```cpp
// myplugin.h
#pragma once
#include <extensionsystem/iplugin.h>

class MyPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.pluginsystem.IPlugin"
                      FILE "myplugin.json")

public:
    bool initialize(QString *errorString) override;
    void extensionsInitialized() override;
    bool delayedInitialize() override;
    ShutdownFlag aboutToShutdown() override;
};
```

**步骤 2：实现生命周期方法**

```cpp
// myplugin.cpp
#include "myplugin.h"
#include <extensionsystem/pluginmanager.h>
#include "myservice.h"

bool MyPlugin::initialize(QString *errorString)
{
    Q_UNUSED(errorString)

    // 将本插件提供的服务注册到对象池，供其他插件访问
    auto *service = new MyService(this);
    ExtensionSystem::PluginManager::addObject(service);

    return true;
}

void MyPlugin::extensionsInitialized()
{
    // 此时所有插件的 initialize() 均已完成
    // 可以安全地从对象池获取其他插件的服务
    auto *otherService = ExtensionSystem::PluginManager::getObject<IOtherService>();
    if (otherService) {
        otherService->doSomething();
    }
}

bool MyPlugin::delayedInitialize()
{
    // 耗时操作放在这里，不阻塞主窗口显示
    loadHeavyResources();
    return true;  // 返回 true 才会被调用
}

ExtensionSystem::IPlugin::ShutdownFlag MyPlugin::aboutToShutdown()
{
    // 同步关闭（默认）
    return SynchronousShutdown;
}
```

**步骤 3：添加 CMakeLists.txt**

```cmake
set(PLUGIN_NAME MyPlugin)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    myplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    extensionsystem
    uibase
)

fz2_target_msvc_options(${PLUGIN_NAME})
fz2_setup_plugin(${PLUGIN_NAME} "plugins/myplugin")
```

### 4.3 插件元数据（JSON）

每个插件目录下须有一个与 `FILE` 参数同名的 JSON 文件：

```json
{
    "Name"        : "MyPlugin",
    "Version"     : "1.0.0",
    "Description" : "My custom plugin",
    "Category"    : "Business",
    "Required"    : false,
    "Dependencies": ["Core"]
}
```

| 字段 | 类型 | 说明 |
|---|---|---|
| `Name` | string | 插件名（唯一标识） |
| `Version` | string | 语义化版本号 |
| `Description` | string | 描述 |
| `Category` | string | 分组（VS 解决方案文件夹） |
| `Required` | bool | `true` 表示不可禁用 |
| `Dependencies` | array | 依赖的其他插件名，框架保证先加载依赖 |

### 4.4 对象池

对象池是插件间**轻量级服务共享**的机制，适合简单的"公告板"模式。

```cpp
// 注册
ExtensionSystem::PluginManager::addObject(myServiceInstance);

// 按类型获取第一个匹配对象
auto *svc = ExtensionSystem::PluginManager::getObject<IMyService>();

// 按类型 + 条件获取
auto *svc = ExtensionSystem::PluginManager::getObject<IMyService>(
    [](IMyService *s) { return s->name() == "target"; }
);

// 获取所有匹配对象
QList<IMyService*> list = ExtensionSystem::PluginManager::getObjects<IMyService>();

// 从对象池移除
ExtensionSystem::PluginManager::removeObject(myServiceInstance);
```

> **注意：** 对象池不管理对象生命周期，注册者负责在适当时机调用 `removeObject` 并释放内存（通常在插件析构时）。

### 4.5 动态方法调用 — Invoker

`Invoker` 通过 Qt 元对象系统实现**运行时方法调用**，无需编译期引入目标类头文件。

```cpp
#include <extensionsystem/invoker.h>

// 获取目标对象（来自对象池）
QObject *target = ExtensionSystem::PluginManager::getObjectByName("CalculatorService");

// 无参无返回值调用
ExtensionSystem::invoke<void>(target, "reset");

// 带参数，有返回值
int sum = ExtensionSystem::invoke<int>(target, "add", 3, 5);

// 两个参数
QString result = ExtensionSystem::invoke<QString>(target, "format", value, unit);

// 类模板版本（保持对象引用）
ExtensionSystem::Invoker<int> inv(target, "multiply", 4, 6);
if (inv.wasSuccessful()) {
    int result = inv;  // 隐式转换获取结果
}
```

> **前提条件：** 目标方法必须声明为 `Q_INVOKABLE`：
> ```cpp
> class CalculatorService : public QObject {
>     Q_OBJECT
> public:
>     Q_INVOKABLE int add(int a, int b) { return a + b; }
>     Q_INVOKABLE void reset() { m_value = 0; }
> };
> ```

---

## 5. IoC 服务容器 — Container

服务容器参考 Laravel 框架设计，提供类型安全的依赖注入机制，特别适合**跨插件服务注册与发现**。

### 5.1 绑定与生命周期

框架支持三种绑定生命周期：

#### 瞬态绑定 `bind` — 每次解析返回新实例

```cpp
#include "container/application.h"

Container::Application app;

// 方式1：工厂 Lambda（接收容器引用，可递归解析依赖）
app.bind<ILogger>([](Container::Container& c) {
    return std::make_shared<FileLogger>(c.make<IConfig>()->get("log.path"));
});

// 方式2：无参工厂（简单情况）
app.bind<ILogger>([] {
    return std::make_shared<ConsoleLogger>();
});

// 方式3：自动构造（Concrete 必须继承 Abstract）
app.bind<ILogger, ConsoleLogger>();
```

#### 单例绑定 `singleton` — 首次解析后复用

```cpp
app.singleton<IDatabase>([](Container::Container& c) {
    return std::make_shared<MySqlDatabase>(c.make<IConfig>());
});

// 自动构造变体
app.singleton<IConfig, AppConfig>();
```

#### 预建实例 `instance` — 注册已有对象

```cpp
auto config = std::make_shared<AppConfig>("config.json");
app.instance<IConfig>(config);
// 此后 make<IConfig>() 始终返回 config
```

#### 解析服务

```cpp
// 解析（自动处理生命周期）
auto logger = app.make<ILogger>();   // shared_ptr<ILogger>
auto db     = app.make<IDatabase>(); // shared_ptr<IDatabase>（单例）
```

#### 绑定装饰 `extend`

在不修改原注册代码的情况下，为已有绑定包装新行为（AOP/装饰器）：

```cpp
// 为所有 ILogger 实例添加时间戳前缀
app.extend<ILogger>([](std::shared_ptr<ILogger> base, Container::Container&) {
    return std::make_shared<TimestampLoggerDecorator>(base);
});
```

### 5.2 服务提供者 — ServiceProvider

将服务注册逻辑从业务代码中解耦到独立的 **Provider** 类中，并确保两阶段正确执行：

```cpp
#include "container/serviceprovider.h"

class LogServiceProvider : public Container::ServiceProvider {
public:
    using ServiceProvider::ServiceProvider;  // 继承构造函数

    // 第一阶段：只做绑定注册，不解析其他服务
    void doRegister() override {
        app_.singleton<ILogger>([] {
            return std::make_shared<FileLogger>("app.log");
        });
    }

    // 第二阶段：所有 Provider 的 doRegister() 完成后调用
    // 在此可安全解析其他服务
    void boot() override {
        auto logger = app_.make<ILogger>();
        logger->info("LogServiceProvider booted.");
    }
};
```

**注册与启动：**

```cpp
auto app = std::make_unique<Container::Application>();

// 链式注册 Provider
app->registerProvider<ConfigServiceProvider>()
   .registerProvider<LogServiceProvider>()
   .registerProvider<DatabaseServiceProvider>();

// 设置全局单例（供 Facade 使用）
Container::Application::setInstance(app.get());

// 一键引导：doRegister() → boot()
app->bootstrap();
```

**延迟加载 Provider：**

```cpp
class HeavyServiceProvider : public Container::ServiceProvider {
public:
    // 返回 true 表示跳过初始化阶段的 doRegister()
    bool isDeferred() const override { return true; }

    void doRegister() override {
        app_.singleton<IHeavyService>([] {
            return std::make_shared<HeavyServiceImpl>();
        });
    }
};
```

### 5.3 上下文绑定

为**特定调用者**提供不同的服务实现，实现精细化的依赖注入：

```cpp
// 声明：当解析 OrderController 时需要 ILogger，给它 FileLogger
app.when<OrderController>()
   .needs<ILogger>()
   .give<FileLogger>();

// 带工厂版本（可传入自定义初始化逻辑）
app.when<ReportController>()
   .needs<ILogger>()
   .give<ILogger>([](Container::Container& c) {
       return std::make_shared<DatabaseLogger>(c.make<IDatabase>());
   });

// 解析时自动应用上下文绑定
// OrderController 的构造函数中 make<ILogger>() 会得到 FileLogger
auto order = app.make<OrderController>();
```

### 5.4 标签分组

将多个接口打上同一标签，批量解析：

```cpp
// 注册各服务
app.singleton<IMailer, SmtpMailer>();
app.singleton<ISmsNotifier, TwilioSmsNotifier>();
app.singleton<IPushNotifier, FirebasePushNotifier>();

// 打标签
app.tag<IMailer, ISmsNotifier, IPushNotifier>("notifiers");

// 批量解析并使用
auto services = app.tagged("notifiers");
for (auto& svc : services) {
    auto notifier = std::static_pointer_cast<INotifier>(svc);
    notifier->send(message);
}
```

### 5.5 外观模式 — Facade

`Facade` 是对 `Application::instance()->make<T>()` 的语法糖，提供全局静态访问：

```cpp
#include "container/facade.h"

// 定义 Facade（最简版）
class Log : public Container::Facade<ILogger> {};

// 带业务方法封装版
class Log : public Container::Facade<ILogger> {
public:
    static void info(const std::string& msg) {
        getFacadeRoot()->info(msg);
    }
    static void error(const std::string& msg) {
        getFacadeRoot()->error(msg);
    }
};

// 使用（须在 Application::setInstance() 调用后）
Log::info("Application started.");
Log::getFacadeRoot()->error("Something went wrong.");
```

### 5.6 动态命名绑定（跨插件反射调用）

当调用方**不能引入**被调用方的头文件时（如跨插件），使用命名绑定实现零依赖调用：

**注册方（插件 A，知道具体类型）：**

```cpp
// 只有插件 A 需要包含 FileLogger.h
app_.singletonNamed(
    "logger",                                          // 字符串键
    [](Container::Container&) -> std::shared_ptr<void> {
        return std::make_shared<FileLogger>("app.log");
    },
    [](Container::MethodDispatcher& d) {               // 描述反射方法
        d.on("info",    &FileLogger::info);            // void(const string&)
        d.on("error",   &FileLogger::error);
        d.on("flush",   &FileLogger::flush);           // void()
        d.on("isOpen",  &FileLogger::isOpen);          // bool() const
        d.readOnly ("level", &FileLogger::level);      // 只读属性
        d.readWrite("path",  &FileLogger::path, &FileLogger::setPath); // 读写属性
    }
);
```

**调用方（插件 B，只需 container.h）：**

```cpp
// 只需包含容器头文件
#include "container/container.h"

auto logger = container_.makeNamed("logger");

if (!logger) return;   // 服务不存在时为 false

// 调用方法（参数用 std::any 包装）
logger.call("info", { std::any{std::string("Hello from Plugin B")} });

// 读取属性
auto level = std::any_cast<std::string>(logger.get("level"));

// 写入属性
logger.set("path", std::any{std::string("/var/log/app.log")});

// 安全调用（先检查方法是否存在）
if (logger.hasMethod("flush")) {
    logger.call("flush");
}
```

> **推荐：** 将字符串键定义为常量，避免拼写错误：
> ```cpp
> namespace ServiceName {
>     inline constexpr const char* Logger   = "logger";
>     inline constexpr const char* Database = "database";
> }
> ```

---

## 6. MVVM 架构 — mvvm

MVVM（Model-View-ViewModel）模式将 UI 与业务逻辑彻底分离，`ViewModel` 作为中间层管理 UI 状态，`View` 只负责显示和用户交互，`Model` 处理数据持久化。

**核心组件：**

| 组件 | 职责 |
|---|---|
| `ViewModel` | 管理 UI 状态属性、封装业务逻辑、提供属性变更通知 |
| `DataBinding` | 将 UI 控件自动绑定到 ViewModel 属性（双向/单向） |
| `Command` | 封装用户操作，支持启用/禁用控制和中间件拦截 |
| `IPropertyMiddleware` | 属性变更中间件（日志、校验、转换） |
| `ICommandMiddleware` | 命令执行中间件（权限、计时、审计） |

---

### 6.1 ViewModel

`ViewModel` 是 MVVM 模式的核心，负责管理 UI 状态、封装业务逻辑、提供属性变更通知。

#### 6.1.1 定义 ViewModel

**基础类型属性：**

```cpp
#include "mvvm/viewmodel.h"

// 使用常量定义属性名（推荐，避免硬编码字符串）
class UserViewModel : public ViewModel
{
    Q_OBJECT

public:
    // 属性名常量
    static constexpr const char* PROP_USER_NAME = "userName";
    static constexpr const char* PROP_USER_AGE  = "age";
    static constexpr const char* PROP_IS_ACTIVE = "isActive";

    explicit UserViewModel(QObject *parent = nullptr)
        : ViewModel(parent)
    {
        // 必须在构造函数中注册属性（在使用 VIEWMODEL_PROPERTY 之前）
        registerProperty(PROP_USER_NAME, QString(""));
        registerProperty(PROP_USER_AGE,  0);
        registerProperty(PROP_IS_ACTIVE, false);
    }

    // VIEWMODEL_PROPERTY(Type, name, Name) 自动生成：
    //   - Type name() const;                    // getter
    //   - void setName(const Type& value);      // setter（值变化时发出信号）
    VIEWMODEL_PROPERTY(QString, userName, UserName)
    VIEWMODEL_PROPERTY(int,     age,      Age)
    VIEWMODEL_PROPERTY(bool,    isActive, IsActive)

signals:
    // 必须声明对应的变更信号（宏会自动 emit）
    void userNameChanged();
    void ageChanged();
    void isActiveChanged();

public slots:
    void loadUser(const QString& name) {
        beginBatchUpdate();  // 批量更新，避免多次信号
        setUserName(name);
        setAge(25);
        setIsActive(true);
        endBatchUpdate();    // 统一发出所有变更信号
    }
};
```

**属性访问方法：**

```cpp
// 直接使用 getter/setter（推荐）
QString name = vm->userName();
vm->setUserName("Alice");

// 通用属性访问（适用于动态属性名）
QVariant value = vm->getProperty("userName");
vm->setProperty("userName", QVariant("Bob"));

// 安全获取（属性不存在时返回 false）
QVariant out;
if (vm->getProperty("userName", out)) {
    QString name = out.toString();
}

// 检查属性是否存在
bool exists = vm->hasProperty("userName");
```

#### 6.1.2 结构体类型属性

当需要将多个相关字段作为一个整体管理时，使用结构体属性：

```cpp
// 1. 定义结构体并注册元类型
struct DeviceInfo {
    QString name;
    int     port = 0;
    bool    connected = false;
};
Q_DECLARE_METATYPE(DeviceInfo)

// 2. 在 ViewModel 中注册并声明
class DeviceViewModel : public ViewModel {
    Q_OBJECT
public:
    explicit DeviceViewModel(QObject *parent = nullptr)
        : ViewModel(parent)
    {
        // 注册结构体属性（需提供默认值）
        registerProperty("device", QVariant::fromValue(DeviceInfo{}));
    }

    // VIEWMODEL_STRUCT_PROPERTY 生成：
    //   - DeviceInfo device() const;                    // getter：返回完整结构体副本
    //   - void setDevice(const DeviceInfo& value);      // setter：整体替换
    //   - void modifyDevice(std::function<void(DeviceInfo&)> modifier);  // 字段级修改器
    VIEWMODEL_STRUCT_PROPERTY(DeviceInfo, device, Device)

signals:
    void deviceChanged();
};

// 3. 使用方式
DeviceViewModel vm;

// 整体替换
DeviceInfo info;
info.name = "Device1";
info.port = 8080;
vm.setDevice(info);

// 字段级修改（推荐，避免整体复制）
vm.modifyDevice([](DeviceInfo& d) {
    d.connected = true;
    d.port = 9090;
});
```

> **注意：** 若需精确去重（避免值未变时重复发信号），请在应用初始化时注册比较器：
> ```cpp
> QMetaType::registerComparators<DeviceInfo>();
> ```

#### 6.1.3 批量更新

当需要同时更新多个属性时，使用批量更新避免多次信号发射：

```cpp
vm->beginBatchUpdate();
vm->setUserName("Bob");
vm->setAge(30);
vm->setIsActive(true);
vm->endBatchUpdate();   // 在此统一发出所有变更信号（仅一次 propertyChanged）
```

**嵌套批量更新：** 框架支持嵌套调用，只有最外层 `endBatchUpdate()` 才会真正发出信号。

#### 6.1.4 在 View 中连接

```cpp
// View 持有 ViewModel 引用
auto *vm = new UserViewModel(this);

// 方式1：连接通用信号（适用于动态属性名）
connect(vm, &ViewModel::propertyChanged,
        this, [this](const QString& name, const QVariant& value) {
    if (name == "userName") {
        m_nameLabel->setText(value.toString());
    } else if (name == "age") {
        m_ageLabel->setText(QString::number(value.toInt()));
    }
});

// 方式2：连接具名信号（推荐，更高效，编译期检查）
connect(vm, &UserViewModel::userNameChanged, this, [this, vm]() {
    m_nameLabel->setText(vm->userName());
});
connect(vm, &UserViewModel::isActiveChanged, this, &LoginView::updateLoginState);

// 方式3：使用 DataBinding（见 6.2 节，最简洁）
DataBinding::bind(vm, "userName", m_nameEdit);
```

---

### 6.2 DataBinding（数据绑定）

`DataBinding` 提供简洁的 API 将 UI 控件自动绑定到 ViewModel 属性，支持**双向绑定**、**单向绑定**、**状态绑定**和**结构体字段绑定**。

#### 6.2.1 双向绑定（ViewModel ↔ View）

双向绑定意味着：ViewModel 属性变更时自动更新控件，用户修改控件时自动更新 ViewModel。

**支持的控件类型：**

```cpp
#include "mvvm/databinding.h"

// QLineEdit（QString）
DataBinding::bind(vm, "userName", m_nameEdit);

// QSpinBox（int）
DataBinding::bind(vm, "age", m_ageSpinBox);

// QDoubleSpinBox（double）
DataBinding::bind(vm, "price", m_priceSpinBox);

// QSlider（int）
DataBinding::bind(vm, "volume", m_volumeSlider);

// QCheckBox（bool）
DataBinding::bind(vm, "isActive", m_activeCheckBox);

// QRadioButton（bool）
DataBinding::bind(vm, "isSelected", m_radioButton);

// QComboBox（当前文本，QString）
DataBinding::bind(vm, "category", m_categoryComboBox);

// QComboBox（当前索引，int）
DataBinding::bindIndex(vm, "selectedIndex", m_categoryComboBox);

// QTextEdit（QString）
DataBinding::bind(vm, "description", m_descTextEdit);

// QDateEdit（QDate）
DataBinding::bind(vm, "birthDate", m_birthDateEdit);

// QTimeEdit（QTime）
DataBinding::bind(vm, "startTime", m_startTimeEdit);

// QDateTimeEdit（QDateTime）
DataBinding::bind(vm, "eventTime", m_eventDateTimeEdit);
```

**完整示例：**

```cpp
class UserView : public QWidget {
    Q_OBJECT
public:
    void setupBindings() {
        auto *vm = new UserViewModel(this);

        // 表单字段双向绑定
        DataBinding::bind(vm, "userName", m_nameEdit);
        DataBinding::bind(vm, "age", m_ageSpinBox);
        DataBinding::bind(vm, "email", m_emailEdit);
        DataBinding::bind(vm, "isActive", m_activeCheckBox);

        // 日期选择
        DataBinding::bind(vm, "birthDate", m_birthDateEdit);

        // 下拉选择
        DataBinding::bind(vm, "country", m_countryComboBox);

        // 文本区域
        DataBinding::bind(vm, "notes", m_notesTextEdit);
    }
};
```

#### 6.2.2 单向绑定（ViewModel → View）

单向绑定仅从 ViewModel 流向 View，适用于只读显示控件：

```cpp
// QLabel（显示文本）
DataBinding::bindOneWay(vm, "userName", m_nameLabel);

// QPushButton（按钮文本）
DataBinding::bindOneWay(vm, "buttonText", m_actionButton);

// QProgressBar（进度值，int）
DataBinding::bindOneWay(vm, "progress", m_progressBar);
```

#### 6.2.3 状态绑定（属性驱动控件状态）

将 bool 属性绑定到控件的 `enabled` 或 `visible` 状态：

```cpp
// 绑定控件可用状态（bool 属性 -> enabled）
DataBinding::bindEnabled(vm, "canSave", m_saveButton);
DataBinding::bindEnabled(vm, "isFormValid", m_submitButton);

// 绑定控件可见状态（bool 属性 -> visible）
DataBinding::bindVisible(vm, "showAdvanced", m_advancedWidget);
DataBinding::bindVisible(vm, "hasError", m_errorLabel);
```

**典型场景：** 表单验证通过时启用保存按钮：

```cpp
class FormViewModel : public ViewModel {
    Q_OBJECT
public:
    VIEWMODEL_PROPERTY(bool, canSave, CanSave)

    void validateForm() {
        bool valid = !name().isEmpty() && !email().isEmpty();
        setCanSave(valid);  // 自动更新按钮状态
    }
};

// View 中
DataBinding::bindEnabled(vm, "canSave", m_saveButton);
```

#### 6.2.4 结构体字段绑定

将结构体属性中的某个字段绑定到 UI 控件，实现字段级别的双向绑定：

```cpp
// 定义结构体
struct Address {
    QString street;
    QString city;
    QString zipCode;
};
Q_DECLARE_METATYPE(Address)

// ViewModel 中注册
class AddressViewModel : public ViewModel {
    Q_OBJECT
public:
    VIEWMODEL_STRUCT_PROPERTY(Address, address, Address)
};

// View 中绑定字段
AddressViewModel vm;

// 绑定 street 字段到 QLineEdit
DataBinding::bindStructField<Address>(
    &vm, "address",
    [](const Address &a) { return a.street; },        // getter
    [](Address &a, const QString &v) { a.street = v; }, // setter
    m_streetEdit
);

// 绑定 city 字段
DataBinding::bindStructField<Address>(
    &vm, "address",
    [](const Address &a) { return a.city; },
    [](Address &a, const QString &v) { a.city = v; },
    m_cityEdit
);

// 绑定 zipCode 字段到 QSpinBox（int）
DataBinding::bindStructField<Address>(
    &vm, "address",
    [](const Address &a) { return a.zipCode.toInt(); },
    [](Address &a, int v) { a.zipCode = QString::number(v); },
    m_zipSpinBox
);
```

#### 6.2.5 通用属性绑定

对于框架未直接支持的控件或自定义属性，使用通用绑定：

```cpp
// 绑定 ViewModel 属性到任意 QObject 的 Qt 属性
DataBinding::bindProperty(
    vm, "fontSize",           // ViewModel 属性名
    m_label, "fontPointSize"   // Widget 属性名
);

// 自定义控件示例
class CustomSlider : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    // ...
};

DataBinding::bindProperty(vm, "volume", m_customSlider, "value");
```

---

### 6.3 Command（命令模式）

`Command` 封装用户操作，支持启用/禁用控制、命令中间件拦截，实现 UI 与业务逻辑的解耦。

#### 6.3.1 FunctionalCommand（函数式命令）

`FunctionalCommand` 是最常用的命令类型，使用 Lambda 或函数对象定义执行逻辑：

```cpp
#include "mvvm/command.h"

class UserViewModel : public ViewModel {
    Q_OBJECT
public:
    explicit UserViewModel(QObject *parent = nullptr)
        : ViewModel(parent)
    {
        // 创建保存命令（带可执行条件）
        m_saveCommand = new FunctionalCommand(
            [this]() { saveUser(); },              // 执行函数
            [this]() { return canSave(); },        // 可执行条件（可选）
            this
        );

        // 创建清除命令（无条件，始终可执行）
        m_clearCommand = new FunctionalCommand(
            [this]() { clearForm(); },
            nullptr,  // 无条件，始终可执行
            this
        );

        // 监听属性变更，动态更新命令状态
        connect(this, &ViewModel::propertyChanged, this, [this]() {
            m_saveCommand->updateCanExecute();  // 重新计算 canExecute
        });
    }

    bool canSave() const {
        return !userName().isEmpty() && userAge() > 0;
    }

private:
    FunctionalCommand *m_saveCommand;
    FunctionalCommand *m_clearCommand;
};
```

#### 6.3.2 命令绑定到按钮

**方式1：使用 DataBinding（推荐）**

```cpp
#include "mvvm/databinding.h"

// 自动绑定：按钮点击 → command->run()，按钮 enabled ↔ command->canExecute()
DataBinding::bindCommand(vm->saveCommand(), m_saveButton);
DataBinding::bindCommand(vm->clearCommand(), m_clearButton);
```

**方式2：手动连接**

```cpp
// 连接按钮点击到命令执行
connect(m_saveButton, &QPushButton::clicked, vm->saveCommand(), &Command::run);

// 连接命令状态到按钮启用状态
connect(vm->saveCommand(), &Command::canExecuteChanged,
        m_saveButton, &QPushButton::setEnabled);
m_saveButton->setEnabled(vm->saveCommand()->canExecute());  // 初始状态
```

#### 6.3.3 命令中间件

命令中间件以"洋葱"模式拦截命令执行，可用于**权限检查**、**日志记录**、**性能计时**等：

```cpp
#include "mvvm/imiddleware.h"

// 权限检查中间件
class AuthCommandMiddleware : public ICommandMiddleware
{
public:
    void process(CommandContext& ctx, Next next) override
    {
        // 前置：权限检查
        if (ctx.commandName == "saveUser") {
            if (!hasPermission("user:write")) {
                ctx.cancelled = true;
                qWarning() << "Permission denied:" << ctx.commandName;
                return;  // 不调用 next，命令不会执行
            }
        }

        next(ctx);  // 继续执行

        // 后置：记录操作日志
        if (!ctx.cancelled) {
            qDebug() << "Command executed:" << ctx.commandName;
        }
    }
};

// 计时中间件
class TimingCommandMiddleware : public ICommandMiddleware
{
public:
    void process(CommandContext& ctx, Next next) override
    {
        QElapsedTimer timer;
        timer.start();

        next(ctx);

        qDebug() << "Command" << ctx.commandName
                 << "took" << timer.elapsed() << "ms";
    }
};

// 注册到命令
auto *saveCmd = new FunctionalCommand([this]() { saveUser(); }, nullptr, this);
saveCmd->setName("saveUser");
saveCmd->setViewModel(this);  // 中间件可通过 ctx.viewModel 访问

saveCmd->addMiddleware(std::make_shared<TimingCommandMiddleware>());  // 最外层
saveCmd->addMiddleware(std::make_shared<AuthCommandMiddleware>());   // 内层
```

**执行顺序：** 中间件按注册顺序由外到内包裹，执行路径为：

```
[Timing] → 开始计时
  [Auth] → 权限检查
    ★ execute() 执行命令
  [Auth] ← 记录日志
[Timing] ← 输出耗时
```

#### 6.3.4 命令状态管理

```cpp
// 手动设置可执行状态
command->setCanExecute(false);  // 禁用命令（会发出 canExecuteChanged 信号）

// 查询当前状态
bool canExec = command->canExecute();

// 对于 FunctionalCommand，更新可执行状态（重新计算条件）
auto *funcCmd = qobject_cast<FunctionalCommand*>(command);
if (funcCmd) {
    funcCmd->updateCanExecute();
}
```

---

### 6.4 属性中间件管道

属性中间件以"洋葱"模式拦截属性变更，可用于**日志记录**、**数据校验**、**格式转换**、**审计追踪**等。

#### 6.4.1 实现属性中间件

```cpp
#include "mvvm/imiddleware.h"
#include "mvvm/middlewarecontext.h"

// 数据校验中间件
class ValidationMiddleware : public IPropertyMiddleware
{
public:
    void process(PropertyContext& ctx, Next next) override
    {
        // 前置处理：校验
        if (ctx.propertyName == "age") {
            int val = ctx.newValue.toInt();
            if (val < 0 || val > 150) {
                ctx.cancelled = true;   // 取消本次属性变更
                qWarning() << "Invalid age:" << val;
                return;                 // 不调用 next，管道在此中断
            }
        } else if (ctx.propertyName == "email") {
            QString email = ctx.newValue.toString();
            if (!email.contains("@")) {
                ctx.cancelled = true;
                qWarning() << "Invalid email format:" << email;
                return;
            }
        }

        next(ctx);   // 继续向后传递（校验通过）

        // 后置处理（next 返回后执行）
    }
};

// 日志记录中间件
class LoggingMiddleware : public IPropertyMiddleware
{
public:
    void process(PropertyContext& ctx, Next next) override
    {
        qDebug() << "[Before]" << ctx.propertyName
                 << ":" << ctx.oldValue << "->" << ctx.newValue;

        next(ctx);  // 继续执行

        qDebug() << "[After]" << ctx.propertyName
                 << (ctx.cancelled ? "CANCELLED" : "OK");
    }
};

// 审计追踪中间件
class AuditLogMiddleware : public IPropertyMiddleware
{
public:
    void process(PropertyContext& ctx, Next next) override
    {
        next(ctx);  // 先执行，确保值已写入

        // 后置：记录变更历史（仅在成功时）
        if (!ctx.cancelled) {
            QString log = QString("Property '%1' changed: %2 -> %3")
                          .arg(ctx.propertyName)
                          .arg(ctx.oldValue.toString())
                          .arg(ctx.newValue.toString());
            writeAuditLog(log);
        }
    }
};
```

#### 6.4.2 注册中间件

```cpp
class ProductViewModel : public ViewModel {
    Q_OBJECT
public:
    explicit ProductViewModel(QObject *parent = nullptr)
        : ViewModel(parent)
    {
        registerProperty("name", QString());
        registerProperty("price", 0.0);

        // 注册中间件（执行顺序：Logging → Validation → Audit → 实际写入）
        addPropertyMiddleware(std::make_shared<LoggingMiddleware>());    // 第 1 层（最外层）
        addPropertyMiddleware(std::make_shared<ValidationMiddleware>());  // 第 2 层
        addPropertyMiddleware(std::make_shared<AuditLogMiddleware>());     // 第 3 层（最内层）
    }
};
```

**执行流程示意：**

```
setProperty("price", 100.0)
    │
    ▼
[Logging] → 前置日志
    │
    ▼
[Validation] → 校验通过
    │
    ▼
[Audit] → 记录审计
    │
    ▼
★ 属性写入（最深处）
    │
    ▼
[Audit] ← 后置处理
    │
    ▼
[Validation] ← 后置处理
    │
    ▼
[Logging] ← 后置日志
```

#### 6.4.3 中间件上下文（PropertyContext）

中间件可通过 `PropertyContext` 访问属性变更的完整信息：

```cpp
struct PropertyContext {
    QString    propertyName;  // 属性名
    QVariant   oldValue;      // 变更前的值（只读）
    QVariant   newValue;      // 变更后的值（中间件可修改）
    ViewModel *viewModel;     // 所属 ViewModel（可读取其他属性）
    bool       cancelled;     // true = 阻止本次属性变更
};
```

**修改新值示例：**

```cpp
class FormatMiddleware : public IPropertyMiddleware
{
public:
    void process(PropertyContext& ctx, Next next) override
    {
        // 修改新值：自动去除首尾空格
        if (ctx.propertyName == "userName") {
            QString trimmed = ctx.newValue.toString().trimmed();
            ctx.newValue = QVariant(trimmed);
        }

        next(ctx);
    }
};
```

**访问其他属性示例：**

```cpp
class CrossPropertyValidationMiddleware : public IPropertyMiddleware
{
public:
    void process(PropertyContext& ctx, Next next) override
    {
        // 通过 viewModel 访问其他属性
        if (ctx.propertyName == "endDate") {
            QDate endDate = ctx.newValue.toDate();
            QDate startDate = ctx.viewModel->getProperty("startDate").toDate();
            if (endDate < startDate) {
                ctx.cancelled = true;
                qWarning() << "End date must be after start date";
                return;
            }
        }

        next(ctx);
    }
};
```

---

### 6.5 完整使用示例

以下是一个完整的 MVVM 使用示例，展示 ViewModel、DataBinding、Command 和中间件的协同工作：

```cpp
// ── ViewModel ─────────────────────────────────────────────────────────────
class ProductViewModel : public ViewModel {
    Q_OBJECT
public:
    explicit ProductViewModel(QObject *parent = nullptr)
        : ViewModel(parent)
    {
        // 注册属性
        registerProperty("name", QString());
        registerProperty("price", 0.0);
        registerProperty("stock", 0);
        registerProperty("status", QString("就绪"));

        // 注册属性中间件
        addPropertyMiddleware(std::make_shared<LoggingMiddleware>());
        addPropertyMiddleware(std::make_shared<ValidationMiddleware>());

        // 创建命令
        m_addCommand = new FunctionalCommand(
            [this]() { addProduct(); },
            [this]() { return canAdd(); },
            this
        );
        m_addCommand->setName("addProduct");
        m_addCommand->setViewModel(this);

        // 注册命令中间件
        m_addCommand->addMiddleware(std::make_shared<TimingCommandMiddleware>());
        m_addCommand->addMiddleware(std::make_shared<AuthCommandMiddleware>());

        // 监听属性变更，更新命令状态
        connect(this, &ViewModel::propertyChanged, this, [this]() {
            m_addCommand->updateCanExecute();
        });
    }

    VIEWMODEL_PROPERTY(QString, name, Name)
    VIEWMODEL_PROPERTY(double, price, Price)
    VIEWMODEL_PROPERTY(int, stock, Stock)
    VIEWMODEL_PROPERTY(QString, status, Status)

    FunctionalCommand* addCommand() const { return m_addCommand; }

    bool canAdd() const {
        return !name().isEmpty() && price() >= 0.0 && stock() >= 0;
    }

private:
    void addProduct() {
        // 业务逻辑：保存产品
        Product p;
        p.name = name();
        p.price = price();
        p.stock = stock();
        ProductRepository::save(p);

        setStatus("产品已添加");
        clearForm();
    }

    void clearForm() {
        beginBatchUpdate();
        setName("");
        setPrice(0.0);
        setStock(0);
        endBatchUpdate();
    }

    FunctionalCommand *m_addCommand;
};

// ── View ───────────────────────────────────────────────────────────────────
class ProductView : public QWidget {
    Q_OBJECT
public:
    void setupBindings() {
        m_viewModel = new ProductViewModel(this);

        // 双向绑定表单字段
        DataBinding::bind(m_viewModel, "name", m_nameEdit);
        DataBinding::bind(m_viewModel, "price", m_priceSpinBox);
        DataBinding::bind(m_viewModel, "stock", m_stockSpinBox);

        // 单向绑定状态显示
        DataBinding::bindOneWay(m_viewModel, "status", m_statusLabel);

        // 命令绑定
        DataBinding::bindCommand(m_viewModel->addCommand(), m_addButton);

        // 状态绑定（表单验证通过时启用按钮）
        DataBinding::bindEnabled(m_viewModel, "canAdd", m_addButton);
    }

private:
    ProductViewModel *m_viewModel;
    QLineEdit *m_nameEdit;
    QDoubleSpinBox *m_priceSpinBox;
    QSpinBox *m_stockSpinBox;
    QLabel *m_statusLabel;
    QPushButton *m_addButton;
};
```

---

### 6.6 MVVM 最佳实践

1. **属性命名规范**
   - 使用常量定义属性名，避免硬编码字符串
   - 属性名使用 camelCase，与 Qt 属性系统一致

2. **ViewModel 职责边界**
   - ViewModel **不应**持有任何 `QWidget*` 指针
   - ViewModel **不应**直接操作 UI，只通过属性变更信号通知
   - 业务逻辑放在 ViewModel 的 `public slots` 中

3. **批量更新使用场景**
   - 初始化时加载多个属性
   - 表单重置时清空多个字段
   - 从外部数据源一次性更新多个属性

4. **中间件设计原则**
   - 每个中间件只负责一个关注点（日志/校验/审计）
   - 中间件应是无状态的（或通过上下文传递状态）
   - 使用 `ctx.cancelled` 而非抛出异常来阻止变更

5. **命令与属性联动**
   - 命令的可执行条件应基于 ViewModel 属性
   - 属性变更时调用 `updateCanExecute()` 更新命令状态
   - 使用 `DataBinding::bindCommand()` 自动同步按钮状态

---

## 7. UI 基础设施 — uibase

### 7.1 无边框窗口 — CFramelessWindow

`CFramelessWindow` 提供跨平台无边框主窗口，支持拖拽移动、调整大小、圆角等特性。

```cpp
#include "uibase/framelesswindow.h"

class MainWindow : public CFramelessWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr)
        : CFramelessWindow(parent)
    {
        // 设置自定义标题栏（禁止系统标题栏拖拽区域与标题栏重叠）
        setTitleBar(m_titleBarWidget);

        // 标题栏中的按钮不触发窗口拖拽
        addIgnoreWidget(m_closeBtn);
        addIgnoreWidget(m_minBtn);
        addIgnoreWidget(m_maxBtn);

        // 允许调整大小，边缘宽度 5px
        setResizeable(true);
        setResizeableAreaWidth(5);

        // 圆角（像素）
        setCornerRadius(8);
    }
};
```

### 7.2 导航管理 — NavigationManager

框架提供**一级水平导航**和**二级垂直侧栏导航**两种管理器。

**一级导航（PrimaryNavigationManager）：**

```cpp
#include "uibase/navigationmanager.h"

// 在主窗口中初始化
auto *nav = new PrimaryNavigationManager(m_navBar, m_stackedLayout, this);

// 注册导航项（id、翻译 Key、页面 Widget）
nav->addNavigation("nav.home",     "menu.home",     homePage);
nav->addNavigation("nav.features", "menu.features", featuresPage);
nav->addNavigation("nav.settings", "menu.settings", settingsPage);
nav->addNavigation("nav.help",     "menu.help",     helpPage);

// 在导航栏右侧添加自定义控件（如主题切换按钮）
nav->addRightWidget(m_themeToggleBtn);

// 跳转到指定页面
nav->switchToPage("nav.home");

// 动态显示/隐藏某导航项（权限控制）
nav->setNavItemVisible("nav.settings", false);

// 监听页面切换
connect(nav, &PrimaryNavigationManager::pageChanged,
        this, &MainWindow::onPageChanged);
```

**二级导航（SecondaryNavigationManager）：**

```cpp
// 在具有二级导航的页面中使用
auto *subNav = new SecondaryNavigationManager(m_sideBar, m_subStackedLayout, this);

subNav->addNavigation("sub.general",  "settings.general",  generalPage);
subNav->addNavigation("sub.account",  "settings.account",  accountPage);
subNav->addNavigation("sub.privacy",  "settings.privacy",  privacyPage);

// 动态刷新当前页
subNav->refreshCurrentPage();
```

### 7.3 权限运行时 — PermService

主导航与子页可见性由 `PermService`（`PS` 宏）根据当前用户 grants 决定。各 UI 插件在 `initialize()` 注册页面时同步调用 `registerNav` / `registerSubPage`；Permission 插件登录成功后调用 `reloadForUser(userId)`。

```cpp
#include "permservice.h"

// 查询导航权限
if (PS->canNav(QStringLiteral("nav.settings"))) { /* ... */ }

// 查询子页 / 操作权限
if (PS->canSubPage(QStringLiteral("nav.settings"), QStringLiteral("sub.general"))) { /* ... */ }
if (PS->canAction(QStringLiteral("perm.users.create"))) { /* ... */ }

// 响应权限变更（MainWindow 已内置 refreshNavigation）
connect(PS, &PermService::permissionsChanged, this, [this]() {
    refreshMyActionButtons();
});
```

完整说明见 `common/plugins/permission/user_guide.md`。

### 7.4 主题管理 — ThemeManager

单例主题管理器，支持亮色/暗色切换，并广播 `themeChanged` 信号：

```cpp
#include "uibase/thememanager.h"

// 全局宏访问（等价于 ThemeManager::instance()）
TM->setTheme(ThemeManager::Light);
TM->setTheme(ThemeManager::Dark);
TM->toggleTheme();

// 查询
bool isDark = TM->isDark();
ThemeManager::Theme current = TM->currentTheme();

// 获取应用样式表（由 ThemeManager 内部维护）
QString qss = TM->applicationStyleSheet();
qApp->setStyleSheet(qss);

// 监听主题变更
connect(TM, &ThemeManager::themeChanged,
        this, [this](ThemeManager::Theme t) {
    // 重新应用样式
    applyTheme(t);
});
```

**推荐初始化位置：** 在 `main()` 中，`QApplication` 创建后、主窗口创建前：

```cpp
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    TM->setTheme(ThemeManager::Light);   // ← 必须在主窗口前调用
    // ...
}
```

### 7.5 语言国际化 — LanguageManager

内置中英文切换，通过 `t(key)` 函数查询翻译字典：

```cpp
#include "uibase/languagemanager.h"

// 切换语言
LM->setLanguage(LanguageManager::Chinese);
LM->setLanguage(LanguageManager::English);
LM->toggleLanguage();

// 翻译（TR 宏为简写）
QString text = TR("menu.home");           // → "首页" 或 "Home"
QString text = TR("menu.settings");       // → "设置" 或 "Settings"

// 带参数的翻译
QString msg = LM->t("error.file_not_found", filePath);

// 监听语言变更（自动刷新 UI）
connect(LM, &LanguageManager::languageChanged,
        this, &MyWidget::retranslateUi);
```

**在 Widget 中自动响应语言切换：**

```cpp
void MyWidget::retranslateUi()
{
    m_titleLabel->setText(TR("page.home.title"));
    m_saveBtn->setText(TR("button.save"));
}
```

> `NavigationManager` 内部已连接 `languageChanged` 信号，会自动刷新导航按钮文字，无需手动处理。

---

## 8. CMake 构建辅助 — FZ2Helpers

`common/cmake/FZ2Helpers.cmake` 提供四个辅助函数，简化重复的 CMake 配置：

### `fz2_setup_target(target, vs_folder)`

设置**可执行程序或共享库**的输出目录，并分配到 VS 解决方案文件夹：

```cmake
# 输出到 bin/<Config>/，VS 分组到 "common/core"
fz2_setup_target(container "common/core")
fz2_setup_target(extensionsystem "common/core")
fz2_setup_target(uibase "common/ui")
```

### `fz2_setup_plugin(target, vs_folder)`

设置**插件 DLL** 的输出目录（`bin/<Config>/plugins/`）并清除 `lib` 前缀：

```cmake
# 输出到 bin/<Config>/plugins/，VS 分组到 "plugins/myplugin"
fz2_setup_plugin(MyPlugin "plugins/myplugin")
```

### `fz2_collect_sources(out_sources, out_headers)`

递归收集当前目录下所有 `.cpp` / `.h` / `.hpp` 文件，支持 `CONFIGURE_DEPENDS`（文件增减时自动触发重新配置）：

```cmake
fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(MyPlugin SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})
```

### `fz2_target_msvc_options(target)`

在 MSVC 编译器下为 target 添加 `/utf-8 /W3` 编译选项：

```cmake
fz2_target_msvc_options(MyPlugin)
```

---

## 9. 内置示例插件

框架附带多个示例插件，涵盖各模块的典型用法：

| 插件 | 目录 | 演示内容 |
|---|---|---|
| **Core** | `common/plugins/core/` | 主窗口创建、无边框窗口、一级导航初始化 |
| **Home** | `plugins/home/` | 最简单的单页插件实现 |
| **Features** | `plugins/features/` | 带二级子导航的多页面插件 |
| **Settings** | `plugins/settings/` | 设置容器（通用/账号/隐私/关于） |
| **Help** | `plugins/help/` | 帮助页面 |
| **MVVM** | `plugins/mvvm/` | ViewModel 属性绑定、DataBinding、结构体属性 |
| **Middleware** | `plugins/middleware/` | 属性中间件管道（日志 + 校验） |
| **Invoker** | `plugins/invoker/` | ExtensionSystem::Invoker 动态调用演示 |
| **Container** | `plugins/container/` | IoC 容器类型绑定 + 动态命名绑定跨插件调用 |

---

## 10. 最佳实践

### 插件设计

- **单一职责**：每个插件只负责一个业务域，避免功能蔓延
- **接口分离**：对外暴露的服务应定义为纯虚接口，实现细节放在插件内部
- **避免循环依赖**：使用 `Dependencies` 字段声明依赖，框架保证加载顺序

### 服务通信选择

| 场景 | 推荐方式 |
|---|---|
| 简单服务共享，调用方知道接口类型 | `PluginManager::getObject<T>()` |
| 复杂依赖注入，需要生命周期管理 | `Container::make<T>()` |
| 跨插件，不能引入对方头文件 | `Container::makeNamed()` + 反射调用 |
| 运行时动态调用已知 QObject 方法 | `ExtensionSystem::invoke<T>()` |

### MVVM 分层原则

```
View（Widget）
    │  只负责显示和用户输入
    │  通过信号/槽或属性绑定连接 ViewModel
    ↓
ViewModel
    │  持有 UI 状态，封装业务逻辑
    │  不包含任何 Qt Widget 代码
    ↓
Model / Service
    │  纯业务数据和服务（通过 Container 注入）
    │  完全无 UI 依赖
```

- ViewModel **不应**持有任何 `QWidget*` 指针
- View **不应**直接访问 Model/Service，必须通过 ViewModel
- 使用**中间件**处理通用横切关注点（日志、校验、权限），而不是在 ViewModel 中堆砌 if-else

### 权限控制

```cpp
// 权限变更时刷新 UI（MainWindow 已处理导航；业务侧刷新 Command/按钮）
connect(PS, &PermService::permissionsChanged, this, [this]() {
    updateActionVisibility();
});
```

### 主题与语言

- 主题相关样式应通过 `ThemeManager::applicationStyleSheet()` 统一管理，避免硬编码颜色
- 所有 UI 文字使用 `TR("key")` 宏，禁止硬编码中文/英文字符串
- Widget 重写 `retranslateUi()` 方法并连接 `LanguageManager::languageChanged` 信号

---

## 11. 常见问题

**Q: 插件加载失败，日志提示 "No such file or directory"**

检查插件 DLL 及其依赖（Qt DLL、框架 DLL）是否都在 `bin/<Config>/` 目录下；确认 `Q_PLUGIN_METADATA` 中的 `IID` 与 `PluginManager::setPluginIID()` 设置的值完全一致。

---

**Q: `Container::make<T>()` 抛出 `No binding found for type`**

检查对应的 `ServiceProvider::doRegister()` 是否已被注册到 `Application`，且 `bootstrap()` 已被调用。如果是跨 DLL 访问，确认 `container` 是以 SHARED 库编译的。

---

**Q: Facade 调用时抛出 `Global instance not set`**

需要在调用 Facade 之前先执行 `Container::Application::setInstance(app.get())`，通常放在 `main()` 中 `bootstrap()` 之前。

---

**Q: 语言切换后导航按钮文字没有更新**

`NavigationManager` 内部已连接 `LanguageManager::languageChanged` 信号，若注册导航时传入的 `titleKey` 在翻译字典中存在对应条目，则会自动刷新。检查 `LanguageManager::initTranslations()` 中是否有该 key 的翻译。

---

**Q: 权限变更后页面没有隐藏**

`MainWindow` 已连接 `PermService::permissionsChanged` 并调用 `refreshNavigation()`。若自定义控件未更新，检查是否监听同一信号；并确认对应 nav/sub_page 已在插件中 `registerNav` / `registerSubPage`，且当前用户 grants 含该 key。

---

**Q: `extend()` 对通过 `instance()` 注册的绑定无效**

`extend()` 要求原绑定有工厂函数，而 `instance()` 注册的绑定没有工厂，直接调用 `extend()` 会导致崩溃。若需要装饰，请改用 `singleton()` 或 `bind()` 方式注册原始服务。

---

**Q: 命名绑定 `call()` 报 `bad_any_cast`**

参数的 C++ 类型必须与注册时 `MethodDispatcher::on()` 推导的参数类型完全一致（经 `std::decay` 处理后）。例如方法签名为 `void info(const std::string&)`，调用时应传入 `std::any{std::string("msg")}`，而不是 `std::any{"msg"}`（字面量为 `const char*`，类型不匹配）。

---

*如需更深入了解 IoC 容器的设计原理，请参阅 [`docs/container_design.md`](container_design.md)。*
