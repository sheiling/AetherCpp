/**
 * @file  Config.cpp
 * @brief Aether Platform 配置模块实现
 *
 * 内部实现说明：
 *   - 全局状态通过匿名命名空间中的静态变量存储
 *   - 所有公共静态方法通过 std::shared_mutex 保护（读写分离）
 *   - 点分路径解析：splitKey() 按 '.' 分割
 *   - 热重载：后台线程轮询文件 mtime，有变化则自动 reload()
 *   - 变更通知：支持通配符模式匹配（前缀 + '*'）
 */

#include "Config.h"

// ── 标准库 ────────────────────────────────────────────────────────────────────
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <vector>

namespace config {

// =============================================================================
// 内部全局状态（匿名命名空间）
// =============================================================================
namespace {

// ── 配置存储 ────────────────────────────────────────────────────────────────
toml::value          g_root;              ///< 根节点（type_config::value）
std::string          g_filePath;          ///< 当前加载的文件路径
bool                 g_loaded  = false;   ///< 是否已加载
std::shared_mutex g_mutex;                ///< 读写锁（C++17 shared_mutex）

// ── 变更回调 ─────────────────────────────────────────────────────────────────
struct Listener {
    std::string                        pattern;   ///< 匹配模式（空=全部，末尾*=前缀）
    ConfigManager::ChangeCallback      callback;
};
std::vector<Listener>  g_listeners;
std::mutex             g_listenerMutex;

// ── 文件热重载 ───────────────────────────────────────────────────────────────
std::atomic<bool>       g_watcherRunning{false};
std::thread             g_watcherThread;

// ── 辅助：获取文件最后修改时间（返回 time_t，-1 表示不可读） ─────────────────
inline std::time_t fileModTime(const std::string& path) noexcept {
    struct stat st{};
    if (::stat(path.c_str(), &st) != 0) return static_cast<std::time_t>(-1);
    return st.st_mtime;
}

// ── 辅助：按 '.' 分割点分路径 ─────────────────────────────────────────────────
inline std::vector<std::string> splitKey(const std::string& dotKey) {
    std::vector<std::string> parts;
    if (dotKey.empty()) return parts;
    std::string seg;
    for (char ch : dotKey) {
        if (ch == '.') {
            if (!seg.empty()) { parts.push_back(std::move(seg)); seg.clear(); }
        } else {
            seg += ch;
        }
    }
    if (!seg.empty()) parts.push_back(std::move(seg));
    return parts;
}

// ── 辅助：模式匹配（支持末尾 '*'） ────────────────────────────────────────────
inline bool matchPattern(const std::string& pattern, const std::string& key) {
    if (pattern.empty()) return true;  // 空模式 = 匹配全部
    if (pattern.back() == '*') {
        // 前缀匹配：如 "server.*" 匹配所有以 "server." 开头的键
        const std::string prefix = pattern.substr(0, pattern.size() - 1);
        return key.substr(0, prefix.size()) == prefix;
    }
    return pattern == key;  // 精确匹配
}

}  // anonymous namespace

// =============================================================================
// ConfigSection — 非模板成员实现
// =============================================================================

bool ConfigSection::has(const std::string& key) const noexcept {
    return ConfigManager::has(fullKey(key));
}

bool ConfigSection::remove(const std::string& key) {
    return ConfigManager::remove(fullKey(key));
}

std::vector<std::string> ConfigSection::keys() const {
    return ConfigManager::keys(m_prefix);
}

ConfigSection ConfigSection::section(const std::string& key) const {
    return ConfigSection(fullKey(key));
}

bool ConfigSection::valid() const noexcept {
    if (!ConfigManager::isLoaded()) return false;
    const toml::value* node = ConfigManager::findNode(m_prefix);
    return node != nullptr && node->is_table();
}

// =============================================================================
// ConfigManager — 静态成员实现
// =============================================================================

// ── 加载 ────────────────────────────────────────────────────────────────────

bool ConfigManager::load(const std::string& filePath) {
    try {
        toml::value parsed = toml::parse(filePath);
        {
            std::unique_lock<std::shared_mutex> lk(g_mutex);
            g_root     = std::move(parsed);
            g_filePath = filePath;
            g_loaded   = true;
        }
        notifyChange("");  // 通知所有监听者：全量更新
        return true;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "[config] load(\"%s\") failed: %s\n",
                     filePath.c_str(), e.what());
        return false;
    }
}

