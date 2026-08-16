#pragma once

#include "container_global.h"
#include "binding.h"

#include <any>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace Container {

// ============================================================
// MethodDispatcher — 类型擦除的方法分发表
//
// 持有一组"方法名 → 可调用对象"的映射。
// 注册时需要知道具体类型（通过成员函数指针或 lambda）；
// 调用时只需传入 void* 实例指针 + string 方法名，无需类型信息。
//
// ── 注册方式 ─────────────────────────────────────────────────
//
//   // 1. 成员函数指针（0 / 1 / 2 参数，const 与非 const 均支持）
//   d.on("info",  &FileLogger::info);    // void info(const string&)
//   d.on("level", &FileLogger::level);   // string level() const
//
//   // 2. 自定义 Lambda（全控制，适合参数类型复杂的场景）
//   d.registerMethod("reset", [](void* inst, vector<any>) -> any {
//       static_cast<FileLogger*>(inst)->reset();
//       return {};
//   });
//
// ── 调用方式 ─────────────────────────────────────────────────
//
//   d.invoke("info", rawPtr, { any{string("hello")} });
//   d.invoke("level", rawPtr, {});          // 返回 any{string}
// ============================================================
class CONTAINER_EXPORT MethodDispatcher
{
public:
    /// 类型擦除的底层调用签名
    using InvokeFn = std::function<std::any(void* instance, std::vector<std::any> args)>;

    // ── 底层注册 ──────────────────────────────────────────────────────────

    /// 手动注册任意签名（适合参数复杂或需要自定义逻辑的场景）
    void registerMethod(const std::string& name, InvokeFn fn);

    /// 检查方法是否已注册
    bool hasMethod(const std::string& name) const;

    /// 按名称调用方法
    /// @throws std::runtime_error 方法未注册 或 any_cast 类型不匹配
    std::any invoke(const std::string& name, void* instance,
                    std::vector<std::any> args = {}) const;

    // ── 成员函数指针便捷注册（0 参数）────────────────────────────────────

    template <typename Svc, typename Ret>
    void on(const std::string& name, Ret (Svc::*method)())
    {
        registerMethod(name, [method](void* inst, std::vector<std::any>) -> std::any {
            auto* svc = static_cast<Svc*>(inst);
            if constexpr (std::is_void_v<Ret>) {
                (svc->*method)();
                return std::any{};
            } else {
                return std::any{(svc->*method)()};
            }
        });
    }

    template <typename Svc, typename Ret>
    void on(const std::string& name, Ret (Svc::*method)() const)
    {
        registerMethod(name, [method](void* inst, std::vector<std::any>) -> std::any {
            const auto* svc = static_cast<const Svc*>(inst);
            if constexpr (std::is_void_v<Ret>) {
                (svc->*method)();
                return std::any{};
            } else {
                return std::any{(svc->*method)()};
            }
        });
    }

    // ── 成员函数指针便捷注册（1 参数）────────────────────────────────────

    template <typename Svc, typename Ret, typename A1>
    void on(const std::string& name, Ret (Svc::*method)(A1))
    {
        registerMethod(name, [method, name](void* inst, std::vector<std::any> args) -> std::any {
            if (args.size() < 1) {
                throw std::runtime_error(
                    "[MethodDispatcher] '" + name + "' expects 1 argument, got "
                    + std::to_string(args.size()));
            }
            auto* svc = static_cast<Svc*>(inst);
            auto  a1  = std::any_cast<std::decay_t<A1>>(args.at(0));
            if constexpr (std::is_void_v<Ret>) {
                (svc->*method)(std::move(a1));
                return std::any{};
            } else {
                return std::any{(svc->*method)(std::move(a1))};
            }
        });
    }

    template <typename Svc, typename Ret, typename A1>
    void on(const std::string& name, Ret (Svc::*method)(A1) const)
    {
        registerMethod(name, [method, name](void* inst, std::vector<std::any> args) -> std::any {
            if (args.size() < 1) {
                throw std::runtime_error(
                    "[MethodDispatcher] '" + name + "' expects 1 argument, got "
                    + std::to_string(args.size()));
            }
            const auto* svc = static_cast<const Svc*>(inst);
            auto        a1  = std::any_cast<std::decay_t<A1>>(args.at(0));
            if constexpr (std::is_void_v<Ret>) {
                (svc->*method)(std::move(a1));
                return std::any{};
            } else {
                return std::any{(svc->*method)(std::move(a1))};
            }
        });
    }

    // ── 成员函数指针便捷注册（2 参数）────────────────────────────────────

    template <typename Svc, typename Ret, typename A1, typename A2>
    void on(const std::string& name, Ret (Svc::*method)(A1, A2))
    {
        registerMethod(name, [method, name](void* inst, std::vector<std::any> args) -> std::any {
            if (args.size() < 2) {
                throw std::runtime_error(
                    "[MethodDispatcher] '" + name + "' expects 2 arguments, got "
                    + std::to_string(args.size()));
            }
            auto* svc = static_cast<Svc*>(inst);
            auto  a1  = std::any_cast<std::decay_t<A1>>(args.at(0));
            auto  a2  = std::any_cast<std::decay_t<A2>>(args.at(1));
            if constexpr (std::is_void_v<Ret>) {
                (svc->*method)(std::move(a1), std::move(a2));
                return std::any{};
            } else {
                return std::any{(svc->*method)(std::move(a1), std::move(a2))};
            }
        });
    }

