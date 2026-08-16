#pragma once

/**
 * @file  Config.h
 * @brief Aether Platform 配置模块主头文件
 *
 * 基于 toml11 v4（header-only）封装，提供：
 *   - ConfigManager  全局配置管理器（门面/单例），支持：
 *       · load / reload / save          文件加载、重新加载、保存
 *       · get<T> / set<T>               点分路径读写（"server.port"）
 *       · has / remove / keys           存在检测 / 删除 / 枚举
 *       · getArray<T>                   数组读取
 *       · section()                     创建作用域视图
 *       · addChangeListener()           配置变更回调
 *       · setAutoReload()               文件自动热重载（后台线程）
 *       · toTomlString()                序列化为 TOML 文本
 *   - ConfigSection  节点作用域代理，基于前缀委托到 ConfigManager
 *
 * 用法速览：
 * @code
 *   // 1. 加载配置文件
 *   config::ConfigManager::load("config/app.toml");
 *
 *   // 2. 读取配置（带默认值）
 *   auto port = config::ConfigManager::get<int>("server.port", 8080);
 *   auto host = config::ConfigManager::get<std::string>("server.host", "localhost");
 *
 *   // 3. 写入配置
 *   config::ConfigManager::set("server.port", 9090);
 *
 *   // 4. 作用域视图
 *   auto db = config::ConfigManager::section("database");
 *   auto url = db.get<std::string>("url", "");
 *
 *   // 5. 变更监听
 *   config::ConfigManager::addChangeListener("server.*", [](const std::string& key){
 *       std::cout << "配置变更: " << key << "\n";
 *   });
 *
 *   // 6. 自动热重载（每 2 秒检测一次文件修改）
 *   config::ConfigManager::setAutoReload(true, 2000);
 *
 *   // 7. 便捷宏
 *   int timeout = CFG_GET("server.timeout", int, 30);
 * @endcode
 */

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "config_global.h"

// ── toml11 v4（header-only）─────────────────────────────────────────────────
#include <toml.hpp>

namespace config {

// =============================================================================
// ConfigSection — 配置节作用域代理
//
// 通过 ConfigManager::section("database") 获取，对该节点的所有操作
// 自动拼接前缀后委托给 ConfigManager，无需持有底层 toml::value 引用。
// =============================================================================
class CONFIG_EXPORT ConfigSection {
public:
    /**
     * @brief 构造配置节视图
     * @param prefix 节点路径前缀，如 "database"、"server.tls"
     */
    explicit ConfigSection(std::string prefix);

    // ── 读取 ─────────────────────────────────────────────────────────────────

    /**
     * @brief 读取键值，支持默认值（不抛异常）
     * @tparam T  目标类型（int / double / bool / std::string 等）
     * @param key 相对于本节点的点分路径（如 "port"、"tls.cert"）
     * @param def 找不到键或类型转换失败时的返回值
     * @return 转换后的值，或 def
     */
    template<typename T>
    T get(const std::string& key, const T& def = T{}) const noexcept;

    /**
     * @brief 读取数组
     * @tparam T  元素类型
     * @param key 相对键路径
     * @param def 默认数组
     */
    template<typename T>
    std::vector<T> getArray(const std::string& key,
                             const std::vector<T>& def = {}) const noexcept;

    /**
     * @brief 以 std::optional 包装返回，有值时 has_value() 为 true
     */
    template<typename T>
    std::optional<T> find(const std::string& key) const noexcept;

    // ── 写入 ─────────────────────────────────────────────────────────────────

    /**
     * @brief 写入键值
     * @tparam T  值类型（须可转换为 toml::value）
     * @param key 相对键路径
     * @param val 写入的值
     */
    template<typename T>
    void set(const std::string& key, T val);

    // ── 检测 / 删除 / 枚举 ────────────────────────────────────────────────────

    /** @brief 检查键是否存在 */
    bool has(const std::string& key) const noexcept;