bool ConfigManager::loadFromString(const std::string& content,
                                    const std::string& sourceName) {
    try {
        // toml::parse_str 第二参数为 spec，不是来源名称；
        // 使用 std::istringstream 代理并传入来源名称
        std::istringstream iss(content);
        toml::value parsed = toml::parse(iss, sourceName);
        {
            std::unique_lock<std::shared_mutex> lk(g_mutex);
            g_root     = std::move(parsed);
            g_filePath = "";  // 来自字符串，无文件路径
            g_loaded   = true;
        }
        notifyChange("");
        return true;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "[config] loadFromString(\"%s\") failed: %s\n",
                     sourceName.c_str(), e.what());
        return false;
    }
}

bool ConfigManager::reload() {
    std::string path;
    {
        std::shared_lock<std::shared_mutex> lk(g_mutex);
        path = g_filePath;
    }
    if (path.empty()) {
        std::fprintf(stderr, "[config] reload() — 未指定文件路径（从字符串加载时不支持 reload）\n");
        return false;
    }
    return load(path);
}

bool ConfigManager::save(const std::string& filePath) {
    std::string path = filePath;
    if (path.empty()) {
        std::shared_lock<std::shared_mutex> lk(g_mutex);
        path = g_filePath;
    }
    if (path.empty()) {
        std::fprintf(stderr, "[config] save() — 未指定目标文件路径\n");
        return false;
    }
    try {
        std::string content;
        {
            std::shared_lock<std::shared_mutex> lk(g_mutex);
            if (!g_loaded) {
                std::fprintf(stderr, "[config] save() — 配置尚未加载\n");
                return false;
            }
            std::ostringstream oss;
            oss << toml::format(g_root);
            content = oss.str();
        }
        std::ofstream ofs(path, std::ios::out | std::ios::trunc);
        if (!ofs.is_open()) {
            std::fprintf(stderr, "[config] save(\"%s\") — 无法打开文件写入\n", path.c_str());
            return false;
        }
        ofs << content;
        return true;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "[config] save(\"%s\") failed: %s\n", path.c_str(), e.what());
        return false;
    }
}

bool ConfigManager::isLoaded() noexcept {
    std::shared_lock<std::shared_mutex> lk(g_mutex);
    return g_loaded;
}

const std::string& ConfigManager::filePath() noexcept {
    // 注意：返回 const 引用；调用方应在持锁期间使用，此处简化处理
    std::shared_lock<std::shared_mutex> lk(g_mutex);
    return g_filePath;
}

// ── 节点查找 ─────────────────────────────────────────────────────────────────

const toml::value* ConfigManager::findNode(const std::string& key) noexcept {
    std::shared_lock<std::shared_mutex> lk(g_mutex);
    if (!g_loaded) return nullptr;
    try {
        const auto parts = splitKey(key);
        const toml::value* cur = &g_root;
        for (const auto& part : parts) {
            if (!cur->is_table()) return nullptr;
            const auto& tbl = cur->as_table();
            auto it = tbl.find(part);
            if (it == tbl.end()) return nullptr;
            cur = &it->second;
        }
        return cur;
    } catch (...) {
        return nullptr;
    }
}

toml::value& ConfigManager::ensureNode(const std::string& key) {
    std::unique_lock<std::shared_mutex> lk(g_mutex);
    if (!g_loaded) {
        g_root   = toml::value{toml::table{}};
        g_loaded = true;
    }
    const auto parts = splitKey(key);
    toml::value* cur = &g_root;
    for (const auto& part : parts) {
        if (!cur->is_table()) {
            *cur = toml::value{toml::table{}};
        }
        auto& tbl = cur->as_table();
        auto  it  = tbl.find(part);
        if (it == tbl.end()) {
            tbl.emplace(part, toml::value{});
        }
        cur = &tbl.at(part);
    }
    return *cur;
}

// ── 检测 / 删除 / 枚举 ────────────────────────────────────────────────────────

bool ConfigManager::has(const std::string& key) noexcept {
    return findNode(key) != nullptr;
}

