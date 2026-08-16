#pragma once

#include "container_global.h"
#include "binding.h"
#include "namedservice.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Container {

// 前向声明
class ContextualBindingBuilder;

// ============================================================
// Container — Laravel 风格 IoC 服务容器
//
// 核心功能一览：
//
//  ┌─────────────────────────────────────────────────────────┐
//  │ 注册                                                     │
//  │  bind<Abstract>(factory)         瞬态：每次新实例         │
//  │  bind<Abstract, Concrete>()      瞬态：自动构造 Concrete  │
//  │  singleton<Abstract>(factory)    单例：共享同一实例        │
//  │  singleton<Abstract, Concrete>() 单例：自动构造 Concrete  │
//  │  instance<Abstract>(ptr)         注册已有实例             │
//  │  extend<Abstract>(decorator)     装饰/扩展已有绑定        │
//  ├─────────────────────────────────────────────────────────┤
//  │ 解析                                                     │
//  │  make<Abstract>()                按类型解析               │
//  │  bound<Abstract>()               检查绑定是否存在          │
//  ├─────────────────────────────────────────────────────────┤
//  │ 上下文绑定 (contextual binding)                           │
//  │  when<ConcreteA>()               开始上下文绑定构建器      │
//  │    ->needs<Abstract>()           指定依赖接口             │
//  │    ->give<ConcreteB>()           提供具体实现             │
//  ├─────────────────────────────────────────────────────────┤
//  │ 标签 (tagging)                                           │
//  │  tag<A, B, ...>(label)           将多个类型归入标签组      │
//  │  tagged(label)                   解析标签下所有实例        │
//  │  taggedAs<Base>(label)           类型化标签解析（推荐）     │
//  ├─────────────────────────────────────────────────────────┤
//  │ 其他                                                     │
//  │  flush()                         清空所有绑定             │
//  └─────────────────────────────────────────────────────────┘
//
// 改进说明（v2）：
//   ✔ 循环依赖检测  — make() 发现类型已在调用栈中时抛异常
//   ✔ thread_local 解析栈 — 每个线程独享解析链，互不干扰
//   ✔ Singleton 双重检查加锁 — std::recursive_mutex 保护首次构建
//   ✔ extend() Instance 保护 — Instance 绑定不可被装饰
//   ✔ taggedAs<Base>() — 省去手动 static_pointer_cast
//   ✔ 延迟加载钩子 — onBindingNotFound() 供 Application 触发延迟 Provider
//
// 示例：
//   Container c;
//   c.singleton<ILogger>([]{ return std::make_shared<FileLogger>(); });
//   c.bind<IMailer>([](Container& app){
//       return std::make_shared<SmtpMailer>(app.make<IConfig>());
//   });
//   auto logger = c.make<ILogger>();
// ============================================================
class CONTAINER_EXPORT Container
{
    friend class ContextualBindingBuilder;

public:
    Container();
    virtual ~Container();

    // ── 瞬态绑定 (Transient) ──────────────────────────────────────────────

    /// 注册瞬态绑定：工厂接收容器引用，可递归解析依赖
    template <typename Abstract>
    void bind(std::function<std::shared_ptr<Abstract>(Container&)> factory)
    {
        Binding b;
        b.factory = [f = std::move(factory)](Container& c) -> std::shared_ptr<void> {
            return f(c);
        };
        b.type = BindingType::Transient;
        typeBindings_[std::type_index(typeid(Abstract))] = std::move(b);
    }

    /// 便捷重载：工厂无参数
    template <typename Abstract>
    void bind(std::function<std::shared_ptr<Abstract>()> factory)
    {
        bind<Abstract>([f = std::move(factory)](Container&) { return f(); });
    }

    /// 便捷重载：自动用 Concrete 实现 Abstract（需继承关系）
    template <typename Abstract, typename Concrete>
    void bind()
    {
        static_assert(std::is_base_of<Abstract, Concrete>::value
                   || std::is_same<Abstract, Concrete>::value,
                      "Concrete must be the same as or derived from Abstract");
        bind<Abstract>([](Container&) {
            return std::make_shared<Concrete>();
        });
    }

    // ── 单例绑定 (Singleton) ──────────────────────────────────────────────