    /** @brief 删除键（不存在时静默） */
    bool remove(const std::string& key);

    /** @brief 返回当前节点下所有直接子键名 */
    std::vector<std::string> keys() const;

    // ── 嵌套节 ───────────────────────────────────────────────────────────────

    /** @brief 返回子节点代理：section("tls") 等价于 section("server.tls") */
    ConfigSection section(const std::string& key) const;

    /** @brief 本节点是否有效（prefix 对应的路径存在且为 table） */
    bool valid() const noexcept;

    /** @brief 返回本节点的完整路径前缀 */
    const std::string& prefix() const noexcept { return m_prefix; }

private:
    std::string m_prefix;  ///< 从根节点到此节点的完整点分路径

    /// 拼接完整键路径
    std::string fullKey(const std::string& relKey) const;
};

// =============================================================================
// ConfigManager — 配置管理器（全局门面 / 单例）
//
// 所有方法均为静态方法，线程安全（内部持有 std::mutex）。
// 禁止实例化。
// =============================================================================
class CONFIG_EXPORT ConfigManager {
public:
    // ── 生命周期 ─────────────────────────────────────────────────────────────

    /**
     * @brief 从文件加载 TOML 配置
     * @param filePath 绝对或相对路径（相对于 cwd）
     * @return 成功返回 true；失败打印错误到 stderr 并返回 false
     * @note   重复调用将覆盖之前的内容
     */
    static bool load(const std::string& filePath);

    /**
     * @brief 从 TOML 字符串加载配置（用于测试、嵌入式场景）
     * @param content    TOML 格式字符串
     * @param sourceName 错误信息中显示的来源名称，如 "<embedded>"
     */
    static bool loadFromString(const std::string& content,
                                const std::string& sourceName = "<string>");

    /**
     * @brief 从原始文件路径重新加载（用于热重载）
     * @return 成功返回 true；未曾加载过文件时返回 false
     */
    static bool reload();

    /**
     * @brief 将当前内存配置写回文件
     * @param filePath 目标路径，留空则写回 load() 时的原始路径
     * @return 成功返回 true
     */
    static bool save(const std::string& filePath = "");

    /** @brief 是否已通过 load / loadFromString 加载过配置 */
    static bool isLoaded() noexcept;

    /** @brief 返回 load() 时传入的文件路径（loadFromString 时返回 ""） */
    static const std::string& filePath() noexcept;

    // ── 读取 ─────────────────────────────────────────────────────────────────

    /**
     * @brief 读取配置值（点分路径，不抛异常）
     * @tparam T   目标类型：bool / int / int64_t / double / std::string
     *             及任何可通过 toml::get<T> 转换的类型
     * @param key  点分路径，如 "server.port"、"database.pool_size"
     * @param def  找不到键或类型错误时的默认返回值
     * @return     T 类型的配置值，或 def
     *
     * @code
     *   int port    = ConfigManager::get<int>("server.port", 8080);
     *   bool debug  = ConfigManager::get<bool>("app.debug", false);
     *   auto name   = ConfigManager::get<std::string>("app.name", "Aether");
     * @endcode
     */
    template<typename T>
    static T get(const std::string& key, const T& def = T{}) noexcept;

    /**
     * @brief 以 std::optional<T> 返回，有值时 has_value() == true
     * @code
     *   if (auto port = ConfigManager::find<int>("server.port")) {
     *       startServer(*port);
     *   }
     * @endcode
     */
    template<typename T>
    static std::optional<T> find(const std::string& key) noexcept;

    /**
     * @brief 读取数组（点分路径）
     * @tparam T   元素类型
     * @param key  点分路径，对应 TOML array
     * @param def  默认数组（路径不存在或类型不符时返回）
     *
     * @code
     *   auto ports = ConfigManager::getArray<int>("server.ports");
     *   auto tags  = ConfigManager::getArray<std::string>("app.tags");
     * @endcode
     */
    template<typename T>
    static std::vector<T> getArray(const std::string& key,
                                    const std::vector<T>& def = {}) noexcept;

