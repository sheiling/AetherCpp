#include "application.h"
#include "serviceprovider.h"

#include <stdexcept>

namespace Container {

// ── 构造 / 析构 ───────────────────────────────────────────────────────────────

Application::Application()  = default;
Application::~Application() = default;

// ── Provider 注册 ─────────────────────────────────────────────────────────────

Application& Application::addProvider(std::unique_ptr<ServiceProvider> provider)
{
    providers_.push_back(std::move(provider));
    return *this;
}

void Application::registerProviders()
{
    for (auto& provider : providers_) {
        if (!provider->isDeferred()) {
            provider->doRegister();
        }
        // Deferred Provider 此时跳过，等 make<T>() 时按需触发
    }
}

// ── 延迟加载钩子 ──────────────────────────────────────────────────────────────
//
// 当 Container::make<T>() 找不到类型绑定时调用。
// 遍历所有标记为 isDeferred() 且尚未注册的 Provider，依次调用 doRegister()。
// 只要有任何一个 Provider 完成注册，make<T>() 就会重新查找并可能命中该类型。
//
// 设计说明：
//   - 每个 Deferred Provider 只会被触发一次（deferredRegistered_ 防重入）。
//   - 此处不做"精准匹配"（不知道每个 Provider 提供哪些类型），而是批量注册
//     所有待注册的 Deferred Provider。成本在于首次 miss 时略多一次遍历，
//     但相比引入 provides() 接口要求 Provider 自报类型，更简洁实用。

void Application::onBindingNotFound(std::type_index /*key*/)
{
    for (auto& provider : providers_) {
        if (provider->isDeferred()
            && deferredRegistered_.find(provider.get()) == deferredRegistered_.end())
        {
            provider->doRegister();
            deferredRegistered_.insert(provider.get());
        }
    }
}

void Application::bootProviders()
{
    for (auto& provider : providers_) {
        provider->boot();
    }
    booted_ = true;
}

void Application::bootstrap()
{
    if (booted_) {
        return; // 防止重复引导
    }
    registerProviders();
    bootProviders();
}

// ── 全局单例 ──────────────────────────────────────────────────────────────────

Application* Application::instance()
{
    // C++11 及之后，函数内部局部静态对象的初始化是线程安全的
    static Application s_instance;
    return &s_instance;
}

} // namespace Container