    /// 注册单例绑定：首次 make() 时构建，后续复用同一实例
    template <typename Abstract>
    void singleton(std::function<std::shared_ptr<Abstract>(Container&)> factory)
    {
        Binding b;
        b.factory = [f = std::move(factory)](Container& c) -> std::shared_ptr<void> {
            return f(c);
        };
        b.type = BindingType::Singleton;
        typeBindings_[std::type_index(typeid(Abstract))] = std::move(b);
    }

    /// 便捷重载：工厂无参数
    template <typename Abstract>
    void singleton(std::function<std::shared_ptr<Abstract>()> factory)
    {
        singleton<Abstract>([f = std::move(factory)](Container&) { return f(); });
    }

    /// 便捷重载：自动用 Concrete 实现 Abstract 的单例
    template <typename Abstract, typename Concrete>
    void singleton()
    {
        static_assert(std::is_base_of<Abstract, Concrete>::value
                   || std::is_same<Abstract, Concrete>::value,
                      "Concrete must be the same as or derived from Abstract");
        singleton<Abstract>([](Container&) {
            return std::make_shared<Concrete>();
        });
    }

    // ── 预建实例 (Instance) ───────────────────────────────────────────────

    /// 注册一个已构建好的实例，后续每次 make<Abstract>() 都返回此实例
    template <typename Abstract>
    void instance(std::shared_ptr<Abstract> inst)
    {
        Binding b;
        b.type     = BindingType::Instance;
        b.resolved = true;
        b.instance = std::move(inst);
        typeBindings_[std::type_index(typeid(Abstract))] = std::move(b);
    }

    // ── 装饰/扩展 (Extend) ───────────────────────────────────────────────

    /// 装饰已注册的绑定：decorator 接收已解析的实例，可包装后返回新实例
    /// 适用于 AOP / 代理模式
    template <typename Abstract>
    void extend(
        std::function<std::shared_ptr<Abstract>(std::shared_ptr<Abstract>, Container&)> decorator)
    {
        auto key = std::type_index(typeid(Abstract));
        auto it  = typeBindings_.find(key);
        if (it == typeBindings_.end()) {
            throw std::runtime_error(
                std::string("[Container::extend] No binding found for: ")
                + typeid(Abstract).name());
        }
        // Instance 绑定已是固化实例，无法再包裹工厂
        if (it->second.type == BindingType::Instance) {
            throw std::runtime_error(
                std::string("[Container::extend] Cannot extend an Instance binding for: ")
                + typeid(Abstract).name()
                + ". Use container.instance<T>() to replace it instead.");
        }

        // 保存原工厂，用新工厂包裹
        FactoryFn original = it->second.factory;
        it->second.factory = [original, dec = std::move(decorator)](
                                 Container& c) -> std::shared_ptr<void>
        {
            auto base = std::static_pointer_cast<Abstract>(original(c));
            return dec(base, c);
        };
        // 已缓存的单例需要重置
        it->second.resolved = false;
        it->second.instance.reset();
    }

    // ── 解析 (Resolve) ────────────────────────────────────────────────────

    /// 按类型解析服务；若存在上下文绑定则优先使用
    ///
    /// 改进（v2）：
    ///   - 循环依赖检测：同一解析链中出现相同类型时立即抛出异常
    ///   - thread_local 解析栈：每个线程拥有独立解析链，多线程并发安全
    ///   - 延迟加载钩子：类型未找到时调用 onBindingNotFound()，允许子类
    ///     （如 Application）按需触发尚未注册的 Deferred ServiceProvider
    template <typename Abstract>
    std::shared_ptr<Abstract> make()
    {
        auto  key   = std::type_index(typeid(Abstract));
        auto& stack = resolutionStack();   // thread_local，每线程独立

        // ── 循环依赖检测 ─────────────────────────────────────────────────
        if (std::find(stack.begin(), stack.end(), key) != stack.end()) {
            throw std::runtime_error(
                std::string("[Container] Circular dependency detected for: ")
                + typeid(Abstract).name()
                + ". Resolution chain contains the type more than once.");
        }

        // ── 检查上下文绑定 ──────────────────────────────────────────────
        if (!stack.empty()) {
            auto callerKey = stack.back();
            auto ctxIt     = contextualBindings_.find(callerKey);
            if (ctxIt != contextualBindings_.end()) {
                auto needsIt = ctxIt->second.find(key);
                if (needsIt != ctxIt->second.end()) {
                    stack.push_back(key);
                    auto ptr = needsIt->second(*this);
                    stack.pop_back();
                    return std::static_pointer_cast<Abstract>(ptr);
                }
            }
        }

        // ── 普通绑定 ────────────────────────────────────────────────────
        auto it = typeBindings_.find(key);
        if (it == typeBindings_.end()) {
            onBindingNotFound(key);        // 触发延迟加载（子类可重写）
            it = typeBindings_.find(key);  // 重新查找
            if (it == typeBindings_.end()) {
                throw std::runtime_error(
                    std::string("[Container] No binding found for type: ")
                    + typeid(Abstract).name());
            }
        }
        return std::static_pointer_cast<Abstract>(resolve(it->second, key));
    }

