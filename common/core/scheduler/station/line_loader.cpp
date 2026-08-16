#include "line_loader.h"
#include "station_manager.h"
#include "../engine/flow_loader.h"
#include "../engine/scheduler.h"

#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <Logger.h>

namespace scheduler {

// ============================================================
// LineLoader — 解析
// ============================================================

LineSpec LineLoader::parseFile(const std::string& filePath) const {
    std::ifstream file(filePath);
    if (!file.is_open())
        throw std::runtime_error("[LineLoader] 无法打开产线配置文件: " + filePath);

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "[LineLoader] JSON 解析失败: " + filePath + " → " + e.what());
    }

    LOG_INFO("[LineLoader] 正在加载产线配置: {}", filePath);
    return parseJson(j);
}

LineSpec LineLoader::parseString(const std::string& jsonStr) const {
    json j;
    try { j = json::parse(jsonStr); }
    catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[LineLoader] JSON 字符串解析失败: ") + e.what());
    }
    return parseJson(j);
}

LineSpec LineLoader::parseJson(const json& j) const {
    LineSpec spec;
    spec.lineId      = j.value("lineId",      "");
    spec.description = j.value("description", "");

    // ── 解析工位 ─────────────────────────────────────────────
    if (j.contains("stations") && j["stations"].is_array()) {
        for (const auto& sj : j["stations"]) {
            StationSpec ss;
            ss.stationId            = sj.value("stationId",            "");
            ss.name                 = sj.value("name",                 "");
            ss.flowFile             = sj.value("flowFile",             "");
            ss.continuousMode       = sj.value("continuousMode",       false);
            ss.continuousIntervalMs = sj.value("continuousIntervalMs", 0);
            ss.resubmitOnFail       = sj.value("resubmitOnFail",       true);
            ss.params               = sj.value("params",               json::object());

            if (sj.contains("ownedDevices") && sj["ownedDevices"].is_array())
                for (auto& d : sj["ownedDevices"])
                    ss.ownedDevices.push_back(d.get<std::string>());

            // 内联流程定义
            if (sj.contains("flow") && !sj["flow"].is_null())
                ss.flow = sj["flow"];

            if (ss.stationId.empty())
                throw std::runtime_error("[LineLoader] 工位缺少 stationId");

            spec.stations.push_back(std::move(ss));
        }
    }

    // ── 解析通道 ─────────────────────────────────────────────
    if (j.contains("channels") && j["channels"].is_array()) {
        for (const auto& cj : j["channels"]) {
            ChannelSpec cs;
            cs.name     = cj.value("name",     "");
            cs.capacity = cj.value("capacity", 0);
            if (!cs.name.empty())
                spec.channels.push_back(std::move(cs));
        }
    }

    // ── 解析屏障 ─────────────────────────────────────────────
    if (j.contains("barriers") && j["barriers"].is_array()) {
        for (const auto& bj : j["barriers"]) {
            BarrierSpec bs;
            bs.name = bj.value("name", "");
            if (bj.contains("participants") && bj["participants"].is_array())
                for (auto& p : bj["participants"])
                    bs.participants.push_back(p.get<std::string>());
            if (!bs.name.empty())
                spec.barriers.push_back(std::move(bs));
        }
    }

    LOG_INFO("[LineLoader] 产线 '{}' 解析完成: {} 工位, {} 通道, {} 屏障",
             spec.lineId, spec.stations.size(),
             spec.channels.size(), spec.barriers.size());
    return spec;
}

// ============================================================
// LineLoader — 应用
// ============================================================

void LineLoader::loadAndApply(const std::string& filePath,
                               StationManager&    stationMgr,
                               Scheduler&         scheduler,
                               const FlowLoader&  flowLoader,
                               const std::string& baseDir) const
{
    auto spec = parseFile(filePath);

    // 如果 baseDir 为空，以配置文件所在目录为基准
    std::string base = baseDir;
    if (base.empty()) {
        namespace fs = std::filesystem;
        base = fs::path(filePath).parent_path().string();
    }

    apply(spec, stationMgr, scheduler, flowLoader, base);
}

void LineLoader::apply(const LineSpec&    spec,
                        StationManager&    stationMgr,
                        Scheduler&         scheduler,
                        const FlowLoader&  flowLoader,
                        const std::string& baseDir) const
{
    namespace fs = std::filesystem;

    // ── 1. 创建流水线通道 ─────────────────────────────────────
    for (const auto& cs : spec.channels) {
        stationMgr.createChannel(cs.name, cs.capacity);
    }

    // ── 2. 创建同步屏障（同时注册到 BarrierRegistry）──────────
    for (const auto& bs : spec.barriers) {
        stationMgr.createBarrier(bs.name, bs.participants);
    }

    // ── 3. 加载并注册流程，创建工位 ──────────────────────────
    for (const auto& ss : spec.stations) {
        // 加载流程定义
        std::string flowName;

        if (!ss.flowFile.empty()) {
            // 外部文件引用
            std::string fullPath = ss.flowFile;
            if (!fs::path(fullPath).is_absolute() && !baseDir.empty())
                fullPath = (fs::path(baseDir) / ss.flowFile).string();

            auto def = flowLoader.loadFromFile(fullPath);
            scheduler.registerFlow(def);
            flowName = def->name;
            LOG_INFO("[LineLoader] 工位 '{}' 注册流程: {} ({})",
                     ss.stationId, flowName, fullPath);

        } else if (!ss.flow.is_null() && !ss.flow.empty()) {
            // 内联流程定义
            auto def = flowLoader.loadFromJson(ss.flow);
            scheduler.registerFlow(def);
            flowName = def->name;
            LOG_INFO("[LineLoader] 工位 '{}' 注册内联流程: {}", ss.stationId, flowName);

        } else {
            LOG_WARN("[LineLoader] 工位 '{}' 未配置流程（flowFile 和 flow 均为空）",
                     ss.stationId);
        }

        // 创建工位
        StationConfig stCfg;
        stCfg.stationId    = ss.stationId;
        stCfg.name         = ss.name;
        stCfg.mainFlowName = flowName;
        stCfg.ownedDevices = ss.ownedDevices;
        stCfg.params       = ss.params;

        stationMgr.addStation(std::move(stCfg));

        // 如需持续运行，在工位添加后立即启用
        if (ss.continuousMode) {
            auto* st = stationMgr.getStation(ss.stationId);
            if (st) {
                StationContinuousConfig contCfg;
                contCfg.intervalMs     = ss.continuousIntervalMs;
                contCfg.resubmitOnFail = ss.resubmitOnFail;
                st->enableContinuousMode(contCfg);
            }
        }
    }

    LOG_INFO("[LineLoader] 产线 '{}' 配置应用完成", spec.lineId);
}

} // namespace scheduler
