#include "flow_loader.h"
#include "scheduler.h"
#include "../step/step_factory.h"
#include "../step/step_interceptor.h"
#include "../step/control/sequential_step.h"
#include "../step/control/parallel_step.h"
#include "../step/control/loop_step.h"
#include "../step/control/delay_step.h"
#include "../step/control/script_step.h"
#include "../step/control/exclusive_gateway.h"
#include "../step/sync/sync_steps.h"
#include "../step/sync/pipeline_channel.h"

#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <Logger.h>

namespace scheduler {

// ============================================================
// 步骤初始化辅助（reflectFromJson + Init）
// ============================================================
// 调用时机：setConfig() 之后，子步骤递归之前
// 职责：
//   1. reflectFromJson(cfg.params) — 将步骤参数 JSON 反序列化到步骤私有成员
//   2. Init()                       — 执行硬件/资源初始化；返回 false 则快速失败
static void initStep(IStep* step, const StepConfig& cfg) {
    step->reflectFromJson(cfg.params);

    if (!step->Init()) {
        throw std::runtime_error(
            "[FlowLoader] 步骤 '" + cfg.name +
            "' (type='" + cfg.type + "') Init() 返回 false，流程加载中止");
    }
}

// ============================================================
// FlowLoader
// ============================================================

std::shared_ptr<FlowDefinition> FlowLoader::loadFromFile(
    const std::string& filePath) const
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("[FlowLoader] 无法打开文件: " + filePath);
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        throw std::runtime_error("[FlowLoader] JSON 解析失败: " + filePath
                                 + " → " + e.what());
    }

    LOG_INFO("[FlowLoader] 正在加载流程文件: {}", filePath);
    return loadFromJson(j);
}

std::shared_ptr<FlowDefinition> FlowLoader::loadFromString(
    const std::string& jsonStr) const
{
    json j;
    try {
        j = json::parse(jsonStr);
    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[FlowLoader] JSON 字符串解析失败: ") + e.what());
    }
    return loadFromJson(j);
}

std::shared_ptr<FlowDefinition> FlowLoader::loadFromJson(const json& j) const {
    auto def = std::make_shared<FlowDefinition>();

    def->name               = j.value("name",               "");
    def->description        = j.value("description",        "");
    def->version            = j.value("version",            1);
    def->timeoutMs          = j.value("timeoutMs",          0);
    def->priority           = j.value("priority",           0);
    def->maxInstances       = j.value("maxInstances",       1);
    def->enableCompensation = j.value("enableCompensation", true);

    if (def->name.empty())
        throw std::runtime_error("[FlowLoader] 流程定义缺少 'name' 字段");

    // 解析所需设备列表
    if (j.contains("requiredDevices") && j["requiredDevices"].is_array()) {
        for (const auto& d : j["requiredDevices"])
            def->requiredDevices.push_back(d.get<std::string>());
    }

    // ── 解析流程级拦截器（对本流程所有步骤生效）──────────────
    // JSON: { "interceptors": ["tracing", "error_detail"] }
    if (j.contains("interceptors") && j["interceptors"].is_array()) {
        auto& reg = StepInterceptorRegistry::instance();
        for (const auto& name : j["interceptors"]) {
            std::string n = name.get<std::string>();
            if (!reg.hasInterceptor(n)) {
                LOG_WARN("[FlowLoader] 流程级拦截器 '{}' 未在 StepInterceptorRegistry 注册", n);
            }
            def->interceptorNames.push_back(n);
        }
    }

    // 构建步骤树
    if (j.contains("steps") && !j["steps"].is_null()) {
        def->rootStep = buildStep(j["steps"]);
    } else {
        // 若未定义步骤，使用空的根顺序步骤
        auto root = std::make_shared<SequentialStep>();
        StepConfig rootCfg;
        rootCfg.name = def->name + "_root";
        rootCfg.type = "control.sequential";
        root->setConfig(rootCfg);
        initStep(root.get(), rootCfg);   // reflectFromJson + Init
        def->rootStep = root;
    }

    LOG_INFO("[FlowLoader] 流程 '{}' 加载成功 (v{}, steps={}, interceptors={})",
             def->name, def->version,
             def->rootStep ? def->rootStep->children().size() : 0,
             def->interceptorNames.size());

    return def;
}

