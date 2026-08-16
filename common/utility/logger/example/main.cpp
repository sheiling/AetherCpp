/**
 * @file  main.cpp
 * @brief Logger 模块使用示例
 *
 * 演示内容：
 *   1.  零配置快速启动（默认 Both 输出）
 *   2.  仅控制台输出（OutputTarget::Console）
 *   3.  仅文件输出（OutputTarget::File）
 *   4.  运行时动态切换输出目标（setOutputTarget）
 *   5.  混合轮转（Both + 按日期+大小）
 *   6.  自定义 Pattern 格式
 *   7.  运行时修改日志级别
 *   8.  直接使用底层 spdlog::logger 接口
 *   9.  全量 Pattern 占位符演示（所有 spdlog 格式符对照）
 *   10. fmt 全量格式化演示（日志消息体中的 {} 格式规范完整速查）
 *       §1  基本类型（bool/char/int/float/string/pointer）
 *       §2  整数进制（d/b/o/x/X，带/不带 # 前缀）
 *       §3  整数符号（+ / 空格 / 默认）
 *       §4  宽度、对齐、填充字符（< > ^ 0 自定义填充）
 *       §5  浮点精度类型（f/e/E/g/G/a/A/百分比%）
 *       §6  浮点宽度+精度+符号组合
 *       §7  浮点特殊值（inf/-inf/nan）
 *       §8  字符串宽度、对齐、截断
 *       §9  调试格式 {:?}（字符串/字符自动转义）
 *       §10 参数索引与重用（{0} {1} 位置/重复）
 *       §11 花括号转义（{{ }}）
 *       §12 容器/范围格式化（vector/array/set/map/pair/tuple，需 ranges.h）
 *       §13 自定义类型格式化（fmt::formatter<T> 特化）
 *       §14 std::optional / std::variant（需 spdlog/fmt/std.h）
 *       §15 chrono 时间格式化（duration/time_point，需 spdlog/fmt/chrono.h）
 *       §16 二进制 HEX 转储（spdlog::to_hex，需 spdlog/fmt/bin_to_hex.h）
 */

// ── Logger 门面（已内置 spdlog/spdlog.h 及基础 fmt）─────────────────────────
#include <Logger.h>

// ── fmt 扩展头（须在 Logger.h 之后包含）───────────────────────────────────────
#include <spdlog/fmt/bin_to_hex.h> // spdlog::to_hex() 二进制 HEX 转储
#include <spdlog/fmt/chrono.h>     // std::chrono duration/time_point 格式化
#include <spdlog/fmt/ranges.h>     // std::vector/array/set/map/pair/tuple 格式化
#include <spdlog/fmt/std.h>        // std::optional / std::variant / thread::id 格式化

// ── 标准库 ────────────────────────────────────────────────────────────────────
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <thread>
#include <tuple>
#include <variant>
#include <vector>

// ── 命名空间别名，简化书写 ────────────────────────────────────────────────────
namespace elog = logger;

// =============================================================================
// 自定义类型 Point3D：用于示例 10 §13 演示 fmt::formatter<T> 特化
// =============================================================================
struct Point3D {
    double x, y, z;
};

/// fmt::formatter<Point3D> 特化
///   {}   → Point3D(1.000, 2.500, -3.140)   （默认 fixed 小数点后3位）
///   {:e} → Point3D(1.000000e+00, ...)        （科学计数法）
template <>
struct fmt::formatter<Point3D> {
    char pres = 'f';  // 'f'=fixed（默认），'e'=scientific