    // ── 写入 ─────────────────────────────────────────────────────────────────

    /**
     * @brief 写入配置值（自动创建中间节点）
     * @tparam T  值类型，须可构造 toml::value
     * @param key 点分路径
     * @param val 写入的值
     *
     * @code
     *   ConfigManager::set("server.port", 9090);
     *   ConfigManager::set("app.debug", true);
     *   ConfigManager::set("app.name", std::string("MyApp"));
     * @endcode
     */
    template<typename T>
    static void set(const std::string& key, T val);

    // ── 检测 / 删除 / 枚举 ────────────────────────────────────────────────────

    /**
     * @brief 检查键是否存在（点分路径）
     * @param key 点分路径，如 "server.port"
     */
    static bool has(const std::string& key) noexcept;

    /**
     * @brief 删除键（不存在时静默返回 false）
     * @param key 点分路径
     * @return 删除成功返回 true
     */
    static bool remove(const std::string& key);

    /**
     * @brief 枚举键名列表
     * @param prefix 若非空，只返回以该前缀开头的节点的直接子键（不含前缀本身）
     * @return 直接子键名列表（不含路径前缀）
     *
     * @code
     *   // 获取根节点下所有顶级键（如 "app", "server", "database"）
     *   auto topKeys = ConfigManager::keys();
     *
     *   // 获取 "server" 节点下所有直接子键
     *   auto svrKeys = ConfigManager::keys("server");
     * @endcode
     */
    static std::vector<std::string> keys(const std::string& prefix = "");

    // ── 作用域视图 ────────────────────────────────────────────────────────────

    /**
     * @brief 返回某节点的 ConfigSection 代理
     * @param key 点分路径，如 "database"、"server.tls"
     * @return ConfigSection 代理对象（轻量值类型，可随意复制）
     *
     * @code
     *   auto db = ConfigManager::section("database");
     *   auto url  = db.get<std::string>("url");
     *   auto pool = db.get<int>("pool_size", 5);
     * @endcode
     */
    static ConfigSection section(const std::string& key);

    // ── 变更监听 ──────────────────────────────────────────────────────────────

    /**
     * @brief 注册配置变更回调
     *
     * 当通过 set() / remove() / reload() 修改配置时触发。
     * pattern 支持通配符 '*'（仅末尾，如 "server.*"）：
     *   - ""         任何键变更都触发
     *   - "server.*" 仅 "server." 开头的键触发
     *   - "app.debug" 精确匹配
     *
     * @param pattern  键路径匹配模式（空字符串 = 匹配所有）
     * @param callback 回调函数，参数为实际变更的完整键路径
     */
    using ChangeCallback = std::function<void(const std::string& key)>;
    static void addChangeListener(const std::string& pattern, ChangeCallback callback);

    /** @brief 清除所有已注册的变更监听器 */
    static void clearChangeListeners();

    // ── 自动热重载 ────────────────────────────────────────────────────────────

    /**
     * @brief 开启/关闭文件自动热重载
     *
     * 开启后启动后台线程，每隔 intervalMs 毫秒检测文件修改时间，
     * 有变化时自动调用 reload() 并触发所有已注册的变更回调（key=""）。
     *
     * @param enabled     true = 开启，false = 停止后台线程
     * @param intervalMs  检测间隔（毫秒，最小 100）
     * @note  程序退出前请调用 setAutoReload(false) 以正常停止线程
     */
    static void setAutoReload(bool enabled, int intervalMs = 2000);

    // ── 序列化 ────────────────────────────────────────────────────────────────

    /**
     * @brief 将当前内存配置序列化为 TOML 格式字符串
     * @return TOML 文本字符串；未加载时返回空字符串
     */
    static std::string toTomlString();

    // ── 内部接口（供模板实现调用，不建议直接使用） ────────────────────────────

