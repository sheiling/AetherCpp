#pragma once

#include "container_global.h"
#include "container.h"

#include <memory>
#include <set>
#include <vector>

namespace Container {

class ServiceProvider;

// ============================================================
// Application — 应用程序级服务容器
//
// Application 继承自 Container，并在其之上叠加了：
//   1. ServiceProvider 生命周期管理
//   2. 全局单例访问（Application::instance()）
//
// 典型用法：
//
//   // 1. 创建并配置应用
//   auto app = std::make_unique<Application>();
//   app->registerProvider<LogServiceProvider>()
//      .registerProvider<DatabaseServiceProvider>()
//      .registerProvider<CacheServiceProvider>();
//
//   // 2. 引导（register → boot）
//   app->bootstrap();
//
//   // 3. 解析服务
//   auto logger = app->make<ILogger>();
//   auto db     = app->make<IDatabase>();
//
//   // 4. 可选：通过全局单例访问（内部懒汉式构建）
//   auto logger2 = Application::instance()->make<ILogger>();
// ============================================================
class CONTAINER_EXPORT Application : public Container
{
public:
    Application();
    ~Application() override;

    // Application 持有 vector<unique_ptr>，不可拷贝
    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    // ── ServiceProvider 管理 ──────────────────────────────────────────────

    /// 注册服务提供者（自动以 *this 为参数构造）
    template <typename Provider>
    Application& registerProvider()
    {
        static_assert(std::is_base_of<ServiceProvider, Provider>::value,
                      "Provider must be derived from ServiceProvider");
        providers_.emplace_back(std::make_unique<Provider>(*this));
        return *this;   // 支持链式调用
    }

    /// 注册已构建好的服务提供者
    Application& addProvider(std::unique_ptr<ServiceProvider> provider);

    /// 第一阶段：依次调用所有 Provider 的 doRegister()
    void registerProviders();

    /// 第二阶段：依次调用所有 Provider 的 boot()
    void bootProviders();

    /// 一键引导：registerProviders() + bootProviders()
    void bootstrap();

    bool isBooted() const { return booted_; }

    // ── 全局单例访问 ──────────────────────────────────────────────────────
    ///
    /// 返回全局 Application 实例（首次调用时在内部构建单例）
    static Application* instance();

protected:
    /// 延迟加载钩子（重写 Container::onBindingNotFound）
    ///
    /// 当 make<T>() 找不到类型绑定时自动调用。
    /// 遍历所有尚未 doRegister() 的 Deferred Provider，依次执行注册，
    /// 注册后 make<T>() 会重新查找，若命中则无异常。
    void onBindingNotFound(std::type_index key) override;

private:
    std::vector<std::unique_ptr<ServiceProvider>> providers_;
    bool booted_ = false;

    /// 已触发 doRegister() 的延迟 Provider 集合（防止重复注册）
    std::set<ServiceProvider*> deferredRegistered_;

    // 旧的外部设定单例指针方案已废弃，改为在 instance() 内部使用局部静态对象
};

} // namespace Container

// 全局单例宏：便于在任意位置访问 Application 并解析服务
#define CAPP (Container::Application::instance())
