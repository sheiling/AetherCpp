#pragma once

#include "application.h"

namespace Container {

// ============================================================
// Facade<Abstract> — 服务外观模板（Laravel Facades 风格）
//
// Facade 提供对底层服务的静态访问入口，避免到处传递容器引用。
// 每个具体 Facade 只需继承此模板并提供服务类型即可。
//
// 使用前提：
//   Application::setInstance(&app) 已被调用。
//
// ──────────────────────────────────────────────────────────
// 使用方式
// ──────────────────────────────────────────────────────────
//
// ① 定义服务接口（纯虚基类）
//
//   class ILogger {
//   public:
//       virtual ~ILogger() = default;
//       virtual void info(const std::string& msg) = 0;
//   };
//
// ② 定义具体 Facade（只需一行）
//
//   class Log : public Facade<ILogger> {};
//
// ③ 在任意位置通过静态方法访问
//
//   // 获取底层 shared_ptr
//   auto logger = Log::getFacadeRoot();
//   logger->info("hello");
//
//   // 直接调用（返回裸指针，生命周期由容器管理）
//   Log::resolve()->info("hello");
//
// ④ 也可以在 Facade 子类中包装成静态方法（更像 Laravel）
//
//   class Log : public Facade<ILogger> {
//   public:
//       static void info(const std::string& msg) {
//           getFacadeRoot()->info(msg);
//       }
//   };
//
//   // 直接静态调用：
//   Log::info("Application started.");
//
// ============================================================
template <typename Abstract>
class Facade
{
public:
    /// 返回容器中注册的服务（shared_ptr）
    static std::shared_ptr<Abstract> getFacadeRoot()
    {
        return Application::instance()->make<Abstract>();
    }

    /// 返回裸指针（便于链式调用，生命周期由容器管理）
    static Abstract* resolve()
    {
        return getFacadeRoot().get();
    }

protected:
    // 纯静态工具类，禁止实例化
    Facade()  = delete;
    ~Facade() = delete;
};

} // namespace Container
