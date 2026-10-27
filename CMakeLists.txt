cmake_minimum_required(VERSION 3.16)

project(PluginApp VERSION 1.0 LANGUAGES CXX)

# ── 构建类型（单配置生成器：Makefile / Ninja）─────────────────────────────────
# 默认使用 RelWithDebInfo；多配置生成器（VS）忽略此变量，由 IDE 下拉选择
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

# ── 多配置生成器（VS / Xcode）──────────────────────────────────────────────────
# 只保留三种有意义的配置，避免 VS 多出 MinSizeRel 选项
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo"
        CACHE STRING "Available build configurations" FORCE)
endif()

# ── 测试框架 ───────────────────────────────────────────────────────────────────
option(BUILD_TESTING "Build unit/integration tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    include(CTest)
endif()

# ── 全局 C++ 标准 ─────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Qt 自动化工具 ─────────────────────────────────────────────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

# ── 查找 Qt（优先 Qt6，回退 Qt5 5.15）──────────────────────────────────────────
find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Using Qt6")
else()
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Widgets)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Using Qt5")
endif()

# ── MSVC 全局选项（作用于本工程全部 target，含 app / plugins / 示例）────────
# /utf-8：源文件与执行字符集均为 UTF-8（spdlog/fmt 等 FMT_UNICODE 静态断言依赖）
# CMAKE_*_FLAGS 同步 /utf-8，保证 VS IntelliSense / 旧缓存亦生效
# /W3：警告级别 3
# /wd*：与 fz2_target_msvc_options 一致；同方案内编译、链接时可安全忽略
if(MSVC)
    add_compile_options(
        /utf-8 /W3
        /wd4251   # C4251：DLL 导出类含 STL 成员（std::string/vector/map 等）未单独导出
        /wd4275   # C4275：DLL 导出类继承未导出的基类（如 std::exception、纯虚接口）
        /wd4267   # C4267：size_t 与 int 等整型隐式转换可能丢失数据（串口长度、容器 size）
        /wd4244   # C4244：浮点/整型窄化转换可能丢失数据（含 3rdparty serial 宽窄字符）
        /wd4996   # C4996：使用了标记为弃用的 CRT/STL API（localtime、codecvt 等，逐步替换期间屏蔽）
        /wd4828   # C4828：源文件含与当前源字符集不一致的字节（厂商头文件非 UTF-8，如海康 MVS）
        /wd5208   # C5208：匿名 struct/union 上使用 typedef 名（CISDK 等厂商头文件）
        /wd4819   # C4819：文件含当前代码页无法表示的字符（建议源文件存 UTF-8；未转码前临时屏蔽）
    )
    string(APPEND CMAKE_CXX_FLAGS " /utf-8")
    string(APPEND CMAKE_C_FLAGS " /utf-8")
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()

# 供 VS Code / clangd 生成 compile_commands.json（需在 build 目录 configure 后生效）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── VS 解决方案：按代码目录分组 ──────────────────────────────────────────────
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ── 子目录（各层 CMakeLists 自行 include 本层模块与选项）────────────────────
message(STATUS "=== Configuring PluginApp ===")

# 1. 公共库（extensionsystem / uibase / mvvm）
add_subdirectory(common)

# 2. 主程序可执行文件
message(STATUS "[app] Configuring...")
add_subdirectory(app)

# 3. 所有插件
# add_subdirectory(plugins)

# 4. 单元测试
# message(STATUS "[tests] Configuring...")
# add_subdirectory(tests)

message(STATUS "=== Configuration complete ===")
