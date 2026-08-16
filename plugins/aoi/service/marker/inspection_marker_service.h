#pragma once

/**
 * @file inspection_marker_service.h
 * @brief 检测工步 Marker 定位服务：运动采图、模板匹配、结果写入 Session
 *
 * 职责边界：
 * - 读取配方 `marks[slot].capturePos` 作为运动目标（标定坐标，只读）
 * - 调用设备网关采图、可选 CISDK 模板匹配
 * - 将定位物理坐标写入 `InspectionSession::markerLocatedPhysical[slot]`
 * - 上报 Mark 预览图与进度日志
 * - **不**回写 `recipe.marks[slot].capturePos`
 */

#include "inspection_marker_types.h"
#include "inspection_session.h"

#include <scheduler/context/flow_context.h>
#include <scheduler/core/flow_types.h>

namespace aoi::inspection {

/**
 * @brief 对 [firstSlot, firstSlot + slotCount) 槽位执行 Marker 定位
 *
 * @param session    单板检测会话（recipe / 设备 / 进度桥）
 * @param ctx        工步上下文（当前未使用，保留扩展）
 * @param firstSlot  起始槽位（含），双轨时轨1 为 0，轨2 为 2
 * @param slotCount  连续槽位数，单轨为 recipeMarkCount，双轨每轨通常为 2
 * @return 任一步运动/采图失败或标定坐标未配置则 fail
 */
scheduler::StepResult runMarkerLocateSlots(const std::shared_ptr<InspectionSession> &session,
                                           scheduler::FlowContext &ctx,
                                           int firstSlot,
                                           int slotCount);

namespace marker {

/**
 * @brief 定位单槽位 Marker，结果写入 session 与可选 outResult
 *
 * @param session   检测会话
 * @param slot      槽位索引
 * @param outResult 若非 nullptr，填充本次定位明细
 * @return 失败时返回错误 StepResult
 */
scheduler::StepResult locateMarkerSlot(const std::shared_ptr<InspectionSession> &session,
                                       int slot,
                                       MarkerLocateSlotResult *outResult = nullptr);

/**
 * @brief 批量定位（runMarkerLocateSlots 的内部实现入口）
 */
scheduler::StepResult runMarkerLocateService(const std::shared_ptr<InspectionSession> &session,
                                             scheduler::FlowContext &ctx,
                                             int firstSlot,
                                             int slotCount);

} // namespace marker
} // namespace aoi::inspection
