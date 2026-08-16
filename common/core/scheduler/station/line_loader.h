#pragma once

// ============================================================
// line_loader.h — 多工位产线 JSON 配置加载器
//
// 【LineLoader】从 JSON 文件加载产线完整配置，包括：
//   - 工位列表（每个工位绑定流程文件或内联流程定义）
//   - 工位间流水线通道（PipelineChannel）
//   - 工位间同步屏障（StationBarrier）
//
// JSON 格式（line_config.json）：
// {
//   "lineId":      "SMT-Line-1",
//   "description": "SMT贴片产线",
//   "stations": [
//     {
//       "stationId":            "st-load",
//       "name":                 "上料工位",
//       "flowFile":             "flow_st_load.json",   // 引用外部流程文件
//       "continuousMode":       true,
//       "continuousIntervalMs": 200,
//       "resubmitOnFail":       true,
//       "params":               {}
//     },
//     {
//       "stationId": "st-inspect",
//       "name":      "检测工位",
//       "flow": {                                      // 内联流程定义
//         "name": "检测流程",
//         "steps": { ... }
//       },
//       "continuousMode": true
//     }
//   ],
//   "channels": [
//     {
//       "name":     "ch-load-to-inspect",
//       "capacity": 3
//     }
//   ],
//   "barriers": [
//     {
//       "name":         "sync-inspect-unload",
//       "participants": ["st-inspect", "st-unload"]
//     }
//   ]
// }
//
// 使用示例：
//   LineLoader loader;
//   loader.loadAndApply("line_config.json",
//                        *stationMgr, *scheduler, flowLoader, "./flows/");
// ============================================================

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <common/3rdparty/nlohmann/json.hpp>

#include "../scheduler_global.h"

namespace scheduler {

using json = nlohmann::json;

class Scheduler;
class StationManager;
class FlowLoader;

// ── 中间结构：避免直接依赖所有头文件 ────────────────────────

struct BarrierSpec {
    std::string              name;
    std::vector<std::string> participants;
};

struct ChannelSpec {
    std::string name;
    int         capacity{0};  // 0=无界
};

struct StationSpec {
    std::string stationId;
    std::string name;
    std::string flowFile;          // 引用外部 .json 文件（相对于 baseDir）
    json        flow;              // 内联流程定义（flowFile 为空时使用）
    json        params;
    std::vector<std::string> ownedDevices;
    bool        continuousMode{false};
    int         continuousIntervalMs{0};
    bool        resubmitOnFail{true};
};

struct LineSpec {
    std::string              lineId;
    std::string              description;
    std::vector<StationSpec> stations;
    std::vector<ChannelSpec> channels;
    std::vector<BarrierSpec> barriers;
};

// ============================================================
// LineLoader
// ============================================================
class SCHEDULER_EXPORT LineLoader {
public:
    LineLoader() = default;

    // ── 解析 ─────────────────────────────────────────────────

    // 从 JSON 文件解析 LineSpec（不执行任何创建操作）
    LineSpec parseFile(const std::string& filePath) const;

    // 从 JSON 字符串解析
    LineSpec parseString(const std::string& jsonStr) const;

    // 从 json 对象解析（底层实现）
    LineSpec parseJson(const json& j) const;

    // ── 应用（一步到位）──────────────────────────────────────

    // 解析并应用产线配置：创建工位、通道、屏障，注册流程，启动持续模式
    // baseDir：flowFile 的基础路径（为空则相对于当前目录）
    void loadAndApply(const std::string& filePath,
                      StationManager&    stationMgr,
                      Scheduler&         scheduler,
                      const FlowLoader&  flowLoader,
                      const std::string& baseDir = "") const;

    // 将已解析的 LineSpec 应用到 StationManager / Scheduler
    void apply(const LineSpec&    spec,
               StationManager&    stationMgr,
               Scheduler&         scheduler,
               const FlowLoader&  flowLoader,
               const std::string& baseDir = "") const;
};

} // namespace scheduler