std::vector<std::shared_ptr<FlowDefinition>> FlowLoader::loadDirectory(
    const std::string& dirPath) const
{
    std::vector<std::shared_ptr<FlowDefinition>> defs;
    namespace fs = std::filesystem;

    std::error_code ec;
    if (!fs::exists(dirPath, ec) || !fs::is_directory(dirPath, ec)) {
        LOG_WARN("[FlowLoader] 目录不存在或无法访问: {}", dirPath);
        return defs;
    }

    for (const auto& entry : fs::directory_iterator(dirPath, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;

        try {
            auto def = loadFromFile(entry.path().string());
            defs.push_back(def);
        } catch (const std::exception& e) {
            const std::string path = entry.path().string();
            const std::string what = e.what();
            LOG_WARN("[FlowLoader] 跳过文件 {}: {}", path, what);
            if (what.find("Unknown step type") != std::string::npos) {
                LOG_WARN("[FlowLoader] 未注册的工步 type：须在 FlowLoader 加载前完成 "
                         "SCHEDULER_REGISTER_STEP_CLASS（业务插件 DLL 加载之后）。"
                         "当前已注册: {}",
                         StepFactory::instance().registeredTypeNames().size());
            }
        }
    }

    LOG_INFO("[FlowLoader] 目录 '{}' 共加载 {} 个流程定义",
             dirPath, defs.size());
    return defs;
}

void FlowLoader::registerAll(
    Scheduler& sched,
    const std::vector<std::shared_ptr<FlowDefinition>>& defs)
{
    for (auto& def : defs) {
        sched.registerFlow(def);
        LOG_INFO("[FlowLoader] 已注册流程: {}", def->name);
    }
}

// ── 步骤树构建（递归）─────────────────────────────────────────
// 每个步骤完整生命周期：create → setConfig → reflectFromJson → Init → addChild(children...)
std::shared_ptr<IStep> FlowLoader::buildStep(const json& stepJson) const {
    std::string typeName = stepJson.value("type", "");
    if (typeName.empty())
        throw std::runtime_error("[FlowLoader] 步骤缺少 'type' 字段");

    std::shared_ptr<IStep> step;

    auto params = stepJson.value("params", json::object());

    // ── 脚本步骤：通过 ScriptRegistry 查找处理函数 ────────────
    if (typeName == "control.script") {
        std::string scriptName = stepJson.value("script", "");
        auto fn = ScriptRegistry::instance().getScript(scriptName);
        if (!fn) {
            if (scriptName.empty()) {
                LOG_WARN("[FlowLoader] 步骤 '{}' 未指定 script 名称，将被跳过",
                         stepJson.value("name", "?"));
            } else {
                LOG_WARN("[FlowLoader] 脚本 '{}' 未在 ScriptRegistry 中注册，"
                         "步骤将返回 skip()", scriptName);
            }
        }
        step = std::make_shared<ScriptStep>(fn);
    }
    // ── 延时步骤：从 params.ms 读取延时时长 ──────────────────
    else if (typeName == "control.delay") {
        int ms = params.value("ms", 0);
        step = std::make_shared<DelayStep>(ms);
    }
    // ── 循环步骤：从 params 读取循环次数 ─────────────────────
    else if (typeName == "control.loop") {
        int times = params.value("times", 1);
        step = std::make_shared<LoopStep>(times);
    }
    // ── 屏障同步步骤 ──────────────────────────────────────────
    else if (typeName == "sync.barrier") {
        std::string barrierName = params.value("barrier",   "");
        std::string stationId   = params.value("stationId", "");
        step = std::make_shared<BarrierStep>(barrierName, stationId);
    }
    // ── 流水线通道发送步骤 ─────────────────────────────────────
    else if (typeName == "sync.channel_send") {
        std::string channelName = params.value("channel", "");
        std::string ctxKey      = params.value("ctxKey",  "");
        step = std::make_shared<ChannelSendStep>(channelName, ctxKey);
    }
    // ── 流水线通道接收步骤 ─────────────────────────────────────
    else if (typeName == "sync.channel_receive") {
        std::string channelName = params.value("channel", "");
        std::string ctxKey      = params.value("ctxKey",  "");
        step = std::make_shared<ChannelReceiveStep>(channelName, ctxKey);
    }
    // ── 其余类型：由 StepFactory（含业务 registerType / registerStepClass）创建 ─
    else {
        step = StepFactory::instance().create(typeName);
    }

    // ── 填充公共配置 ─────────────────────────────────────────
    StepConfig cfg;
    cfg.name            = stepJson.value("name",            "");
    cfg.type            = typeName;
    cfg.timeoutMs       = stepJson.value("timeoutMs",       5000);
    cfg.retryCount      = stepJson.value("retryCount",      0);
    cfg.retryIntervalMs = stepJson.value("retryIntervalMs", 500);
    cfg.skipOnFail      = stepJson.value("skipOnFail",      false);
    cfg.optional        = stepJson.value("optional",        false);
    cfg.isCheckpoint    = stepJson.value("isCheckpoint",    false);
    cfg.params          = params;

    // 步骤级拦截器名称（JSON: "interceptors": ["timing", "logging"]）
    if (stepJson.contains("interceptors") && stepJson["interceptors"].is_array()) {
        auto& reg = StepInterceptorRegistry::instance();
        for (const auto& name : stepJson["interceptors"]) {
            std::string n = name.get<std::string>();
            if (!reg.hasInterceptor(n)) {
                LOG_WARN("[FlowLoader] 步骤 '{}' 的拦截器 '{}' 未注册",
                         cfg.name, n);
            }
            cfg.interceptorNames.push_back(n);
        }
    }

    // 补偿步骤（可选，递归构建）
    if (stepJson.contains("compensation") && !stepJson["compensation"].is_null()) {
        cfg.compensationStep = buildStep(stepJson["compensation"]);
    }

    // ── setConfig → reflectFromJson → Init ────────────────────
    step->setConfig(cfg);
    initStep(step.get(), cfg);

    // ── 递归构建子步骤（子步骤各自独立完成初始化）────────────
    if (stepJson.contains("children") && stepJson["children"].is_array()) {
        for (const auto& child : stepJson["children"])
            step->addChild(buildStep(child));
    }

    return step;
}

} // namespace scheduler