    /**
     * @brief 按点分路径查找节点（读锁保护）
     * @return 节点指针，不存在时返回 nullptr
     */
    static const toml::value* findNode(const std::string& key) noexcept;

    /**
     * @brief 按点分路径确保节点存在并返回引用（写锁保护，自动创建中间节点）
     */
    static toml::value& ensureNode(const std::string& key);

    /** @brief 触发变更回调 */
    static void notifyChange(const std::string& key);

    ConfigManager()                              = delete;
    ~ConfigManager()                             = delete;
    ConfigManager(const ConfigManager&)          = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
};

// =============================================================================
// ConfigSection — 模板实现（委托 ConfigManager）
// =============================================================================

inline ConfigSection::ConfigSection(std::string prefix)
    : m_prefix(std::move(prefix)) {}

inline std::string ConfigSection::fullKey(const std::string& relKey) const {
    return m_prefix.empty() ? relKey : m_prefix + '.' + relKey;
}

template<typename T>
T ConfigSection::get(const std::string& key, const T& def) const noexcept {
    return ConfigManager::get<T>(fullKey(key), def);
}

template<typename T>
std::vector<T> ConfigSection::getArray(const std::string& key,
                                        const std::vector<T>& def) const noexcept {
    return ConfigManager::getArray<T>(fullKey(key), def);
}

template<typename T>
std::optional<T> ConfigSection::find(const std::string& key) const noexcept {
    return ConfigManager::find<T>(fullKey(key));
}

template<typename T>
void ConfigSection::set(const std::string& key, T val) {
    ConfigManager::set<T>(fullKey(key), std::move(val));
}

// =============================================================================
// ConfigManager — 模板实现（依赖 toml::get）
// =============================================================================

template<typename T>
T ConfigManager::get(const std::string& key, const T& def) noexcept {
    try {
        const toml::value* node = findNode(key);
        if (!node) return def;
        return toml::get<T>(*node);
    } catch (...) {
        return def;
    }
}

template<typename T>
std::optional<T> ConfigManager::find(const std::string& key) noexcept {
    try {
        const toml::value* node = findNode(key);
        if (!node) return std::nullopt;
        return toml::get<T>(*node);
    } catch (...) {
        return std::nullopt;
    }
}

template<typename T>
std::vector<T> ConfigManager::getArray(const std::string& key,
                                        const std::vector<T>& def) noexcept {
    try {
        const toml::value* node = findNode(key);
        if (!node || !node->is_array()) return def;
        return toml::get<std::vector<T>>(*node);
    } catch (...) {
        return def;
    }
}

template<typename T>
void ConfigManager::set(const std::string& key, T val) {
    try {
        toml::value& node = ensureNode(key);
        node = toml::value(std::move(val));
        notifyChange(key);
    } catch (...) {
        // 静默处理
    }
}

}  // namespace config

// =============================================================================
// 便捷宏
//
// 用法：
//   int port = CFG_GET("server.port", int, 8080);
//   CFG_SET("app.debug", true);
//   if (CFG_HAS("database.url")) { ... }
// =============================================================================
#define CFG_LOAD(path)            ::config::ConfigManager::load(path)
#define CFG_RELOAD()              ::config::ConfigManager::reload()
#define CFG_SAVE()                ::config::ConfigManager::save()
#define CFG_GET(key, type, def)   ::config::ConfigManager::get<type>(key, def)
#define CFG_FIND(key, type)       ::config::ConfigManager::find<type>(key)
#define CFG_SET(key, val)         ::config::ConfigManager::set(key, val)
#define CFG_HAS(key)              ::config::ConfigManager::has(key)
#define CFG_REMOVE(key)           ::config::ConfigManager::remove(key)
#define CFG_SECTION(key)          ::config::ConfigManager::section(key)
#define CFG_ARRAY(key, type)      ::config::ConfigManager::getArray<type>(key)