bool ConfigManager::remove(const std::string& key) {
    const auto parts = splitKey(key);
    if (parts.empty()) return false;
    {
        std::unique_lock<std::shared_mutex> lk(g_mutex);
        if (!g_loaded) return false;
        try {
            // 导航到父节点
            toml::value* cur = &g_root;
            for (std::size_t i = 0; i + 1 < parts.size(); ++i) {
                if (!cur->is_table()) return false;
                auto& tbl = cur->as_table();
                auto  it  = tbl.find(parts[i]);
                if (it == tbl.end()) return false;
                cur = &it->second;
            }
            if (!cur->is_table()) return false;
            auto& tbl = cur->as_table();
            auto  it  = tbl.find(parts.back());
            if (it == tbl.end()) return false;
            tbl.erase(it);
        } catch (...) {
            return false;
        }
    }
    notifyChange(key);
    return true;
}

std::vector<std::string> ConfigManager::keys(const std::string& prefix) {
    std::shared_lock<std::shared_mutex> lk(g_mutex);
    if (!g_loaded) return {};
    try {
        const toml::value* node = &g_root;
        if (!prefix.empty()) {
            const auto parts = splitKey(prefix);
            for (const auto& part : parts) {
                if (!node->is_table()) return {};
                const auto& tbl = node->as_table();
                auto it = tbl.find(part);
                if (it == tbl.end()) return {};
                node = &it->second;
            }
        }
        if (!node->is_table()) return {};
        std::vector<std::string> result;
        for (const auto& kv : node->as_table()) {
            result.push_back(kv.first);
        }
        return result;
    } catch (...) {
        return {};
    }
}

// ── 作用域视图 ────────────────────────────────────────────────────────────────

ConfigSection ConfigManager::section(const std::string& key) {
    return ConfigSection(key);
}

// ── 变更通知 ─────────────────────────────────────────────────────────────────

void ConfigManager::notifyChange(const std::string& key) {
    std::vector<Listener> snapshot;
    {
        std::lock_guard<std::mutex> lk(g_listenerMutex);
        snapshot = g_listeners;
    }
    for (const auto& li : snapshot) {
        if (matchPattern(li.pattern, key)) {
            try { li.callback(key); } catch (...) {}
        }
    }
}

void ConfigManager::addChangeListener(const std::string& pattern,
                                       ChangeCallback callback) {
    std::lock_guard<std::mutex> lk(g_listenerMutex);
    g_listeners.push_back({pattern, std::move(callback)});
}

void ConfigManager::clearChangeListeners() {
    std::lock_guard<std::mutex> lk(g_listenerMutex);
    g_listeners.clear();
}

// ── 自动热重载 ────────────────────────────────────────────────────────────────

void ConfigManager::setAutoReload(bool enabled, int intervalMs) {
    if (enabled) {
        if (g_watcherRunning.exchange(true)) return;  // 已在运行

        const int safeInterval = std::max(100, intervalMs);

        // 停止旧线程（如果有）
        if (g_watcherThread.joinable()) {
            g_watcherRunning = false;
            g_watcherThread.join();
            g_watcherRunning = true;
        }

        g_watcherThread = std::thread([safeInterval]() {
            std::string watchPath;
            {
                std::shared_lock<std::shared_mutex> lk(g_mutex);
                watchPath = g_filePath;
            }
            auto lastMtime = fileModTime(watchPath);

            while (g_watcherRunning.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(safeInterval));

                // 文件路径可能因 reload/load 改变，每次重新读取
                {
                    std::shared_lock<std::shared_mutex> lk(g_mutex);
                    watchPath = g_filePath;
                }
                if (watchPath.empty()) continue;

                const auto curMtime = fileModTime(watchPath);
                if (curMtime != static_cast<std::time_t>(-1) && curMtime != lastMtime) {
                    lastMtime = curMtime;
                    ConfigManager::reload();
                }
            }
        });
    } else {
        if (!g_watcherRunning.exchange(false)) return;  // 已停止
        if (g_watcherThread.joinable()) {
            g_watcherThread.join();
        }
    }
}

// ── 序列化 ────────────────────────────────────────────────────────────────────

std::string ConfigManager::toTomlString() {
    std::shared_lock<std::shared_mutex> lk(g_mutex);
    if (!g_loaded) return {};
    try {
        std::ostringstream oss;
        oss << toml::format(g_root);
        return oss.str();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "[config] toTomlString() failed: %s\n", e.what());
        return {};
    }
}

}  // namespace config
