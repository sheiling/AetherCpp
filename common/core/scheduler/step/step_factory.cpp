#include "step_factory.h"
#include "step_interceptor.h"
#include "control/sequential_step.h"
#include "control/parallel_step.h"
#include "control/exclusive_gateway.h"
#include "control/loop_step.h"
#include "control/delay_step.h"
#include "control/script_step.h"
#include <Logger.h>
#include <stdexcept>

namespace scheduler {

StepFactory& StepFactory::instance() {
    static StepFactory inst;
    return inst;
}

void StepFactory::registerType(const std::string& typeName, Creator creator) {
    std::lock_guard<std::mutex> lk(m_mutex_);
    m_creators_[typeName] = std::move(creator);
}

void StepFactory::registerTypes(
    std::initializer_list<std::pair<std::string, Creator>> list)
{
    std::lock_guard<std::mutex> lk(m_mutex_);
    for (auto& [name, fn] : list)
        m_creators_[name] = std::move(fn);
}

std::shared_ptr<IStep> StepFactory::tryCreate(const std::string& typeName) const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    auto it = m_creators_.find(typeName);
    if (it == m_creators_.end())
        return nullptr;
    return it->second();
}

std::shared_ptr<IStep> StepFactory::create(const std::string& typeName) const {
    auto step = tryCreate(typeName);
    if (!step)
        throw std::runtime_error("[StepFactory] Unknown step type: " + typeName);
    return step;
}

bool StepFactory::hasType(const std::string& typeName) const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    return m_creators_.count(typeName) > 0;
}

std::vector<std::string> StepFactory::registeredTypeNames() const {
    std::lock_guard<std::mutex> lk(m_mutex_);
    std::vector<std::string> names;
    names.reserve(m_creators_.size());
    for (auto& [k, _] : m_creators_) names.push_back(k);
    return names;
}

void StepFactory::clear() {
    std::lock_guard<std::mutex> lk(m_mutex_);
    m_creators_.clear();
}

// ── 步骤初始化辅助（reflectFromJson + Init）──────────────────
// 职责：在 setConfig 之后立即调用，将步骤参数反序列化并执行初始化。
// 若 Init() 返回 false，抛出 std::runtime_error 阻止加载（快速失败）。
static void initStep(IStep* step, const StepConfig& cfg) {
    // 1. 从步骤参数（params）初始化步骤私有字段
    step->reflectFromJson(cfg.params);

    // 2. 执行步骤硬件/资源初始化
    if (!step->Init()) {
        throw std::runtime_error(
            "[StepFactory] 步骤 '" + cfg.name +
            "' (type='" + cfg.type + "') Init() 返回 false，流程加载中止");
    }
}

std::shared_ptr<IStep> StepFactory::createFromJson(const json& j) const {
    std::string typeName = j.value("type", "");
    auto step = create(typeName);

    StepConfig cfg;
    cfg.name            = j.value("name",            "");
    cfg.type            = typeName;
    cfg.timeoutMs       = j.value("timeoutMs",       5000);
    cfg.retryCount      = j.value("retryCount",      0);
    cfg.retryIntervalMs = j.value("retryIntervalMs", 500);
    cfg.skipOnFail      = j.value("skipOnFail",      false);
    cfg.optional        = j.value("optional",        false);
    cfg.isCheckpoint    = j.value("isCheckpoint",    false);
    cfg.params          = j.value("params",          json::object());

    // 步骤级拦截器名称列表
    if (j.contains("interceptors") && j["interceptors"].is_array()) {
        for (const auto& name : j["interceptors"])
            cfg.interceptorNames.push_back(name.get<std::string>());
    }

    // 补偿步骤（可选，递归构建）
    if (j.contains("compensation")) {
        cfg.compensationStep = createFromJson(j["compensation"]);
    }

    step->setConfig(cfg);   // 注意：先传 cfg 的副本，initStep 再读

    // ── 调用 reflectFromJson + Init ────────────────────────────
    initStep(step.get(), cfg);

    // 递归处理子步骤（容器步骤，子步骤各自独立完成 reflect + Init）
    if (j.contains("children") && j["children"].is_array()) {
        for (auto& child : j["children"])
            step->addChild(createFromJson(child));
    }

    return step;
}

void registerBuiltinSteps() {
    auto& f = StepFactory::instance();
    f.registerType("control.sequential",  []{ return std::make_shared<SequentialStep>(); });
    f.registerType("control.parallel",    []{ return std::make_shared<ParallelStep>(); });
    f.registerType("gateway.exclusive",   []{ return std::make_shared<ExclusiveGateway>(); });
    f.registerType("control.loop",        []{ return std::make_shared<LoopStep>(); });
    f.registerType("control.delay",       []{ return std::make_shared<DelayStep>(); });
    f.registerType("control.script",      []{ return std::make_shared<ScriptStep>(); });
}

} // namespace scheduler
