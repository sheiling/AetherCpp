/**
 * @file  main.cpp
 * @brief config 模块使用示例
 *
 * 演示内容：
 *   示例 1  — 从文件加载（CFG_LOAD / ConfigManager::load）
 *   示例 2  — 基本类型读取（get<T> 带默认值）
 *   示例 3  — std::optional<T> 读取（find<T>）
 *   示例 4  — 数组读取（getArray<T>）
 *   示例 5  — 键存在检测 / 枚举（has / keys）
 *   示例 6  — ConfigSection 作用域视图（section）
 *   示例 7  — 运行时写入与删除（set / remove）
 *   示例 8  — 变更监听回调（addChangeListener）
 *   示例 9  — 从字符串加载（loadFromString）
 *   示例 10 — 序列化导出（toTomlString / save）
 *   示例 11 — 便捷宏（CFG_GET / CFG_SET / CFG_HAS / CFG_FIND）
 *   示例 12 — 自动热重载（setAutoReload，演示开启与关闭）
 */

#include <Config.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
namespace cfg = config;

// ─────────────────────────────────────────────────────────────────────────────
// 工具函数：打印分隔线
// 修复：使用有符号比较避免 size_t 无符号下溢
// ─────────────────────────────────────────────────────────────────────────────
static void printSep(const std::string& title) {
    const std::size_t maxPad = 54;
    const std::size_t pad = (title.size() < maxPad) ? (maxPad - title.size()) : 0u;
    std::cout << "\n╔══════════════════════════════════════════════════════╗\n"
              << "║  " << title
              << std::string(pad, ' ')
              << "║\n"
              << "╚══════════════════════════════════════════════════════╝\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 1：从文件加载
// ─────────────────────────────────────────────────────────────────────────────
void example_load_file() {
    printSep("示例 1：从文件加载 TOML 配置");

    const std::string tomlPath = "res/app.toml";

    if (cfg::ConfigManager::load(tomlPath)) {
        std::cout << "  ✓ 配置已加载：" << tomlPath << "\n";
        std::cout << "  ✓ isLoaded() = "
                  << (cfg::ConfigManager::isLoaded() ? "true" : "false") << "\n";
        std::cout << "  ✓ filePath() = " << cfg::ConfigManager::filePath() << "\n";
    } else {
        std::cerr << "  ✗ 加载失败，请确认 res/app.toml 存在\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 2：基本类型读取（get<T> 带默认值）
// ─────────────────────────────────────────────────────────────────────────────
void example_basic_get() {
    printSep("示例 2：基本类型读取 get<T>");

    auto appName = cfg::ConfigManager::get<std::string>("app.name", "Unknown");
    auto version = cfg::ConfigManager::get<std::string>("app.version", "0.0.0");
    std::cout << "  app.name    = " << appName << "\n";
    std::cout << "  app.version = " << version << "\n";

    bool debug = cfg::ConfigManager::get<bool>("app.debug", false);
    std::cout << "  app.debug   = " << std::boolalpha << debug << "\n";

    int64_t maxThreads = cfg::ConfigManager::get<int64_t>("app.max_threads", 4);
    std::cout << "  app.max_threads = " << maxThreads << "\n";

    double pi = cfg::ConfigManager::get<double>("app.pi_value", 3.14);
    std::cout << "  app.pi_value = " << pi << "\n";

    auto host = cfg::ConfigManager::get<std::string>("server.host", "127.0.0.1");
    int64_t port = cfg::ConfigManager::get<int64_t>("server.port", 8080);
    int64_t timeout = cfg::ConfigManager::get<int64_t>("server.timeout", 30);
    std::cout << "  server.host    = " << host << "\n";
    std::cout << "  server.port    = " << port << "\n";
    std::cout << "  server.timeout = " << timeout << " s\n";

    bool tlsEnabled = cfg::ConfigManager::get<bool>("server.tls.enabled", false);
    auto tlsCert    = cfg::ConfigManager::get<std::string>("server.tls.cert", "");
    std::cout << "  server.tls.enabled = " << tlsEnabled << "\n";
    std::cout << "  server.tls.cert    = " << tlsCert << "\n";

    int64_t missing = cfg::ConfigManager::get<int64_t>("non.existent.key", -1);
    std::cout << "  non.existent.key   = " << missing << "  (默认值)\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 3：std::optional<T> 读取（find<T>）
// ─────────────────────────────────────────────────────────────────────────────
void example_optional_find() {
    printSep("示例 3：std::optional<T> 读取 find<T>");

    auto port = cfg::ConfigManager::find<int64_t>("server.port");
    if (port.has_value()) {
        std::cout << "  server.port = " << *port << "  (has_value=true)\n";
    }

    auto missing = cfg::ConfigManager::find<int64_t>("server.missing");
    std::cout << "  server.missing: has_value = "
              << std::boolalpha << missing.has_value()
              << "  (期望 false)\n";

    if (auto dbPort = cfg::ConfigManager::find<int64_t>("database.port")) {
        std::cout << "  database.port = " << *dbPort << "\n";
    } else {
        std::cout << "  database.port 不存在\n";
    }

    auto badType = cfg::ConfigManager::find<int64_t>("app.name");
    std::cout << "  app.name 以 int64_t 读取: has_value = "
              << badType.has_value() << "  (类型不匹配，期望 false)\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 4：数组读取（getArray<T>）
// ─────────────────────────────────────────────────────────────────────────────
void example_array_get() {
    printSep("示例 4：数组读取 getArray<T>");

    auto tags = cfg::ConfigManager::getArray<std::string>("app.tags");
    std::cout << "  app.tags (" << tags.size() << " 个)：";
    for (const auto& t : tags) std::cout << "\"" << t << "\" ";
    std::cout << "\n";

    auto allowedPorts = cfg::ConfigManager::getArray<int64_t>("app.allowed_ports");
    std::cout << "  app.allowed_ports (" << allowedPorts.size() << " 个)：";
    for (auto p : allowedPorts) std::cout << p << " ";
    std::cout << "\n";

    auto origins = cfg::ConfigManager::getArray<std::string>("server.cors.origins");
    std::cout << "  server.cors.origins (" << origins.size() << " 个)：\n";
    for (const auto& o : origins) std::cout << "    - " << o << "\n";

    auto empty = cfg::ConfigManager::getArray<std::string>("nonexistent.array");
    std::cout << "  nonexistent.array size = " << empty.size()
              << "  (默认空数组)\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 5：键存在检测 / 枚举
// ─────────────────────────────────────────────────────────────────────────────
void example_has_and_keys() {
    printSep("示例 5：键存在检测 has() / 枚举 keys()");

    std::cout << "  has(\"server.port\")       = "
              << std::boolalpha << cfg::ConfigManager::has("server.port") << "\n";
    std::cout << "  has(\"server.tls\")        = "
              << cfg::ConfigManager::has("server.tls") << "\n";
    std::cout << "  has(\"server.tls.cert\")   = "
              << cfg::ConfigManager::has("server.tls.cert") << "\n";
    std::cout << "  has(\"missing.key\")       = "
              << cfg::ConfigManager::has("missing.key") << "\n";

    auto rootKeys = cfg::ConfigManager::keys();
    std::cout << "\n  根节点顶级键 (" << rootKeys.size() << " 个)：";
    for (const auto& k : rootKeys) std::cout << k << " ";
    std::cout << "\n";

    auto serverKeys = cfg::ConfigManager::keys("server");
    std::cout << "  server.* 子键 (" << serverKeys.size() << " 个)：";
    for (const auto& k : serverKeys) std::cout << k << " ";
    std::cout << "\n";

    auto tlsKeys = cfg::ConfigManager::keys("server.tls");
    std::cout << "  server.tls.* 子键 (" << tlsKeys.size() << " 个)：";
    for (const auto& k : tlsKeys) std::cout << k << " ";
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 6：ConfigSection 作用域视图
// ─────────────────────────────────────────────────────────────────────────────
void example_section() {
    printSep("示例 6：ConfigSection 作用域视图");

    auto db = cfg::ConfigManager::section("database");
    std::cout << "  [database section]\n";
    std::cout << "  prefix:     " << db.prefix() << "\n";
    std::cout << "  valid():    " << std::boolalpha << db.valid() << "\n";
    std::cout << "  host:       " << db.get<std::string>("host", "") << "\n";
    std::cout << "  port:       " << db.get<int64_t>("port", 5432) << "\n";
    std::cout << "  pool_size:  " << db.get<int64_t>("pool_size", 5) << "\n";
    std::cout << "  log_queries:" << db.get<bool>("log_queries", false) << "\n";

    auto dbKeys = db.keys();
    std::cout << "  所有子键 (" << dbKeys.size() << " 个)：";
    for (const auto& k : dbKeys) std::cout << k << " ";
    std::cout << "\n";

    auto tls = cfg::ConfigManager::section("server.tls");
    std::cout << "\n  [server.tls section]\n";
    std::cout << "  enabled: " << tls.get<bool>("enabled", false) << "\n";
    std::cout << "  min_ver: " << tls.get<std::string>("min_ver", "TLSv1.3") << "\n";

    auto server = cfg::ConfigManager::section("server");
    auto cors   = server.section("cors");
    std::cout << "\n  [server.cors via section.section]\n";
    std::cout << "  enabled: " << cors.get<bool>("enabled", false) << "\n";

    if (auto timeout = server.find<int64_t>("timeout")) {
        std::cout << "  server.timeout (via section.find) = " << *timeout << "\n";
    }

    auto methods = cors.getArray<std::string>("methods");
    std::cout << "  cors.methods (" << methods.size() << " 个)：";
    for (const auto& m : methods) std::cout << m << " ";
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 7：运行时写入与删除
// ─────────────────────────────────────────────────────────────────────────────
void example_set_and_remove() {
    printSep("示例 7：运行时写入 set() 与删除 remove()");

    std::cout << "  修改前 server.port = "
              << cfg::ConfigManager::get<int64_t>("server.port", 0) << "\n";
    cfg::ConfigManager::set("server.port", static_cast<int64_t>(9090));
    std::cout << "  修改后 server.port = "
              << cfg::ConfigManager::get<int64_t>("server.port", 0) << "\n";

    cfg::ConfigManager::set("cache.enabled", true);
    cfg::ConfigManager::set("cache.ttl",     static_cast<int64_t>(600));
    cfg::ConfigManager::set("cache.backend", std::string("redis"));
    std::cout << "\n  新增 cache 节点：\n";
    std::cout << "  cache.enabled = "
              << std::boolalpha << cfg::ConfigManager::get<bool>("cache.enabled", false) << "\n";
    std::cout << "  cache.ttl     = "
              << cfg::ConfigManager::get<int64_t>("cache.ttl", 0) << "\n";
    std::cout << "  cache.backend = "
              << cfg::ConfigManager::get<std::string>("cache.backend", "") << "\n";

    auto logging = cfg::ConfigManager::section("logging");
    logging.set("level", std::string("debug"));
    std::cout << "\n  通过 section 修改 logging.level = "
              << cfg::ConfigManager::get<std::string>("logging.level", "") << "\n";

    bool removed = cfg::ConfigManager::remove("cache.backend");
    std::cout << "\n  remove(\"cache.backend\") = " << removed << "\n";
    std::cout << "  has(\"cache.backend\")   = "
              << cfg::ConfigManager::has("cache.backend") << "\n";
    std::cout << "  has(\"cache.enabled\")   = "
              << cfg::ConfigManager::has("cache.enabled") << "\n";

    cfg::ConfigManager::set("server.port", static_cast<int64_t>(8080));
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 8：变更监听回调
// ─────────────────────────────────────────────────────────────────────────────
void example_change_listener() {
    printSep("示例 8：变更监听回调 addChangeListener");

    cfg::ConfigManager::clearChangeListeners();

    cfg::ConfigManager::addChangeListener("", [](const std::string& key) {
        std::cout << "  [监听器 ALL]    变更键: \"" << key << "\"\n";
    });

    cfg::ConfigManager::addChangeListener("server.*", [](const std::string& key) {
        std::cout << "  [监听器 server.*] 变更键: \"" << key << "\"\n";
    });

    cfg::ConfigManager::addChangeListener("database.pool_size",
        [](const std::string& key) {
            std::cout << "  [监听器 db.pool_size] 变更键: \"" << key << "\"\n";
        });

    std::cout << "\n  >> 触发 set(\"server.port\", 7070):\n";
    cfg::ConfigManager::set("server.port", static_cast<int64_t>(7070));

    std::cout << "\n  >> 触发 set(\"database.pool_size\", 20):\n";
    cfg::ConfigManager::set("database.pool_size", static_cast<int64_t>(20));

    std::cout << "\n  >> 触发 set(\"app.debug\", true):\n";
    cfg::ConfigManager::set("app.debug", true);

    std::cout << "\n  >> 触发 remove(\"app.debug\"):\n";
    cfg::ConfigManager::remove("app.debug");

    cfg::ConfigManager::clearChangeListeners();
    std::cout << "\n  已清除所有监听器\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 9：从字符串加载（loadFromString）
// ─────────────────────────────────────────────────────────────────────────────
void example_load_from_string() {
    printSep("示例 9：从 TOML 字符串加载 loadFromString");

    const std::string inlineToml = R"(
[service]
name = "ESSCore"
port = 5001
enabled = true

[service.limits]
max_connections = 1000
rate_per_second = 500.0

[service.tags]
list = ["core", "embedded"]
)";

    bool ok = cfg::ConfigManager::loadFromString(inlineToml, "<inline-config>");
    std::cout << "  loadFromString() = " << std::boolalpha << ok << "\n";

    std::cout << "  service.name          = "
              << cfg::ConfigManager::get<std::string>("service.name", "") << "\n";
    std::cout << "  service.port          = "
              << cfg::ConfigManager::get<int64_t>("service.port", 0) << "\n";
    std::cout << "  service.enabled       = "
              << cfg::ConfigManager::get<bool>("service.enabled", false) << "\n";
    std::cout << "  service.limits.max_connections = "
              << cfg::ConfigManager::get<int64_t>("service.limits.max_connections", 0) << "\n";
    std::cout << "  service.limits.rate_per_second = "
              << cfg::ConfigManager::get<double>("service.limits.rate_per_second", 0.0) << "\n";

    auto tags = cfg::ConfigManager::getArray<std::string>("service.tags.list");
    std::cout << "  service.tags.list (" << tags.size() << " 个)：";
    for (const auto& t : tags) std::cout << "\"" << t << "\" ";
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 10：序列化导出（toTomlString / save）
// ─────────────────────────────────────────────────────────────────────────────
void example_serialize() {
    printSep("示例 10：序列化导出 toTomlString / save");

    cfg::ConfigManager::load("res/app.toml");

    cfg::ConfigManager::set("app.version", std::string("2.1.0"));
    cfg::ConfigManager::set("server.port", static_cast<int64_t>(8888));

    const std::string tomlStr = cfg::ConfigManager::toTomlString();
    std::cout << "  toTomlString() 总字符数：" << tomlStr.size() << "\n";

    const std::string tmpPath = "res/app_modified.toml";
    if (cfg::ConfigManager::save(tmpPath)) {
        std::cout << "  ✓ 已保存到: " << tmpPath << "\n";
    }

    cfg::ConfigManager::load("res/app.toml");
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 11：便捷宏
// ─────────────────────────────────────────────────────────────────────────────
void example_macros() {
    printSep("示例 11：便捷宏（CFG_GET/CFG_SET/CFG_HAS/CFG_FIND）");

    std::string name = CFG_GET("app.name", std::string, "Unknown");
    int64_t     port = CFG_GET("server.port", int64_t, 8080);
    bool        tls  = CFG_GET("server.tls.enabled", bool, false);
    std::cout << "  CFG_GET(\"app.name\")            = " << name << "\n";
    std::cout << "  CFG_GET(\"server.port\")         = " << port << "\n";
    std::cout << "  CFG_GET(\"server.tls.enabled\")  = " << std::boolalpha << tls << "\n";

    std::cout << "  CFG_HAS(\"database.host\")       = " << CFG_HAS("database.host") << "\n";
    std::cout << "  CFG_HAS(\"no.such.key\")         = " << CFG_HAS("no.such.key") << "\n";

    CFG_SET("app.build", std::string("v2.0.0-rc1"));
    std::cout << "  CFG_SET(\"app.build\", v2.0.0-rc1) -> "
              << CFG_GET("app.build", std::string, "") << "\n";

    auto dbPort = CFG_FIND("database.port", int64_t);
    if (dbPort) {
        std::cout << "  CFG_FIND(\"database.port\")      = " << *dbPort << "\n";
    }

    auto svr = CFG_SECTION("server");
    std::cout << "  CFG_SECTION(\"server\").get(host) = "
              << svr.get<std::string>("host", "") << "\n";

    auto tags = CFG_ARRAY("app.tags", std::string);
    std::cout << "  CFG_ARRAY(\"app.tags\") size = " << tags.size() << "\n";

    CFG_SET("tmp.key", std::string("temp"));
    std::cout << "  CFG_REMOVE(\"tmp.key\") = " << std::boolalpha << CFG_REMOVE("tmp.key") << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// 示例 12：自动热重载演示
// ─────────────────────────────────────────────────────────────────────────────
void example_auto_reload() {
    printSep("示例 12：自动热重载 setAutoReload");

    const std::string watchFile = "res/watch_test.toml";
    {
        std::ifstream src("res/app.toml", std::ios::binary);
        std::ofstream dst(watchFile, std::ios::binary | std::ios::trunc);
        if (src && dst) dst << src.rdbuf();
    }

    cfg::ConfigManager::load(watchFile);
    cfg::ConfigManager::clearChangeListeners();

    std::atomic<int> reloadCount{0};
    cfg::ConfigManager::addChangeListener("", [&](const std::string&) {
        ++reloadCount;
        std::cout << "  [热重载] 检测到文件变化，已重新加载配置！"
                  << " version=" << cfg::ConfigManager::get<std::string>("app.version", "?")
                  << "\n";
    });

    cfg::ConfigManager::setAutoReload(true, 500);
    std::cout << "  ✓ 后台文件监控线程已启动（间隔 500ms）\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    {
        std::ofstream f(watchFile, std::ios::app);
        if (f) f << "\n# hot-reload-test\n[_reload_test]\ntrigger = true\n";
    }
    std::cout << "  >> 已修改文件 " << watchFile << "\n";

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (reloadCount == 0 && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (reloadCount > 0) {
        std::cout << "  ✓ 热重载触发次数: " << reloadCount.load() << "\n";
    } else {
        std::cout << "  ✗ 热重载在 3s 内未触发（可能受文件系统时间精度影响）\n";
    }

    cfg::ConfigManager::setAutoReload(false);
    std::cout << "  ✓ 后台监控线程已停止\n";

    cfg::ConfigManager::clearChangeListeners();

    try { fs::remove(watchFile); } catch (...) {}

    cfg::ConfigManager::load("res/app.toml");
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "╔══════════════════════════════════════════════════════╗\n"
              << "║     config 模块使用示例（基于 toml11 v4）              ║\n"
              << "╚══════════════════════════════════════════════════════╝\n";

    fs::create_directories("res");

    example_load_file();
    example_basic_get();
    example_optional_find();
    example_array_get();
    example_has_and_keys();
    example_section();
    example_set_and_remove();
    example_change_listener();
    example_load_from_string();
    example_serialize();
    example_macros();
    example_auto_reload();

    std::cout << "\n╔══════════════════════════════════════════════════════╗\n"
              << "║  所有示例执行完毕                                      ║\n"
              << "╚══════════════════════════════════════════════════════╝\n";

    return 0;
}
