#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <initializer_list>
#include "i_step.h"
#include "../scheduler_global.h"

namespace scheduler {

// ============================================================
// StepFactory — 步骤类型注册与工厂（单例，线程安全）
//   内置步骤与业务自定义 IStep 子类均注册于此；
//   按 typeName 创建实例，并支持 JSON 反序列化构建步骤树。
//   未注册时 tryCreate 返回 nullptr，create 抛出异常。
// ============================================================
class SCHEDULER_EXPORT StepFactory {
public:
    using Creator = std::function<std::shared_ptr<IStep>()>;

    static StepFactory& instance();

    void registerType(const std::string& typeName, Creator creator);
    void registerTypes(std::initializer_list<std::pair<std::string, Creator>> list);

    std::shared_ptr<IStep> create(const std::string& typeName) const;
    /// 未注册时返回 nullptr（供加载器等链式回退）
    std::shared_ptr<IStep> tryCreate(const std::string& typeName) const;

    bool hasType(const std::string& typeName) const;
    std::vector<std::string> registeredTypeNames() const;

    void clear();

    std::shared_ptr<IStep> createFromJson(const json& j) const;

private:
    StepFactory() = default;
    mutable std::mutex m_mutex_;
    std::unordered_map<std::string, Creator> m_creators_;
};

/// 将具体 IStep 子类注册到 StepFactory（typeName 与 JSON 中 "type" 一致）
template<typename T>
void registerStepClass(const std::string& typeName) {
    StepFactory::instance().registerType(
        typeName, [] { return std::make_shared<T>(); });
}

// ── 注册内置步骤（在 scheduler 初始化时调用一次）──────────────
SCHEDULER_EXPORT void registerBuiltinSteps();

} // namespace scheduler

// ── 静态注册宏（通常在业务 .cpp 中写一行，启动阶段自动注册）──────────
//   SCHEDULER_REGISTER_STEP_CLASS(MyInspectStep, "my.product.inspect");
//   StepClass 可为带命名空间的类型；同一物理行请勿写多条（避免 __LINE__ 冲突）。
#ifndef SCHEDULER_REGISTER_STEP_CLASS
// 供 SCHEDULER_REGISTER_STEP_CLASS 展开使用（勿与业务宏重名）
#define SCHEDULER_DETAIL_REG_CAT_(a, b) a##b
#define SCHEDULER_DETAIL_REG_CAT(a, b) SCHEDULER_DETAIL_REG_CAT_(a, b)
#define SCHEDULER_REGISTER_STEP_CLASS(StepClass, typeNameStr) \
    namespace { \
    struct SCHEDULER_DETAIL_REG_CAT(scheduler_step_factory_reg_, __LINE__) { \
        SCHEDULER_DETAIL_REG_CAT(scheduler_step_factory_reg_, __LINE__)() { \
            ::scheduler::registerStepClass<StepClass>(typeNameStr); \
        } \
    } SCHEDULER_DETAIL_REG_CAT(scheduler_step_factory_reg_inst_, __LINE__); \
    }
#endif