    /// 检查指定类型是否已注册
    template <typename Abstract>
    bool bound() const
    {
        return typeBindings_.count(std::type_index(typeid(Abstract))) > 0;
    }

    // ── 标签 (Tagging) ────────────────────────────────────────────────────

    /// 将多个抽象类型归入同一标签组（可变参数模板，C++17 折叠表达式）
    /// 用法：c.tag<IMailer, ISmsNotifier>("notifiers");
    template <typename... Abstracts>
    void tag(const std::string& label)
    {
        std::vector<std::type_index> indices;
        (indices.push_back(std::type_index(typeid(Abstracts))), ...);
        typeTags_[label] = std::move(indices);
    }

    /// 解析标签下所有服务，返回 shared_ptr<void> 列表（调用方自行 cast）
    std::vector<std::shared_ptr<void>> tagged(const std::string& label);

    /// 类型化标签解析（推荐）：直接返回 shared_ptr<Base> 列表，省去手动 cast
    /// 示例：auto loggers = c.taggedAs<ILogger>("loggers");
    template <typename Base>
    std::vector<std::shared_ptr<Base>> taggedAs(const std::string& label)
    {
        auto raw = tagged(label);
        std::vector<std::shared_ptr<Base>> result;
        result.reserve(raw.size());
        for (auto& v : raw)
            result.push_back(std::static_pointer_cast<Base>(v));
        return result;
    }

    // ── 上下文绑定 (Contextual Binding) ──────────────────────────────────

    /// 开始上下文绑定构建器
    /// 用法：c.when<OrderController>()->needs<ILogger>()->give<FileLogger>();
    template <typename When>
    ContextualBindingBuilder when();

    // ── 命名绑定（Named Binding）— 动态字符串键，无需编译期类型信息 ────────
    //
    // 调用方（resolution site）只需包含 container.h，无需引入服务的具体头文件。
    // 注册方（registration site）知道类型，通过 MethodDispatcher 注册反射方法。
    //
    // 典型场景：
    //   插件 A 在 ServiceProvider::doRegister() 中注册：
    //     app.singletonNamed("logger", factory, [](MethodDispatcher& d) {
    //         d.on("info",  &FileLogger::info);
    //         d.on("error", &FileLogger::error);
    //     });
    //
    //   插件 B（不包含 FileLogger.h）动态调用：
    //     auto h = app.makeNamed("logger");
    //     h.call("info", { std::any{std::string("hello")} });

    /// 用于设置 MethodDispatcher 的回调类型
    using DispatchSetupFn = std::function<void(MethodDispatcher&)>;

    /// 注册瞬态命名绑定（含可选反射方法注册）
    void bindNamed(const std::string& name,
                   FactoryFn          factory,
                   DispatchSetupFn    setup = nullptr);

    /// 注册单例命名绑定（含可选反射方法注册）
    void singletonNamed(const std::string& name,
                        FactoryFn          factory,
                        DispatchSetupFn    setup = nullptr);

    /// 注册预建实例的命名绑定（含可选反射方法注册）
    void instanceNamed(const std::string&    name,
                       std::shared_ptr<void> inst,
                       DispatchSetupFn       setup = nullptr);

