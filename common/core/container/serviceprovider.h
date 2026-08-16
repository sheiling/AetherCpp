#pragma once

#include "container_global.h"

namespace Container {

class Application;

// ============================================================
// ServiceProvider — 服务提供者抽象基类
//
// 服务提供者是向容器注册服务绑定的集中入口。
// 继承此类，在 doRegister() 中注册绑定，在 boot() 中执行
// 需要其他服务已就绪后才能进行的初始化工作。
//
// 生命周期（由 Application::bootstrap() 管理）：
//   第一阶段：所有 Provider 的 doRegister() 依次调用
//   第二阶段：所有 Provider 的 boot() 依次调用
//
// 注意：
//   - doRegister() 中 **不应** 调用 app_.make<T>()，
//     此时其他 Provider 尚未完成注册。
//   - boot() 中可安全调用 app_.make<T>()。
//
// 示例：
//   class LogServiceProvider : public ServiceProvider {
//   public:
//       using ServiceProvider::ServiceProvider;
//
//       void doRegister() override {
//           app_.singleton<ILogger>([] {
//               return std::make_shared<FileLogger>("app.log");
//           });
//       }
//
//       void boot() override {
//           // 可安全使用 app_.make<ILogger>() 等
//           auto logger = app_.make<ILogger>();
//           logger->info("LogServiceProvider booted.");
//       }
//   };
// ============================================================
class CONTAINER_EXPORT ServiceProvider
{
public:
    explicit ServiceProvider(Application& app);
    virtual ~ServiceProvider() = default;

    // ── 必须实现 ─────────────────────────────────────────────────────────

    /// 注册阶段：向容器注册服务绑定。
    /// 此时其他 Provider 可能尚未注册，不得解析服务。
    virtual void doRegister() = 0;

    // ── 可选实现 ─────────────────────────────────────────────────────────

    /// 引导阶段：所有 Provider 注册完毕后被调用，可安全解析其他服务。
    virtual void boot() {}

    /// 返回此 Provider 提供的服务类型名列表（延迟加载提示，可选实现）。
    /// 若 isDeferred() 返回 true，则仅在首次解析相关服务时才调用 doRegister()。
    virtual bool isDeferred() const { return false; }

protected:
    Application& app_;  ///< 所属应用程序容器的引用
};

} // namespace Container
