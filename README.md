# Aether Platform — Project Template

> **基于 Qt + CMake 的插件化桌面应用开发框架**  
> 版本：1.0.0 &nbsp;·&nbsp; C++17 &nbsp;·&nbsp; Qt 5.15 / Qt 6.x &nbsp;·&nbsp; Windows / Linux / macOS

---

## 📋 目录

- [项目简介](#-项目简介)
- [功能特性](#-功能特性)
- [架构概览](#-架构概览)
- [项目结构](#-项目结构)
- [依赖要求](#-依赖要求)
- [快速开始](#-快速开始)
- [构建说明](#-构建说明)
- [插件开发指南](#-插件开发指南)
- [模块说明](#-模块说明)
- [配置文件](#-配置文件)
- [开发规范](#-开发规范)
- [文档资源](#-文档资源)
- [许可证](#-许可证)

---

## 🎯 项目简介

**Aether Platform**（设备软件系统）是一套面向工业/设备软件领域的 **Qt 插件化桌面应用框架模板**。

该框架以 **Qt 动态插件（DLL/SO）** 为核心扩展单元，集成了完整的 IoC 服务容器、MVVM 架构支持和 UI 基础设施，帮助开发团队快速构建可维护、可扩展的桌面应用程序。

### 适用场景

- 工业设备上位机软件
- 多模块插件化桌面应用
- 需要权限管理、主题切换的企业级桌面端

---

## ✨ 功能特性

| 特性 | 描述 |
|---|---|
| 🔌 **插件化架构** | 基于 Qt 插件系统，支持动态加载、依赖解析、生命周期管理 |
| 💉 **IoC 服务容器** | Laravel 风格的依赖注入容器，支持 Singleton / Transient / Instance 三种绑定 |
| 📊 **MVVM 框架** | 完整的 ViewModel + DataBinding + Command + 中间件管道实现 |
| 🎨 **主题系统** | 支持亮色 / 暗色主题一键切换，全局样式表驱动 |
| 🪟 **无边框窗口** | 跨平台无边框窗口，支持拖拽、缩放和圆角 |
| 🔐 **权限管理** | 基于角色（RBAC）的菜单访问控制，支持运行时动态切换 |
| 🌐 **多语言支持** | 内置国际化语言管理，支持中英文动态切换 |
| 🧭 **导航管理** | 主/次双级导航管理器，支持懒加载页面 |
| ⚙️ **外置配置** | 菜单结构与角色权限通过 JSON 文件配置，无需重新编译 |
| 🧩 **中间件管道** | 洋葱模式中间件，支持属性变更和命令执行的 AOP 横切拦截 |

---

## 🏗️ 架构概览

```
┌─────────────────────────────────────────────────────────────┐
│                        应用层 (app/)                         │
│            main.cpp — 初始化 QApplication + PluginManager    │
└────────────────────────────┬────────────────────────────────┘
                             │ 加载插件
┌────────────────────────────▼────────────────────────────────┐
│                       插件层 (plugins/)                      │
│   core  ·  mvvm  ·  container  ·  middleware  ·  invoker     │
│   各插件通过 IPlugin 接口注入框架，实现具体业务功能            │
└────────────────────────────┬────────────────────────────────┘
                             │ 依赖
┌────────────────────────────▼────────────────────────────────┐
│                    公共基础层 (common/)                       │
│  ┌───────────────────────────────────────────────────────┐  │
│  │  core/extensionsystem  — 插件加载 / 对象池 / 依赖管理   │  │
│  │  core/container        — IoC 容器 / ServiceProvider    │  │
│  ├───────────────────────────────────────────────────────┤  │
│  │  ui/mvvm               — ViewModel / DataBinding / Cmd │  │
│  │  ui/uibase             — 主题 / 导航 / 权限 / 国际化    │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 模块依赖关系

```
app
 └─ ExtensionSystem (pluginmanager)
      └─ IPlugin / PluginSpec
           └─ 业务插件 (core, mvvm, container, middleware, invoker...)
                ├─ container (Application / ServiceProvider / Facade)
                └─ mvvm (ViewModel / DataBinding / Command)
                     ├─ middlewarepipeline
                     └─ uibase (ThemeManager / NavigationManager / PermService...)
```

---

## 📁 项目结构

```
project-template/
├── app/                        # 主程序入口
│   ├── main.cpp                # 应用程序入口，初始化 PluginManager
│   └── CMakeLists.txt
│
├── common/                     # 公共基础库（核心框架）
│   ├── cmake/
│   │   └── FZ2Helpers.cmake    # CMake 辅助函数（目标配置、插件构建等）
│   ├── core/
│   │   ├── extensionsystem/    # 插件系统（PluginManager / IPlugin / PluginSpec）
│   │   ├── container/          # IoC 容器（Container / Application / ServiceProvider）
│   │   ├── communication/      # 通信模块
│   │   └── device/             # 设备模块
│   ├── ui/
│   │   ├── mvvm/               # MVVM 框架（ViewModel / DataBinding / Command）
│   │   ├── uibase/             # UI 基础设施（主题 / 导航 / 权限 / 语言）
│   │   ├── component/          # 通用 UI 组件
│   │   └── resources/          # 样式表、图标等资源
│   ├── utility/                # 工具类
│   ├── include/                # 公共头文件
│   └── README.md               # common 模块详细文档
│
├── plugins/                    # 业务插件目录
│   ├── core/                   # 核心插件（创建主窗口，注册 IMainWindowService）
│   ├── container/              # 容器演示插件
│   ├── mvvm/                   # MVVM 演示插件
│   ├── middleware/             # 中间件演示插件
│   ├── invoker/                # Invoker 反射调用演示插件
│   ├── home/                   # 首页插件
│   ├── settings/               # 设置页插件
│   └── CMakeLists.txt
│
├── config/                     # 运行时配置文件
│   ├── app.toml                # 应用配置（主题等）
│   └── app_zh_CN.ts / app_en_US.ts  # 全局翻译
│
├── docs/                       # 项目文档
│   ├── source_analysis.md      # 框架源码深度分析
│   ├── git_management.md       # Git 分支与提交规范
│   └── user_guide.md           # 用户使用指南
│
├── scripts/
│   └── build.bat               # Windows 一键构建脚本
│
├── tests/                      # 单元测试
│   ├── test_container.cpp
│   └── CMakeLists.txt
│
└── CMakeLists.txt              # 顶层 CMake 配置
```

---

## 🔧 依赖要求

### 必需依赖

| 依赖项 | 最低版本 | 说明 |
|---|---|---|
| **Qt** | 5.15 或 6.x | QtCore · QtGui · QtWidgets |
| **CMake** | 3.16+ | 构建系统 |
| **C++ 编译器** | C++17 支持 | 见下表 |

### 支持的编译器

| 平台 | 编译器 | 最低版本 |
|---|---|---|
| Windows | MSVC | 2019 / 2022 |
| Windows | MinGW | 8.0+ |
| Linux | GCC | 7+ |
| Linux / macOS | Clang | 5+ |

---

## 🚀 快速开始

### 1. 克隆项目

```bash
git clone <repository-url>
cd project-template
```

### 2. Windows 一键构建（推荐）

```bat
scripts\build.bat Release
```

> 脚本会自动检测 MSVC 环境、定位 Qt5，完成 CMake 配置和编译。

### 3. 运行程序

```
build\bin\Release\myapp.exe
```

---

## 🏗️ 构建说明

### Windows（使用 build.bat）

```bat
:: 默认 Release 构建
scripts\build.bat

:: 指定构建类型
scripts\build.bat Debug
scripts\build.bat Release
scripts\build.bat RelWithDebInfo

:: 清理后重新构建
scripts\build.bat clean
scripts\build.bat clean Debug
```

### Windows（手动 CMake）

```powershell
mkdir build
cd build

# Visual Studio 2022 x64
cmake .. -G "Visual Studio 17 2022" -A x64 -DQt5_DIR="C:\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5"

# 编译
cmake --build . --config Release --parallel
```

### Linux / macOS

```bash
mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_PREFIX_PATH=/path/to/Qt/5.15.2/gcc_64

make -j$(nproc)
```

### CMake 选项

| 选项 | 默认值 | 说明 |
|---|---|---|
| `CMAKE_BUILD_TYPE` | `RelWithDebInfo` | 构建类型（单配置生成器） |
| `CMAKE_PREFIX_PATH` | — | Qt 安装根路径 |
| `Qt5_DIR` | — | Qt5 CMake 配置文件目录 |

### 输出目录结构

```
build/bin/<Config>/
├── myapp.exe           # 主程序
├── plugins/            # 插件 DLL
│   ├── core.dll
│   ├── mvvm.dll
│   └── ...
└── config/             # 运行时配置
    └── app.toml
```

---

## 🔌 插件开发指南

### 创建一个新插件

**第一步：定义插件类**

```cpp
// myplugin.h
#include "extensionsystem/iplugin.h"

class MyPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.pluginsystem.IPlugin"
                      FILE "myplugin.json")

public:
    bool initialize(QString *errorString) override;
    void extensionsInitialized() override;
    bool delayedInitialize() override;       // 可选，延迟初始化
    ShutdownFlag aboutToShutdown() override; // 可选，应用关闭前
};
```

**第二步：编写插件元数据**

```json
// myplugin.json
{
    "Name"        : "MyPlugin",
    "Version"     : "1.0.0",
    "CompatVersion": "1.0.0",
    "Category"    : "MyCategory",
    "Description" : "My custom plugin",
    "Dependencies": [
        { "Name": "Core", "Version": "1.0.0" }
    ]
}
```

**第三步：实现插件逻辑**

```cpp
// myplugin.cpp
bool MyPlugin::initialize(QString *errorString)
{
    Q_UNUSED(errorString)
    // 在此注册服务、创建对象并加入对象池
    return true;
}

void MyPlugin::extensionsInitialized()
{
    // 所有插件均已初始化后调用，可安全访问其他插件的服务
    auto *mws = ExtensionSystem::PluginManager::getObject<IMainWindowService>();
    if (mws)
        mws->addPage("myplugin", "nav.myplugin", new MyPage(), 50);
}
```

**第四步：配置 CMakeLists.txt**

```cmake
fz2_setup_plugin(myplugin
    SOURCES myplugin.cpp myplugin.h
    LINKS   extensionsystem uibase
)
```

---

## 📚 模块说明

### ExtensionSystem — 插件系统

插件系统提供完整的插件生命周期管理。

**插件状态机：**

```
Invalid → Read → Resolved → Loaded → Initialized → Running → Stopped → Deleted
```

**核心类：**

| 类名 | 说明 |
|---|---|
| `IPlugin` | 插件接口基类，定义 `initialize` / `extensionsInitialized` 等生命周期回调 |
| `PluginManager` | 插件管理器，负责扫描、加载、对象池管理 |
| `PluginSpec` | 插件描述符，持有元数据和状态机 |

---

### Container — IoC 服务容器

Laravel 风格的依赖注入容器，支持类型安全的服务绑定与解析。

**绑定示例：**

```cpp
// 注册服务（三种生命周期）
app->bind<ILogger>([](Container& c) {
    return std::make_shared<FileLogger>();
});
app->singleton<IDatabase>([](Container& c) {
    return std::make_shared<SqliteDatabase>();
});
app->instance<IConfig>(std::make_shared<AppConfig>());

// 解析服务
auto logger = app->make<ILogger>();
logger->info("Hello, world!");
```

**核心类：**

| 类名 | 说明 |
|---|---|
| `Container` | 基础 IoC 容器，提供 `bind` / `singleton` / `make` 等核心 API |
| `Application` | 容器的运行期增强版，支持 ServiceProvider 两阶段引导 |
| `ServiceProvider` | 服务提供者基类，将注册逻辑模块化 |
| `Facade<T>` | 外观模式静态访问入口（`App::make<T>()` 的语法糖） |

---

### MVVM — 视图模型框架

基于 Qt 信号槽机制的完整 MVVM 实现。

**ViewModel 示例：**

```cpp
#include "mvvm/viewmodel.h"

class UserViewModel : public ViewModel
{
    Q_OBJECT
public:
    VIEWMODEL_PROPERTY(QString, userName, UserName)
    VIEWMODEL_PROPERTY(int,     age,      Age)

    explicit UserViewModel(QObject *parent = nullptr)
        : ViewModel(parent) {}
};
```

**DataBinding 示例：**

```cpp
// 双向绑定：ViewModel ↔ QLineEdit
DataBinding::bind(vm, "userName", ui->lineEditName);

// 单向绑定：ViewModel → QLabel
DataBinding::bindOneWay(vm, "statusText", ui->labelStatus);

// 命令绑定：按钮 → Command
DataBinding::bindCommand(saveCmd, ui->btnSave);
```

**核心类：**

| 类名 | 说明 |
|---|---|
| `ViewModel` | 视图模型基类，以 `QHash<QString, QVariant>` 存储属性 |
| `DataBinding` | 数据绑定工具类，支持控件双向绑定、可见性绑定、结构体字段绑定 |
| `Command` / `FunctionalCommand` | 命令封装，支持 `canExecute` 状态和中间件管道 |
| `PropertyMiddlewarePipeline` | 属性变更中间件管道（洋葱模式） |
| `CommandMiddlewarePipeline` | 命令执行中间件管道（洋葱模式） |

---

### UIBase — UI 基础设施

提供主题、导航、权限、国际化等全局 UI 管理能力。

```cpp
// 主题切换
TM->setTheme(ThemeManager::Dark);
TM->toggleTheme();

// 多语言
button->setText(TR("nav.home"));

// 权限检查（PermService）
if (PS->canNav(QStringLiteral("nav.settings"))) { /* ... */ }

// 登录后刷新 grants（由 Permission 插件调用）
PS->reloadForUser(userId);
```

**核心管理器：**

| 管理器 | 宏 | 说明 |
|---|---|---|
| `ThemeManager` | `TM` | 亮色 / 暗色主题管理 |
| `LanguageManager` | `LM` / `TR(key)` | 多语言翻译 |
| `PermService` | `PS` | 导航/子页/操作权限运行时 |
| `PrimaryNavigationManager` | — | 主导航管理 |
| `SecondaryNavigationManager` | — | 次级导航管理 |

---

## ⚙️ 配置文件

导航与权限由**各插件注册页面时**调用 `PermService::registerNav` / `registerSubPage` 写入权限目录；登录后 `reloadForUser` 从 DB 加载 grants。详见 `common/plugins/permission/user_guide.md`。

---

## 📐 开发规范

### 代码规范

- 遵循 Qt 编码规范，类名使用大驼峰，成员变量以 `m_` 前缀
- 使用 **C++17** 标准特性（`if constexpr`、折叠表达式、`std::optional` 等）
- 所有公共 API 必须添加 Doxygen 文档注释
- 头文件使用 `#pragma once`，禁止使用传统头文件守卫
- 使用命名空间组织代码（`ExtensionSystem` / `Container` / `Mvvm` / `UIBase`）

### 插件规范

- 每个插件对应一个独立子目录，包含 `.json` 元数据文件
- 插件间通信优先通过 **对象池**（`PluginManager::addObject` / `getObject`）
- 跨插件零头文件依赖时，使用 **命名绑定 + MethodDispatcher**
- 插件不得持有其他插件的头文件（接口除外）

### 提交规范

参考 [Git 管理规范](docs/git_management.md)，遵循 Conventional Commits：

```
feat(mvvm): 新增批量更新 API
fix(container): 修复循环依赖检测误报
docs(readme): 更新构建说明
refactor(uibase): 重构导航管理器
```

---

## 📖 文档资源

| 文档 | 路径 | 说明 |
|---|---|---|
| 框架源码分析 | [`docs/source_analysis.md`](docs/source_analysis.md) | 各模块深度源码解析、设计模式说明、执行链路全景 |
| Git 管理规范 | [`docs/git_management.md`](docs/git_management.md) | 分支策略、提交规范、版本发布流程 |
| 用户使用指南 | [`docs/user_guide.md`](docs/user_guide.md) | 面向最终用户的操作手册 |
| common 模块文档 | [`common/README.md`](common/README.md) | 公共基础库详细 API 说明与使用示例 |

---

## 🤝 贡献指南

1. Fork 本仓库并创建特性分支

   ```bash
   git checkout -b feature/your-feature-name
   ```

2. 遵循代码规范进行开发，并完善单元测试

3. 提交更改（遵循 Conventional Commits 规范）

   ```bash
   git commit -m "feat(plugin): add new feature"
   ```

4. 推送分支并发起 Pull Request

   ```bash
   git push origin feature/your-feature-name
   ```

---

## 📄 许可证

本项目采用 [MIT License](LICENSE)。详情请参阅 `LICENSE` 文件。

---

## 📞 联系方式

如有问题或建议，请通过以下方式联系：

- **Email**: contact@aether.tech
- **Issues**: 请在仓库 Issue 区提交问题报告

---

<div align="center">

**Aether Platform** · 基于 Qt + CMake 的插件化桌面应用框架  
版本 1.0.0 · C++17 · 文档更新于 2026-03-11

</div>