    /// 按字符串名称解析服务，返回 ServiceHandle（支持动态方法调用）
    ServiceHandle makeNamed(const std::string& name);

    /// 检查命名绑定是否存在
    bool boundNamed(const std::string& name) const;

    // ── 其他 ──────────────────────────────────────────────────────────────

    /// 清空所有绑定（类型绑定 + 命名绑定 + 标签 + 上下文绑定）
    void flush();

protected:
    /// 内部解析函数（处理 Transient / Singleton / Instance 三种情形）
    std::shared_ptr<void> resolve(Binding& binding, std::type_index key);

    /// 内部命名解析函数（与 resolve() 逻辑对应，但作用于 NamedBinding）
    std::shared_ptr<void> resolveNamed(NamedBinding& nb);

    /// thread_local 解析调用栈（每个线程独享，避免多线程下 push/pop 交叉）
    /// 用于：① 上下文绑定识别调用方  ② 循环依赖检测
    static std::vector<std::type_index>& resolutionStack();

    /// 绑定未找到时的扩展钩子（默认空实现）
    /// Application 子类重写此方法，在 make<T>() 找不到类型时
    /// 自动触发尚未 doRegister() 的 Deferred ServiceProvider。
    virtual void onBindingNotFound(std::type_index /*key*/) {}

    // ── 数据成员 ──────────────────────────────────────────────────────────

    /// 类型键 → 绑定描述符
    std::unordered_map<std::type_index, Binding> typeBindings_;

    /// 字符串键 → 命名绑定描述符（含反射分发器）
    std::unordered_map<std::string, NamedBinding> namedBindings_;

    /// 标签名 → 类型索引列表
    std::unordered_map<std::string, std::vector<std::type_index>> typeTags_;

    /// 上下文绑定：contextualBindings_[when_type][needs_type] = factory
    std::unordered_map<std::type_index,
        std::unordered_map<std::type_index, FactoryFn>> contextualBindings_;

    /// Singleton 初始化互斥锁
    /// 使用 recursive_mutex：工厂函数可能递归调用 make()，允许同线程重入。
    /// Transient 解析无需加锁（每次创建新对象，无共享状态）。
    mutable std::recursive_mutex mutex_;
};

// ============================================================
// ContextualBindingBuilder — 上下文绑定构建器
//
// 通过链式调用描述"当 A 需要 B 时，提供 C"：
//   container.when<A>().needs<B>().give<C>();
//   container.when<A>().needs<B>().give([](Container& c){ ... });
// ============================================================
class CONTAINER_EXPORT ContextualBindingBuilder
{
public:
    ContextualBindingBuilder(Container& container, std::type_index whenType)
        : container_(container)
        , whenType_(whenType)
        , needsType_(typeid(void))
    {}

    /// 指定需要被替换的依赖接口
    template <typename Needs>
    ContextualBindingBuilder& needs()
    {
        needsType_ = std::type_index(typeid(Needs));
        return *this;
    }

    /// 提供具体实现（自动默认构造 Give）
    template <typename Give>
    void give()
    {
        FactoryFn fn = [](Container&) -> std::shared_ptr<void> {
            return std::make_shared<Give>();
        };
        container_.contextualBindings_[whenType_][needsType_] = std::move(fn);
    }

    /// 提供具体实现（工厂接收容器引用）
    template <typename Give>
    void give(std::function<std::shared_ptr<Give>(Container&)> factory)
    {
        FactoryFn fn = [f = std::move(factory)](Container& c) -> std::shared_ptr<void> {
            return f(c);
        };
        container_.contextualBindings_[whenType_][needsType_] = std::move(fn);
    }

    /// 提供具体实现（工厂无参数）
    template <typename Give>
    void give(std::function<std::shared_ptr<Give>()> factory)
    {
        give<Give>([f = std::move(factory)](Container&) { return f(); });
    }

private:
    Container&      container_;
    std::type_index whenType_;
    std::type_index needsType_;
};

// ── Container::when<When>() 定义（需在 ContextualBindingBuilder 完整定义后）─────
template <typename When>
ContextualBindingBuilder Container::when()
{
    return ContextualBindingBuilder(*this, std::type_index(typeid(When)));
}

} // namespace Container