    template <typename Svc, typename Ret, typename A1, typename A2>
    void on(const std::string& name, Ret (Svc::*method)(A1, A2) const)
    {
        registerMethod(name, [method, name](void* inst, std::vector<std::any> args) -> std::any {
            if (args.size() < 2) {
                throw std::runtime_error(
                    "[MethodDispatcher] '" + name + "' expects 2 arguments, got "
                    + std::to_string(args.size()));
            }
            const auto* svc = static_cast<const Svc*>(inst);
            auto        a1  = std::any_cast<std::decay_t<A1>>(args.at(0));
            auto        a2  = std::any_cast<std::decay_t<A2>>(args.at(1));
            if constexpr (std::is_void_v<Ret>) {
                (svc->*method)(std::move(a1), std::move(a2));
                return std::any{};
            } else {
                return std::any{(svc->*method)(std::move(a1), std::move(a2))};
            }
        });
    }

    // ── 成员函数指针便捷注册（3 参数）────────────────────────────────────

    template <typename Svc, typename Ret, typename A1, typename A2, typename A3>
    void on(const std::string& name, Ret (Svc::*method)(A1, A2, A3))
    {
        registerMethod(name, [method, name](void* inst, std::vector<std::any> args) -> std::any {
            if (args.size() < 3) {
                throw std::runtime_error(
                    "[MethodDispatcher] '" + name + "' expects 3 arguments, got "
                    + std::to_string(args.size()));
            }
            auto* svc = static_cast<Svc*>(inst);
            auto  a1  = std::any_cast<std::decay_t<A1>>(args.at(0));
            auto  a2  = std::any_cast<std::decay_t<A2>>(args.at(1));
            auto  a3  = std::any_cast<std::decay_t<A3>>(args.at(2));
            if constexpr (std::is_void_v<Ret>) {
                (svc->*method)(std::move(a1), std::move(a2), std::move(a3));
                return std::any{};
            } else {
                return std::any{(svc->*method)(std::move(a1), std::move(a2), std::move(a3))};
            }
        });
    }

    // ── 属性访问（getter / setter）───────────────────────────────────────

    /// 注册只读属性（成员函数 getter）
    template <typename Svc, typename Prop>
    void readOnly(const std::string& name, Prop (Svc::*getter)() const)
    {
        registerMethod("get:" + name, [getter](void* inst, std::vector<std::any>) -> std::any {
            return std::any{(static_cast<const Svc*>(inst)->*getter)()};
        });
    }

    /// 注册读写属性（getter + setter）
    template <typename Svc, typename Prop>
    void readWrite(const std::string& name,
                   Prop (Svc::*getter)() const,
                   void (Svc::*setter)(Prop))
    {
        readOnly(name, getter);
        registerMethod("set:" + name, [setter, name](void* inst, std::vector<std::any> args) -> std::any {
            if (args.size() < 1) {
                throw std::runtime_error(
                    "[MethodDispatcher] Property setter '" + name + "' expects 1 argument, got "
                    + std::to_string(args.size()));
            }
            auto val = std::any_cast<std::decay_t<Prop>>(args.at(0));
            (static_cast<Svc*>(inst)->*setter)(std::move(val));
            return std::any{};
        });
    }

    /// 获取属性（等价于 invoke("get:name", ...)）
    std::any getProperty(const std::string& name, void* instance) const;

    /// 设置属性（等价于 invoke("set:name", ...)）
    void setProperty(const std::string& name, void* instance, std::any value) const;

private:
    std::unordered_map<std::string, InvokeFn> methods_;
};

// ============================================================
// NamedBinding — 带名称键与可选反射分发器的绑定描述符
// ============================================================
struct NamedBinding
{
    FactoryFn   factory;
    BindingType type     = BindingType::Transient;
    bool        resolved = false;
    std::shared_ptr<void>           instance;
    std::shared_ptr<MethodDispatcher> dispatcher;  ///< null = 不支持反射调用
};

// ============================================================
// ServiceHandle — 动态服务句柄
//
// 由 Container::makeNamed() 返回，封装了：
//   - 类型擦除的服务实例（shared_ptr<void>）
//   - 可选的方法分发器（MethodDispatcher）
//
// 调用者不需要引入服务的头文件，通过字符串名称完成方法调用。
//
// 示例（调用方不知道 FileLogger 类型）：
//   auto handle = app.makeNamed("logger");
//   handle.call("info",  { std::any{std::string("started")} });
//   auto lvl = handle.get("level");  // 返回 std::any
//
// 若调用方确实知道类型：
//   auto* logger = handle.as<ILogger>().get();
// ============================================================
class CONTAINER_EXPORT ServiceHandle
{
public:
    ServiceHandle() = default;
    ServiceHandle(std::shared_ptr<void>             instance,
                  std::shared_ptr<MethodDispatcher> dispatcher);

    // ── 动态调用 ──────────────────────────────────────────────────────────

    /// 按名称调用方法，args 为按序排列的参数（std::any 包装）
    std::any call(const std::string& method,
                  std::vector<std::any> args = {}) const;

    /// 检查方法是否存在
    bool hasMethod(const std::string& method) const;

    // ── 属性访问 ──────────────────────────────────────────────────────────

    /// 读取属性（需已通过 readOnly / readWrite 注册 getter）
    std::any get(const std::string& propName) const;

    /// 写入属性（需已通过 readWrite 注册 setter）
    void set(const std::string& propName, std::any value) const;

    // ── 类型转换（知道类型时使用）────────────────────────────────────────

    /// 转换为已知类型的 shared_ptr（编译期已知类型时使用）
    template <typename T>
    std::shared_ptr<T> as() const
    {
        return std::static_pointer_cast<T>(instance_);
    }

    // ── 生命周期检测 ──────────────────────────────────────────────────────

    std::shared_ptr<void> raw() const { return instance_; }
    bool isNull() const { return !instance_; }
    explicit operator bool() const { return !isNull(); }

private:
    std::shared_ptr<void>             instance_;
    std::shared_ptr<MethodDispatcher> dispatcher_;
};

} // namespace Container
