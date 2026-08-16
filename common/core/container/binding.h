#pragma once

#include <functional>
#include <memory>

namespace Container {

// 前向声明，避免循环头文件依赖
class Container;

// ============================================================
// FactoryFn — 类型擦除的工厂函数
//
// 接收容器引用，使工厂内部可以递归调用 make<T>() 解析依赖。
// 返回 shared_ptr<void>，外层 make<T>() 再做 static_pointer_cast。
// ============================================================
using FactoryFn = std::function<std::shared_ptr<void>(Container&)>;

// ============================================================
// BindingType — 绑定的生命周期类型
// ============================================================
enum class BindingType
{
    Transient,  ///< 每次 make() 都创建新实例
    Singleton,  ///< 首次 make() 时创建，后续复用同一实例
    Instance    ///< 预先构建好的实例，直接返回
};

// ============================================================
// Binding — 单条服务绑定的描述符
// ============================================================
struct Binding
{
    FactoryFn   factory;                     ///< 工厂函数（Instance 类型可为空）
    BindingType type     = BindingType::Transient;
    bool        resolved = false;            ///< Singleton 是否已构建
    std::shared_ptr<void> instance;          ///< 缓存的单例 / 预建实例
};

} // namespace Container
