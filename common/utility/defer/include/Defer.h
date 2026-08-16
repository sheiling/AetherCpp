#pragma once

#include <functional>
#include <utility>

/**
 * @file Defer.h
 * @brief 类似 Go 语言的 defer 功能实现（header-only）
 * 
 * 使用 RAII 模式实现延迟执行，确保在作用域退出时（正常返回或异常）执行清理操作。
 * 
 * @example
 * ```cpp
 * #include "utility/defer/include/Defer.h"
 * 
 * void example() {
 *     FILE* fp = fopen("test.txt", "r");
 *     DEFER([&]() { if (fp) fclose(fp); });
 *     
 *     // 无论函数如何退出，fp 都会被关闭
 *     if (!fp) return;
 *     // ... 使用 fp ...
 * }
 * ```
 */

namespace utility {

/**
 * @brief Defer 类 - 延迟执行清理操作
 * 
 * 在对象析构时执行保存的可调用对象，确保资源清理。
 * 支持移动语义，禁止拷贝以避免重复执行。
 */
class Defer {
public:
    /**
     * @brief 构造函数，保存要延迟执行的可调用对象
     * @param fn 可调用对象（函数、lambda、函数对象等）
     */
    template<typename Fn>
    explicit Defer(Fn&& fn) 
        : fn_(std::forward<Fn>(fn))
        , active_(true) {
    }

    /**
     * @brief 移动构造函数
     */
    Defer(Defer&& other) noexcept
        : fn_(std::move(other.fn_))
        , active_(other.active_) {
        other.active_ = false;
    }

    /**
     * @brief 禁止拷贝构造
     */
    Defer(const Defer&) = delete;

    /**
     * @brief 析构函数，执行延迟的操作
     */
    ~Defer() {
        if (active_ && fn_) {
            fn_();
        }
    }

    /**
     * @brief 禁止拷贝赋值
     */
    Defer& operator=(const Defer&) = delete;

    /**
     * @brief 移动赋值运算符
     */
    Defer& operator=(Defer&& other) noexcept {
        if (this != &other) {
            if (active_ && fn_) {
                fn_();
            }
            fn_ = std::move(other.fn_);
            active_ = other.active_;
            other.active_ = false;
        }
        return *this;
    }

    /**
     * @brief 取消延迟执行
     * 
     * 调用后，析构时不会执行保存的操作。
     */
    void cancel() {
        active_ = false;
    }

    /**
     * @brief 立即执行并取消延迟执行
     * 
     * 立即执行保存的操作，之后析构时不再执行。
     */
    void execute() {
        if (active_ && fn_) {
            fn_();
            active_ = false;
        }
    }

private:
    std::function<void()> fn_;
    bool active_;
};

} // namespace utility

/**
 * @def DEFER(expr)
 * @brief 便捷宏，创建 Defer 对象
 * 
 * 使用示例：
 * ```cpp
 * DEFER([]() { std::cout << "清理操作" << std::endl; });
 * DEFER([&]() { cleanup(); });
 * ```
 * 
 * @param expr 可调用对象表达式（通常是 lambda）
 */
#define DEFER_CONCAT_IMPL(a, b) a##b
#define DEFER_CONCAT(a, b) DEFER_CONCAT_IMPL(a, b)
#ifdef __COUNTER__
#define DEFER(expr) \
    auto DEFER_CONCAT(_defer_, __COUNTER__) = ::utility::Defer(expr)
#else
#define DEFER(expr) \
    auto DEFER_CONCAT(_defer_, __LINE__) = ::utility::Defer(expr)
#endif

/**
 * @def DEFER_NAMED(name, expr)
 * @brief 带名称的 defer 宏，可以手动控制执行
 * 
 * 使用示例：
 * ```cpp
 * DEFER_NAMED(cleanup, []() { cleanup(); });
 * // ... 某些条件 ...
 * cleanup.execute();  // 提前执行
 * ```
 * 
 * @param name 变量名称
 * @param expr 可调用对象表达式
 */
#define DEFER_NAMED(name, expr) \
    auto name = ::utility::Defer(expr)