    constexpr auto parse(fmt::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && (*it == 'e' || *it == 'f')) pres = *it++;
        return it;
    }

    auto format(const Point3D& p, fmt::format_context& ctx) const {
        return (pres == 'e')
            ? fmt::format_to(ctx.out(), "Point3D({:e}, {:e}, {:e})", p.x, p.y, p.z)
            : fmt::format_to(ctx.out(), "Point3D({:.3f}, {:.3f}, {:.3f})", p.x, p.y, p.z);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// 示例 1：零配置快速启动
// ─────────────────────────────────────────────────────────────────────────────
void example_quick_start() {
    std::cout << "\n========== 示例 1：零配置快速启动 ==========\n";

    // 一行初始化（所有默认值）：
    //   appName = "app"，logDir = "logs"
    //   level = Info，target = Both，rotateStrategy = Both
    elog::Logger::init();

    LOG_INFO("Aether Platform 启动成功");
    LOG_WARN("磁盘剩余空间不足 {}%", 10);
    LOG_ERROR("配置文件加载失败: {}", "config.json not found");

    elog::Logger::shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 2：仅控制台输出
// ─────────────────────────────────────────────────────────────────────────────
void example_console_only() {
    std::cout << "\n========== 示例 2：仅控制台输出 ==========\n";

    elog::LogConfig cfg;
    cfg.appName      = "console-demo";
    cfg.level        = elog::LogLevel::Trace;
    cfg.target       = elog::OutputTarget::Console;  // 仅控制台，不写文件
    cfg.enableColor  = true;

    elog::Logger::init(cfg);

    LOG_TRACE("[TRACE]    最详细调试跟踪");
    LOG_DEBUG("[DEBUG]    调试信息");
    LOG_INFO("[INFO]     一般运行信息");
    LOG_WARN("[WARN]     黄色警告");
    LOG_ERROR("[ERROR]    红色错误");
    LOG_CRITICAL("[CRITICAL] 高亮严重错误");

    elog::Logger::shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 3：仅文件输出
// ─────────────────────────────────────────────────────────────────────────────
void example_file_only() {
    std::cout << "\n========== 示例 3：仅文件输出 ==========\n";

    elog::LogConfig cfg;
    cfg.appName        = "ess-file-only";
    cfg.logDir         = "logs/file_only_example";
    cfg.level          = elog::LogLevel::Debug;
    cfg.target         = elog::OutputTarget::File;    // 仅写文件，不输出控制台
    cfg.rotateStrategy = elog::RotateStrategy::Both;

    elog::Logger::init(cfg);

    LOG_DEBUG("此消息只写入文件，控制台不可见");
    LOG_INFO("服务启动完成，监听端口 {}", 8080);
    LOG_WARN("内存使用率 {}%，接近阈值", 85);
    LOG_ERROR("数据库连接失败，重试中...");

    std::cout << "  (以上日志已写入文件，控制台不显示)\n";
    std::cout << "  文件位置: logs/file_only_example/YYYY/MM/ess-file-only_YYYY-MM-DD.log\n";

    elog::Logger::shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 4：运行时动态切换输出目标
// ─────────────────────────────────────────────────────────────────────────────
void example_dynamic_output_target() {
    std::cout << "\n========== 示例 4：运行时切换输出目标 ==========\n";

    elog::LogConfig cfg;
    cfg.appName        = "ess-dynamic";
    cfg.logDir         = "logs/dynamic_example";
    cfg.level          = elog::LogLevel::Debug;
    cfg.target         = elog::OutputTarget::Both;   // 初始：控制台 + 文件
    cfg.rotateStrategy = elog::RotateStrategy::Both;

    elog::Logger::init(cfg);

    LOG_INFO("[Both] 同时输出到控制台和文件");
    LOG_DEBUG("[Both] Debug 信息");

    // ── 切换为仅控制台 ────────────────────────────────────────────────────────
    std::cout << "  >> 切换为 OutputTarget::Console（仅控制台）\n";
    elog::Logger::setOutputTarget(elog::OutputTarget::Console);

    LOG_INFO("[Console] 现在只输出到控制台，不写文件");
    LOG_WARN("[Console] 这条 Warn 只在控制台可见");

    // ── 切换为仅文件 ──────────────────────────────────────────────────────────
    std::cout << "  >> 切换为 OutputTarget::File（仅文件）\n";
    elog::Logger::setOutputTarget(elog::OutputTarget::File);

    LOG_INFO("[File] 现在只写文件，控制台不可见");
    LOG_ERROR("[File] 这条 Error 只在文件中存在");

    std::cout << "  (上方的 [File] 日志已静默写入文件)\n";

    // ── 切换回两者同时 ────────────────────────────────────────────────────────
    std::cout << "  >> 切换回 OutputTarget::Both（控制台 + 文件）\n";
    elog::Logger::setOutputTarget(elog::OutputTarget::Both);

    LOG_INFO("[Both] 恢复同时输出到控制台和文件");
    LOG_INFO("[Both] 完整的 OutputTarget 切换演示结束");

    elog::Logger::shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 5：Both 混合轮转（按日期 + 大小）
// ─────────────────────────────────────────────────────────────────────────────
void example_hybrid_rotation() {
    std::cout << "\n========== 示例 5：Both 混合轮转 ==========\n";

    elog::LogConfig cfg;
    cfg.appName        = "ess-gateway";
    cfg.logDir         = "logs/rotation_example";
    cfg.level          = elog::LogLevel::Debug;
    cfg.target         = elog::OutputTarget::Both;
    cfg.rotateStrategy = elog::RotateStrategy::Both;
    cfg.maxFileSize    = 5 * 1024 * 1024;   // 5 MB
    cfg.maxRotatedFiles = 10;

    // 文件路径规则：
    //   logs/rotation_example/2026/03/ess-gateway_2026-03-12.log
    //   超出 5MB 后：ess-gateway_2026-03-12.1.log, .2.log, ...
    //   次日：      ess-gateway_2026-03-13.log（重置序号）

    elog::Logger::init(cfg);

    LOG_DEBUG("连接池初始化，size={}", 10);
    LOG_INFO("Gateway 启动完成，port={}", 8080);
    LOG_WARN("上游响应缓慢，latency={}ms", 2500);
    LOG_ERROR("路由规则加载失败，code={}", -1);
    LOG_CRITICAL("内存溢出，available={}MB", 0);

    elog::Logger::shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 6：自定义 Pattern 格式
// ─────────────────────────────────────────────────────────────────────────────
void example_custom_pattern() {
    std::cout << "\n========== 示例 6：自定义 Pattern ==========\n";

    elog::LogConfig cfg;
    cfg.appName  = "pattern-demo";
    cfg.target   = elog::OutputTarget::Console;  // 仅控制台方便观察
    cfg.level    = elog::LogLevel::Debug;

    // 自定义格式：简化版（仅时间 + 级别 + 消息）
    cfg.pattern  = "[%H:%M:%S.%e] [%^%5l%$] %v";

    elog::Logger::init(cfg);

    LOG_DEBUG("使用简洁自定义格式，value={:.2f}", 3.14159);
    LOG_INFO("路径：/api/v1/users");
    LOG_WARN("响应超时，url={}", "https://example.com");

    // 运行时切换回带完整源码位置的格式
    elog::Logger::setPattern("[%Y-%m-%d %H:%M:%S.%e] [%^%-8l%$] [tid:%-5t] [%s:%# %!()] %v");
    LOG_INFO("切换格式后，可以看到源文件和行号了");
    LOG_ERROR("错误消息，code={}", -1);

    elog::Logger::shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 7：运行时动态修改日志级别
// ─────────────────────────────────────────────────────────────────────────────
void example_dynamic_level() {
    std::cout << "\n========== 示例 7：运行时动态修改级别 ==========\n";

    elog::LogConfig cfg;
    cfg.appName = "dynamic-level";
    cfg.target  = elog::OutputTarget::Console;
    cfg.level   = elog::LogLevel::Warn;   // 初始：只输出 Warn 以上

    elog::Logger::init(cfg);

    LOG_DEBUG("不会输出（当前级别 Warn）");
    LOG_INFO("不会输出（当前级别 Warn）");
    LOG_WARN("这条会输出 ✓");

    std::cout << "  >> 动态调整为 Debug\n";
    elog::Logger::setLevel(elog::LogLevel::Debug);

    LOG_DEBUG("Debug 消息现在可以看到了 ✓");
    LOG_INFO("Info 消息也可以看到了 ✓");

    std::cout << "  >> 动态调整为 Error\n";
    elog::Logger::setLevel(elog::LogLevel::Error);

    LOG_WARN("Warn 消息又看不见了");
    LOG_ERROR("Error 依然可见 ✓");

    elog::Logger::shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 8：直接使用底层 spdlog::logger 接口
// ─────────────────────────────────────────────────────────────────────────────
void example_raw_spdlog() {
    std::cout << "\n========== 示例 8：直接使用 spdlog::logger 接口 ==========\n";

    elog::LogConfig cfg;
    cfg.appName = "raw-spdlog";
    cfg.target  = elog::OutputTarget::Console;
    cfg.level   = elog::LogLevel::Debug;

    elog::Logger::init(cfg);

    // 通过 Logger::get() 获取底层 spdlog::logger
    auto log = elog::Logger::get();
    if (log) {
        // 直接调用（不含源码位置，%s %# %! 显示为空）
        log->info("直接 spdlog::logger::info()（无源码位置）");
        log->debug("fmtlib 格式: {} + {} = {}", 1, 2, 3);
        log->flush();
    }

    // 通过 spdlog 全局注册表按名称获取
    if (auto same = spdlog::get("raw-spdlog")) {
        same->info("通过 spdlog::get('raw-spdlog') 获取同一 logger");
    }

    elog::Logger::shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 9：全量 Pattern 占位符演示
//
// ┌─────────────────────────────────────────────────────────────────────────┐
// │              spdlog 全量内置格式占位符速查表（100% 覆盖）                │
// ├──────────┬──────────────────────────────────────────────────────────────┤
// │ 占位符   │ 说明与示例输出                                               │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 年份     │                                                              │
// │  %Y      │ 4 位年份                     → 2026                         │
// │  %C      │ 2 位年份（世纪后两位）        → 26                           │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 月份     │                                                              │
// │  %m      │ 月份数字（01-12）             → 03                           │
// │  %b/%h   │ 月份英文缩写                  → Mar                          │
// │  %B      │ 月份英文全称                  → March                        │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 日/星期  │                                                              │
// │  %d      │ 日期（01-31）                 → 12                           │
// │  %a      │ 星期英文缩写                  → Thu                          │
// │  %A      │ 星期英文全称                  → Thursday                     │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 小时     │                                                              │
// │  %H      │ 24 小时制（00-23）            → 15                           │
// │  %I      │ 12 小时制（01-12）            → 03                           │
// │  %p      │ AM / PM                       → PM                           │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 分/秒    │                                                              │
// │  %M      │ 分钟（00-59）                 → 08                           │
// │  %S      │ 秒（00-59）                   → 33                           │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 亚秒精度 │                                                              │
// │  %e      │ 毫秒（000-999）               → 123                          │
// │  %f      │ 微秒（000000-999999）         → 123456                       │
// │  %F      │ 纳秒（000000000-999999999）   → 123456789                    │
// │  %E      │ 自 Unix epoch 起的秒数        → 1741782513                   │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 复合时间 │                                                              │
// │  %T/%X   │ ISO 8601 时间 HH:MM:SS        → 15:08:33                     │
// │  %R      │ 24 小时 HH:MM                 → 15:08                        │
// │  %r      │ 12 小时含 AM/PM               → 03:08:33 PM                  │
// │  %D/%x   │ 美式日期 MM/DD/YY             → 03/12/26                     │
// │  %c      │ 完整本地日期时间              → Thu Mar 12 15:08:33 2026     │
// │  %z      │ UTC 时区偏移                  → +0800                        │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 级别     │                                                              │
// │  %l      │ 级别全名小写                  → info                         │
// │  %L      │ 级别首字母大写                → I                            │
// │  %-Nl    │ 左对齐，宽 N（如 %-8l）        → "info    "                  │
// │  %Nl     │ 右对齐，宽 N（如 %8l）         → "    info"                  │
// │  %=Nl    │ 居中对齐，宽 N（如 %=8l）      → "  info  "                  │
// │  %N!l    │ 截断至 N 字符（如 %3!l）       → "inf"                       │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 颜色     │                                                              │
// │  %^      │ 颜色区域开始（级别对应颜色）                                  │
// │  %$      │ 颜色区域结束                                                 │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 进程/线程│                                                              │
// │  %P      │ 进程 ID                       → 12345                        │
// │  %t      │ 线程 ID                       → 67890                        │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ Logger   │                                                              │
// │  %n      │ Logger 名称（LogConfig::appName）→ my-app                    │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 消息体   │                                                              │
// │  %v      │ 日志消息正文                  → Hello World                  │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 源码位置 │ ⚠ 须使用 LOG_* 宏，直接调用 spdlog API 则为空               │
// │  %s      │ 源文件 basename               → main.cpp                     │
// │  %g      │ 源文件完整路径                → /path/to/main.cpp            │
// │  %#      │ 源文件行号                    → 42                           │
// │  %!      │ 函数名                        → example_full_pattern         │
// │  %@      │ 源文件:行号（合并）           → main.cpp:42                  │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 日志间隔 │ 距离上一条日志的时间差（非时钟时间，用于性能分析）            │
// │  %u      │ 间隔纳秒                      → 1234567                      │
// │  %i      │ 间隔微秒                      → 1234                         │
// │  %o      │ 间隔毫秒                      → 50                           │
// │  %O      │ 间隔秒                        → 0                            │
// ├──────────┼──────────────────────────────────────────────────────────────┤
// │ 特殊     │                                                              │
// │  %+      │ spdlog 内置默认完整格式                                      │
// │  %%      │ 输出字面量 %                  → %                            │
// └──────────┴──────────────────────────────────────────────────────────────┘
//
// 对齐修饰语法（可应用于大多数字段）：
//   %-Nx   左对齐，宽度 N（不足补空格）
//   %Nx    右对齐，宽度 N（不足补空格）
//   %=Nx   居中对齐，宽度 N
//   %N!x   截断至 N 字符（超出则截断）
// ─────────────────────────────────────────────────────────────────────────────
void example_full_pattern() {
    std::cout << "\n========== 示例 9：全量 Pattern 占位符演示 ==========\n";

    elog::LogConfig cfg;
    cfg.appName = "pattern-all";
    cfg.target  = elog::OutputTarget::Console;
    cfg.level   = elog::LogLevel::Trace;  // 开启所有级别

    elog::Logger::init(cfg);

    // ─────────────────────────────────────────────────────────────────────
    // §0  spdlog 内置默认格式 %+
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §0  内置默认格式 %+ ---\n";
    elog::Logger::setPattern("%+");
    LOG_INFO("%%+ 等价于 spdlog 内置完整格式（含时间/级别/名称）");

    // ─────────────────────────────────────────────────────────────────────
    // §1  年份占位符：%Y（4位）  %C（2位）
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §1  年份占位符 ---\n";
    elog::Logger::setPattern("%%Y=[%Y]  %%C=[%C]  →  4位年 / 2位年(世纪后两位): %v");
    LOG_INFO("year demo");

    // ─────────────────────────────────────────────────────────────────────
    // §2  月份占位符：%m（数字）  %b/%h（英文缩写）  %B（英文全称）
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §2  月份占位符 ---\n";
    elog::Logger::setPattern("%%m=[%m]  %%b=[%b]  %%h=[%h]  %%B=[%B]  →  数字/缩写/缩写/全称: %v");
    LOG_INFO("month demo");

    // ─────────────────────────────────────────────────────────────────────
    // §3  日/星期占位符：%d  %a（缩写）  %A（全称）  %D/%x（美式日期）
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §3  日/星期占位符 ---\n";
    elog::Logger::setPattern("%%d=[%d]  %%a=[%a]  %%A=[%A]  →  日/星期缩写/星期全称: %v");
    LOG_INFO("day-of-week demo");

    elog::Logger::setPattern("%%D=[%D]  %%x=[%x]  →  美式日期 MM/DD/YY (两者等价): %v");
    LOG_INFO("US date demo");

    // ─────────────────────────────────────────────────────────────────────
    // §4  24小时制：%H  %M  %S
    //     12小时制：%I  %p（AM/PM）  %r（完整12h时间串）
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §4  小时/分钟/秒占位符 ---\n";
    elog::Logger::setPattern("%%H=[%H]  %%M=[%M]  %%S=[%S]  →  24h时:分:秒: %v");
    LOG_INFO("24h time demo");

    elog::Logger::setPattern("%%I=[%I]  %%p=[%p]  %%r=[%r]  →  12h时 / AM-PM / 完整12h: %v");
    LOG_INFO("12h time demo");

    // ─────────────────────────────────────────────────────────────────────
    // §5  亚秒精度：%e（毫秒）  %f（微秒）  %F（纳秒）  %E（epoch秒）
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §5  亚秒精度占位符 ---\n";
    elog::Logger::setPattern("%%e=[%e ms]  %%f=[%f us]  %%F=[%F ns]  %%E=[%E epoch_s]: %v");
    LOG_INFO("sub-second precision demo");

    // ─────────────────────────────────────────────────────────────────────
    // §6  复合时间格式：%T/%X  %R  %c  %z
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §6  复合时间格式占位符 ---\n";
    elog::Logger::setPattern("%%T=[%T]  %%X=[%X]  →  ISO 8601 时间 HH:MM:SS (等价): %v");
    LOG_INFO("ISO time demo");

    elog::Logger::setPattern("%%R=[%R]  →  24小时 HH:MM (无秒): %v");
    LOG_INFO("HH:MM demo");

    elog::Logger::setPattern("%%c=[%c]  →  完整本地日期时间: %v");
    LOG_INFO("full datetime demo");

    elog::Logger::setPattern("%%z=[%z]  →  UTC 时区偏移 (如 +0800): %v");
    LOG_INFO("timezone offset demo");

    // ─────────────────────────────────────────────────────────────────────
    // §7  日志级别：%l  %L
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §7  日志级别占位符（全级别展示）---\n";
    elog::Logger::setPattern("%%l=[%l]  %%L=[%L]  →  级别全名 / 首字母: %v");
    LOG_TRACE   ("trace    level");
    LOG_DEBUG   ("debug    level");
    LOG_INFO    ("info     level");
    LOG_WARN    ("warning  level");
    LOG_ERROR   ("error    level");
    LOG_CRITICAL("critical level");

    // ─────────────────────────────────────────────────────────────────────
    // §8  对齐修饰：左对齐 %-Nl  右对齐 %Nl  居中 %=Nl  截断 %N!l
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §8  对齐修饰符（以 %l 为例）---\n";
    elog::Logger::setPattern("[%-8l] ← 左对齐宽8: %v");
    LOG_DEBUG("%-8l demo");
    LOG_CRITICAL("%-8l demo");

    elog::Logger::setPattern("[%8l] ← 右对齐宽8: %v");
    LOG_DEBUG("%8l demo");
    LOG_CRITICAL("%8l demo");

    elog::Logger::setPattern("[%=8l] ← 居中宽8: %v");
    LOG_DEBUG("%=8l demo");
    LOG_CRITICAL("%=8l demo");

    elog::Logger::setPattern("[%3!l] ← 截断至3字符: %v");
    LOG_DEBUG("%3!l demo  →  deb");
    LOG_CRITICAL("%3!l demo  →  cri");

    // ─────────────────────────────────────────────────────────────────────
    // §9  彩色区域：%^  %$
    //     终端支持 ANSI 时显示颜色；重定向到文件时自动去除控制码
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §9  彩色占位符 %%^ ... %%$ ---\n";
    elog::Logger::setPattern("[%^%-8l%$] %v");
    LOG_TRACE   ("trace    → 白色/默认色");
    LOG_DEBUG   ("debug    → 青色");
    LOG_INFO    ("info     → 绿色");
    LOG_WARN    ("warning  → 黄色/橙色");
    LOG_ERROR   ("error    → 红色");
    LOG_CRITICAL("critical → 高亮红色 / 白底红字");

    // ─────────────────────────────────────────────────────────────────────
    // §10 进程/线程：%P  %t
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §10 进程/线程占位符 ---\n";
    elog::Logger::setPattern("[pid:%-6P] [tid:%-6t]  %%P=进程ID  %%t=线程ID: %v");
    LOG_INFO("process & thread demo");

    // ─────────────────────────────────────────────────────────────────────
    // §11 Logger 名称：%n
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §11 Logger 名称占位符 ---\n";
    elog::Logger::setPattern("[%n]  %%n = LogConfig::appName 的值: %v");
    LOG_INFO("logger name demo  →  pattern-all");

    // ─────────────────────────────────────────────────────────────────────
    // §12 消息体：%v
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §12 消息体占位符 %%v ---\n";
    elog::Logger::setPattern("PREFIX >> %v << SUFFIX");
    LOG_INFO("这段文字就是 %%v 的值");

    // ─────────────────────────────────────────────────────────────────────
    // §13 源码位置（须使用 LOG_* 宏，直接调用 spdlog API 则为空）
    //     %s  源文件 basename
    //     %g  源文件完整路径
    //     %#  行号
    //     %!  函数名
    //     %@  文件:行号（合并）
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §13 源码位置占位符（LOG_* 宏自动注入）---\n";

    elog::Logger::setPattern("%%s=[%s]  →  源文件 basename: %v");
    LOG_INFO("source basename demo");

    elog::Logger::setPattern("%%g=[%g]  →  源文件完整路径: %v");
    LOG_INFO("source fullpath demo");

    elog::Logger::setPattern("%%#=[%#]  →  行号: %v");
    LOG_INFO("line number demo");

    elog::Logger::setPattern("%%!=[%!]  →  函数名: %v");
    LOG_INFO("function name demo");

    elog::Logger::setPattern("%%@=[%@]  →  文件:行号 合并 (等价于 %%s:%%#): %v");
    LOG_INFO("source location combined demo");

    // ─────────────────────────────────────────────────────────────────────
    // §14 日志间隔（距上一条日志的时间差，用于性能分析）
    //     %u  纳秒   %i  微秒   %o  毫秒   %O  秒
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §14 日志间隔占位符（%%u/%%i/%%o/%%O）---\n";
    elog::Logger::setPattern("[+%uns / +%ius / +%oms / +%Os]  %%u=ns %%i=us %%o=ms %%O=s: %v");

    LOG_INFO("第 1 条（作为基准，间隔接近 0）");
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    LOG_INFO("第 2 条（距上条 ~30 ms）");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    LOG_INFO("第 3 条（距上条 ~100 ms）");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    LOG_INFO("第 4 条（距上条 ~200 ms）");

    // ─────────────────────────────────────────────────────────────────────
    // §15 特殊占位符：%% → 字面量 %
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §15 特殊占位符 %%%% ---\n";
    elog::Logger::setPattern("100%% complete  →  %%%%=字面量%%: %v");
    LOG_INFO("percent literal demo");

    // ─────────────────────────────────────────────────────────────────────
    // §16 生产推荐完整格式（综合以上所有关键字段）
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §16 综合：生产推荐格式 ---\n";
    elog::Logger::setPattern(
        "[%Y-%m-%d %H:%M:%S.%e]"   // §1+§4+§5  时间戳（毫秒精度）
        " [%^%-8l%$]"              // §7+§8+§9  彩色级别，左对齐 8 位
        " [pid:%-5P tid:%-5t]"     // §10       进程ID + 线程ID
        " [%n]"                    // §11       Logger 名称
        " [%s:%# %!()]"            // §13       源文件:行号 函数名
        " %v"                      // §12       消息体
    );
    LOG_TRACE   ("TRACE  — 最详细的调试跟踪");
    LOG_DEBUG   ("DEBUG  — 调试信息，port={}", 8080);
    LOG_INFO    ("INFO   — 服务启动成功，version={}", "2.0.1");
    LOG_WARN    ("WARN   — 内存使用率 {}%%，接近阈值", 88);
    LOG_ERROR   ("ERROR  — 连接超时，host={}, code={}", "db.local", -1);
    LOG_CRITICAL("CRITICAL — 核心服务崩溃，立即告警！");

    elog::Logger::shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 10：fmt 全量格式化演示
//
// 演示日志消息体（%v）中 {} 格式规范的完整用法。
// 使用极简 pattern "  | %v" 聚焦 fmt 输出，屏蔽 spdlog 自身字段干扰。
// ─────────────────────────────────────────────────────────────────────────────
void example_fmt_formatting() {
    std::cout << "\n========== 示例 10：fmt 全量格式化演示 ==========\n";

    elog::LogConfig cfg;
    cfg.appName = "fmt-demo";
    cfg.target  = elog::OutputTarget::Console;
    cfg.level   = elog::LogLevel::Trace;
    cfg.pattern = "  | %v";  // 极简前缀，突出 fmt 输出内容
    elog::Logger::init(cfg);

    // ─────────────────────────────────────────────────────────────────────
    // §1  基本类型（Basic Types）
    //     {} 自动选择最合适的默认表示
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §1  基本类型 (bool / char / int / float / string / pointer) ---\n";
    LOG_INFO("bool    true  = {}  /  false = {}",    true,    false);
    LOG_INFO("char    'A'   = {}  /  65(int)= {}",   'A',     static_cast<int>('A'));
    LOG_INFO("int     42    = {}",  42);
    LOG_INFO("long    -1L   = {}",  -1L);
    LOG_INFO("uint    255U  = {}",  255U);
    LOG_INFO("int64   max   = {}",  std::numeric_limits<int64_t>::max());
    LOG_INFO("uint64  max   = {}",  std::numeric_limits<uint64_t>::max());
    LOG_INFO("float   3.14f = {}",  3.14f);
    LOG_INFO("double  pi    = {}",  3.141592653589793);
    LOG_INFO("string        = {}",  std::string("hello world"));
    LOG_INFO("c-string      = {}",  "c-string literal");
    LOG_INFO("string_view   = {}",  std::string_view("sv demo"));
    const int n = 42;
    LOG_INFO("pointer       = {}",  static_cast<const void*>(&n));

    // ─────────────────────────────────────────────────────────────────────
    // §2  整数进制格式（Integer Bases）
    //     d=十进制  b=二进制  o=八进制  x=十六进制小写  X=大写
    //     加 # 前缀：0b… 0… 0x… 0X…
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §2  整数进制：d / b / o / x / X，# 添加进制前缀 ---\n";
    LOG_INFO("255  {{:d}}={:d}  {{:b}}={:b}  {{:o}}={:o}  {{:x}}={:x}  {{:X}}={:X}",
             255, 255, 255, 255, 255);
    LOG_INFO("255  {{:#b}}={:#b}  {{:#o}}={:#o}  {{:#x}}={:#x}  {{:#X}}={:#X}",
             255, 255, 255, 255);
    LOG_INFO("-42  {{:d}}={:d}  {{:x}}={:x}  {{:#010x}}={:#010x}",
             -42, -42, -42);

    // ─────────────────────────────────────────────────────────────────────
    // §3  整数符号控制（Sign Flags）
    //     +  强制显示正负号
    //     ' '(空格)  正数前置空格，负数保持负号（便于列对齐）
    //     - (默认)   仅负数显示负号
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §3  整数符号控制：+ / 空格 / 默认(-) ---\n";
    LOG_INFO("{{:+d}} → {:+d}  {:+d}  (强制显示正负号)",   42, -42);
    LOG_INFO("{{: d}} → {: d}  {: d}  (正数前置空格对齐)",  42, -42);
    LOG_INFO("{{:d}}  → {:d}   {:d}   (默认，仅负号)",      42, -42);

    // ─────────────────────────────────────────────────────────────────────
    // §4  宽度、对齐、填充字符（Width / Align / Fill）
    //     <  左对齐（字符串默认）
    //     >  右对齐（数字默认）
    //     ^  居中对齐
    //     0  零填充（等价于 0> 右对齐）
    //     任意填充字符写在对齐符之前，如 *^10
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §4  宽度/对齐/填充：< > ^ 0 自定义填充字符 ---\n";
    LOG_INFO("[{{:<10d}}]  = [{:<10d}]  左对齐 宽10",   42);
    LOG_INFO("[{{:>10d}}]  = [{:>10d}]  右对齐 宽10（数字默认）", 42);
    LOG_INFO("[{{:^10d}}]  = [{:^10d}]  居中   宽10",   42);
    LOG_INFO("[{{:010d}}]  = [{:010d}]  零填充 宽10",   42);
    LOG_INFO("[{{:*<10d}}] = [{:*<10d}] * 左填充 宽10", 42);
    LOG_INFO("[{{:*>10d}}] = [{:*>10d}] * 右填充 宽10", 42);
    LOG_INFO("[{{:*^10d}}] = [{:*^10d}] * 居中填充宽10", 42);
    LOG_INFO("[{{:-^14s}}] = [{:-^14s}] - 居中填充宽14（字符串）", "hello");

    // ─────────────────────────────────────────────────────────────────────
    // §5  浮点精度类型（Float Presentation Types）
    //     f / F  固定小数点
    //     e / E  科学计数法（小写/大写 e）
    //     g / G  通用格式（自动选 f 或 e，G=大写）
    //     a / A  十六进制浮点
    //     %      百分比（×100 后追加 %）
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §5  浮点精度类型：f / e / E / g / G / a / A / % ---\n";
    constexpr double pi = 3.14159265358979;
    LOG_INFO("pi {{:.6f}}  = {:.6f}   fixed，小数点后6位",    pi);
    LOG_INFO("pi {{:.3e}}  = {:.3e}   科学计数（小写e）",      pi);
    LOG_INFO("pi {{:.3E}}  = {:.3E}   科学计数（大写E）",      pi);
    LOG_INFO("pi {{:.6g}}  = {:.6g}   通用格式（6位有效数字）", pi);
    LOG_INFO("pi {{:.6G}}  = {:.6G}   通用格式（大写）",        pi);
    LOG_INFO("pi {{:.6a}}  = {:.6a}   十六进制浮点（小写）",    pi);
    LOG_INFO("pi {{:.6A}}  = {:.6A}   十六进制浮点（大写）",    pi);
    LOG_INFO("0.1234 {{:.2%}} = {:.2%}  百分比（×100 追加%%）",  0.1234);

    // ─────────────────────────────────────────────────────────────────────
    // §6  浮点宽度 + 精度 + 符号组合
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §6  浮点宽度+精度+符号组合 ---\n";
    LOG_INFO("[{{:12.4f}}]  = [{:12.4f}]  右对齐 宽12 精度4", pi);
    LOG_INFO("[{{:<12.4f}}] = [{:<12.4f}]  左对齐 宽12 精度4", pi);
    LOG_INFO("[{{:^12.4f}}] = [{:^12.4f}]  居中   宽12 精度4", pi);
    LOG_INFO("[{{:012.4f}}] = [{:012.4f}]  零填充 宽12 精度4", pi);
    LOG_INFO("[{{:+.4f}}]   = [{:+.4f}]    强制正号",           pi);
    LOG_INFO("[{{:+.4f}}]   = [{:+.4f}]    负数",              -pi);
    LOG_INFO("[{{: .4f}}]   = [{: .4f}]    正数前置空格",        pi);

    // ─────────────────────────────────────────────────────────────────────
    // §7  浮点特殊值（inf / -inf / nan）
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §7  浮点特殊值：inf / -inf / nan ---\n";
    LOG_INFO("infinity  = {}",   std::numeric_limits<double>::infinity());
    LOG_INFO("-infinity = {}",  -std::numeric_limits<double>::infinity());
    LOG_INFO("quiet_NaN = {}",   std::numeric_limits<double>::quiet_NaN());
    LOG_INFO("{{:e}} inf  = {:e}", std::numeric_limits<double>::infinity());

    // ─────────────────────────────────────────────────────────────────────
    // §8  字符串宽度、对齐、截断
    //     {:Ns}  右对齐宽N    {:<Ns}  左对齐    {:^Ns}  居中
    //     {:.Ns}  截断至N字符（超出部分丢弃）
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §8  字符串宽度/对齐/截断 ---\n";
    LOG_INFO("[{{:<12s}}]  = [{:<12s}]  左对齐 宽12", "left");
    LOG_INFO("[{{:>12s}}]  = [{:>12s}]  右对齐 宽12", "right");
    LOG_INFO("[{{:^12s}}]  = [{:^12s}]  居中   宽12", "center");
    LOG_INFO("[{{:*^14s}}] = [{:*^14s}]  * 居中 宽14", "hello");
    LOG_INFO("[{{:.5s}}]   = [{:.5s}]   截断至5字符", "truncated_string");
    LOG_INFO("[{{:*<12.5s}}] = [{:*<12.5s}]  截断+左对齐填充", "truncated_string");

    // ─────────────────────────────────────────────────────────────────────
    // §9  调试格式 {:?}（Debug / Escaped Format，fmt 9.0+）
    //     字符串加双引号，特殊字符转义（\n \t \\ \"）
    //     字符加单引号，控制字符转义
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §9  调试格式 {:?}：字符串/字符自动转义（fmt 9.0+）---\n";
    LOG_INFO("{:?}  →  普通字符串加引号",                   "hello world");
    LOG_INFO("{:?}  →  含 tab（\\t）转义",                  "col1\tcol2");
    LOG_INFO("{:?}  →  含换行（\\n）转义",                  "line1\nline2");
    LOG_INFO("{:?}  →  含双引号（\\\"）转义",               "say \"hello\"");
    LOG_INFO("{:?}  →  含反斜杠（\\\\）转义",               "path: C:\\dir");
    LOG_INFO("{:?}  →  char 'A' 加单引号",                  'A');
    LOG_INFO("{:?}  →  控制字符 '\\n' 转义",                '\n');
    LOG_INFO("{:?}  →  控制字符 '\\t' 转义",                '\t');

    // ─────────────────────────────────────────────────────────────────────
    // §10 参数索引与重用（Argument Indexing）
    //     {N}    按位置引用第 N 个参数（从 0 开始）
    //     混用位置与自动索引是错误的，但可全部使用位置索引
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §10 参数索引与重用：{0} {1} 位置 / {0}{0} 重用 ---\n";
    LOG_INFO("{0} {1} {2}  →  顺序",          "alpha", "beta", "gamma");
    LOG_INFO("{2} {1} {0}  →  倒序",          "alpha", "beta", "gamma");
    LOG_INFO("{0} {0} {0}  →  重用同一参数",  "echo");
    LOG_INFO("({0},{1}) 与 ({1},{0})  →  交换", "x", "y");
    LOG_INFO("{0:>8s} = {1:.4f}  →  位置+格式规范同用", "pi", pi);

    // ─────────────────────────────────────────────────────────────────────
    // §11 花括号转义（Escape Braces）
    //     {{  →  {     }}  →  }
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §11 花括号转义：{{ → {  /  }} → } ---\n";
    LOG_INFO("{{}}  →  {{}}   字面量 {{}}");
    LOG_INFO("{{ {} }}  →  {{ {} }}  花括号包裹值", 42);
    LOG_INFO("{{0}} 不是占位符，{0} 才是，值={0}", "hello");
    LOG_INFO("100{{%%}}完成  →  100{{%%}}完成");  // %% 在 pattern 中，这里{{ }} 演示

    // ─────────────────────────────────────────────────────────────────────
    // §12 容器与范围格式化（Ranges）
    //     需要 #include <spdlog/fmt/ranges.h>
    //     vector/array → [e1, e2, ...]
    //     set          → {e1, e2, ...}
    //     map          → {(k1, v1), (k2, v2), ...}
    //     pair         → (first, second)
    //     tuple        → (e1, e2, e3)
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §12 容器/范围格式化（#include <spdlog/fmt/ranges.h>）---\n";
    std::vector<int>              vi  = {10, 20, 30, 40, 50};
    std::array<double, 4>         ad  = {1.1, 2.2, 3.3, 4.4};
    std::set<std::string>         ss  = {"banana", "apple", "cherry"};
    std::map<std::string, int>    mi  = {{"one",1},{"two",2},{"three",3}};
    std::pair<int, std::string>   pv  = {42, "hello"};
    std::tuple<int, double, bool> tv  = {7, 3.14, true};
    std::vector<std::pair<int,int>> vp = {{1,2},{3,4},{5,6}};

    LOG_INFO("vector<int>             = {}",  vi);
    LOG_INFO("array<double,4>         = {}",  ad);
    LOG_INFO("set<string>（自动排序） = {}",  ss);
    LOG_INFO("map<string,int>         = {}",  mi);
    LOG_INFO("pair<int,string>        = {}",  pv);
    LOG_INFO("tuple<int,double,bool>  = {}",  tv);
    LOG_INFO("vector<pair<int,int>>   = {}",  vp);

    // ─────────────────────────────────────────────────────────────────────
    // §13 自定义类型格式化（Custom fmt::formatter<T> Specialization）
    //     在文件顶部定义了 Point3D 和对应的 formatter 特化
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §13 自定义类型格式化（fmt::formatter<Point3D> 特化）---\n";
    Point3D pt = {1.0, 2.5, -3.14159};
    LOG_INFO("Point3D {{}}  = {}",   pt);     // 默认 fixed 3位小数
    LOG_INFO("Point3D {{:e}} = {:e}", pt);    // 科学计数法

    // ─────────────────────────────────────────────────────────────────────
    // §14 std::optional / std::variant
    //     需要 #include <spdlog/fmt/std.h>
    //     optional(value) → 值本身，optional(nullopt) → none
    //     variant         → 当前活跃类型的值
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §14 std::optional / std::variant（#include <spdlog/fmt/std.h>）---\n";
    std::optional<int>   opt_val  = 42;
    std::optional<int>   opt_none = std::nullopt;
    std::variant<int, double, std::string> var_i = 99;
    std::variant<int, double, std::string> var_d = 3.14;
    std::variant<int, double, std::string> var_s = std::string("hello");

    LOG_INFO("optional(42)    = {}",  opt_val);
    LOG_INFO("optional(none)  = {}",  opt_none);
    LOG_INFO("variant<int>    = {}",  var_i);
    LOG_INFO("variant<double> = {}",  var_d);
    LOG_INFO("variant<string> = {}",  var_s);

    // ─────────────────────────────────────────────────────────────────────
    // §15 chrono 时间格式化（std::chrono）
    //     需要 #include <spdlog/fmt/chrono.h>
    //     duration  → 自动选择合适单位（如 "1234ms"）
    //     time_point → 可使用 {:%Y-%m-%d %H:%M:%S} 格式字符串
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §15 chrono 时间格式化（#include <spdlog/fmt/chrono.h>）---\n";
    using namespace std::chrono_literals;
    auto dur_ms   = 1234ms;
    auto dur_s    = 75s;
    auto dur_comp = 2h + 30min + 15s + 500ms;

    LOG_INFO("1234ms       = {}",           dur_ms);
    LOG_INFO("75s          = {}",           dur_s);
    LOG_INFO("2h30m15s500ms= {}",           dur_comp);
    LOG_INFO("{{:%H:%M:%S}}  = {:%H:%M:%S}  自定义 duration 格式", dur_comp);

    auto tp = std::chrono::system_clock::now();
    LOG_INFO("time_point   = {}",                    tp);
    LOG_INFO("{{:%Y-%m-%d %H:%M:%S}} = {:%Y-%m-%d %H:%M:%S}", tp);
    LOG_INFO("{{:%F %T}}    = {:%F %T}   ISO 8601",  tp);
    LOG_INFO("{{:%s}}       = {:%s}      Unix epoch 秒数", tp);

    // ─────────────────────────────────────────────────────────────────────
    // §16 二进制 HEX 转储（spdlog::to_hex）
    //     需要 #include <spdlog/fmt/bin_to_hex.h>
    //     格式标志（可组合）：
    //       {:X}  大写十六进制
    //       {:s}  不在字节间插入空格
    //       {:p}  不显示每行起始位置偏移
    //       {:n}  不分行（所有字节输出在一行）
    //       {:a}  {:n} 未设置时在右侧显示 ASCII 字符
    // ─────────────────────────────────────────────────────────────────────
    std::cout << "\n--- §16 二进制 HEX 转储（spdlog::to_hex，格式标志 X/s/p/n/a）---\n";
    // 内容为 "Hello, World!\r\n" + 几个控制字节
    const std::vector<uint8_t> buf = {
        0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20,  // Hello,
        0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0x0d, 0x0a,  // World!\r\n
        0x00, 0x01, 0x7e, 0x7f, 0xff               // 边界字节
    };

    LOG_INFO("{{}}     默认（小写，有空格，有偏移，自动换行）:\n{}",
             spdlog::to_hex(buf));
    LOG_INFO("{{:X}}   大写十六进制:\n{:X}",
             spdlog::to_hex(buf));
    LOG_INFO("{{:Xs}}  大写+无空格分隔:\n{:Xs}",
             spdlog::to_hex(buf));
    LOG_INFO("{{:Xp}}  大写+不显示偏移:\n{:Xp}",
             spdlog::to_hex(buf));
    LOG_INFO("{{:Xn}}  大写+不分行（一行输出）: {:Xn}",
             spdlog::to_hex(buf));
    LOG_INFO("{{:a}}   显示 ASCII 列（右侧可见字符）:\n{:a}",
             spdlog::to_hex(buf));
    LOG_INFO("{{:Xa}}  大写+ASCII:\n{:Xa}",
             spdlog::to_hex(buf));
    // 自定义每行字节数（第二参数）
    LOG_INFO("每行8字节:\n{}",
             spdlog::to_hex(buf.begin(), buf.end(), 8));
    LOG_INFO("每行8字节+ASCII:\n{:a}",
             spdlog::to_hex(buf.begin(), buf.end(), 8));

    elog::Logger::shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║       框架日志模块（Logger）使用示例                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";

    example_quick_start();
    example_console_only();
    example_file_only();
    example_dynamic_output_target();
    example_hybrid_rotation();
    example_custom_pattern();
    example_dynamic_level();
    example_raw_spdlog();
    example_full_pattern();
    example_fmt_formatting();

    std::cout << "\n所有示例执行完毕。\n";
    std::cout << "文件日志已生成至: logs/*/YYYYMM/\n";
    return 0;
}
