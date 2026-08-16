#include "container.h"

namespace Container {

// ── 构造 / 析构 ───────────────────────────────────────────────────────────────

Container::Container()  = default;
Container::~Container() = default;

// ── thread_local 解析调用栈 ───────────────────────────────────────────────────
//
// 每个线程持有独立的解析链，彻底避免多线程并发 make() 时 push/pop 互相干扰。
// 使用静态函数内的 thread_local 变量，是 C++11 保证线程安全初始化的惯用做法。

/*static*/
std::vector<std::type_index>& Container::resolutionStack()
{
    thread_local std::vector<std::type_index> s_stack;
    return s_stack;
}

// ── 内部解析 ──────────────────────────────────────────────────────────────────

std::shared_ptr<void> Container::resolve(Binding& binding, std::type_index key)
{
    switch (binding.type) {

    case BindingType::Instance:
        // 预建实例：直接返回，无需工厂
        return binding.instance;

    case BindingType::Singleton: {
        // 单例：双重检查加锁（Double-Checked Locking）
        //
        // 外层 if：快速路径，已初始化的单例无需竞争锁（读 bool 在主流平台是原子的）。
        // lock_guard：确保首次构建只发生一次；使用 recursive_mutex 允许工厂内
        //   再次调用 make()（同线程重入）而不死锁。
        // 内层 if：持锁后二次确认，防止多线程同时通过外层检查的竞态。
        if (!binding.resolved) {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            if (!binding.resolved) {
                auto& stack = resolutionStack();
                stack.push_back(key);
                binding.instance = binding.factory(*this);
                binding.resolved = true;
                stack.pop_back();
            }
        }
        return binding.instance;
    }

    case BindingType::Transient:
    default: {
        // 瞬态：每次调用都创建新实例，无需加锁（无共享状态）
        auto& stack = resolutionStack();
        stack.push_back(key);
        auto result = binding.factory(*this);
        stack.pop_back();
        return result;
    }
    }
}

// ── tagged ────────────────────────────────────────────────────────────────────

std::vector<std::shared_ptr<void>> Container::tagged(const std::string& label)
{
    auto it = typeTags_.find(label);
    if (it == typeTags_.end()) {
        return {};
    }

    std::vector<std::shared_ptr<void>> results;
    results.reserve(it->second.size());

    for (auto& typeIdx : it->second) {
        auto bit = typeBindings_.find(typeIdx);
        if (bit != typeBindings_.end()) {
            results.push_back(resolve(bit->second, typeIdx));
        }
    }
    return results;
}

// ── flush ─────────────────────────────────────────────────────────────────────

void Container::flush()
{
    typeBindings_.clear();
    namedBindings_.clear();
    typeTags_.clear();
    contextualBindings_.clear();
    resolutionStack().clear();   // 清空当前线程的解析栈
}

// ── 命名绑定 — 内部解析 ────────────────────────────────────────────────────────

std::shared_ptr<void> Container::resolveNamed(NamedBinding& nb)
{
    switch (nb.type) {
    case BindingType::Instance:
        return nb.instance;

    case BindingType::Singleton:
        // 与类型绑定相同的双重检查加锁模式
        if (!nb.resolved) {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            if (!nb.resolved) {
                nb.instance = nb.factory(*this);
                nb.resolved = true;
            }
        }
        return nb.instance;

    case BindingType::Transient:
    default:
        return nb.factory(*this);
    }
}

// ── 命名绑定 — 注册 ───────────────────────────────────────────────────────────

void Container::bindNamed(const std::string& name,
                          FactoryFn          factory,
                          DispatchSetupFn    setup)
{
    NamedBinding nb;
    nb.factory = std::move(factory);
    nb.type    = BindingType::Transient;
    if (setup) {
        nb.dispatcher = std::make_shared<MethodDispatcher>();
        setup(*nb.dispatcher);
    }
    namedBindings_[name] = std::move(nb);
}

void Container::singletonNamed(const std::string& name,
                               FactoryFn          factory,
                               DispatchSetupFn    setup)
{
    NamedBinding nb;
    nb.factory = std::move(factory);
    nb.type    = BindingType::Singleton;
    if (setup) {
        nb.dispatcher = std::make_shared<MethodDispatcher>();
        setup(*nb.dispatcher);
    }
    namedBindings_[name] = std::move(nb);
}

void Container::instanceNamed(const std::string&    name,
                              std::shared_ptr<void> inst,
                              DispatchSetupFn       setup)
{
    NamedBinding nb;
    nb.type     = BindingType::Instance;
    nb.resolved = true;
    nb.instance = std::move(inst);
    if (setup) {
        nb.dispatcher = std::make_shared<MethodDispatcher>();
        setup(*nb.dispatcher);
    }
    namedBindings_[name] = std::move(nb);
}

// ── 命名绑定 — 解析 ───────────────────────────────────────────────────────────

ServiceHandle Container::makeNamed(const std::string& name)
{
    auto it = namedBindings_.find(name);
    if (it == namedBindings_.end()) {
        throw std::runtime_error(
            "[Container] No named binding found for: '" + name + "'");
    }
    auto inst = resolveNamed(it->second);
    return ServiceHandle(std::move(inst), it->second.dispatcher);
}

bool Container::boundNamed(const std::string& name) const
{
    return namedBindings_.count(name) > 0;
}

} // namespace Container
