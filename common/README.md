# Aether Platform - Common Library

Aether Platform（设备软件系统）平台公共库模块，提供核心框架和基础功能支持。

## 📋 目录

- [项目简介](#项目简介)
- [功能特性](#功能特性)
- [项目结构](#项目结构)
- [依赖要求](#依赖要求)
- [编译构建](#编译构建)
- [使用示例](#使用示例)
- [模块说明](#模块说明)
- [许可证](#许可证)

## 🎯 项目简介

`common` 是 Aether Platform 的核心公共库，基于 Qt 框架构建，提供插件化架构、依赖注入容器、MVVM 框架以及 UI 基础组件等核心功能。该库采用模块化设计，各模块之间依赖关系清晰，便于维护和扩展。

## ✨ 功能特性

### 核心模块

- **ExtensionSystem（插件系统）**：提供完整的插件框架，支持动态加载、生命周期管理和插件间通信
- **Container（IoC 容器）**：Laravel 风格的依赖注入容器，支持服务提供者模式
- **UIBase（UI 基础库）**：提供主题管理、无边框窗口、导航管理、权限管理等 UI 基础设施
- **MVVM（MVVM 框架）**：完整的 MVVM 模式实现，包括 ViewModel、Command、DataBinding 和中间件管道

### 主要特性

- 🔌 **插件化架构**：支持动态插件加载和管理
- 💉 **依赖注入**：基于服务提供者的 IoC 容器
- 🎨 **主题系统**：支持亮色/暗色主题切换
- 🪟 **无边框窗口**：现代化的无边框窗口支持
- 📊 **MVVM 模式**：完整的 MVVM 框架实现
- 🔐 **权限管理**：统一的权限管理系统
- 🌐 **多语言支持**：国际化语言管理
- 🧩 **中间件管道**：支持中间件模式的请求处理

## 📁 项目结构

```
common/
├── 3rdparty/              # 第三方依赖库
├── cmake/                 # CMake 辅助脚本
│   └── FZ2Helpers.cmake
├── core/                  # 核心功能模块
│   ├── communication/     # 通信模块
│   ├── container/         # IoC 容器（Laravel 风格）
│   ├── device/            # 设备模块
│   └── extensionsystem/   # 插件框架核心
├── include/               # 公共头文件
├── plugins/               # 插件目录
├── ui/                    # UI 相关模块
│   ├── component/         # UI 组件
│   ├── mvvm/              # MVVM 框架
│   ├── resources/         # UI 资源文件
│   └── uibase/            # UI 基础库
└── utility/               # 工具类
```

## 🔧 依赖要求

### 必需依赖

- **Qt 5.12+** 或 **Qt 6.x**
  - QtCore
  - QtGui
  - QtWidgets（UI 模块需要）

### 编译工具

- **CMake 3.12+**
- **C++17** 或更高版本的编译器
  - MSVC 2017+（Windows）
  - GCC 7+（Linux）
  - Clang 5+（macOS/Linux）

## 🏗️ 编译构建

### 基本构建步骤

```bash
# 1. 创建构建目录
mkdir build
cd build

# 2. 配置 CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# 3. 编译
cmake --build . --config Release

# 4. 安装（可选）
cmake --install . --config Release
```

### Windows 平台

```powershell
# 使用 Visual Studio
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Linux 平台

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### CMake 选项

- `CMAKE_BUILD_TYPE`: 构建类型（Debug/Release）
- `BUILD_SHARED_LIBS`: 是否构建共享库（默认 ON）
- `CMAKE_INSTALL_PREFIX`: 安装路径

## 💡 使用示例

### 1. 使用 IoC 容器

```cpp
#include "container/application.h"

using namespace Container;

// 创建应用容器
auto app = std::make_unique<Application>();

// 注册服务提供者
app->registerProvider<LogServiceProvider>()
   .registerProvider<DatabaseServiceProvider>();

// 引导应用
app->bootstrap();

// 解析服务
auto logger = app->make<ILogger>();
logger->info("Application started");
```

### 2. 创建插件

```cpp
#include "extensionsystem/iplugin.h"

using namespace ExtensionSystem;

class MyPlugin : public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXTENSIONSYSTEM_IID)

public:
    bool initialize(QString *errorString) override {
        // 初始化逻辑
        return true;
    }

    void extensionsInitialized() override {
        // 扩展初始化
    }
};
```

### 3. 使用 MVVM 框架

```cpp
#include "mvvm/viewmodel.h"

class UserViewModel : public ViewModel
{
    Q_OBJECT

public:
    VIEWMODEL_PROPERTY(QString, userName, UserName)
    VIEWMODEL_PROPERTY(int, age, Age)

    UserViewModel(QObject *parent = nullptr)
        : ViewModel(parent) {}
};
```

### 4. 使用主题管理器

```cpp
#include "uibase/thememanager.h"

// 切换主题
ThemeManager::instance()->setTheme(ThemeManager::Dark);

// 或使用快捷宏
TM->toggleTheme();
```

## 📚 模块说明

### ExtensionSystem（插件系统）

提供完整的插件框架，支持：
- 插件元数据定义（PluginSpec）
- 插件生命周期管理
- 插件依赖解析
- 插件间通信

**核心类：**
- `IPlugin`: 插件接口基类
- `PluginManager`: 插件管理器
- `PluginSpec`: 插件规格定义

### Container（IoC 容器）

Laravel 风格的依赖注入容器，提供：
- 服务绑定和解析
- 服务提供者模式
- 单例和工厂模式支持
- 延迟加载

**核心类：**
- `Container`: 基础容器类
- `Application`: 应用程序级容器
- `ServiceProvider`: 服务提供者基类

### UIBase（UI 基础库）

提供 UI 基础设施：
- `ThemeManager`: 主题管理（亮色/暗色）
- `FramelessWindow`: 无边框窗口
- `NavigationManager`: 导航管理
- `PermService`: 权限运行时（导航/子页/操作 grants）
- `LanguageManager`: 多语言管理

### MVVM（MVVM 框架）

完整的 MVVM 模式实现：
- `ViewModel`: 视图模型基类
- `Command`: 命令模式实现
- `DataBinding`: 数据绑定支持
- `MiddlewarePipeline`: 中间件管道

## 🤝 贡献指南

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 📝 开发规范

- 遵循 Qt 编码规范
- 使用 C++17 标准特性
- 所有公共 API 必须添加文档注释
- 头文件使用 `#pragma once`
- 使用命名空间组织代码

## 📄 许可证

本项目采用 [许可证类型] 许可证。详情请参阅 LICENSE 文件。

## 📞 联系方式

如有问题或建议，请通过以下方式联系：
- Email: [contact@aether.tech]

---

**注意**：本项目是 Aether Platform 的核心组件，建议在了解整体架构后再进行开发。
