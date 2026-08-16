#pragma once

/**
 * @file scan_flow_context.h
 * @brief AOI 扫描工步 FlowContext 键名与协作类型
 *
 * 流程位置：FlowHostCoordinator onStart 注入 Session；各 aoi.* 工步读写 ctx。
 * 键名字符串须与历史持久化/调试日志兼容，迁移目录时勿改字面量。
 */

#include <atomic>
#include <memory>

namespace aoi::inspection {
struct InspectionSession;
} // namespace aoi::inspection

namespace aoi::flow {

/** FlowContext 中 InspectionSession 的键名 */
inline constexpr const char *kCtxSession         = "aoi.inspection_session";
/** 当前加载的配方 id */
inline constexpr const char *kCtxRecipeId        = "aoi.recipe_id";
/** FOV 几何规划结果（可选镜像，主数据在 Session） */
inline constexpr const char *kCtxFovGeometryPlan = "aoi.fov_geometry_plan";
/** 采集运动路径规划结果（可选镜像） */
inline constexpr const char *kCtxCaptureMotionPlan = "aoi.capture_motion_plan";
/** 当前 FOV 序号（1-based） */
inline constexpr const char *kCtxCurrentImageIndex = "aoi.current_image_index";
/** 已完成采图张数 */
inline constexpr const char *kCtxCapturedImages  = "aoi.captured_image_count";

/** 流程 abort 协作标志；onFlowAbort 置 true，工步循环内 isAbortRequested 检测 */
using AbortFlag = std::shared_ptr<std::atomic<bool>>;

} // namespace aoi::flow
