#pragma once

#include <memory>
#include <string>
#include <unordered_map>

// =============================================================================
// 服务接口定义（Service Interfaces）
// =============================================================================

/**
 * @brief IAppLogger — 日志服务接口
 *
 * 用于演示 Container 的 bind<>、singleton<>、instance<>、extend<>、
 * 上下文绑定（contextual binding）以及命名绑定（named binding）。
 */
class IAppLogger
{
public:
    virtual ~IAppLogger() = default;

    virtual void        info(const std::string& msg) = 0;
    virtual void        warn(const std::string& msg) = 0;
    virtual std::string name()    const = 0;
    virtual std::string lastMsg() const = 0;
};

/**
 * @brief IAppStorage — 键值存储服务接口
 *
 * 用于演示 singleton<> 共享实例特性，以及 named binding 的多参数方法调用。
 */
class IAppStorage
{
public:
    virtual ~IAppStorage() = default;

    virtual void        store(const std::string& key, const std::string& value) = 0;
    virtual std::string retrieve(const std::string& key) const = 0;
    virtual bool        has(const std::string& key) const = 0;
    virtual int         count() const = 0;
    virtual void        clear() = 0;
    virtual std::string storageName() const = 0;
};

// =============================================================================
// 具体实现（Concrete Implementations）
// =============================================================================

/**
 * @brief ConsoleAppLogger — 简单内存日志（适合瞬态绑定演示）
 *
 * 每次创建都是一个独立实例，没有副作用，适合演示 bind<> 每次返回新实例。
 */
class ConsoleAppLogger : public IAppLogger
{
public:
    ConsoleAppLogger() = default;

    void info(const std::string& msg) override { lastMsg_ = "[INFO] " + msg; }
    void warn(const std::string& msg) override { lastMsg_ = "[WARN] " + msg; }

    std::string name()    const override { return "ConsoleLogger"; }
    std::string lastMsg() const override { return lastMsg_; }

private:
    std::string lastMsg_;
};

/**
 * @brief FileAppLogger — 带文件路径属性的日志实现
 *
 * 适合演示：
 *  - contextual binding（ClientB 获得此实现）
 *  - named binding + readWrite 属性访问（"path" 属性可读写）
 */
class FileAppLogger : public IAppLogger
{
public:
    explicit FileAppLogger(const std::string& path = "app.log")
        : path_(path) {}

    void info(const std::string& msg) override
        { lastMsg_ = "[INFO→" + path_ + "] " + msg; }
    void warn(const std::string& msg) override
        { lastMsg_ = "[WARN→" + path_ + "] " + msg; }

    std::string name()    const override { return "FileLogger"; }
    std::string lastMsg() const override { return lastMsg_; }

    // 属性访问（供 MethodDispatcher::readWrite 注册，用于命名绑定演示）
    std::string path() const           { return path_; }
    void        setPath(std::string p) { path_ = std::move(p); }

private:
    std::string path_;
    std::string lastMsg_;
};

/**
 * @brief TimestampLoggerDecorator — 时间戳装饰器
 *
 * 包装任意 IAppLogger，在消息前添加 "[TS]" 标记。
 * 用于演示 extend<IAppLogger>() AOP 装饰扩展。
 */
class TimestampLoggerDecorator : public IAppLogger
{
public:
    explicit TimestampLoggerDecorator(std::shared_ptr<IAppLogger> inner)
        : inner_(std::move(inner)) {}

    void info(const std::string& msg) override
    {
        lastMsg_ = "[TS+INFO] " + msg;
        inner_->info(msg);
    }
    void warn(const std::string& msg) override
    {
        lastMsg_ = "[TS+WARN] " + msg;
        inner_->warn(msg);
    }

    std::string name()    const override { return "TimestampDecorator→" + inner_->name(); }
    std::string lastMsg() const override { return lastMsg_; }

private:
    std::shared_ptr<IAppLogger> inner_;
    std::string lastMsg_;
};

/**
 * @brief MemoryAppStorage — 内存键值存储（单例演示）
 *
 * 所有对同一实例的操作共享同一份数据，
 * 用于演示 singleton<> 绑定下不同 make() 调用返回同一实例。
 */
class MemoryAppStorage : public IAppStorage
{
public:
    MemoryAppStorage() = default;

    void store(const std::string& key, const std::string& value) override
        { data_[key] = value; }

    std::string retrieve(const std::string& key) const override
    {
        auto it = data_.find(key);
        return it != data_.end() ? it->second : "(not found)";
    }

    bool has(const std::string& key)  const override { return data_.count(key) > 0; }
    int  count()                      const override { return static_cast<int>(data_.size()); }
    void clear()                            override { data_.clear(); }
    std::string storageName()         const override { return "MemoryStorage"; }

private:
    std::unordered_map<std::string, std::string> data_;
};

// =============================================================================
// 上下文绑定演示辅助类型（Context Identifiers for Contextual Binding Demo）
// =============================================================================

/**
 * @brief ClientA — 上下文绑定演示：使用默认绑定（ConsoleAppLogger）
 *
 * 注册方式：
 *   c.bind<ClientA>([](Container& c) {
 *       return std::make_shared<ClientA>(c.make<IAppLogger>());
 *   });
 *   // 没有上下文绑定 → ClientA 得到默认的 ConsoleAppLogger
 */
struct ClientA {
    explicit ClientA(std::shared_ptr<IAppLogger> logger)
        : logger(std::move(logger)) {}
    std::shared_ptr<IAppLogger> logger;
};

/**
 * @brief ClientB — 上下文绑定演示：通过 when/needs/give 获得 FileAppLogger
 *
 * 注册方式：
 *   c.when<ClientB>().needs<IAppLogger>().give<FileAppLogger>(factory);
 *   c.bind<ClientB>([](Container& c) {
 *       return std::make_shared<ClientB>(c.make<IAppLogger>());
 *   });
 *   // 解析 ClientB 时，Container 会检测上下文绑定 → 给出 FileAppLogger
 */
struct ClientB {
    explicit ClientB(std::shared_ptr<IAppLogger> logger)
        : logger(std::move(logger)) {}
    std::shared_ptr<IAppLogger> logger;
};
